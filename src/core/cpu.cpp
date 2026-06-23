#include<iostream>
#include<iomanip>

#include "cpu.h"

CPU::CPU(std::unique_ptr<Bus>&& bus) : m_bus(std::move(bus)) {

}

CPU::~CPU() = default;

void CPU::fetch_execute_instruction() {
  uint8_t ins = m_bus->memory_read(m_pc);
  m_pc++;
  switch(ins) {
    case 0x00: case 0x08: case 0x10: case 0x18: case 0x20: case 0x28: case 0x30:
    case 0x38: case 0xcb: case 0xd9: case 0xdd: case 0xed: case 0xfd:
       break; // nop - 00000000 or undefined behaviour so will be left unimplemented
    default:
       std::cerr << "unimplemented instruction: " << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(ins) << '\n';
  }
}
