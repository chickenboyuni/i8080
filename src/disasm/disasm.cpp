#include<cerrno>
#include<iostream>
#include<cstring>
#include<unordered_map>

#include "disasm.h"
#include "../common/binary_reader.h"
#include "../common/utils.h"


const std::unordered_map<uint8_t, std::string> rp_strings = {
  {0b00, "bc"},
  {0b01, "de"},
  {0b10, "hl"},
  {0b11, "sp"}
};

const std::unordered_map<uint8_t, std::string> rg_strings = {
  {0b111, "a"},
  {0b000, "b"},
  {0b001, "c"},
  {0b010, "d"},
  {0b011, "e"},
  {0b100, "h"},
  {0b101, "l"},

  {0b110, "M"} // not a register but adding for convenience for now, replace later with actual memory value
};

const std::unordered_map<uint8_t, std::string> conditional_flags = {
  {0b000, "nz"},
  {0b001, "z"},
  {0b010, "nc"},
  {0b011, "c"},
  {0b100, "po"},
  {0b101, "pe"},
  {0b110, "p"},
  {0b111, "m"}
};

size_t disassemble_from_file(const std::filesystem::path& bin_path){

  uint8_t rom[MAX_ROM_SIZE] {};

  size_t rom_size = read_bin_file(rom, MAX_ROM_SIZE, bin_path);

  std::array<DisassembledInstruction, MAX_ROM_SIZE> disassembled_rom {};

  return disassemble_rom(disassembled_rom, rom, rom_size);

}

// narg = no argument
void op_narg(std::stringstream& iss, const std::string& ins_name) { 
  iss << ins_name;
}

void op_rp(std::stringstream& iss, const std::string& ins_name, uint8_t rp) { 
  std::string dest = rp_strings.at(rp);
  if((ins_name == "push" || ins_name == "pop") && rp == 0b11){
    dest = "psw";
  }
  iss << boost::format("%s %s") % ins_name % dest; 
}

void op_rg(std::stringstream& iss, const std::string& ins_name, uint8_t rg) { 
  iss << boost::format("%s %s") % ins_name % rg_strings.at(rg); 
}

void op_addr(std::stringstream& iss, const std::string& ins_name, uint16_t addr) { 
  iss << boost::format("%s 0x%04x") % ins_name % addr; 
} 

void op_d8(std::stringstream& iss, const std::string& ins_name, uint8_t data) { 
  iss << ins_name << ' ';
  boost::format ins_frmt = (ins_name == "rst") ? boost::format("%d") : boost::format("0x%02x");
  iss << ins_frmt % static_cast<int>(data);
}

void op_condition(std::stringstream& iss, const char ins_name, uint8_t cf, const std::optional<uint16_t>& addr) { 
  iss << ins_name << conditional_flags.at(cf); 
  if(addr){
    iss << boost::format(" 0x%04x") % *addr;
  }
}

void op_lxi(std::stringstream& ins_ss, uint8_t rp, uint16_t data){
  ins_ss << boost::format("lxi %s, 0x%04x") % rp_strings.at(rp) % data;
}

void op_mvi(std::stringstream& ins_ss, uint8_t rg, uint8_t data){
  ins_ss << boost::format("mvi %s, 0x%02x") % rg_strings.at(rg) % static_cast<int>(data);
}

void op_mov(std::stringstream& ins_ss, uint8_t r1, uint8_t r2){
  ins_ss << boost::format("mov %s, %s") % rg_strings.at(r1) % rg_strings.at(r2); 
}
