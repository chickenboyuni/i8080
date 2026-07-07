#include<iostream>
#include<iomanip>

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

  m_pc = 0;

  m_rgs = (const Registers){ };
  m_rps = (const RegisterPairs){ };

  m_bus->reset();
}

uint8_t CPU::fetch_next_word(){
  return m_bus->memory_read(m_pc++);
}

void CPU::fetch_execute_instruction() {
  uint8_t ins = fetch_next_word();

  switch(ins) {
    case 0x00: case 0x08: case 0x10: case 0x18: case 0x20: case 0x28: case 0x30:
    case 0x38: case 0xcb: case 0xd9: case 0xdd: case 0xed: case 0xfd:
       break; // nop - 00000000 or undefined behaviour so will be left unimplemented
    case 0x01: case 0x11: case 0x21: case 0x31:
      lxi(INS_EXTRACT_REGISTERPAIR(ins)); break; // lxi rp, d16 - 00rp0001 d8 d8
    case 0x02: case 0x12:
      stax(INS_EXTRACT_REGISTERPAIR(ins)); break; // stax rp - 00rp0010
    case 0x06: case 0x0e: case 0x16: case 0x1e: case 0x26: case 0x2e: case 0x36: case 0x3e: //0x36 is unhandled MVI M, D8
      mvi(INS_EXTRACT_REGISTER(ins)); break; // mvi r, d8 - 00ddd110 d8
    case 0x3a:
      lda(); break; // lda addr - 00111010 laddr haddr
    default:
      std::stringstream ss;
      ss << "unimplemented instruction \e[1m0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(ins) 
         << "\e[0m at address \e[1m0x" << std::setfill('0') << std::setw(4) << m_pc-1;
      PANIC(ss.str());
  }
}

void CPU::set_register_pair(uint8_t rp, uint8_t bl, uint8_t bh){
  uint16_t data = (bh << 8) | bl;
  *(m_rps_map.at(rp)) = data;
}

uint16_t CPU::get_register_pair(uint8_t rp){
  return *(m_rps_map.at(rp));
}

void CPU::set_register(uint8_t rg, uint8_t data) {
  *(m_rgs_map.at(rg)) = data;
}

uint8_t CPU::get_register(uint8_t rg) {
  return *(m_rgs_map.at(rg));
}

CpuState CPU::get_cpu_state() {
  CpuState cpu_state{};
  cpu_state.pc = m_pc;
  cpu_state.rgs = m_rgs;
  cpu_state.rps = m_rps;
  return cpu_state;
}

void CPU::lxi(uint8_t rp){
  uint8_t d1 = fetch_next_word();
  uint8_t d2 = fetch_next_word();
  set_register_pair(rp, d1, d2);
}

void CPU::stax(uint8_t rp){
  m_bus->memory_write(get_register_pair(rp), m_rgs.a);
}

void CPU::mvi(uint8_t rg){
  uint8_t d = fetch_next_word();
  set_register(rg, d);
}

void CPU::lda(){
  uint8_t d1 = fetch_next_word();
  uint8_t d2 = fetch_next_word();
  m_rgs.a = m_bus->memory_read(INS_MAKE_ADDRESS(d1, d2)); 
}

