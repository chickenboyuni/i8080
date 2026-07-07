#ifndef DISASM_H
#define DISASM_H

#include<memory>
#include<cstdint>
#include<optional>
#include<filesystem>

#define MAX_ROM_SIZE 1024 * 2 // 2 KiB
#define DISASSEMBLED_STRING_MAX_SIZE 64

typedef struct DisassembledInstruction {
  char ins_str[DISASSEMBLED_STRING_MAX_SIZE];
  uint8_t byte_count;
} DisassembledInstruction;

size_t disassemble_rom(DisassembledInstruction disassembled_instructions[], size_t disassembled_instructions_size, const uint8_t rom[], size_t rom_size);
size_t disassemble_from_file(const std::filesystem::path& bin_path);

void op_narg(char* instruction_str, const char* instruction_name);
void op_rp(char* instruction_str, const char* instruction_name, uint8_t rp);
void op_rg(char* instruction_str, const char* instruction_name, uint8_t rg);
void op_addr(char* instruction_str, const char* instruction_name, uint16_t addr);
void op_d8(char* instruction_str, const char* instruction_name, uint8_t data);
void op_condition(char* instruction_str, const char instruction_name, uint8_t cf, const std::optional<uint16_t>& addr = std::nullopt);
void op_lxi(char* instruction_str, uint8_t rp, uint16_t data);
void op_mvi(char* instruction_str, uint8_t rg, uint8_t data);
void op_mov(char* instruction_str, uint8_t r1, uint8_t r2);

#endif /* DISASM_H */
