#include <cassert>

#include "invaders.h"

InvadersBus::InvadersBus() = default;
InvadersBus::~InvadersBus() = default;

uint8_t InvadersBus::memory_read(uint16_t addr) {

  uint16_t msn = addr >> 12; // most significant nibble

  // TEMPORARY: just want a temporary assert till i figure out a proper logging system
  assert(msn < MAX_MEMORY && "hey, the program is accessing memory out of bounds >:(");

  /* 0000-1fff 8K ROM */
  if(msn < 0x2){
    return m_rom[addr];
  }

  /*
   2000-23ff 1K RAM
   2400-3fff 7K vRAM 
  */
  if(msn >= 0x2 && msn < 0x4){ 
    return m_ram[addr];
  }

  /* 4000- RAM mirror */
  if(msn >= 0x4 && msn < 0x6){
    return m_ram[addr-0x2000];
  }

  return 0x0;
}

void InvadersBus::load_rom(uint8_t* rom_data, size_t rom_size){
  memcpy(m_rom, rom_data, rom_size);
}
