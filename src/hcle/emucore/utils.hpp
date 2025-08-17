#ifndef __CYNES_UTILS__
#define __CYNES_UTILS__

#include <cstdint>
#include <cstring>
#include <array>

static constexpr size_t MAX_PRG_SIZE = 1024 * 1024;   // 1 MB
static constexpr size_t MAX_CPU_RAM_SIZE = 32 * 1024; // 32 KB
static constexpr size_t MAX_PPU_RAM_SIZE = 4 * 1024;  // 4 KB
static constexpr size_t MAX_CHR_SIZE = 1024 * 1024;   // Optional block 1 MB (for mappers like MMC2/MMC5)
static constexpr size_t TOTAL_MAPPER_MEM = MAX_PRG_SIZE + MAX_CPU_RAM_SIZE + MAX_PPU_RAM_SIZE + MAX_CHR_SIZE;

static constexpr size_t PRG_BASE = 0;
static constexpr size_t CPU_RAM_BASE = PRG_BASE + MAX_PRG_SIZE;
static constexpr size_t PPU_RAM_BASE = CPU_RAM_BASE + MAX_CPU_RAM_SIZE;
// Place CHR_BASE last, after all other mapper memory
static constexpr size_t CHR_BASE = PPU_RAM_BASE + MAX_PPU_RAM_SIZE;

enum SpriteEvaluationStep
{
    LOAD_SECONDARY_OAM,
    INCREMENT_POINTER,
    IDLE
};

struct MemoryBank
{
    size_t offset = 0;
    bool read_only = true;
    bool mapped = false;
};

struct FullState
{
    // ======== APU Variables ========
    bool apu_latch_cycle;

    uint8_t delay_dma;
    uint8_t address_dma;

    bool pending_dma;

    uint32_t frame_counter_clock;
    uint32_t delay_frame_reset;

    uint8_t channels_counters[0x4];

    bool channel_enabled[0x4];
    bool channel_halted[0x4];

    bool step_mode;

    bool inhibit_frame_interrupt;
    bool send_frame_interrupt;

    uint16_t delta_channel_remaining_bytes;
    uint16_t delta_channel_sample_length;
    uint16_t delta_channel_period_counter;
    uint16_t delta_channel_period_load;

    uint8_t delta_channel_bits_in_buffer;

    bool delta_channel_should_loop;
    bool delta_channel_enable_interrupt;
    bool delta_channel_sample_buffer_empty;

    bool enable_dmc;
    bool send_delta_channel_interrupt;

    // ======== PPU Variables ========
    uint16_t current_x;
    uint16_t current_y;

    bool frame_ready;

    bool rendering_enabled;
    bool rendering_enabled_delayed;
    bool prevent_vertical_blank;

    bool control_increment_mode;
    bool control_foreground_table;
    bool control_background_table;
    bool control_foreground_large;
    bool control_interrupt_on_vertical_blank;

    bool mask_grayscale_mode;
    bool mask_render_background_left;
    bool mask_render_foreground_left;
    bool mask_render_background;
    bool mask_render_foreground;

    uint8_t mask_color_emphasize;

    bool status_sprite_overflow;
    bool status_sprite_zero_hit;
    bool status_vertical_blank;

    uint8_t clock_decays[3];
    uint8_t register_decay;

    bool ppu_latch_cycle;
    bool latch_address;

    uint16_t register_t_;
    uint16_t register_v;
    uint16_t delayed_register_v;

    uint8_t scroll_x;
    uint8_t delay_data_read_counter;
    uint8_t delay_data_write_counter;
    uint8_t buffer_data;

    uint8_t background_data[0x4];
    uint16_t background_shifter[0x4];

    uint8_t foreground_data[0x20];
    uint8_t foreground_shifter[0x10];
    uint8_t foreground_attributes[0x8];
    uint8_t foreground_positions[0x8];

    uint8_t foreground_data_pointer;
    uint8_t foreground_sprite_count;
    uint8_t foreground_sprite_count_next;
    uint8_t foreground_sprite_pointer;
    uint8_t foreground_read_delay_counter;

    uint16_t foreground_sprite_address;

    bool foreground_sprite_zero_line;
    bool foreground_sprite_zero_should;
    bool foreground_sprite_zero_hit;

    SpriteEvaluationStep foreground_evaluation_step;

    // ======== CPU VARIABLES ========
    uint8_t register_a, register_x, register_y, register_m, stack_pointer;
    uint16_t program_counter;

    // Internal State
    uint16_t target_address;
    bool frozen;

    uint8_t status;

    // Interrupt State
    bool delay_interrupt;
    bool should_issue_interrupt;
    bool line_mapper_interrupt;
    bool line_frame_interrupt;
    bool line_delta_interrupt;
    bool line_non_maskable_interrupt;
    bool edge_detector_non_maskable_interrupt;
    bool delay_non_maskable_interrupt;
    bool should_issue_non_maskable_interrupt;

    // ====== NES MEM VARS =======
    uint8_t mem_cpu[0x800];
    uint8_t mem_oam[0x100];
    uint8_t mem_palette[0x20];

    // ====== MAPPER VARS =======
    MemoryBank banks_cpu[0x40];
    MemoryBank banks_ppu[0x10];

    // Generalised mapper-specific data
    // MMC1
    uint32_t tick;           // uint_8 for MMC1
    uint32_t registers[0x8]; // 0x4 uint8 for MMC1, 0x8 uint32 for MMC3
    uint8_t reg;
    uint16_t counter; // uint_8 for MMC1
    // MMC3
    uint16_t counter_reset_value;
    uint8_t register_target;
    bool mode_prg;
    bool mode_chr;
    bool enable_interrupt;
    bool should_reload_interrupt;
    // MMC
    bool latches[0x2];
    uint8_t selected_banks[0x4];

    // General
    uint8_t memory[TOTAL_MAPPER_MEM];
    // std::array<uint8_t, TOTAL_MAPPER_MEM> memory{};
};

namespace cynes
{
    static constexpr size_t FULL_STATE_SIZE = sizeof(FullState);

    enum class DumpOperation
    {
        SIZE,
        DUMP,
        LOAD
    };

    template <DumpOperation operation, typename T>
    constexpr void dump(uint8_t *&buffer, T &value)
    {
        if constexpr (operation == DumpOperation::DUMP)
        {
            memcpy(buffer, &value, sizeof(T));
        }
        else if constexpr (operation == DumpOperation::LOAD)
        {
            memcpy(&value, buffer, sizeof(T));
        }

        buffer += sizeof(T);
    }

    template <DumpOperation operation, typename T>
    constexpr void dump(unsigned int &buffer_size, T &)
    {
        if constexpr (operation == DumpOperation::SIZE)
        {
            buffer_size += sizeof(T);
        }
    }

    template <DumpOperation operation, typename T>
    constexpr void dump(uint8_t *&buffer, T *values, unsigned int size)
    {
        if constexpr (operation == DumpOperation::DUMP)
        {
            memcpy(buffer, values, sizeof(T) * size);
        }
        else if constexpr (operation == DumpOperation::LOAD)
        {
            memcpy(values, buffer, sizeof(T) * size);
        }

        buffer += sizeof(T) * size;
    }

    template <DumpOperation operation, typename T>
    constexpr void dump(unsigned int &buffer_size, T *, unsigned int size)
    {
        if constexpr (operation == DumpOperation::SIZE)
        {
            buffer_size += sizeof(T) * size;
        }
    }
}

#endif
