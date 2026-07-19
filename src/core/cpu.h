#ifndef CPU_H
#define CPU_H

#include<cstdint>
#include<memory>
#include<unordered_map>

#include "../common/utils.h"
#include "cpu_state.h"
#include "bus.h"

#define NUM_OF_INSTRUCTIONS 0x100
#define CPU_CLOCK_RATE 2000000 // 2 MHz

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

uint8_t condition_is_met(bool condition);
uint8_t binary_add(uint8_t a, uint8_t b, unsigned int& carry, unsigned int& aux_carry);

class CPU {

  enum AddressingMode : unsigned int {
    ADDRESSING_MODE_REGISTER,
    ADDRESSING_MODE_MEMORY,
    ADDRESSING_MODE_IMMEDIATE,
  };

  typedef struct InterruptSystem {
    bool enabled{false};
    bool waiting{false};
    uint8_t instruction {};
  } InterruptSystem;

  const uint8_t instruction_clock_cycles[NUM_OF_INSTRUCTIONS] = {
    4, 10, 7, 5, 5, 5, 7, 4, 4, 10, 7, 5, 5, 5, 7, 4,
    4, 10, 7, 5, 5, 5, 7, 4, 4, 10, 7, 5, 5, 5, 7, 4,
    4, 10, 16, 5, 5, 5, 7, 4, 4, 10, 16, 5, 5, 5, 7, 4,
    4, 10, 13, 5, 10, 10, 10, 4, 4, 10, 13, 5, 5, 5, 7, 4,
    5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
    5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
    5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
    7, 7, 7, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 7, 5,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
    5, 10, 10, 10, 11, 11, 7, 11, 5, 10, 10, 11, 11, 17, 7, 11,
    5, 10, 10, 10, 11, 11, 7, 11, 5, 10, 10, 10, 11, 17, 7, 11,
    5, 10, 10, 18, 11, 11, 7, 11, 5, 5, 10, 4, 11, 17, 7, 11,
    5, 10, 10, 4, 11, 11, 7, 11, 5, 5, 10, 4, 11, 17, 7, 11
  };

  const std::unordered_map<uint8_t, unsigned int> regpairs_map = {
    {0b00, REGISTER_PAIR_BC},
    {0b01, REGISTER_PAIR_DE},
    {0b10, REGISTER_PAIR_HL}
  };

  const std::unordered_map<uint8_t, unsigned int> regs_map = {
    {0b111, REGISTER_A},
    {0b000, REGISTER_B},
    {0b001, REGISTER_C},
    {0b010, REGISTER_D},
    {0b011, REGISTER_E},
    {0b100, REGISTER_H},
    {0b101, REGISTER_L}
  }; 

public:

  bool running{true};

  CPU(Bus* bus);
  ~CPU() = default;

  bool halted() { return m_halted; }; 
  void reset();

  CpuState get_cpu_state();
  uint16_t get_pc() { return m_pc; };

  void request_interrupt(uint8_t ins);
  uint8_t fetch_execute_instruction();

private:

  bool m_halted{false};

  Bus* m_bus;
  InterruptSystem m_interrupt {};

  uint16_t m_pc = 0x00; // program counter
  uint16_t m_sp = 0x00; // stack pointer

  uint8_t m_psw = 0b00000010; // processor status word
  uint8_t m_regs[REGISTER_COUNT] {};

  CpuState m_cpu_state {};

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

  uint8_t get_addressing_mode_data(uint8_t rg, uint8_t addressing_mode);

#define X(flag, name) void update_##name##_flag(bool is_##name);
  LIST_OF_FLAGS
#undef X

  bool get_status_flag(uint8_t flag_type);

  void push_rp(uint8_t rp);
  void push_psw();
  void pop_rp(uint8_t rp);
  void pop_psw();

  void xthl();
  void sphl();

  void out();
  void in();

  void ei();
  void di();

  void hlt();

  void boolean_operation(uint8_t rg, unsigned int addressing_mode, uint8_t (*operator_func_ptr)(uint8_t, uint8_t));

  void cmp(uint8_t rg, unsigned int addressing_mode);

  void rotate_left(bool through_carry);
  void rotate_right(bool through_carry);

  void cma();
  void cmc();
  void stc();

  bool branch_condition_is_met(uint8_t condition);

  void jmp();
  bool jmp_conditional(uint8_t condition_bit_pattern);

  void call();
  bool call_conditional(uint8_t condition_bit_pattern);

  void ret();
  bool ret_conditional(uint8_t condition_bit_pattern);

  void rst(uint8_t number);

  void pchl();

  void add(uint8_t rg, unsigned int addressing_mode, bool with_carry);
  void sub(uint8_t rg, unsigned int addressing_mode, bool with_borrow);
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
};

#endif /* CPU_H */
