#include<iostream>
#include<iomanip>
#include<cstring>

#include "cpu.h"
#include "../common/logging.h"

CPU::CPU(std::unique_ptr<Bus>&& bus) : m_bus(std::move(bus)) {
  reset();
}

CPU::~CPU() = default;

bool CPU::running() {
  return m_running;
}

void CPU::reset() {
  m_running = true;

  m_pc = 0x00;
  m_sp = 0x00;
  m_psw = 0x0;

  memset(m_regs, 0x0, REGISTER_COUNT);

  m_bus->reset();
}

CpuState CPU::get_cpu_state() {
  CpuState cpu_state {};
  cpu_state.pc = m_pc;

  cpu_state.rgs = { .a = m_regs[REGISTER_A],
                    .b = m_regs[REGISTER_B],
                    .c = m_regs[REGISTER_C],
                    .d = m_regs[REGISTER_D],
                    .e = m_regs[REGISTER_E],
                    .h = m_regs[REGISTER_H],
                    .l = m_regs[REGISTER_L] };

  cpu_state.rps = { .bc = get_register_pair_from_idx(REGISTER_PAIR_BC),
                    .de = get_register_pair_from_idx(REGISTER_PAIR_DE),
                    .hl = get_register_pair_from_idx(REGISTER_PAIR_HL),
                    .sp = m_sp,
                    .psw = { .z  = get_status_flag(FLAG_Z),
                             .s  = get_status_flag(FLAG_S),
                             .p  = get_status_flag(FLAG_P),
                             .cy = get_status_flag(FLAG_CY),
                             .ac = get_status_flag(FLAG_AC) }
  };
  return cpu_state;
}

Bus* CPU::get_bus() {
  return m_bus.get();
}

uint8_t CPU::fetch_byte(){
  return m_bus->memory_read(m_pc++);
}
uint16_t CPU::fetch_2bytes() {
  uint8_t low_order_byte = fetch_byte();
  uint8_t high_order_byte = fetch_byte();

  return (high_order_byte << 8) | low_order_byte; 
}

void CPU::fetch_execute_instruction() {
  uint8_t ins = fetch_byte();

  switch(ins) {
    case 0x00: case 0x08: case 0x10: case 0x18: case 0x20: case 0x28: case 0x30:
    case 0x38: case 0xcb: case 0xd9: case 0xdd: case 0xed: case 0xfd:
       break; // nop - 00000000 or undefined behaviour so will be left unimplemented

    case 0x22:
      shld(); break; // shld addr - 00100010 laddr haddr
    case 0x2a:
      lhld(); break; // lhld addr - 00101010 laddr haddr

    case 0xeb:
      xchg(); break; // xchg - 11101011

    case 0x32:
      sta(); break; // sta addr - 00110010 laddr haddr
    case 0x02: case 0x12:
      stax(INS_EXTRACT_REGISTERPAIR(ins)); break; // stax rp - 00rp0010

    case 0x06: case 0x0e: case 0x16: case 0x1e: case 0x26: case 0x2e: case 0x3e:
      mvi_r(INS_EXTRACT_DDD_REGISTER(ins)); break; // mvi r, d8 - 00ddd110 d8
    case 0x36:
      mvi_m(); break; // mvi M, d8 - 00110110 d8

    case 0x01: case 0x11: case 0x21: case 0x31:
      lxi(INS_EXTRACT_REGISTERPAIR(ins)); break; // lxi rp, d16 - 00rp0001 d8 d8
    case 0x3a:
      lda(); break; // lda addr - 00111010 laddr haddr
    case 0x0a: case 0x1a:
      ldax(INS_EXTRACT_REGISTERPAIR(ins)); break; // ldax rp - 00rp1010

    case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x47:
    case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4f:
    case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x57:
    case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5f:
    case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x67:
    case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6f:
    case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7f:
      mov_rr(INS_EXTRACT_DDD_REGISTER(ins), INS_EXTRACT_SSS_REGISTER(ins)); break; // mov r1, r2 - 01dddsss
    case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
      mov_mr(INS_EXTRACT_SSS_REGISTER(ins)); break; // mov M, r - 01110sss
    case 0x46: case 0x4e: case 0x56: case 0x5e: case 0x66: case 0x6e: case 0x7e:
      mov_rm(INS_EXTRACT_DDD_REGISTER(ins)); break; // mov r, M - 01ddd110

    // add without carry
    case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x87:
      add(INS_EXTRACT_SSS_REGISTER(ins), ArithmeticMode::Register, false); break; // add r - 10000sss 
    case 0x86:
      add(INS_EXTRACT_SSS_REGISTER(ins), ArithmeticMode::Memory, false); break; // add m - 10000110
    case 0xc6:
      add(INS_EXTRACT_SSS_REGISTER(ins), ArithmeticMode::Immediate, false); break; // adi d8 - 11000110 d8
    // add with carry
    case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8f:
      add(INS_EXTRACT_SSS_REGISTER(ins), ArithmeticMode::Register, true); break; // adc r - 10001sss 
    case 0x8e: 
      add(INS_EXTRACT_SSS_REGISTER(ins), ArithmeticMode::Memory, true); break; // adc m - 10001110
    case 0xce:
      add(INS_EXTRACT_SSS_REGISTER(ins), ArithmeticMode::Immediate, true); break; // aci d8 - 11001110 d8

    // sub without borrow 
    case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x97:
      sub(INS_EXTRACT_SSS_REGISTER(ins), ArithmeticMode::Register, false); break; // sub r - 10010sss
    case 0x96:
      sub(INS_EXTRACT_SSS_REGISTER(ins), ArithmeticMode::Memory, false); break; // sub m - 10010110
    case 0xd6:
      sub(INS_EXTRACT_SSS_REGISTER(ins), ArithmeticMode::Immediate, false); break; // sui data - 11010110 d8
    // sub with borrow
    case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9f:
      sub(INS_EXTRACT_SSS_REGISTER(ins), ArithmeticMode::Register, true); break; // sbb r - 10011sss
    case 0x9e: 
      sub(INS_EXTRACT_SSS_REGISTER(ins), ArithmeticMode::Memory, true); break; // sbb m - 10011110
    case 0xde:
      sub(INS_EXTRACT_SSS_REGISTER(ins), ArithmeticMode::Immediate, true); break; // sbi d8 - 11011110 d8

    case 0x04: case 0x0c: case 0x14: case 0x1c: case 0x24: case 0x2c: case 0x3c:
      inr_r(INS_EXTRACT_DDD_REGISTER(ins), false); break; // inr r - 00ddd100
    case 0x34: 
      inr_m(false); break; // inr m - 00110100
    case 0x05: case 0x0d: case 0x15: case 0x1d: case 0x25: case 0x2d: case 0x3d:
      inr_r(INS_EXTRACT_DDD_REGISTER(ins), true); break; // dcr r - 00ddd101
    case 0x35: 
      inr_m(true); break; // dcr m - 00110101
    case 0x03: case 0x13: case 0x23: case 0x33:
      inx(INS_EXTRACT_REGISTERPAIR(ins), false); break; // inx rp - 00rp0011
    case 0x0b: case 0x1b: case 0x2b: case 0x3b:
      inx(INS_EXTRACT_REGISTERPAIR(ins), true); break; // dcx rp - 00rp1011

    case 0x09: case 0x19: case 0x29: case 0x39:
      dad(INS_EXTRACT_REGISTERPAIR(ins)); break; // dad rp - 00rp1001
    case 0x27:
      daa(); break; // daa - 00100111

    case 0xc3:
      jmp(); break; // jmp addr - 11000011 laddr haddr
    case 0xc2: case 0xca: case 0xd2: case 0xda: case 0xe2: case 0xea: case 0xf2: case 0xfa:
      jmp_conditional(INS_EXTRACT_CONDITION(ins)); break; // jcondition addr - 11ccc010 laddr haddr

    case 0xcd:
      call(); break; // call addr - 11001101 laddr haddr
    case 0xc4: case 0xcc: case 0xd4: case 0xdc: case 0xe4: case 0xec: case 0xf4: case 0xfc: case 0xfe:
      call_conditional(INS_EXTRACT_CONDITION(ins)); break; // cconditon addr - 11ccc100 laddr haddr

    case 0xc9:
      ret(); break; // ret - 11001001
    case 0xc0: case 0xc8: case 0xd0: case 0xd8: case 0xe0: case 0xe8: case 0xf0: case 0xf8:
      ret_conditional(INS_EXTRACT_CONDITION(ins)); break; // rcondition - 11ccc000

    case 0xc7: case 0xcf: case 0xd7: case 0xdf: case 0xe7: case 0xef: case 0xf7: case 0xff:
      rst((ins & 0b00111000) >> 3); break; // rst n - 11nnn111

    case 0xe9:
      pchl(); break; // pchl - 11101001

    default:
      std::stringstream ss;
      ss << "unimplemented instruction \e[1m0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(ins) 
         << "\e[0m at address \e[1m0x" << std::setfill('0') << std::setw(4) << m_pc-1;
      PANIC(ss.str());
  }
}

void CPU::set_register_pair(uint8_t rp, uint8_t bl, uint8_t bh){
  if(rp == 0b11){
    m_sp = (bh << 8) | bl;
    return;
  }
  unsigned int first_register_idx = m_regpairs_map.at(rp);
  m_regs[first_register_idx] = bh;
  m_regs[first_register_idx+1] = bl;
}

uint16_t CPU::get_register_pair(uint8_t rp){
  if(rp == 0b11){
    return m_sp;
  }
  unsigned int register_pair_idx = m_regpairs_map.at(rp);
  return get_register_pair_from_idx(register_pair_idx);
}

void CPU::set_register(uint8_t rg, uint8_t data) {
  unsigned int register_idx = m_regs_map.at(rg);
  m_regs[register_idx] = data;
}

uint8_t CPU::get_register(uint8_t rg) {
  unsigned int register_idx = m_regs_map.at(rg);
  return m_regs[register_idx];
}

uint16_t CPU::get_register_pair_from_idx(unsigned int register_pair_idx) {
  return (m_regs[register_pair_idx] << 8) | m_regs[register_pair_idx+1];
}

uint8_t condition_is_met(bool condition) {
  if(condition) {
    return 0b11111111;
  } 
  return 0;
}

#define X(flag, name) void CPU::update_##name##_flag(bool is_##name){ \
                    m_psw &= FLAG_##flag ^ 0b11111111; \
                    m_psw |= FLAG_##flag & condition_is_met(is_##name); \
                  }
LIST_OF_FLAGS
#undef X

bool CPU::get_status_flag(uint8_t flag_type) {
  return !!(m_psw & flag_type);
}

bool CPU::branch_condition_is_met(uint8_t condition) {
  switch(condition) {
    case 0b000: return !get_status_flag(FLAG_Z);
    case 0b001: return get_status_flag(FLAG_Z);

    case 0b010: return !get_status_flag(FLAG_CY);
    case 0b011: return get_status_flag(FLAG_CY);

    case 0b100: return !get_status_flag(FLAG_P);
    case 0b101: return get_status_flag(FLAG_P);

    case 0b110: return !get_status_flag(FLAG_S);
    case 0b111: return get_status_flag(FLAG_S);
  }

  return false;
}

void CPU::jmp() {
  uint16_t addr = fetch_2bytes();
  m_pc = addr;
}

void CPU::jmp_conditional(uint8_t condition_bit_pattern) {
  uint16_t addr = fetch_2bytes();
  if(branch_condition_is_met(condition_bit_pattern)) {
    m_pc = addr;
  }
}

void CPU::call() {
  uint16_t addr = fetch_2bytes();
  m_bus->memory_write(m_sp-1, m_pc >> 8);
  m_bus->memory_write(m_sp-2, m_pc & 0x0f);
  m_sp -= 2;
  m_pc = addr;
}

void CPU::call_conditional(uint8_t condition_bit_pattern) {
  uint16_t addr = fetch_2bytes();
  if(branch_condition_is_met(condition_bit_pattern)) {
    m_bus->memory_write(m_sp-1, m_pc >> 8);
    m_bus->memory_write(m_sp-2, m_pc & 0x0f);
    m_sp -= 2;
    m_pc = addr;
  }
}

void CPU::ret() {
  m_pc = (m_bus->memory_read(m_sp+1) << 8) | m_bus->memory_read(m_sp);
  m_sp += 2;
}

void CPU::ret_conditional(uint8_t condition_bit_pattern) {
  if(branch_condition_is_met(condition_bit_pattern)) {
    m_pc = (m_bus->memory_read(m_sp+1) << 8) | m_bus->memory_read(m_sp);
    m_sp += 2;
  }
}

void CPU::rst(uint8_t number) {
  m_bus->memory_write(m_sp-1, m_pc >> 8);
  m_bus->memory_write(m_sp-2, m_pc & 0x0f);
  m_sp -= 2;
  m_pc = 8 * number;
}

void CPU::pchl() {
  m_pc = get_register_pair_from_idx(REGISTER_PAIR_HL);
}

uint8_t binary_add(uint8_t a, uint8_t b, unsigned int& carry, unsigned int& aux_carry) {
  uint8_t res = 0;
  for(size_t i = 0; i < 8 ; i++){
    uint8_t bit_a = a & (1 << i);
    uint8_t bit_b = b & (1 << i);
    res = res | ((bit_a ^ bit_b) ^ carry); 
    carry = ((bit_a & bit_b) | (bit_a & carry) | (bit_b & carry)) << 1; 
    if(i == 3) { aux_carry = !!carry; } // If the instruction caused a carry out of bit 3 and into bit 4, the auxiliary carry is set
  }
  carry = !!carry; // if carry >= 1, let it just be 1
  return res;
}

void CPU::add(uint8_t rg, ArithmeticMode mode, bool with_carry){
  unsigned int carry = 0;
  unsigned int aux_carry = 0;
  uint8_t data = 0;

  switch(mode) {
    case ArithmeticMode::Register:
      data = get_register(rg);
      break;
    case ArithmeticMode::Memory:
      data = m_bus->memory_read(get_register_pair_from_idx(REGISTER_PAIR_HL));
      break;
    case ArithmeticMode::Immediate:
      data = fetch_byte();
      break;
  }

  m_regs[REGISTER_A] = binary_add(m_regs[REGISTER_A], data, carry, aux_carry);
  if(with_carry) {
    m_regs[REGISTER_A] = binary_add(m_regs[REGISTER_A], get_status_flag(FLAG_CY), carry, aux_carry);
  }

  update_zero_flag(m_regs[REGISTER_A] == 0);
  update_sign_flag(m_regs[REGISTER_A] & 0b10000000);
  update_parity_flag((m_regs[REGISTER_A] % 2) == 0);
  update_carry_flag(carry);
  update_aux_carry_flag(aux_carry);
}

void CPU::sub(uint8_t rg, ArithmeticMode mode, bool with_borrow){
  unsigned int carry = 0;
  unsigned int aux_carry = 0;
  uint8_t data = 0;

  switch(mode) {
    case ArithmeticMode::Register:
      data = get_register(rg);
      break;
    case ArithmeticMode::Memory:
      data = m_bus->memory_read(get_register_pair_from_idx(REGISTER_PAIR_HL));
      break;
    case ArithmeticMode::Immediate:
      data = fetch_byte();
      break;
  }
  
  // two's complement
  data = ~data + 1;

  m_regs[REGISTER_A] = binary_add(m_regs[REGISTER_A], data, carry, aux_carry);
  if(with_borrow) {
    m_regs[REGISTER_A] = binary_add(m_regs[REGISTER_A], ~((uint8_t)get_status_flag(FLAG_CY)) + 1, carry, aux_carry);
  }

  update_zero_flag(m_regs[REGISTER_A] == 0);
  update_sign_flag(m_regs[REGISTER_A] & 0b10000000);
  update_parity_flag((m_regs[REGISTER_A] % 2) == 0);
  update_carry_flag(carry);
  update_aux_carry_flag(aux_carry);
}

void CPU::inr_r(uint8_t rg, bool decrement) {
  unsigned int carry = 0;
  unsigned int aux_carry = 0;
  uint8_t increment_value = (decrement) ? 0xff : 0x01;

  set_register(rg, binary_add(get_register(rg), increment_value, carry, aux_carry));

  uint8_t result = get_register(rg);
  update_zero_flag(result == 0);
  update_sign_flag(result & 0b10000000);
  update_parity_flag((result % 2) == 0);
  update_aux_carry_flag(aux_carry);
}

void CPU::inr_m(bool decrement) {
  unsigned int carry = 0;
  unsigned int aux_carry = 0;
  uint8_t increment_value = (decrement) ? 0xff : 0x01;

  m_bus->memory_write(
                      get_register_pair_from_idx(REGISTER_PAIR_HL),
                      binary_add(m_bus->memory_read(get_register_pair_from_idx(REGISTER_PAIR_HL)), increment_value, carry, aux_carry)
                     );

  uint8_t result = m_bus->memory_read(get_register_pair_from_idx(REGISTER_PAIR_HL));
  update_zero_flag(result == 0);
  update_sign_flag(result & 0b10000000);
  update_parity_flag((result % 2) == 0);
  update_aux_carry_flag(aux_carry);
}

void CPU::inx(uint8_t rp, bool decrement) {
  unsigned int carry = 0;
  unsigned int aux_carry = 0;

  uint8_t rl_value = 0;
  uint8_t rh_value = 0;

  if(decrement){
    rl_value = binary_add(get_register_pair(rp), 0xff, carry, aux_carry);
    rh_value = binary_add(get_register_pair(rp) >> 8, 0xff, carry, aux_carry);
  } else {
    rl_value = binary_add(get_register_pair(rp), 1, carry, aux_carry);
    rh_value = binary_add(get_register_pair(rp) >> 8, 0, carry, aux_carry);
  }
  set_register_pair(rp, rl_value, rh_value);
}

void CPU::dad(uint8_t rp) {
  unsigned int carry = 0;
  unsigned int aux_carry = 0;

  uint16_t rp_content = get_register_pair(rp);
  uint8_t rl_value = binary_add(get_register_pair_from_idx(REGISTER_PAIR_HL), rp_content, carry, aux_carry);
  uint8_t rh_value = binary_add(get_register_pair_from_idx(REGISTER_PAIR_HL) >> 8, rp_content >> 8, carry, aux_carry);
  // 0b10 is the bit pattern for register pair HL
  set_register_pair(0b10, rl_value, rh_value);
  update_carry_flag(carry);
}

void CPU::daa() {
  unsigned int carry = 0;
  unsigned int aux_carry = 0;

  if((m_regs[REGISTER_A] & 0x0f) > 0x09 || get_status_flag(FLAG_AC)){
    m_regs[REGISTER_A] = binary_add(m_regs[REGISTER_A], 0x06, carry, aux_carry);
  }
  unsigned int aux_carry_copy = aux_carry;
  carry = 0;
  if((m_regs[REGISTER_A] >> 4) > 0x09 || get_status_flag(FLAG_CY)){
    m_regs[REGISTER_A] = binary_add(m_regs[REGISTER_A], 0x60, carry, aux_carry);
  }
  
  update_zero_flag(m_regs[REGISTER_A] == 0);
  update_sign_flag(m_regs[REGISTER_A] & 0b10000000);
  update_parity_flag((m_regs[REGISTER_A] % 2) == 0);
  update_carry_flag(carry);
  update_aux_carry_flag(aux_carry_copy);
}

void CPU::shld(){
  uint16_t addr = fetch_2bytes();
  m_bus->memory_write(addr, m_regs[REGISTER_L]);
  m_bus->memory_write(addr+1, m_regs[REGISTER_H]);
}
void CPU::lhld(){
  uint16_t addr = fetch_2bytes();
  m_regs[REGISTER_L] = m_bus->memory_read(addr);
  m_regs[REGISTER_H] = m_bus->memory_read(addr+1);
}

void CPU::xchg(){
  uint8_t reg_d_copy = m_regs[REGISTER_D];
  m_regs[REGISTER_D] = m_regs[REGISTER_H];
  m_regs[REGISTER_H] = reg_d_copy;

  uint8_t reg_e_copy = m_regs[REGISTER_E];
  m_regs[REGISTER_E] = m_regs[REGISTER_L];
  m_regs[REGISTER_L] = reg_e_copy;
}

void CPU::lxi(uint8_t rp){
  uint8_t data1 = fetch_byte();
  uint8_t data2 = fetch_byte();
  set_register_pair(rp, data1, data2);
}

void CPU::sta(){
  uint16_t addr = fetch_2bytes();
  m_bus->memory_write(addr, m_regs[REGISTER_A]);
}
void CPU::stax(uint8_t rp){
  m_bus->memory_write(get_register_pair(rp), m_regs[REGISTER_A]);
}

void CPU::mov_rr(uint8_t r1, uint8_t r2) {
  set_register(r1, get_register(r2));
}
void CPU::mov_rm(uint8_t rg) {
  uint8_t data = m_bus->memory_read(get_register_pair_from_idx(REGISTER_PAIR_HL));
  set_register(rg, data);
}
void CPU::mov_mr(uint8_t rg) {
  m_bus->memory_write(get_register_pair_from_idx(REGISTER_PAIR_HL), get_register(rg));
}

void CPU::mvi_r(uint8_t rg){
  uint8_t data = fetch_byte();
  set_register(rg, data);
}
void CPU::mvi_m(){
  uint8_t data = fetch_byte();
  m_bus->memory_write(get_register_pair_from_idx(REGISTER_PAIR_HL), data);
}

void CPU::lda(){
  uint16_t addr = fetch_2bytes();
  m_regs[REGISTER_A] = m_bus->memory_read(addr); 
}
void CPU::ldax(uint8_t rp){
  m_regs[REGISTER_A] = m_bus->memory_read(get_register_pair(rp)); 
}

