#ifndef SPACE_INVADERS_H
#define SPACE_INVADERS_H

#include<memory>
#include<cstring>

#include "../bus.h"
#include "shift_register.h"

#define MAX_MEMORY 0x10000 // 64 KiB
#define INVADERS_ROM_SIZE 1024 * 8 // 8 KiB
#define INVADERS_RAM_SIZE 1024 * 8 // 1 KiB RAM and 7 KiB VRAM

/* Interface for anythting that needs access to the memory state */
typedef struct MemoryState {
  uint8_t* rom_state; 
  uint8_t* ram_state;
} MemoryState;

class InvadersBus : public Bus {

  enum IPorts : uint8_t {
    IPORT_INP0 = 0,
    IPORT_INP1,
    IPORT_INP2,
    IPORT_SHFT_IN
  };

  enum OPorts : uint8_t {
    OPORT_SHFT_AMNT = 2,
    OPORT_SOUND1,
    OPORT_SHFT_DATA,
    OPORT_SOUND2,
    OPORT_WATCHDOG
  };

public:

  InvadersBus(ShiftRegister* shift_register = nullptr);
  ~InvadersBus() = default;

  uint8_t memory_read(uint16_t addr) override;
  void memory_write(uint16_t addr, uint8_t data) override;

  uint8_t io_read(uint8_t port) override;
  void io_write(uint8_t port, uint8_t data) override;

  void reset() override;
  void load_rom(uint8_t* rom_data, size_t rom_size);

  MemoryState get_memory_state();

private: 

  uint8_t m_rom[INVADERS_ROM_SIZE] {}; 
  uint8_t m_ram[INVADERS_RAM_SIZE] {};

  ShiftRegister* m_shift_register = nullptr;

  MemoryState m_memory_state {};
};

#endif /* SPACE_INVADERS_H */

