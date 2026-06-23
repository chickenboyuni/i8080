#include <cassert>

#include "invaders.h"

InvadersBus::InvadersBus() = default;
InvadersBus::~InvadersBus() = default;

uint8_t InvadersBus::memory_read(uint16_t addr) {

  uint16_t msn = addr >> 12; // most significant nibble

  /* 0000-1fff 8K ROM */
  if(msn >= 0x0 && msn < 0x2){
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
}

void InvadersBus::load_rom(std::unique_ptr<uint8_t[]>& rom_data, size_t rom_size){
  memcpy(m_rom.get(), rom_data.get(), rom_size);
}
