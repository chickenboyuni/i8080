#ifndef SPACE_INVADERS_H
#define SPACE_INVADERS_H

#include<memory>
#include<cstring>

#include "../bus.h"

/*
ROM LAYOUT
invaders.h 0000-07FF
invaders.g 0800-0FFF
invaders.f 1000-17FF
invaders.e 1800-1FFF
*/

#define MAX_MEMORY 0x10000 // 64 KiB
#define INVADERS_ROM_SIZE 1024 * 8 // 2 KiB

#define INVADERS_H_START_ADDRESS 0x0000
#define INVADERS_G_START_ADDRESS 0x0800
#define INVADERS_F_START_ADDRESS 0x1000
#define INVADERS_E_START_ADDRESS 0x1800

class InvadersBus : public Bus {

public:

  InvadersBus();
  ~InvadersBus();
  
  uint8_t memory_read(uint16_t addr) override;
  void memory_write(uint16_t addr, uint8_t data) override;

  uint8_t io_read(uint16_t addr) override {}
  void io_write(uint16_t addr, uint8_t data) override {}

  void load_rom(uint8_t* rom_data, size_t rom_size);

private: 

  uint8_t m_rom[INVADERS_ROM_SIZE] {}; 
  uint8_t m_ram[];
};

#endif /* SPACE_INVADERS_H */

