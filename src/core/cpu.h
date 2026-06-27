#ifndef CPU_H
#define CPU_H

#include<cstdint>
#include<memory>

#include "../common/utils.h"
#include "bus.h"

#define NUM_OF_INSTRUCTIONS 0x100

#pragma pack(push, 1)
typedef struct FlagBits {
  bool z:1;
  bool s:1;
  bool p:1;
  bool c:1;
  bool xc:1;
} FlagBits;
#pragma pack(pop)

typedef struct Regs {
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t d;
  uint8_t e;
  uint8_t h;
  uint8_t l;
} Regs;

typedef struct RegPairs {
  uint16_t bc;
  uint16_t de;
  uint16_t hl;
  uint16_t sp;

  FlagBits psw;
} RegPairs;

class CPU {
public:
  CPU(std::unique_ptr<Bus>&& bus);
  ~CPU();

  bool running(); 

  uint8_t fetch_next_word();
  void fetch_execute_instruction();

  /**
   * @param  rp:  register pair bit pattern
   * @param  bl:  low-order byte
   * @param  bh:  high-order byte
   */
  void set_register_pair(uint8_t rp, uint8_t bl, uint8_t bh);

  void lxi(uint8_t rp);

private:

  bool m_running{true};

  std::unique_ptr<Bus> m_bus;

  uint16_t m_pc{};
  uint16_t m_sp{};

  Regs m_rgs{};
  RegPairs m_rps{};

};

#endif /* CPU_H */
