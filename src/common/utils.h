// the word size is 8 bits for the i8080 microprocessor
#define INS_EXTRACT_REGISTER(word) word >> 3
#define INS_EXTRACT_REGISTERPAIR(word) word >> 4

#define INS_MAKE_ADDRESS(laddr, haddr) (haddr << 8) | laddr
