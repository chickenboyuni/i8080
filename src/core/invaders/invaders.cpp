#include<iomanip>

#include "../../common/logging.h"
#include "invaders.h"

constexpr unsigned int ram_offset = INVADERS_ROM_SIZE;
constexpr unsigned int ram_mirror_offset = INVADERS_ROM_SIZE + INVADERS_RAM_SIZE;

InvadersBus::InvadersBus(ShiftRegister* shift_register) : m_shift_register(shift_register) {

}

MemoryState InvadersBus::get_memory_state() {
  m_memory_state.rom_state = m_rom;
  m_memory_state.ram_state = m_ram;
  return m_memory_state;
}

void InvadersBus::reset() {
  memset(m_ram, 0, sizeof(m_ram));
}

// TODO: Improve log messages to include pc and instruction where error occured
uint8_t InvadersBus::memory_read(uint16_t addr) {

  uint16_t msn = addr >> 12; // most significant nibble

  if(msn >= 0x6){
    LOG_ERROR("the program is trying to access memory out of the memory map") << "\e[1m`0x" << std::setfill('0') << std::setw(4) << std::hex << addr << "`";
    return 0x0;
  }

  /* 0000-1fff 8K ROM */
  if(msn < 0x2){
    return m_rom[addr];
  }

  /*
   2000-23ff 1K RAM
   2400-3fff 7K vRAM 
  */
  if(msn >= 0x2 && msn < 0x4){ 
    return m_ram[addr-ram_offset];
  }

  /* 4000- RAM mirror */
  if(msn >= 0x4 && msn < 0x6){
    return m_ram[addr-ram_mirror_offset];
  }

  return 0x0;
}

void InvadersBus::memory_write(uint16_t addr, uint8_t data) {

  uint16_t msn = addr >> 12; // most significant nibble

  if(msn >= 0x6){
    LOG_ERROR("the program is trying to write to memory out of the memory map") << "\e[1m`0x" << std::setfill('0') << std::setw(4) << std::hex << addr << "`";
    return;
  }

  /* 0000-1fff 8K ROM */
  if(msn < 0x2){
    LOG_ERROR("the program is trying to write to read-only memory") << "\e[1m`0x" << std::setfill('0') << std::setw(4) << std::hex << addr << "`";
    return;
  }

  /*
   2000-23ff 1K RAM
   2400-3fff 7K vRAM 
  */ 
  if(msn >= 0x2 && msn < 0x4){ 
    m_ram[addr-ram_offset] = data;
  }

  /* 4000- RAM mirror */
  if(msn >= 0x4 && msn < 0x6){
    m_ram[addr-ram_mirror_offset] = data;
  }
}

uint8_t InvadersBus::io_read(uint8_t port) {

  switch(port) {
    case IPORT_INP0:
      break;
    case IPORT_INP1:
      break;
    case IPORT_INP2:
      break;
    case IPORT_SHFT_IN:
      if(m_shift_register) {
        return m_shift_register->get_shift_result_r();
      }
      LOG_ERROR("program trying to read from the shift register when it is not present");
      break;
  }

  return 0;
}

void InvadersBus::io_write(uint8_t port, uint8_t data) {

  switch(port) {
    case OPORT_SHFT_AMNT:
      if(m_shift_register) {
        m_shift_register->set_shift_offset_w(data);
        return;
      }
      LOG_ERROR("program trying to write to the shift register when it is not present");
      break;
    case OPORT_SOUND1:
      break;
    case OPORT_SHFT_DATA:
      if(m_shift_register) {
        m_shift_register->shift_data_w(data);
        return;
      }
      LOG_ERROR("program trying to write to the shift register when it is not present");
      break;
    case OPORT_SOUND2:
      break;
    case OPORT_WATCHDOG:
      break;
  }

}

void InvadersBus::load_rom(uint8_t* rom_data, size_t rom_size){
  memcpy(m_rom, rom_data, rom_size);
}
