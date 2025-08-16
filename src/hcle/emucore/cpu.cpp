#include "cpu.hpp"
#include "nes.hpp"

#include <cstring>

using _addr_ptr = void (cynes::CPU::*)();
;
const _addr_ptr cynes::CPU::ADDRESSING_MODES[256] = {
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ixr, &cynes::CPU::addr_acc, &cynes::CPU::addr_ixr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_acc, &cynes::CPU::addr_imm, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr,
    &cynes::CPU::addr_rel, &cynes::CPU::addr_iyr, &cynes::CPU::addr_acc, &cynes::CPU::addr_iym, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ayr, &cynes::CPU::addr_imp, &cynes::CPU::addr_aym, &cynes::CPU::addr_axr, &cynes::CPU::addr_axr, &cynes::CPU::addr_axm, &cynes::CPU::addr_axm,
    &cynes::CPU::addr_abw, &cynes::CPU::addr_ixr, &cynes::CPU::addr_acc, &cynes::CPU::addr_ixr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_acc, &cynes::CPU::addr_imm, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr,
    &cynes::CPU::addr_rel, &cynes::CPU::addr_iyr, &cynes::CPU::addr_acc, &cynes::CPU::addr_iym, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ayr, &cynes::CPU::addr_imp, &cynes::CPU::addr_aym, &cynes::CPU::addr_axr, &cynes::CPU::addr_axr, &cynes::CPU::addr_axm, &cynes::CPU::addr_axm,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ixr, &cynes::CPU::addr_acc, &cynes::CPU::addr_ixr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_acc, &cynes::CPU::addr_imm, &cynes::CPU::addr_abw, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr,
    &cynes::CPU::addr_rel, &cynes::CPU::addr_iyr, &cynes::CPU::addr_acc, &cynes::CPU::addr_iym, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ayr, &cynes::CPU::addr_imp, &cynes::CPU::addr_aym, &cynes::CPU::addr_axr, &cynes::CPU::addr_axr, &cynes::CPU::addr_axm, &cynes::CPU::addr_axm,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ixr, &cynes::CPU::addr_acc, &cynes::CPU::addr_ixr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_acc, &cynes::CPU::addr_imm, &cynes::CPU::addr_ind, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr,
    &cynes::CPU::addr_rel, &cynes::CPU::addr_iyr, &cynes::CPU::addr_acc, &cynes::CPU::addr_iym, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ayr, &cynes::CPU::addr_imp, &cynes::CPU::addr_aym, &cynes::CPU::addr_axr, &cynes::CPU::addr_axr, &cynes::CPU::addr_axm, &cynes::CPU::addr_axm,
    &cynes::CPU::addr_imm, &cynes::CPU::addr_ixw, &cynes::CPU::addr_imm, &cynes::CPU::addr_ixw, &cynes::CPU::addr_zpw, &cynes::CPU::addr_zpw, &cynes::CPU::addr_zpw, &cynes::CPU::addr_zpw,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_abw, &cynes::CPU::addr_abw, &cynes::CPU::addr_abw, &cynes::CPU::addr_abw,
    &cynes::CPU::addr_rel, &cynes::CPU::addr_iyw, &cynes::CPU::addr_acc, &cynes::CPU::addr_iyw, &cynes::CPU::addr_zxw, &cynes::CPU::addr_zxw, &cynes::CPU::addr_zyw, &cynes::CPU::addr_zyw,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ayw, &cynes::CPU::addr_imp, &cynes::CPU::addr_ayw, &cynes::CPU::addr_axw, &cynes::CPU::addr_axw, &cynes::CPU::addr_ayw, &cynes::CPU::addr_ayw,
    &cynes::CPU::addr_imm, &cynes::CPU::addr_ixr, &cynes::CPU::addr_imm, &cynes::CPU::addr_ixr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr,
    &cynes::CPU::addr_rel, &cynes::CPU::addr_iyr, &cynes::CPU::addr_acc, &cynes::CPU::addr_iyr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zyr, &cynes::CPU::addr_zyr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ayr, &cynes::CPU::addr_imp, &cynes::CPU::addr_ayr, &cynes::CPU::addr_axr, &cynes::CPU::addr_axr, &cynes::CPU::addr_ayr, &cynes::CPU::addr_ayr,
    &cynes::CPU::addr_imm, &cynes::CPU::addr_ixr, &cynes::CPU::addr_imm, &cynes::CPU::addr_ixr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr,
    &cynes::CPU::addr_rel, &cynes::CPU::addr_iyr, &cynes::CPU::addr_acc, &cynes::CPU::addr_iym, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ayr, &cynes::CPU::addr_imp, &cynes::CPU::addr_aym, &cynes::CPU::addr_axr, &cynes::CPU::addr_axr, &cynes::CPU::addr_axm, &cynes::CPU::addr_axm,
    &cynes::CPU::addr_imm, &cynes::CPU::addr_ixr, &cynes::CPU::addr_imm, &cynes::CPU::addr_ixr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr, &cynes::CPU::addr_zpr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_imp, &cynes::CPU::addr_imm, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr, &cynes::CPU::addr_abr,
    &cynes::CPU::addr_rel, &cynes::CPU::addr_iyr, &cynes::CPU::addr_acc, &cynes::CPU::addr_iym, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr, &cynes::CPU::addr_zxr,
    &cynes::CPU::addr_imp, &cynes::CPU::addr_ayr, &cynes::CPU::addr_imp, &cynes::CPU::addr_aym, &cynes::CPU::addr_axr, &cynes::CPU::addr_axr, &cynes::CPU::addr_axm, &cynes::CPU::addr_axm};

using _op_ptr = void (cynes::CPU::*)();
;
const _op_ptr cynes::CPU::INSTRUCTIONS[256] = {
    &cynes::CPU::op_brk, &cynes::CPU::op_ora, &cynes::CPU::op_jam, &cynes::CPU::op_slo, &cynes::CPU::op_nop, &cynes::CPU::op_ora, &cynes::CPU::op_asl, &cynes::CPU::op_slo,
    &cynes::CPU::op_php, &cynes::CPU::op_ora, &cynes::CPU::op_aal, &cynes::CPU::op_anc, &cynes::CPU::op_nop, &cynes::CPU::op_ora, &cynes::CPU::op_asl, &cynes::CPU::op_slo,
    &cynes::CPU::op_bpl, &cynes::CPU::op_ora, &cynes::CPU::op_jam, &cynes::CPU::op_slo, &cynes::CPU::op_nop, &cynes::CPU::op_ora, &cynes::CPU::op_asl, &cynes::CPU::op_slo,
    &cynes::CPU::op_clc, &cynes::CPU::op_ora, &cynes::CPU::op_nop, &cynes::CPU::op_slo, &cynes::CPU::op_nop, &cynes::CPU::op_ora, &cynes::CPU::op_asl, &cynes::CPU::op_slo,
    &cynes::CPU::op_jsr, &cynes::CPU::op_and, &cynes::CPU::op_jam, &cynes::CPU::op_rla, &cynes::CPU::op_bit, &cynes::CPU::op_and, &cynes::CPU::op_rol, &cynes::CPU::op_rla,
    &cynes::CPU::op_plp, &cynes::CPU::op_and, &cynes::CPU::op_ral, &cynes::CPU::op_anc, &cynes::CPU::op_bit, &cynes::CPU::op_and, &cynes::CPU::op_rol, &cynes::CPU::op_rla,
    &cynes::CPU::op_bmi, &cynes::CPU::op_and, &cynes::CPU::op_jam, &cynes::CPU::op_rla, &cynes::CPU::op_nop, &cynes::CPU::op_and, &cynes::CPU::op_rol, &cynes::CPU::op_rla,
    &cynes::CPU::op_sec, &cynes::CPU::op_and, &cynes::CPU::op_nop, &cynes::CPU::op_rla, &cynes::CPU::op_nop, &cynes::CPU::op_and, &cynes::CPU::op_rol, &cynes::CPU::op_rla,
    &cynes::CPU::op_rti, &cynes::CPU::op_eor, &cynes::CPU::op_jam, &cynes::CPU::op_sre, &cynes::CPU::op_nop, &cynes::CPU::op_eor, &cynes::CPU::op_lsr, &cynes::CPU::op_sre,
    &cynes::CPU::op_pha, &cynes::CPU::op_eor, &cynes::CPU::op_lar, &cynes::CPU::op_alr, &cynes::CPU::op_jmp, &cynes::CPU::op_eor, &cynes::CPU::op_lsr, &cynes::CPU::op_sre,
    &cynes::CPU::op_bvc, &cynes::CPU::op_eor, &cynes::CPU::op_jam, &cynes::CPU::op_sre, &cynes::CPU::op_nop, &cynes::CPU::op_eor, &cynes::CPU::op_lsr, &cynes::CPU::op_sre,
    &cynes::CPU::op_cli, &cynes::CPU::op_eor, &cynes::CPU::op_nop, &cynes::CPU::op_sre, &cynes::CPU::op_nop, &cynes::CPU::op_eor, &cynes::CPU::op_lsr, &cynes::CPU::op_sre,
    &cynes::CPU::op_rts, &cynes::CPU::op_adc, &cynes::CPU::op_jam, &cynes::CPU::op_rra, &cynes::CPU::op_nop, &cynes::CPU::op_adc, &cynes::CPU::op_ror, &cynes::CPU::op_rra,
    &cynes::CPU::op_pla, &cynes::CPU::op_adc, &cynes::CPU::op_rar, &cynes::CPU::op_arr, &cynes::CPU::op_jmp, &cynes::CPU::op_adc, &cynes::CPU::op_ror, &cynes::CPU::op_rra,
    &cynes::CPU::op_bvs, &cynes::CPU::op_adc, &cynes::CPU::op_jam, &cynes::CPU::op_rra, &cynes::CPU::op_nop, &cynes::CPU::op_adc, &cynes::CPU::op_ror, &cynes::CPU::op_rra,
    &cynes::CPU::op_sei, &cynes::CPU::op_adc, &cynes::CPU::op_nop, &cynes::CPU::op_rra, &cynes::CPU::op_nop, &cynes::CPU::op_adc, &cynes::CPU::op_ror, &cynes::CPU::op_rra,
    &cynes::CPU::op_nop, &cynes::CPU::op_sta, &cynes::CPU::op_nop, &cynes::CPU::op_sax, &cynes::CPU::op_sty, &cynes::CPU::op_sta, &cynes::CPU::op_stx, &cynes::CPU::op_sax,
    &cynes::CPU::op_dey, &cynes::CPU::op_nop, &cynes::CPU::op_txa, &cynes::CPU::op_ane, &cynes::CPU::op_sty, &cynes::CPU::op_sta, &cynes::CPU::op_stx, &cynes::CPU::op_sax,
    &cynes::CPU::op_bcc, &cynes::CPU::op_sta, &cynes::CPU::op_jam, &cynes::CPU::op_sha, &cynes::CPU::op_sty, &cynes::CPU::op_sta, &cynes::CPU::op_stx, &cynes::CPU::op_sax,
    &cynes::CPU::op_tya, &cynes::CPU::op_sta, &cynes::CPU::op_txs, &cynes::CPU::op_tas, &cynes::CPU::op_shy, &cynes::CPU::op_sta, &cynes::CPU::op_shx, &cynes::CPU::op_sha,
    &cynes::CPU::op_ldy, &cynes::CPU::op_lda, &cynes::CPU::op_ldx, &cynes::CPU::op_lax, &cynes::CPU::op_ldy, &cynes::CPU::op_lda, &cynes::CPU::op_ldx, &cynes::CPU::op_lax,
    &cynes::CPU::op_tay, &cynes::CPU::op_lda, &cynes::CPU::op_tax, &cynes::CPU::op_lxa, &cynes::CPU::op_ldy, &cynes::CPU::op_lda, &cynes::CPU::op_ldx, &cynes::CPU::op_lax,
    &cynes::CPU::op_bcs, &cynes::CPU::op_lda, &cynes::CPU::op_jam, &cynes::CPU::op_lax, &cynes::CPU::op_ldy, &cynes::CPU::op_lda, &cynes::CPU::op_ldx, &cynes::CPU::op_lax,
    &cynes::CPU::op_clv, &cynes::CPU::op_lda, &cynes::CPU::op_tsx, &cynes::CPU::op_las, &cynes::CPU::op_ldy, &cynes::CPU::op_lda, &cynes::CPU::op_ldx, &cynes::CPU::op_lax,
    &cynes::CPU::op_cpy, &cynes::CPU::op_cmp, &cynes::CPU::op_nop, &cynes::CPU::op_dcp, &cynes::CPU::op_cpy, &cynes::CPU::op_cmp, &cynes::CPU::op_dec, &cynes::CPU::op_dcp,
    &cynes::CPU::op_iny, &cynes::CPU::op_cmp, &cynes::CPU::op_dex, &cynes::CPU::op_sbx, &cynes::CPU::op_cpy, &cynes::CPU::op_cmp, &cynes::CPU::op_dec, &cynes::CPU::op_dcp,
    &cynes::CPU::op_bne, &cynes::CPU::op_cmp, &cynes::CPU::op_jam, &cynes::CPU::op_dcp, &cynes::CPU::op_nop, &cynes::CPU::op_cmp, &cynes::CPU::op_dec, &cynes::CPU::op_dcp,
    &cynes::CPU::op_cld, &cynes::CPU::op_cmp, &cynes::CPU::op_nop, &cynes::CPU::op_dcp, &cynes::CPU::op_nop, &cynes::CPU::op_cmp, &cynes::CPU::op_dec, &cynes::CPU::op_dcp,
    &cynes::CPU::op_cpx, &cynes::CPU::op_sbc, &cynes::CPU::op_nop, &cynes::CPU::op_isc, &cynes::CPU::op_cpx, &cynes::CPU::op_sbc, &cynes::CPU::op_inc, &cynes::CPU::op_isc,
    &cynes::CPU::op_inx, &cynes::CPU::op_sbc, &cynes::CPU::op_nop, &cynes::CPU::op_usb, &cynes::CPU::op_cpx, &cynes::CPU::op_sbc, &cynes::CPU::op_inc, &cynes::CPU::op_isc,
    &cynes::CPU::op_beq, &cynes::CPU::op_sbc, &cynes::CPU::op_jam, &cynes::CPU::op_isc, &cynes::CPU::op_nop, &cynes::CPU::op_sbc, &cynes::CPU::op_inc, &cynes::CPU::op_isc,
    &cynes::CPU::op_sed, &cynes::CPU::op_sbc, &cynes::CPU::op_nop, &cynes::CPU::op_isc, &cynes::CPU::op_nop, &cynes::CPU::op_sbc, &cynes::CPU::op_inc, &cynes::CPU::op_isc};

cynes::CPU::CPU(NES &nes)
    : _nes{nes},
      _state{
          .register_a = 0x00,
          .register_x = 0x00,
          .register_y = 0x00,
          .register_m = 0x00,
          .stack_pointer = 0x00,
          .program_counter = 0x0000,
          .target_address = 0x0000,
          .frozen = false,
          .status = 0x00,
          .delay_interrupt = false,
          .should_issue_interrupt = false,
          .line_mapper_interrupt = false,
          .line_frame_interrupt = false,
          .line_delta_interrupt = false,
          .line_non_maskable_interrupt = false,
          .edge_detector_non_maskable_interrupt = false,
          .delay_non_maskable_interrupt = false,
          .should_issue_non_maskable_interrupt = false,
      }
{
}

void cynes::CPU::power()
{
    _state.frozen = false;
    _state.line_non_maskable_interrupt = false;
    _state.line_mapper_interrupt = false;
    _state.line_frame_interrupt = false;
    _state.line_delta_interrupt = false;
    _state.should_issue_interrupt = false;
    _state.register_a = 0x00;
    _state.register_x = 0x00;
    _state.register_y = 0x00;
    _state.stack_pointer = 0xFD;
    _state.status = Flag::I;
    _state.program_counter = _nes.read_cpu(0xFFFC);
    _state.program_counter |= _nes.read_cpu(0xFFFD) << 8;
}

void cynes::CPU::reset()
{
    _state.frozen = false;
    _state.line_non_maskable_interrupt = false;
    _state.line_mapper_interrupt = false;
    _state.line_frame_interrupt = false;
    _state.line_delta_interrupt = false;
    _state.stack_pointer -= 3;
    _state.status |= Flag::I;
    _state.program_counter = _nes.read_cpu(0xFFFC);
    _state.program_counter |= _nes.read_cpu(0xFFFD) << 8;
}

void cynes::CPU::tick()
{
    if (_state.frozen)
    {
        return;
    }

    uint8_t instruction = fetch_next();

    (this->*ADDRESSING_MODES[instruction])();
    (this->*INSTRUCTIONS[instruction])();

    if (_state.delay_non_maskable_interrupt || _state.delay_interrupt)
    {
        _nes.read(_state.program_counter);
        _nes.read(_state.program_counter);

        _nes.write(0x100 | _state.stack_pointer--, _state.program_counter >> 8);
        _nes.write(0x100 | _state.stack_pointer--, _state.program_counter & 0x00FF);

        uint16_t address = _state.should_issue_non_maskable_interrupt ? 0xFFFA : 0xFFFE;

        _state.should_issue_non_maskable_interrupt = false;

        _nes.write(0x100 | _state.stack_pointer--, _state.status | Flag::U);

        set_status(Flag::I, true);

        _state.program_counter = _nes.read(address);
        _state.program_counter |= _nes.read(address + 1) << 8;
    }
}

void cynes::CPU::poll()
{
    _state.delay_non_maskable_interrupt = _state.should_issue_non_maskable_interrupt;

    if (!_state.edge_detector_non_maskable_interrupt && _state.line_non_maskable_interrupt)
    {
        _state.should_issue_non_maskable_interrupt = true;
    }

    _state.edge_detector_non_maskable_interrupt = _state.line_non_maskable_interrupt;
    _state.delay_interrupt = _state.should_issue_interrupt;

    _state.should_issue_interrupt = (_state.line_mapper_interrupt || _state.line_frame_interrupt || _state.line_delta_interrupt) && !get_status(Flag::I);
}

void cynes::CPU::set_non_maskable_interrupt(bool interrupt)
{
    _state.line_non_maskable_interrupt = interrupt;
}

void cynes::CPU::set_mapper_interrupt(bool interrupt)
{
    _state.line_mapper_interrupt = interrupt;
}

void cynes::CPU::set_frame_interrupt(bool interrupt)
{
    _state.line_frame_interrupt = interrupt;
}

void cynes::CPU::set_delta_interrupt(bool interrupt)
{
    _state.line_delta_interrupt = interrupt;
}

bool cynes::CPU::is_frozen() const
{
    return _state.frozen;
}

uint8_t cynes::CPU::fetch_next()
{
    return _nes.read(_state.program_counter++);
}

void cynes::CPU::set_status(uint8_t flag, bool value)
{
    if (value)
    {
        _state.status |= flag;
    }
    else
    {
        _state.status &= ~flag;
    }
}

bool cynes::CPU::get_status(uint8_t flag) const
{
    return _state.status & flag;
}

void cynes::CPU::addr_abr()
{
    addr_abw();
    _state.register_m = _nes.read(_state.target_address);
}

void cynes::CPU::addr_abw()
{
    _state.target_address = fetch_next();
    _state.target_address |= fetch_next() << 8;
}

void cynes::CPU::addr_acc()
{
    _state.register_m = _nes.read(_state.program_counter);
}

void cynes::CPU::addr_axm()
{
    addr_axw();
    _state.register_m = _nes.read(_state.target_address);
}

void cynes::CPU::addr_axr()
{
    _state.target_address = fetch_next();

    uint16_t translated = _state.target_address + _state.register_x;
    bool invalid_address = (_state.target_address & 0xFF00) != (translated & 0xFF00);

    _state.target_address = translated & 0x00FF;
    _state.target_address |= fetch_next() << 8;
    _state.register_m = _nes.read(_state.target_address);

    if (invalid_address)
    {
        _state.target_address += 0x100;
        _state.register_m = _nes.read(_state.target_address);
    }
}

void cynes::CPU::addr_axw()
{
    _state.target_address = fetch_next();

    uint16_t translated = _state.target_address + _state.register_x;
    bool invalid_address = (_state.target_address & 0xFF00) != (translated & 0xFF00);

    _state.target_address = translated & 0x00FF;
    _state.target_address |= fetch_next() << 8;
    _state.register_m = _nes.read(_state.target_address);

    if (invalid_address)
    {
        _state.target_address += 0x100;
    }
}

void cynes::CPU::addr_aym()
{
    addr_ayw();
    _state.register_m = _nes.read(_state.target_address);
}

void cynes::CPU::addr_ayr()
{
    _state.target_address = fetch_next();

    uint16_t translated = _state.target_address + _state.register_y;
    bool invalid_address = (_state.target_address & 0xFF00) != (translated & 0xFF00);

    _state.target_address = translated & 0x00FF;
    _state.target_address |= fetch_next() << 8;
    _state.register_m = _nes.read(_state.target_address);

    if (invalid_address)
    {
        _state.target_address += 0x100;
        _state.register_m = _nes.read(_state.target_address);
    }
}

void cynes::CPU::addr_ayw()
{
    _state.target_address = fetch_next();

    uint16_t translated = _state.target_address + _state.register_y;
    bool invalid_address = (_state.target_address & 0xFF00) != (translated & 0xFF00);

    _state.target_address = translated & 0x00FF;
    _state.target_address |= fetch_next() << 8;
    _state.register_m = _nes.read(_state.target_address);

    if (invalid_address)
    {
        _state.target_address += 0x100;
    }
}

void cynes::CPU::addr_imm()
{
    _state.register_m = fetch_next();
}

void cynes::CPU::addr_imp()
{
    _state.register_m = _nes.read(_state.program_counter);
}

void cynes::CPU::addr_ind()
{
    uint16_t pointer = fetch_next();

    pointer |= fetch_next() << 8;

    if ((pointer & 0x00FF) == 0xFF)
    {
        _state.target_address = _nes.read(pointer);
        _state.target_address |= _nes.read(pointer & 0xFF00) << 8;
    }
    else
    {
        _state.target_address = _nes.read(pointer);
        _state.target_address |= _nes.read(pointer + 1) << 8;
    }
}

void cynes::CPU::addr_ixr()
{
    addr_ixw();
    _state.register_m = _nes.read(_state.target_address);
}

void cynes::CPU::addr_ixw()
{
    uint8_t pointer = fetch_next();

    _state.register_m = _nes.read(pointer);

    pointer += _state.register_x;

    _state.target_address = _nes.read(pointer);
    _state.target_address |= _nes.read(++pointer & 0xFF) << 8;
}

void cynes::CPU::addr_iym()
{
    addr_iyw();
    _state.register_m = _nes.read(_state.target_address);
}

void cynes::CPU::addr_iyr()
{
    uint8_t pointer = fetch_next();

    _state.target_address = _nes.read(pointer);

    uint16_t translated = _state.target_address + _state.register_y;
    bool invalid_address = translated & 0xFF00;

    _state.target_address = translated & 0x00FF;
    _state.target_address |= _nes.read(++pointer & 0xFF) << 8;
    _state.register_m = _nes.read(_state.target_address);

    if (invalid_address)
    {
        _state.target_address += 0x100;
        _state.register_m = _nes.read(_state.target_address);
    }
}

void cynes::CPU::addr_iyw()
{
    uint8_t pointer = fetch_next();

    _state.target_address = _nes.read(pointer);

    uint16_t translated = _state.target_address + _state.register_y;
    bool invalid_address = (_state.target_address & 0xFF00) != (translated & 0xFF00);

    _state.target_address = translated & 0x00FF;
    _state.target_address |= _nes.read(++pointer & 0xFF) << 8;
    _state.register_m = _nes.read(_state.target_address);

    if (invalid_address)
    {
        _state.target_address += 0x100;
    }
}

void cynes::CPU::addr_rel()
{
    _state.target_address = fetch_next();

    if (_state.target_address & 0x80)
    {
        _state.target_address |= 0xFF00;
    }
}

void cynes::CPU::addr_zpr()
{
    addr_zpw();
    _state.register_m = _nes.read(_state.target_address);
}

void cynes::CPU::addr_zpw()
{
    _state.target_address = fetch_next();
}

void cynes::CPU::addr_zxr()
{
    addr_zxw();
    _state.register_m = _nes.read(_state.target_address);
}

void cynes::CPU::addr_zxw()
{
    _state.target_address = fetch_next();
    _state.register_m = _nes.read(_state.target_address);
    _state.target_address += _state.register_x;
    _state.target_address &= 0x00FF;
}

void cynes::CPU::addr_zyr()
{
    addr_zyw();
    _state.register_m = _nes.read(_state.target_address);
}

void cynes::CPU::addr_zyw()
{
    _state.target_address = fetch_next();
    _state.register_m = _nes.read(_state.target_address);
    _state.target_address += _state.register_y;
    _state.target_address &= 0x00FF;
}

void cynes::CPU::op_aal()
{
    set_status(Flag::C, _state.register_a & 0x80);

    _state.register_a <<= 1;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}

void cynes::CPU::op_adc()
{
    uint16_t result = _state.register_a + _state.register_m + (get_status(Flag::C) ? 0x01 : 0x00);

    set_status(Flag::C, result & 0xFF00);
    set_status(Flag::V, ~(_state.register_a ^ _state.register_m) & (_state.register_a ^ result) & 0x80);

    _state.register_a = result & 0x00FF;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}

void cynes::CPU::op_alr()
{
    _state.register_a &= _state.register_m;

    set_status(Flag::C, _state.register_a & 0x01);

    _state.register_a >>= 1;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}

void cynes::CPU::op_anc()
{
    _state.register_a &= _state.register_m;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
    set_status(Flag::C, _state.register_a & 0x80);
}

void cynes::CPU::op_and()
{
    _state.register_a &= _state.register_m;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}

void cynes::CPU::op_ane()
{
    _state.register_a = (_state.register_a | 0xEE) & _state.register_x & _state.register_m;
}

void cynes::CPU::op_arr()
{
    _state.register_a &= _state.register_m;
    _state.register_a = (get_status(Flag::C) ? 0x80 : 0x00) | (_state.register_a >> 1);

    set_status(Flag::C, _state.register_a & 0x40);
    set_status(Flag::V, bool(_state.register_a & 0x40) ^ bool(_state.register_a & 0x20));
    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}

void cynes::CPU::op_asl()
{
    _nes.write(_state.target_address, _state.register_m);

    set_status(Flag::C, _state.register_m & 0x80);

    _state.register_m <<= 1;

    set_status(Flag::Z, !_state.register_m);
    set_status(Flag::N, _state.register_m & 0x80);

    _nes.write(_state.target_address, _state.register_m);
}

void cynes::CPU::op_bcc()
{
    if (!get_status(Flag::C))
    {
        if (_state.should_issue_interrupt && !_state.delay_interrupt)
        {
            _state.should_issue_interrupt = false;
        }

        _nes.read(_state.program_counter);

        uint16_t translated = _state.target_address + _state.program_counter;

        if ((translated & 0xFF00) != (_state.program_counter & 0xFF00))
        {
            _nes.read(_state.program_counter);
        }

        _state.program_counter = translated;
    }
}

void cynes::CPU::op_bcs()
{
    if (get_status(Flag::C))
    {
        if (_state.should_issue_interrupt && !_state.delay_interrupt)
        {
            _state.should_issue_interrupt = false;
        }

        _nes.read(_state.program_counter);

        uint16_t translated = _state.target_address + _state.program_counter;

        if ((translated & 0xFF00) != (_state.program_counter & 0xFF00))
        {
            _nes.read(_state.program_counter);
        }

        _state.program_counter = translated;
    }
}

void cynes::CPU::op_beq()
{
    if (get_status(Flag::Z))
    {
        if (_state.should_issue_interrupt && !_state.delay_interrupt)
        {
            _state.should_issue_interrupt = false;
        }

        _nes.read(_state.program_counter);

        uint16_t translated = _state.target_address + _state.program_counter;

        if ((translated & 0xFF00) != (_state.program_counter & 0xFF00))
        {
            _nes.read(_state.program_counter);
        }

        _state.program_counter = translated;
    }
}

void cynes::CPU::op_bit()
{
    set_status(Flag::Z, !(_state.register_a & _state.register_m));
    set_status(Flag::V, _state.register_m & 0x40);
    set_status(Flag::N, _state.register_m & 0x80);
}

void cynes::CPU::op_bmi()
{
    if (get_status(Flag::N))
    {
        if (_state.should_issue_interrupt && !_state.delay_interrupt)
        {
            _state.should_issue_interrupt = false;
        }

        _nes.read(_state.program_counter);

        uint16_t translated = _state.target_address + _state.program_counter;

        if ((translated & 0xFF00) != (_state.program_counter & 0xFF00))
        {
            _nes.read(_state.program_counter);
        }

        _state.program_counter = translated;
    }
}

void cynes::CPU::op_bne()
{
    if (!get_status(Flag::Z))
    {
        if (_state.should_issue_interrupt && !_state.delay_interrupt)
        {
            _state.should_issue_interrupt = false;
        }

        _nes.read(_state.program_counter);

        uint16_t translated = _state.target_address + _state.program_counter;

        if ((translated & 0xFF00) != (_state.program_counter & 0xFF00))
        {
            _nes.read(_state.program_counter);
        }

        _state.program_counter = translated;
    }
}

void cynes::CPU::op_bpl()
{
    if (!get_status(Flag::N))
    {
        if (_state.should_issue_interrupt && !_state.delay_interrupt)
        {
            _state.should_issue_interrupt = false;
        }

        _nes.read(_state.program_counter);

        uint16_t translated = _state.target_address + _state.program_counter;

        if ((translated & 0xFF00) != (_state.program_counter & 0xFF00))
        {
            _nes.read(_state.program_counter);
        }

        _state.program_counter = translated;
    }
}

void cynes::CPU::op_brk()
{
    _state.program_counter++;

    _nes.write(0x100 | _state.stack_pointer--, _state.program_counter >> 8);
    _nes.write(0x100 | _state.stack_pointer--, _state.program_counter & 0x00FF);

    uint16_t address = _state.should_issue_non_maskable_interrupt ? 0xFFFA : 0xFFFE;

    _state.should_issue_non_maskable_interrupt = false;

    _nes.write(0x100 | _state.stack_pointer--, _state.status | Flag::B | Flag::U);

    set_status(Flag::I, true);

    _state.program_counter = _nes.read(address);
    _state.program_counter |= _nes.read(address + 1) << 8;

    _state.delay_non_maskable_interrupt = false;
}

void cynes::CPU::op_bvc()
{
    if (!get_status(Flag::V))
    {
        if (_state.should_issue_interrupt && !_state.delay_interrupt)
        {
            _state.should_issue_interrupt = false;
        }

        _nes.read(_state.program_counter);

        uint16_t translated = _state.target_address + _state.program_counter;

        if ((translated & 0xFF00) != (_state.program_counter & 0xFF00))
        {
            _nes.read(_state.program_counter);
        }

        _state.program_counter = translated;
    }
}

void cynes::CPU::op_bvs()
{
    if (get_status(Flag::V))
    {
        if (_state.should_issue_interrupt && !_state.delay_interrupt)
        {
            _state.should_issue_interrupt = false;
        }

        _nes.read(_state.program_counter);

        uint16_t translated = _state.target_address + _state.program_counter;

        if ((translated & 0xFF00) != (_state.program_counter & 0xFF00))
        {
            _nes.read(_state.program_counter);
        }

        _state.program_counter = translated;
    }
}

void cynes::CPU::op_clc()
{
    set_status(Flag::C, false);
}

void cynes::CPU::op_cld()
{
    set_status(Flag::D, false);
}

void cynes::CPU::op_cli()
{
    set_status(Flag::I, false);
}

void cynes::CPU::op_clv()
{
    set_status(Flag::V, false);
}

void cynes::CPU::op_cmp()
{
    set_status(Flag::C, _state.register_a >= _state.register_m);
    set_status(Flag::Z, _state.register_a == _state.register_m);
    set_status(Flag::N, (_state.register_a - _state.register_m) & 0x80);
}

void cynes::CPU::op_cpx()
{
    set_status(Flag::C, _state.register_x >= _state.register_m);
    set_status(Flag::Z, _state.register_x == _state.register_m);
    set_status(Flag::N, (_state.register_x - _state.register_m) & 0x80);
}

void cynes::CPU::op_cpy()
{
    set_status(Flag::C, _state.register_y >= _state.register_m);
    set_status(Flag::Z, _state.register_y == _state.register_m);
    set_status(Flag::N, (_state.register_y - _state.register_m) & 0x80);
}

void cynes::CPU::op_dcp()
{
    _nes.write(_state.target_address, _state.register_m);

    _state.register_m--;

    set_status(Flag::C, _state.register_a >= _state.register_m);
    set_status(Flag::Z, _state.register_a == _state.register_m);
    set_status(Flag::N, (_state.register_a - _state.register_m) & 0x80);

    _nes.write(_state.target_address, _state.register_m);
}

void cynes::CPU::op_dec()
{
    _nes.write(_state.target_address, _state.register_m);

    _state.register_m--;

    set_status(Flag::Z, !_state.register_m);
    set_status(Flag::N, _state.register_m & 0x80);

    _nes.write(_state.target_address, _state.register_m);
}

void cynes::CPU::op_dex()
{
    _state.register_x--;

    set_status(Flag::Z, !_state.register_x);
    set_status(Flag::N, _state.register_x & 0x80);
}

void cynes::CPU::op_dey()
{
    _state.register_y--;

    set_status(Flag::Z, !_state.register_y);
    set_status(Flag::N, _state.register_y & 0x80);
}

void cynes::CPU::op_eor()
{
    _state.register_a ^= _state.register_m;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}

void cynes::CPU::op_inc()
{
    _nes.write(_state.target_address, _state.register_m);

    _state.register_m++;

    set_status(Flag::Z, !_state.register_m);
    set_status(Flag::N, _state.register_m & 0x80);

    _nes.write(_state.target_address, _state.register_m);
}

void cynes::CPU::op_inx()
{
    _state.register_x++;

    set_status(Flag::Z, !_state.register_x);
    set_status(Flag::N, _state.register_x & 0x80);
}

void cynes::CPU::op_iny()
{
    _state.register_y++;

    set_status(Flag::Z, !_state.register_y);
    set_status(Flag::N, _state.register_y & 0x80);
}

void cynes::CPU::op_isc()
{
    _nes.write(_state.target_address, _state.register_m);

    _state.register_m++;

    uint8_t value = _state.register_m;

    _state.register_m ^= 0xFF;

    uint16_t result = _state.register_a + _state.register_m + (get_status(Flag::C) ? 0x01 : 0x00);

    set_status(Flag::C, result & 0x0100);
    set_status(Flag::V, ~(_state.register_a ^ _state.register_m) & (_state.register_a ^ result) & 0x80);

    _state.register_a = result & 0x00FF;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);

    _nes.write(_state.target_address, value);
}

void cynes::CPU::op_jam()
{
    _state.frozen = true;
}

void cynes::CPU::op_jmp()
{
    _state.program_counter = _state.target_address;
}

void cynes::CPU::op_jsr()
{
    _nes.read(_state.program_counter);

    _state.program_counter--;

    _nes.write(0x100 | _state.stack_pointer--, _state.program_counter >> 8);
    _nes.write(0x100 | _state.stack_pointer--, _state.program_counter & 0x00FF);

    _state.program_counter = _state.target_address;
}

void cynes::CPU::op_lar()
{
    set_status(Flag::C, _state.register_a & 0x01);

    _state.register_a >>= 1;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}

void cynes::CPU::op_las()
{
    uint8_t result = _state.register_m & _state.stack_pointer;

    _state.register_a = result;
    _state.register_x = result;
    _state.stack_pointer = result;
}

void cynes::CPU::op_lax()
{
    _state.register_a = _state.register_m;
    _state.register_x = _state.register_m;

    set_status(Flag::Z, !_state.register_m);
    set_status(Flag::N, _state.register_m & 0x80);
}

void cynes::CPU::op_lda()
{
    _state.register_a = _state.register_m;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}

void cynes::CPU::op_ldx()
{
    _state.register_x = _state.register_m;

    set_status(Flag::Z, !_state.register_x);
    set_status(Flag::N, _state.register_x & 0x80);
}

void cynes::CPU::op_ldy()
{
    _state.register_y = _state.register_m;

    set_status(Flag::Z, !_state.register_y);
    set_status(Flag::N, _state.register_y & 0x80);
}

void cynes::CPU::op_lsr()
{
    _nes.write(_state.target_address, _state.register_m);

    set_status(Flag::C, _state.register_m & 0x01);

    _state.register_m >>= 1;

    set_status(Flag::Z, !_state.register_m);
    set_status(Flag::N, _state.register_m & 0x80);

    _nes.write(_state.target_address, _state.register_m);
}

void cynes::CPU::op_lxa()
{
    _state.register_a = _state.register_m;
    _state.register_x = _state.register_m;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}

void cynes::CPU::op_nop() {}

void cynes::CPU::op_ora()
{
    _state.register_a |= _state.register_m;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}

void cynes::CPU::op_pha()
{
    _nes.write(0x100 | _state.stack_pointer--, _state.register_a);
}

void cynes::CPU::op_php()
{
    _nes.write(0x100 | _state.stack_pointer--, _state.status | Flag::B | Flag::U);
}

void cynes::CPU::op_pla()
{
    _state.stack_pointer++;
    _nes.read(_state.program_counter);
    _state.register_a = _nes.read(0x100 | _state.stack_pointer);

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}

void cynes::CPU::op_plp()
{
    _state.stack_pointer++;
    _nes.read(_state.program_counter);
    _state.status = _nes.read(0x100 | _state.stack_pointer) & 0xCF;
}

void cynes::CPU::op_ral()
{
    bool carry = _state.register_a & 0x80;

    _state.register_a = (get_status(Flag::C) ? 0x01 : 0x00) | (_state.register_a << 1);

    set_status(Flag::C, carry);
    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}

void cynes::CPU::op_rar()
{
    bool carry = _state.register_a & 0x01;

    _state.register_a = (get_status(Flag::C) ? 0x80 : 0x00) | (_state.register_a >> 1);

    set_status(Flag::C, carry);
    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}

void cynes::CPU::op_rla()
{
    _nes.write(_state.target_address, _state.register_m);

    bool carry = _state.register_m & 0x80;

    _state.register_m = (get_status(Flag::C) ? 0x01 : 0x00) | (_state.register_m << 1);
    _state.register_a &= _state.register_m;

    set_status(Flag::C, carry);
    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);

    _nes.write(_state.target_address, _state.register_m);
}

void cynes::CPU::op_rol()
{
    _nes.write(_state.target_address, _state.register_m);

    bool carry = _state.register_m & 0x80;

    _state.register_m = (get_status(Flag::C) ? 0x01 : 0x00) | (_state.register_m << 1);

    set_status(Flag::C, carry);
    set_status(Flag::Z, !_state.register_m);
    set_status(Flag::N, _state.register_m & 0x80);

    _nes.write(_state.target_address, _state.register_m);
}

void cynes::CPU::op_ror()
{
    _nes.write(_state.target_address, _state.register_m);

    bool carry = _state.register_m & 0x01;

    _state.register_m = (get_status(Flag::C) ? 0x80 : 0x00) | (_state.register_m >> 1);

    set_status(Flag::C, carry);
    set_status(Flag::Z, !_state.register_m);
    set_status(Flag::N, _state.register_m & 0x80);

    _nes.write(_state.target_address, _state.register_m);
}

void cynes::CPU::op_rra()
{
    _nes.write(_state.target_address, _state.register_m);

    uint8_t carry = _state.register_m & 0x01;

    _state.register_m = (get_status(Flag::C) ? 0x80 : 0x00) | (_state.register_m >> 1);

    uint16_t result = _state.register_a + _state.register_m + carry;

    set_status(Flag::C, result & 0x0100);
    set_status(Flag::V, ~(_state.register_a ^ _state.register_m) & (_state.register_a ^ result) & 0x80);

    _state.register_a = result & 0x00FF;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);

    _nes.write(_state.target_address, _state.register_m);
}

void cynes::CPU::op_rti()
{
    _state.stack_pointer++;
    _nes.read(_state.program_counter);
    _state.status = _nes.read(0x100 | _state.stack_pointer) & 0xCF;
    _state.program_counter = _nes.read(0x100 | ++_state.stack_pointer);
    _state.program_counter |= _nes.read(0x100 | ++_state.stack_pointer) << 8;
}

void cynes::CPU::op_rts()
{
    _state.stack_pointer++;

    _nes.read(_state.program_counter);
    _nes.read(_state.program_counter);

    _state.program_counter = _nes.read(0x100 | _state.stack_pointer);
    _state.program_counter |= _nes.read(0x100 | ++_state.stack_pointer) << 8;
    _state.program_counter++;
}

void cynes::CPU::op_sax()
{
    _nes.write(_state.target_address, _state.register_a & _state.register_x);
}

void cynes::CPU::op_sbc()
{
    _state.register_m ^= 0xFF;

    uint16_t result = _state.register_a + _state.register_m + (get_status(Flag::C) ? 0x01 : 0x00);

    set_status(Flag::C, result & 0xFF00);
    set_status(Flag::V, ~(_state.register_a ^ _state.register_m) & (_state.register_a ^ result) & 0x80);

    _state.register_a = result & 0x00FF;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}

void cynes::CPU::op_sbx()
{
    _state.register_x &= _state.register_a;

    set_status(Flag::C, _state.register_x >= _state.register_m);
    set_status(Flag::Z, _state.register_x == _state.register_m);

    _state.register_x -= _state.register_m;

    set_status(Flag::N, _state.register_x & 0x80);
}

void cynes::CPU::op_sec()
{
    set_status(Flag::C, true);
}

void cynes::CPU::op_sed()
{
    set_status(Flag::D, true);
}

void cynes::CPU::op_sei()
{
    set_status(Flag::I, true);
}

void cynes::CPU::op_sha()
{
    _nes.write(_state.target_address, _state.register_a & _state.register_x & (uint8_t(_state.target_address >> 8) + 1));
}

void cynes::CPU::op_shx()
{
    uint8_t address_high = 1 + (_state.target_address >> 8);

    _nes.write(((_state.register_x & address_high) << 8) | (_state.target_address & 0xFF), _state.register_x & address_high);
}

void cynes::CPU::op_shy()
{
    uint8_t address_high = 1 + (_state.target_address >> 8);

    _nes.write(((_state.register_y & address_high) << 8) | (_state.target_address & 0xFF), _state.register_y & address_high);
}

void cynes::CPU::op_slo()
{
    _nes.write(_state.target_address, _state.register_m);

    set_status(Flag::C, _state.register_m & 0x80);

    _state.register_m <<= 1;
    _state.register_a |= _state.register_m;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);

    _nes.write(_state.target_address, _state.register_m);
}

void cynes::CPU::op_sre()
{
    _nes.write(_state.target_address, _state.register_m);

    set_status(Flag::C, _state.register_m & 0x01);

    _state.register_m >>= 1;
    _state.register_a ^= _state.register_m;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);

    _nes.write(_state.target_address, _state.register_m);
}

void cynes::CPU::op_sta()
{
    _nes.write(_state.target_address, _state.register_a);
}

void cynes::CPU::op_stx()
{
    _nes.write(_state.target_address, _state.register_x);
}

void cynes::CPU::op_sty()
{
    _nes.write(_state.target_address, _state.register_y);
}

void cynes::CPU::op_tas()
{
    _state.stack_pointer = _state.register_a & _state.register_x;

    _nes.write(_state.target_address, _state.stack_pointer & (uint8_t(_state.target_address >> 8) + 1));
}

void cynes::CPU::op_tax()
{
    _state.register_x = _state.register_a;

    set_status(Flag::Z, !_state.register_x);
    set_status(Flag::N, _state.register_x & 0x80);
}

void cynes::CPU::op_tay()
{
    _state.register_y = _state.register_a;

    set_status(Flag::Z, !_state.register_y);
    set_status(Flag::N, _state.register_y & 0x80);
}

void cynes::CPU::op_tsx()
{
    _state.register_x = _state.stack_pointer;

    set_status(Flag::Z, !_state.register_x);
    set_status(Flag::N, _state.register_x & 0x80);
}

void cynes::CPU::op_txa()
{
    _state.register_a = _state.register_x;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}

void cynes::CPU::op_txs()
{
    _state.stack_pointer = _state.register_x;
}

void cynes::CPU::op_tya()
{
    _state.register_a = _state.register_y;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}

void cynes::CPU::op_usb()
{
    _state.register_m ^= 0xFF;

    uint16_t result = _state.register_a + _state.register_m + (get_status(Flag::C) ? 0x01 : 0x00);

    set_status(Flag::C, result & 0x0100);
    set_status(Flag::V, ~(_state.register_a ^ _state.register_m) & (_state.register_a ^ result) & 0x80);

    _state.register_a = result & 0x00FF;

    set_status(Flag::Z, !_state.register_a);
    set_status(Flag::N, _state.register_a & 0x80);
}
