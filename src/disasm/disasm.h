#ifndef DISASM_H
#define DISASM_H

#include<memory>
#include<cstdint>
#include<fstream>
#include<optional>

void op_narg(std::stringstream& iss, const std::string& ins_name);
void op_rp(std::stringstream& iss, const std::string& ins_name, uint8_t rp);
void op_rg(std::stringstream& iss, const std::string& ins_name, uint8_t rg);
void op_addr(std::stringstream& iss, const std::string& ins_name, uint16_t addr);
void op_d8(std::stringstream& iss, const std::string& ins_name, uint8_t data);
void op_condition(std::stringstream& iss, const char ins_name, uint8_t cf, const std::optional<uint16_t>& addr = std::nullopt);
void op_lxi(std::stringstream& ins_ss, uint8_t rp, uint16_t data);
void op_mvi(std::stringstream& ins_ss, uint8_t rg, uint8_t data);
void op_mov(std::stringstream& ins_ss, uint8_t r1, uint8_t r2);

#endif /* DISASM_H */
