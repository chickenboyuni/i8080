#ifndef CPU_H
#define CPU_H

#include<cstdint>
#include<memory>

#include "bus.h"

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

  void fetch_execute_instruction();
  // TEMPORARY: just for now so i can move forward, will change later
  uint16_t get_pc() { return m_pc; };

private:

  std::unique_ptr<Bus> m_bus;

  uint16_t m_pc{};
  uint16_t m_sp{};

  Regs m_rgs{};
  RegPairs m_rps{};
  
};

#endif /* CPU_H */
