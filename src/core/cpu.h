#ifndef CPU_H
#define CPU_H

#include<cstdint>
#include<memory>
#include<map>

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

typedef struct Registers {
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t d;
  uint8_t e;
  uint8_t h;
  uint8_t l;
} Registers;

typedef struct RegisterPairs {
  uint16_t bc;
  uint16_t de;
  uint16_t hl;
  uint16_t sp;

  FlagBits psw;
} RegisterPairs;

typedef struct CpuState {
  uint16_t pc;

  Registers rgs;
  RegisterPairs rps;

} CpuState;

class CPU {
public:
  CPU(std::unique_ptr<Bus>&& bus);
  ~CPU();

  bool running(); 

  void reset();

  CpuState get_cpu_state();
  Bus* get_bus();

  uint8_t fetch_next_word();
  void fetch_execute_instruction();

  /**
   * @param  rp:  register pair bit pattern
   * @param  bl:  low-order byte
   * @param  bh:  high-order byte
   */
  void set_register_pair(uint8_t rp, uint8_t bl, uint8_t bh);
  uint16_t get_register_pair(uint8_t rp);

  /**
   * @param  rg:  register bit pattern
   * @param  data:  data byte
   */
  void set_register(uint8_t rg, uint8_t data);
  uint8_t get_register(uint8_t rg);

  void lxi(uint8_t rp);
  void stax(uint8_t rp);

  void mvi(uint8_t rg);

  void lda();

private:

  bool m_running{true};

  std::unique_ptr<Bus> m_bus;

  uint16_t m_pc{};

  Registers m_rgs{};
  RegisterPairs m_rps{};

  const std::map<uint8_t, uint16_t*> m_rps_map = {
    {0b00, &m_rps.bc},
    {0b01, &m_rps.de},
    {0b10, &m_rps.hl},
    {0b11, &m_rps.sp},
  };

  const std::map<uint8_t, uint8_t*> m_rgs_map = {
    {0b111, &m_rgs.a},
    {0b000, &m_rgs.b},
    {0b001, &m_rgs.c},
    {0b010, &m_rgs.d},
    {0b011, &m_rgs.e},
    {0b100, &m_rgs.h},
    {0b101, &m_rgs.l},
  };

};

#endif /* CPU_H */
