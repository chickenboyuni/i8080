#include<cerrno>
#include<iostream>
#include<cstring>
#include<boost/format.hpp>
#include<unordered_map>
#include "disasm.h"

BinFile read_bin_file(const std::filesystem::path& path){
  std::ifstream bin(path, std::ios::binary | std::ios::ate);
  if(!bin){
    std::stringstream ss;
    ss <<  path.string() << ": " << std::strerror(errno);
    throw std::runtime_error(ss.str());
  }

  size_t bin_size = bin.tellg();
  bin.seekg(0, std::ios::beg);

  if(bin_size == 0){
    return {}; /* i'll deal with you later */
  }

  std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(bin_size);

  bin.read(reinterpret_cast<char*>(buf.get()), bin_size);
  bin.close();

  BinFile bin_file {.bin_content=std::move(buf), .bin_size=bin_size};

  return bin_file;
} 

const std::unordered_map<uint8_t, std::string> rp_strings = {
  {0b00, "b"},
  {0b01, "d"},
  {0b10, "h"},
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

int main(int argc, char* argv[]){

  if(argc > 2){
    std::cerr << "Too many arguments: " << argc << std::endl;
    return 1;
  }

  std::filesystem::path bin_path{};
  if(argc == 2) { 
    bin_path = argv[1];
  } else {
    std::cerr << "What file did you want me to disassemble?" << '\n';
    return 1;
  }

  auto [bin, bin_size] = read_bin_file(bin_path);

  std::stringstream decoded_ins{};

  for(uint16_t pc=0; pc < bin_size; pc++){
    decoded_ins << boost::format("\n0x%04x: ") % pc; 
    switch(bin[pc]) { 
      case 0x00: case 0x08: case 0x10: case 0x18: case 0x20: case 0x28: case 0x30:
      case 0x38: case 0xcb: case 0xd9: case 0xdd: case 0xed: case 0xfd:
        op_narg(decoded_ins, "nop"); break; // nop - 00000000
      case 0x01: case 0x11: case 0x21: case 0x31:
        op_lxi(decoded_ins, bin[pc] >> 4, (bin[pc+2] << 8) | bin[pc+1]); pc += 2; break; // lxi rp, d16 - 00rp0001 d8 d8
      case 0x02: case 0x12:
        op_rp(decoded_ins, "stax", bin[pc] >> 4); break; // stax rp - 00rp0010
      case 0x03: case 0x13: case 0x23: case 0x33:
        op_rp(decoded_ins, "inx", bin[pc] >> 4); break; // inx rp - 00rp0011
      case 0x04: case 0x0c: case 0x14: case 0x1c: case 0x24: case 0x2c: case 0x34: case 0x3c:
        op_rg(decoded_ins, "inr", bin[pc] >> 3); break; // inr r - 00ddd100
      case 0x05: case 0x0d: case 0x15: case 0x1d: case 0x25: case 0x2d: case 0x35: case 0x3d:
        op_rg(decoded_ins, "dcr", bin[pc] >> 3); break; // dcr r - 00ddd101
      case 0x06: case 0x0e: case 0x16: case 0x1e: case 0x26: case 0x2e: case 0x36: case 0x3e:
        op_mvi(decoded_ins, bin[pc] >> 3, bin[pc+1]); pc+=1; break; // mvi r, d8 - 00ddd110 d8
      case 0x07:
        op_narg(decoded_ins, "rlc"); break; // rlc - 00000111
      case 0x09: case 0x19: case 0x29: case 0x39:
        op_rp(decoded_ins, "dad", bin[pc] >> 4); break; // dad rp - 00rp1001
      case 0x0a: case 0x1a:
        op_rp(decoded_ins, "ldax", bin[pc] >> 4); break; // ldax rp - 00rp1010
      case 0x0b: case 0x1b: case 0x2b: case 0x3b:
        op_rp(decoded_ins, "dcx", bin[pc] >> 4); break; // dcx rp - 00rp1011
      case 0x0f:
        op_narg(decoded_ins, "rrc"); break; // rrc - 00001111
      case 0x17:
        op_narg(decoded_ins, "ral"); break; // ral - 00010111
      case 0x1f:
        op_narg(decoded_ins, "rar"); break; // rar - 00011111
      case 0x22:
        op_addr(decoded_ins, "shld", (bin[pc+2] << 8) | bin[pc+1]); pc += 2; break; // shld addr - 00100010 laddr haddr
      case 0x27:
        op_narg(decoded_ins, "daa"); break; // daa - 00100111
      case 0x2a:
        op_addr(decoded_ins, "lhld", (bin[pc+2] << 8) | bin[pc+1]); pc += 2; break; // lhld addr - 00101010 laddr haddr
      case 0x2f:
        op_narg(decoded_ins, "cma"); break; // cma - 00101111
      case 0x32:
        op_addr(decoded_ins, "sta", (bin[pc+2] << 8) | bin[pc+1]); pc += 2; break; // sta addr - 00110010 laddr haddr
      case 0x37:
        op_narg(decoded_ins, "stc"); break; // stc - 00110111
      case 0x3a:
        op_addr(decoded_ins, "lda", (bin[pc+2] << 8) | bin[pc+1]); pc += 2; break; // lda addr - 00111010 laddr haddr
      case 0x3f:
        op_narg(decoded_ins, "cmc"); break; // cmc - 00111111
      case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
      case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
      case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
      case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
      case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
      case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
      case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
      case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
        op_mov(decoded_ins, (bin[pc] & 0b00111000) >> 3, bin[pc] & 0b00000111); break; // mov r1, r2 - 01dddsss
      case 0x76:
        op_narg(decoded_ins, "hlt"); break; // hlt - 01110110
      case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
        op_rg(decoded_ins, "add", bin[pc] & 0b00000111); break; // add r - 10000sss
      case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
        op_rg(decoded_ins, "adc", bin[pc] & 0b00000111); break; // adc r - 10001sss
      case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
        op_rg(decoded_ins, "sub", bin[pc] & 0b00000111); break; // sub r - 10010sss
      case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
        op_rg(decoded_ins, "sbb", bin[pc] & 0b00000111); break; // sbb r - 10011sss
      case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
        op_rg(decoded_ins, "ana", bin[pc] & 0b00000111); break; // ana r - 10100sss
      case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
        op_rg(decoded_ins, "xra", bin[pc] & 0b00000111); break; // xra r - 10101sss
      case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
        op_rg(decoded_ins, "ora", bin[pc] & 0b00000111); break; // ora r - 10110sss
      case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
        op_rg(decoded_ins, "cmp", bin[pc] & 0b00000111); break; // cmp r - 10111sss
      case 0xc0: case 0xc8: case 0xd0: case 0xd8: case 0xe0: case 0xe8: case 0xf0: case 0xf8:
        op_condition(decoded_ins, 'r', (bin[pc] & 0b00111000) >> 3); break; // rcondition - 11ccc000
      case 0xc1: case 0xd1: case 0xe1: case 0xf1: 
        op_rp(decoded_ins, "pop", (bin[pc] & 0b00110000) >> 4); break; // pop rp - 11rp0001
      case 0xc2: case 0xca: case 0xd2: case 0xda: case 0xe2: case 0xea: case 0xf2: case 0xfa:
        op_condition(decoded_ins, 'j', (bin[pc] & 0b00110000) >> 4, (bin[pc+2] << 8) | bin[pc+1]); pc += 2; break; // jcondition addr - 11ccc010 laddr haddr
      case 0xc3:
        op_addr(decoded_ins, "jmp", (bin[pc+2] << 8) | bin[pc+1]); pc += 2; break; // jmp addr - 11000011 laddr haddr
      case 0xc4: case 0xcc: case 0xd4: case 0xdc: case 0xe4: case 0xec: case 0xf4: case 0xfc: case 0xfe:
        op_condition(decoded_ins, 'c', (bin[pc] & 0b00110000) >> 4, (bin[pc+2] << 8) | bin[pc+1]); pc += 2; break; // cconditon addr - 11ccc100 laddr haddr
      case 0xc5: case 0xd5: case 0xe5: case 0xf5:
        op_rp(decoded_ins, "push", (bin[pc] & 0b00110000) >> 4); break; // push rp - 11rp0101
      case 0xc6:
        op_d8(decoded_ins, "adi", bin[pc+1]); pc+=1; break; // adi d8 - 11000110 d8
      case 0xc7: case 0xcf: case 0xd7: case 0xdf: case 0xe7: case 0xef: case 0xf7: case 0xff:
        op_d8(decoded_ins, "rst", (bin[pc] & 0b00111000) >> 3); break; // rst n - 11nnn111
      case 0xc9:
        op_addr(decoded_ins, "ret", 0x0); break; // ret - 11001001 (ADD RETURN ADDRESS FROM SP LATER)
      case 0xcd:
        op_addr(decoded_ins, "call", (bin[pc+2] << 8) | bin[pc+1]); pc += 2; break; // call addr - 11001101 laddr haddr
      case 0xce:
        op_d8(decoded_ins, "aci", bin[pc+1]); pc+=1; break; // aci d8 - 11001110 d8
      case 0xd3:
        op_d8(decoded_ins, "out", bin[pc+1]); pc+=1; break; // out port - 11010011 d8
      case 0xd6:
        op_d8(decoded_ins, "sui", bin[pc+1]); pc+=1; break; // sui data - 11010110 d8
      case 0xdb:
        op_d8(decoded_ins, "in", bin[pc+1]); pc+=1; break; // in port - 11011011 d8
      case 0xde:
        op_addr(decoded_ins, "sbi", (bin[pc+2] << 8) | bin[pc+1]); pc += 2; break; // sbi addr - 11011110 laddr haddr
      case 0xe3:
        op_narg(decoded_ins, "xthl"); break; // xthl - 11100011
      case 0xe6:
        op_d8(decoded_ins, "ani", bin[pc+1]); pc+=1; break; // ani d8 - 11100110 d8
      case 0xe9:
        op_narg(decoded_ins, "pchl"); break; // pchl - 11101001
      case 0xeb:
        op_narg(decoded_ins, "xchg"); break; // xchg - 11101011
      case 0xee:
        op_d8(decoded_ins, "xri", bin[pc+1]); pc+=1; break; // xri d8 - 11101110 d8
      case 0xf3:
        op_narg(decoded_ins, "di"); break; // di - 11110011
      case 0xf6:
        op_d8(decoded_ins, "ori", bin[pc+1]); pc+=1; break; // ori d8 - 11110110 d8
      case 0xf9:
        op_narg(decoded_ins, "sphl"); break; // sphl - 11111001
      case 0xfb:
        op_narg(decoded_ins, "ei"); break; // ei - 11111011
      default: 
        decoded_ins << boost::format("%02x ") % static_cast<int>(bin[pc]); 
        break;
    }
    std::cout << decoded_ins.str(); 
    decoded_ins.clear();
    decoded_ins.str("");
  }
  std::cout << std::endl;
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
