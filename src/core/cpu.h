#ifndef CPU_H
#define CPU_H

#include<cstdint>
#include<memory>
#include<unordered_map>

#include "../common/utils.h"
#include "cpu_state.h"
#include "bus.h"

#define NUM_OF_INSTRUCTIONS 0x100

#define LIST_OF_FLAGS \
    X(Z, zero) \
    X(S, sign) \
    X(P, parity) \
    X(CY, carry) \
    X(AC, aux_carry) 

enum Register : unsigned int {
  REGISTER_A,
  REGISTER_B,
  REGISTER_C,
  REGISTER_D,
  REGISTER_E,
  REGISTER_H,
  REGISTER_L,
  REGISTER_COUNT
};

enum RegisterPair : unsigned int {
  REGISTER_PAIR_BC = REGISTER_B,
  REGISTER_PAIR_DE = REGISTER_D,
  REGISTER_PAIR_HL = REGISTER_H
};

enum FlagBits : uint8_t {
  FLAG_S  = 1 << 7,
  FLAG_Z  = 1 << 6,
  FLAG_AC = 1 << 4,
  FLAG_P  = 1 << 2,
  FLAG_CY = 1 << 0
};

enum class ArithmeticMode {
  Register,
  Memory,
  Immediate 
};

uint8_t condition_is_met(bool condition);
uint8_t binary_add(uint8_t a, uint8_t b, unsigned int& carry, unsigned int& aux_carry);

class CPU {
public:
  CPU(std::unique_ptr<Bus>&& bus);
  ~CPU();

  bool running(); 

  void reset();

  CpuState get_cpu_state();
  Bus* get_bus();

  void fetch_execute_instruction();

private:

  bool m_running{true};

  std::unique_ptr<Bus> m_bus;

  uint16_t m_pc = 0x00; // program counter
  uint16_t m_sp = 0x00; // stack pointer

  uint8_t m_psw = 0x0; // processor status word
  uint8_t m_regs[REGISTER_COUNT] {};

  uint8_t fetch_byte();
  uint16_t fetch_2bytes();

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

  uint16_t get_register_pair_from_idx(unsigned int register_pair_idx);

#define X(flag, name) void update_##name##_flag(bool is_##name);
  LIST_OF_FLAGS
#undef X

  bool get_status_flag(uint8_t flag_type);

  void add(uint8_t rg, ArithmeticMode mode, bool with_carry);
  void sub(uint8_t rg, ArithmeticMode mode, bool with_borrow);
  void inr_r(uint8_t rg, bool decrement);
  void inr_m(bool decrement);
  void inx(uint8_t rp, bool decrement);

  void dad(uint8_t rp);
  void daa();

  void shld();
  void lhld();

  void xchg();

  void lxi(uint8_t rp);

  void sta();
  void stax(uint8_t rp);

  void mov_rr(uint8_t r1, uint8_t r2);
  void mov_rm(uint8_t rg);
  void mov_mr(uint8_t rg);

  void mvi_r(uint8_t rg);
  void mvi_m();

  void lda();
  void ldax(uint8_t rp);

  const std::unordered_map<uint8_t, unsigned int> m_regpairs_map = {
    {0b00, REGISTER_PAIR_BC},
    {0b01, REGISTER_PAIR_DE},
    {0b10, REGISTER_PAIR_HL}
  };

  const std::unordered_map<uint8_t, unsigned int> m_regs_map = {
    {0b111, REGISTER_A},
    {0b000, REGISTER_B},
    {0b001, REGISTER_C},
    {0b010, REGISTER_D},
    {0b011, REGISTER_E},
    {0b100, REGISTER_H},
    {0b101, REGISTER_L}
  }; 
};

#endif /* CPU_H */
