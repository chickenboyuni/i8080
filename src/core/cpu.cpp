#include<iostream>
#include<iomanip>
#include<cstring>

#include "cpu.h"
#include "../common/logging.h"

CPU::CPU(std::unique_ptr<Bus>&& bus) : m_bus(std::move(bus)) {

}

CPU::~CPU() = default;

bool CPU::running() {
  return m_running;
}

void CPU::reset() {
  m_running = true;

  m_pc = 0x0;
  m_sp = 0x0;
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
                    .psw = {}, // leave as nothing for now
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

    default:
      std::stringstream ss;
      ss << "unimplemented instruction \e[1m0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(ins) 
         << "\e[0m at address \e[1m0x" << std::setfill('0') << std::setw(4) << m_pc-1;
      PANIC(ss.str());
  }
}

void CPU::set_register_pair(uint8_t rp, uint8_t bl, uint8_t bh){
  unsigned int first_register_idx = m_regpairs_map.at(rp);
  m_regs[first_register_idx] = bh;
  m_regs[first_register_idx+1] = bl;
}

uint16_t CPU::get_register_pair(uint8_t rp){
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

