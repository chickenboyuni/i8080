#ifndef CPU_STATE_H
#define CPU_STATE_H

#pragma pack(push, 1)
typedef struct StateFlagBits {
 uint8_t z:1;
 uint8_t s:1;
 uint8_t p:1;
 uint8_t cy:1;
 uint8_t ac:1;
} StateFlagBits;
#pragma pack(pop)

typedef struct StateRegisters {
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t d;
  uint8_t e;
  uint8_t h;
  uint8_t l;
} StateRegisters;

typedef struct StateRegisterPairs {
  uint16_t bc;
  uint16_t de;
  uint16_t hl;
  uint16_t sp;

  StateFlagBits psw;
} StateRegisterPairs;

/* Interface for anythting that needs access to the cpu state */
typedef struct CpuState {
  uint16_t pc;

  StateRegisters rgs;
  StateRegisterPairs rps;

} CpuState;

#endif /* CPU_STATE_H */
