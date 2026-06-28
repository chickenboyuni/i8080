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
    default:
      std::stringstream ss;
      ss << "unimplemented instruction \e[1m0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(ins) 
         << "\e[0m at address \e[1m0x" << std::setfill('0') << std::setw(4) << m_pc-1;
      PANIC(ss.str());
  }
}

void CPU::set_register_pair(uint8_t rp, uint8_t bl, uint8_t bh){
  uint16_t data = (bh << 8) | bl;
  switch(rp) {
    case 0b00:
      m_rps.bc = data; break;
    case 0b01:
      m_rps.de = data; break;
    case 0b10:
      m_rps.hl = data; break;
    case 0b11:
      m_rps.sp = data; break;
  }
}

void CPU::lxi(uint8_t rp){
  uint8_t d1 = fetch_next_word();
  uint8_t d2 = fetch_next_word();
  set_register_pair(rp, d1, d2);
}


