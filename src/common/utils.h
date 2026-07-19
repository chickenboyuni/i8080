// the word size is 8 bits for the i8080 microprocessor
#define INS_EXTRACT_DDD_REGISTER(word) (word & 0b00111000) >> 3
#define INS_EXTRACT_SSS_REGISTER(word) (word & 0b00000111)
#define INS_EXTRACT_REGISTERPAIR(word) (word & 0b00110000) >> 4
#define INS_EXTRACT_CONDITION(word) (word & 0b00111000) >> 3

#define INS_MAKE_ADDRESS(laddr, haddr) (haddr << 8) | laddr
