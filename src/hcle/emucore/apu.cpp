#include "apu.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "nes.hpp"

#include <cstring>

constexpr uint8_t LENGTH_COUNTER_TABLE[0x20] = {
    0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
    0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
    0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
    0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E};

constexpr uint16_t PERIOD_DMC_TABLE[0x10] = {
    0x1AC, 0x17C, 0x154, 0x140, 0x11E, 0x0FE, 0x0E2, 0x0D6,
    0x0BE, 0x0A0, 0x08E, 0x080, 0x06A, 0x054, 0x048, 0x036};

cynes::APU::APU(NES &nes)
    : _nes{nes},
      _internal_open_bus{0x00},
      _state{
          .latch_cycle = false,
          .delay_dma = 0x00,
          .address_dma = 0x00,
          .pending_dma = false,
          .frame_counter_clock = 0x0000,
          .delay_frame_reset = 0x0000,
          .channels_counters = {},
          .channel_enabled = {},
          .channel_halted = {},
          .step_mode = false,
          .inhibit_frame_interrupt = false,
          .send_frame_interrupt = false,
          .delta_channel_remaining_bytes = 0x0000,
          .delta_channel_sample_length = 0x0000,
          .delta_channel_period_counter = 0x0000,
          .delta_channel_period_load = 0x0000,
          .delta_channel_bits_in_buffer = 0x00,
          .delta_channel_should_loop = false,
          .delta_channel_enable_interrupt = false,
          .delta_channel_sample_buffer_empty = false,
          .enable_dmc = false,
          .send_delta_channel_interrupt = false}
{
    std::memset(_state.channels_counters, 0x00, 4);
    std::memset(_state.channel_enabled, false, 4);
    std::memset(_state.channel_halted, false, 4);
}

void cynes::APU::power()
{
    _state.latch_cycle = false;
    _state.delay_dma = 0x00;
    _state.address_dma = 0x00;
    _state.pending_dma = false;
    _internal_open_bus = 0x00;
    _state.frame_counter_clock = 0x0000;
    _state.delay_frame_reset = 0x0000;

    std::memset(_state.channels_counters, 0x00, 4);
    std::memset(_state.channel_enabled, false, 4);
    std::memset(_state.channel_halted, false, 4);

    _state.step_mode = false;
    _state.inhibit_frame_interrupt = false;
    _state.send_frame_interrupt = false;
    _state.delta_channel_remaining_bytes = 0x0000;
    _state.delta_channel_sample_length = 0x0000;
    _state.delta_channel_period_counter = PERIOD_DMC_TABLE[0];
    _state.delta_channel_period_load = PERIOD_DMC_TABLE[0];
    _state.delta_channel_bits_in_buffer = 0x08;
    _state.delta_channel_should_loop = false;
    _state.delta_channel_enable_interrupt = false;
    _state.delta_channel_sample_buffer_empty = true;
    _state.enable_dmc = false;
    _state.send_delta_channel_interrupt = false;
}

void cynes::APU::reset()
{
    _state.enable_dmc = false;

    std::memset(_state.channels_counters, 0x00, 4);
    std::memset(_state.channel_enabled, false, 4);

    _state.send_delta_channel_interrupt = false;
    _state.delta_channel_remaining_bytes = 0;
    _state.latch_cycle = false;
    _state.delay_dma = 0x00;
    _state.send_frame_interrupt = false;
    _state.send_delta_channel_interrupt = false;
    _state.delta_channel_period_counter = PERIOD_DMC_TABLE[0];
    _state.delta_channel_period_load = PERIOD_DMC_TABLE[0];
    _state.delta_channel_remaining_bytes = 0;
    _state.delta_channel_sample_buffer_empty = true;
    _state.delta_channel_bits_in_buffer = 8;

    _nes.write(0x4015, 0x00);
    _nes.write(0x4017, _state.step_mode << 7 | _state.inhibit_frame_interrupt << 6);
}

void cynes::APU::tick(bool reading, bool prevent_load)
{
    if (reading)
    {
        perform_pending_dma();
    }

    _state.latch_cycle = !_state.latch_cycle;

    if (_state.step_mode)
    {
        if (_state.delay_frame_reset > 0 && --_state.delay_frame_reset == 0)
        {
            _state.frame_counter_clock = 0;
        }
        else if (++_state.frame_counter_clock == 37282)
        {
            _state.frame_counter_clock = 0;
        }
        if (_state.frame_counter_clock == 14913 || _state.frame_counter_clock == 37281)
        {
            update_counters();
        }
    }
    else
    {
        if (_state.delay_frame_reset > 0 && --_state.delay_frame_reset == 0)
        {
            _state.frame_counter_clock = 0;
        }
        else if (++_state.frame_counter_clock == 29830)
        {
            _state.frame_counter_clock = 0;

            if (!_state.inhibit_frame_interrupt)
            {
                set_frame_interrupt(true);
            }
        }

        if (_state.frame_counter_clock == 14913 || _state.frame_counter_clock == 29829)
        {
            update_counters();
        }

        if (_state.frame_counter_clock >= 29828 && !_state.inhibit_frame_interrupt)
        {
            set_frame_interrupt(true);
        }
    }

    _state.delta_channel_period_counter--;

    if (_state.delta_channel_period_counter == 0)
    {
        _state.delta_channel_period_counter = _state.delta_channel_period_load;
        _state.delta_channel_bits_in_buffer--;

        if (_state.delta_channel_bits_in_buffer == 0)
        {
            _state.delta_channel_bits_in_buffer = 8;

            if (!_state.delta_channel_sample_buffer_empty)
            {
                _state.delta_channel_sample_buffer_empty = true;
            }

            if (_state.delta_channel_remaining_bytes > 0 && !prevent_load)
            {
                load_delta_channel_byte(reading);
            }
        }
    }
}

void cynes::APU::write(uint8_t address, uint8_t value)
{
    switch (static_cast<Register>(address))
    {
    case Register::PULSE_1_0:
    {
        _state.channel_halted[0x0] = value & 0x20;
        break;
    }

    case Register::PULSE_1_3:
    {
        if (_state.channel_enabled[0x0])
        {
            _state.channels_counters[0x0] = LENGTH_COUNTER_TABLE[value >> 3];
        }
        break;
    }

    case Register::PULSE_2_0:
    {
        _state.channel_halted[0x1] = value & 0x20;
        break;
    }

    case Register::PULSE_2_3:
    {
        if (_state.channel_enabled[0x1])
        {
            _state.channels_counters[0x1] = LENGTH_COUNTER_TABLE[value >> 3];
        }
        break;
    }

    case Register::TRIANGLE_0:
    {
        _state.channel_halted[0x2] = value & 0x80;
        break;
    }

    case Register::TRIANGLE_3:
    {
        if (_state.channel_enabled[0x2])
        {
            _state.channels_counters[0x2] = LENGTH_COUNTER_TABLE[value >> 3];
        }
        break;
    }

    case Register::NOISE_0:
    {
        _state.channel_halted[0x3] = value & 0x20;
        break;
    }

    case Register::NOISE_3:
        if (_state.channel_enabled[0x3])
        {
            _state.channels_counters[0x3] = LENGTH_COUNTER_TABLE[value >> 3];
        }
        break;

    case Register::OAM_DMA:
    {
        perform_dma(value);
        break;
    }

    case Register::DELTA_3:
    {
        _state.delta_channel_sample_length = (value << 4) + 1;
        break;
    }

    case Register::DELTA_0:
    {
        _state.delta_channel_enable_interrupt = value & 0x80;
        _state.delta_channel_should_loop = value & 0x40;
        _state.delta_channel_period_load = PERIOD_DMC_TABLE[value & 0x0F];

        if (!_state.delta_channel_enable_interrupt)
        {
            set_delta_interrupt(false);
        }

        break;
    }

    case Register::CTRL_STATUS:
    {
        _state.enable_dmc = value & 0x10;
        _internal_open_bus = value;

        for (uint8_t channel = 0; channel < 0x4; channel++)
        {
            _state.channel_enabled[channel] = value & (1 << channel);

            if (!_state.channel_enabled[channel])
            {
                _state.channels_counters[channel] = 0;
            }
        }

        set_delta_interrupt(false);

        if (!_state.enable_dmc)
        {
            _state.delta_channel_remaining_bytes = 0;
        }
        else
        {
            if (_state.delta_channel_remaining_bytes == 0)
            {
                _state.delta_channel_remaining_bytes = _state.delta_channel_sample_length;
                if (_state.delta_channel_sample_buffer_empty)
                {
                    load_delta_channel_byte(false);
                }
            }
        }

        break;
    }

    case Register::FRAME_COUNTER:
    {
        _state.step_mode = value & 0x80;
        _state.inhibit_frame_interrupt = value & 0x40;

        if (_state.inhibit_frame_interrupt)
        {
            set_frame_interrupt(false);
        }

        _state.delay_frame_reset = _state.latch_cycle ? 4 : 3;

        if (_state.step_mode)
        {
            update_counters();
        }

        break;
    }
    }
}

// Since $4015 is an internal CPU registers, its open bus behavior is a bit different.
// See https://www.nesdev.org/wiki/APU#Status_($4015).
uint8_t cynes::APU::read(uint8_t address)
{
    if (static_cast<Register>(address) == Register::CTRL_STATUS)
    {
        _internal_open_bus = _state.send_delta_channel_interrupt << 7;
        _internal_open_bus |= _state.send_frame_interrupt << 6;
        _internal_open_bus |= (_state.delta_channel_remaining_bytes > 0) << 4;

        for (uint8_t channel = 0; channel < 0x4; channel++)
        {
            _internal_open_bus |= (_state.channels_counters[channel] > 0) << channel;
        }

        set_frame_interrupt(false);

        return _internal_open_bus;
    }

    return _nes.get_open_bus();
}

void cynes::APU::update_counters()
{
    for (uint8_t channel = 0; channel < 0x4; channel++)
    {
        if (!_state.channel_halted[channel] && _state.channels_counters[channel] > 0)
        {
            _state.channels_counters[channel]--;
        }
    }
}

void cynes::APU::load_delta_channel_byte(bool reading)
{
    uint8_t delay = _state.delay_dma;

    if (delay == 0)
    {
        if (reading)
        {
            delay = 0x4;
        }
        else
        {
            delay = 0x3;
        }
    }

    for (uint8_t i = 0; i < delay; i++)
    {
        tick(false, true);

        _nes.ppu.tick();
        _nes.ppu.tick();
        _nes.ppu.tick();
        _nes.cpu.poll();
    }

    _state.delta_channel_sample_buffer_empty = false;
    _state.delta_channel_remaining_bytes--;

    if (_state.delta_channel_remaining_bytes == 0)
    {
        if (_state.delta_channel_should_loop)
        {
            _state.delta_channel_remaining_bytes = _state.delta_channel_sample_length;
        }
        else if (_state.delta_channel_enable_interrupt)
        {
            set_delta_interrupt(true);
        }
    }
}

void cynes::APU::perform_dma(uint8_t address)
{
    _state.address_dma = address;
    _state.pending_dma = true;
}

void cynes::APU::perform_pending_dma()
{
    if (!_state.pending_dma)
    {
        return;
    }

    _state.pending_dma = false;
    _state.delay_dma = 0x2;

    if (!_state.latch_cycle)
    {
        _nes.dummy_read();
    }

    _nes.dummy_read();

    uint16_t current_address = _state.address_dma << 8;
    uint8_t low_byte = 0x00;

    _nes.write(0x2004, _nes.read(current_address++));

    while ((low_byte = current_address & 0xFF) != 0)
    {
        uint8_t value = _nes.read(current_address++);

        if (low_byte == 254)
        {
            _state.delay_dma = 0x1;
            _nes.write(0x2004, value);
            _state.delay_dma = 0x2;
        }
        else if (low_byte == 255)
        {
            _state.delay_dma = 0x3;
            _nes.write(0x2004, value);
            _state.delay_dma = 0x0;
        }
        else
        {
            _nes.write(0x2004, value);
        }
    }
}

void cynes::APU::set_frame_interrupt(bool interrupt)
{
    _state.send_frame_interrupt = interrupt;
    _nes.cpu.set_frame_interrupt(interrupt);
}

void cynes::APU::set_delta_interrupt(bool interrupt)
{
    _state.send_delta_channel_interrupt = interrupt;
    _nes.cpu.set_delta_interrupt(interrupt);
}
