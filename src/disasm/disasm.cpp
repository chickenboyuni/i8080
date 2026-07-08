#include<cerrno>
#include<cstring>
#include<unordered_map>

#include "disasm.h"
#include "../common/binary_reader.h"
#include "../common/utils.h"
#include "../common/logging.h"

const std::unordered_map<uint8_t, const char*> rp_strings = {
  {0b00, "bc"},
  {0b01, "de"},
  {0b10, "hl"},
  {0b11, "sp"}
};

const std::unordered_map<uint8_t, const char*> rg_strings = {
  {0b111, "a"},
  {0b000, "b"},
  {0b001, "c"},
  {0b010, "d"},
  {0b011, "e"},
  {0b100, "h"},
  {0b101, "l"},

  {0b110, "M"} // not a register but adding for convenience for now, replace later with actual memory value
};

const std::unordered_map<uint8_t, const char*> conditional_flags = {
  {0b000, "nz"},
  {0b001, "z"},
  {0b010, "nc"},
  {0b011, "c"},
  {0b100, "po"},
  {0b101, "pe"},
  {0b110, "p"},
  {0b111, "m"}
};

size_t disassemble_rom(DisassembledInstruction disassembled_instructions[], size_t disassembled_instructions_size, const uint8_t rom[], size_t rom_size) {

  char disassembled_instruction_str[DISASSEMBLED_STRING_MAX_SIZE];

  if(rom_size > disassembled_instructions_size) {
    LOG_DEBUG_ERROR("terminating disassembling as disassembled instructions might not fit in passed in array as rom is bigger");
    return 0;
  }

  uint16_t pc;
  size_t i;
  for(pc=0, i=0; pc < rom_size; pc++, i++) {

    uint16_t pc_before = pc;

    switch(rom[pc]) { 
      case 0x00: case 0x08: case 0x10: case 0x18: case 0x20: case 0x28: case 0x30:
      case 0x38: case 0xcb: case 0xd9: case 0xdd: case 0xed: case 0xfd:
        op_narg(disassembled_instruction_str, "nop"); break; // nop - 00000000
      case 0x01: case 0x11: case 0x21: case 0x31:
        op_lxi(disassembled_instruction_str, INS_EXTRACT_REGISTERPAIR(rom[pc]), INS_MAKE_ADDRESS(rom[pc+1], rom[pc+2])); pc += 2; break; // lxi rp, d16 - 00rp0001 d8 d8
      case 0x02: case 0x12:
        op_rp(disassembled_instruction_str, "stax", INS_EXTRACT_REGISTERPAIR(rom[pc])); break; // stax rp - 00rp0010
      case 0x03: case 0x13: case 0x23: case 0x33:
        op_rp(disassembled_instruction_str, "inx", INS_EXTRACT_REGISTERPAIR(rom[pc])); break; // inx rp - 00rp0011
      case 0x04: case 0x0c: case 0x14: case 0x1c: case 0x24: case 0x2c: case 0x34: case 0x3c:
        op_rg(disassembled_instruction_str, "inr", INS_EXTRACT_REGISTER(rom[pc])); break; // inr r - 00ddd100
      case 0x05: case 0x0d: case 0x15: case 0x1d: case 0x25: case 0x2d: case 0x35: case 0x3d:
        op_rg(disassembled_instruction_str, "dcr", INS_EXTRACT_REGISTER(rom[pc])); break; // dcr r - 00ddd101
      case 0x06: case 0x0e: case 0x16: case 0x1e: case 0x26: case 0x2e: case 0x36: case 0x3e:
        op_mvi(disassembled_instruction_str, INS_EXTRACT_REGISTER(rom[pc]), rom[pc+1]); pc += 1; break; // mvi r, d8 - 00ddd110 d8
      case 0x07:
        op_narg(disassembled_instruction_str, "rlc"); break; // rlc - 00000111
      case 0x09: case 0x19: case 0x29: case 0x39:
        op_rp(disassembled_instruction_str, "dad", INS_EXTRACT_REGISTERPAIR(rom[pc])); break; // dad rp - 00rp1001
      case 0x0a: case 0x1a:
        op_rp(disassembled_instruction_str, "ldax", INS_EXTRACT_REGISTERPAIR(rom[pc])); break; // ldax rp - 00rp1010
      case 0x0b: case 0x1b: case 0x2b: case 0x3b:
        op_rp(disassembled_instruction_str, "dcx", INS_EXTRACT_REGISTERPAIR(rom[pc])); break; // dcx rp - 00rp1011
      case 0x0f:
        op_narg(disassembled_instruction_str, "rrc"); break; // rrc - 00001111
      case 0x17:
        op_narg(disassembled_instruction_str, "ral"); break; // ral - 00010111
      case 0x1f:
        op_narg(disassembled_instruction_str, "rar"); break; // rar - 00011111
      case 0x22:
        op_addr(disassembled_instruction_str, "shld", INS_MAKE_ADDRESS(rom[pc+1], rom[pc+2])); pc += 2; break; // shld addr - 00100010 laddr haddr
      case 0x27:
        op_narg(disassembled_instruction_str, "daa"); break; // daa - 00100111
      case 0x2a:
        op_addr(disassembled_instruction_str, "lhld", INS_MAKE_ADDRESS(rom[pc+1], rom[pc+2])); pc += 2; break; // lhld addr - 00101010 laddr haddr
      case 0x2f:
        op_narg(disassembled_instruction_str, "cma"); break; // cma - 00101111
      case 0x32:
        op_addr(disassembled_instruction_str, "sta", INS_MAKE_ADDRESS(rom[pc+1], rom[pc+2])); pc += 2; break; // sta addr - 00110010 laddr haddr
      case 0x37:
        op_narg(disassembled_instruction_str, "stc"); break; // stc - 00110111
      case 0x3a:
        op_addr(disassembled_instruction_str, "lda", INS_MAKE_ADDRESS(rom[pc+1], rom[pc+2])); pc += 2; break; // lda addr - 00111010 laddr haddr
      case 0x3f:
        op_narg(disassembled_instruction_str, "cmc"); break; // cmc - 00111111
      case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
      case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
      case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
      case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
      case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
      case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
      case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
      case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
        op_mov(disassembled_instruction_str, (rom[pc] & 0b00111000) >> 3, rom[pc] & 0b00000111); break; // mov r1, r2 - 01dddsss
      case 0x76:
        op_narg(disassembled_instruction_str, "hlt"); break; // hlt - 01110110
      case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
        op_rg(disassembled_instruction_str, "add", rom[pc] & 0b00000111); break; // add r - 1/000sss
      case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
        op_rg(disassembled_instruction_str, "adc", rom[pc] & 0b00000111); break; // adc r - 10001sss
      case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
        op_rg(disassembled_instruction_str, "sub", rom[pc] & 0b00000111); break; // sub r - 10010sss
      case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
        op_rg(disassembled_instruction_str, "sbb", rom[pc] & 0b00000111); break; // sbb r - 10011sss
      case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
        op_rg(disassembled_instruction_str, "ana", rom[pc] & 0b00000111); break; // ana r - 10100sss
      case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
        op_rg(disassembled_instruction_str, "xra", rom[pc] & 0b00000111); break; // xra r - 10101sss
      case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
        op_rg(disassembled_instruction_str, "ora", rom[pc] & 0b00000111); break; // ora r - 10110sss
      case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
        op_rg(disassembled_instruction_str, "cmp", rom[pc] & 0b00000111); break; // cmp r - 10111sss
      case 0xc0: case 0xc8: case 0xd0: case 0xd8: case 0xe0: case 0xe8: case 0xf0: case 0xf8:
        op_condition(disassembled_instruction_str, 'r', (rom[pc] & 0b00111000) >> 3); break; // rcondition - 11ccc000
      case 0xc1: case 0xd1: case 0xe1: case 0xf1: 
        op_rp(disassembled_instruction_str, "pop", INS_EXTRACT_REGISTERPAIR(rom[pc])); break; // pop rp - 11rp0001
      case 0xc2: case 0xca: case 0xd2: case 0xda: case 0xe2: case 0xea: case 0xf2: case 0xfa:
        op_condition(disassembled_instruction_str, 'j', INS_EXTRACT_CONDITION(rom[pc]), INS_MAKE_ADDRESS(rom[pc+1], rom[pc+2])); pc += 2; break; // jcondition addr - 11ccc010 laddr haddr
      case 0xc3:
        op_addr(disassembled_instruction_str, "jmp", INS_MAKE_ADDRESS(rom[pc+1], rom[pc+2])); pc += 2; break; // jmp addr - 11000011 laddr haddr
      case 0xc4: case 0xcc: case 0xd4: case 0xdc: case 0xe4: case 0xec: case 0xf4: case 0xfc: case 0xfe:
        op_condition(disassembled_instruction_str, 'c', INS_EXTRACT_CONDITION(rom[pc]), INS_MAKE_ADDRESS(rom[pc+1], rom[pc+2])); pc += 2; break; // cconditon addr - 11ccc100 laddr haddr
      case 0xc5: case 0xd5: case 0xe5: case 0xf5:
        op_rp(disassembled_instruction_str, "push", INS_EXTRACT_REGISTERPAIR(rom[pc])); break; // push rp - 11rp0101
      case 0xc6:
        op_d8(disassembled_instruction_str, "adi", rom[pc+1]); pc += 1; break; // adi d8 - 11000110 d8
      case 0xc7: case 0xcf: case 0xd7: case 0xdf: case 0xe7: case 0xef: case 0xf7: case 0xff:
        op_d8(disassembled_instruction_str, "rst", (rom[pc] & 0b00111000) >> 3); break; // rst n - 11nnn111
      case 0xc9:
        op_addr(disassembled_instruction_str, "ret", 0x0); break; // ret - 11001001 (ADD RETURN ADDRESS FROM SP LATER)
      case 0xcd:
        op_addr(disassembled_instruction_str, "call", INS_MAKE_ADDRESS(rom[pc+1], rom[pc+2])); pc += 2; break; // call addr - 11001101 laddr haddr
      case 0xce:
        op_d8(disassembled_instruction_str, "aci", rom[pc+1]); pc += 1; break; // aci d8 - 11001110 d8
      case 0xd3:
        op_d8(disassembled_instruction_str, "out", rom[pc+1]); pc += 1; break; // out port - 11010011 d8
      case 0xd6:
        op_d8(disassembled_instruction_str, "sui", rom[pc+1]); pc += 1; break; // sui data - 11010110 d8
      case 0xdb:
        op_d8(disassembled_instruction_str, "in", rom[pc+1]); pc += 1; break; // in port - 11011011 d8
      case 0xde:
        op_addr(disassembled_instruction_str, "sbi", INS_MAKE_ADDRESS(rom[pc+1], rom[pc+2])); pc += 2; break; // sbi addr - 11011110 laddr haddr
      case 0xe3:
        op_narg(disassembled_instruction_str, "xthl"); break; // xthl - 11100011
      case 0xe6:
        op_d8(disassembled_instruction_str, "ani", rom[pc+1]); pc += 1; break; // ani d8 - 11100110 d8
      case 0xe9:
        op_narg(disassembled_instruction_str, "pchl"); break; // pchl - 11101001
      case 0xeb:
        op_narg(disassembled_instruction_str, "xchg"); break; // xchg - 11101011
      case 0xee:
        op_d8(disassembled_instruction_str, "xri", rom[pc+1]); pc += 1; break; // xri d8 - 11101110 d8
      case 0xf3:
        op_narg(disassembled_instruction_str, "di"); break; // di - 11110011
      case 0xf6:
        op_d8(disassembled_instruction_str, "ori", rom[pc+1]); pc += 1; break; // ori d8 - 11110110 d8
      case 0xf9:
        op_narg(disassembled_instruction_str, "sphl"); break; // sphl - 11111001
      case 0xfb:
        op_narg(disassembled_instruction_str, "ei"); break; // ei - 11111011
      default: 
        sprintf(disassembled_instruction_str, "%02x ", static_cast<int>(rom[pc]));
        break;
    }

    strcpy(disassembled_instructions[i].ins_str, disassembled_instruction_str);
    disassembled_instructions[i].byte_count = (pc - pc_before) + 1;
    strcpy(disassembled_instruction_str, "");
  }

  return i;
}

size_t disassemble_from_file(const std::filesystem::path& bin_path){

  uint8_t rom[MAX_ROM_SIZE] {};
  size_t rom_size = read_bin_file(rom, MAX_ROM_SIZE, bin_path);

  DisassembledInstruction disassembled_rom[MAX_ROM_SIZE] {};

  return disassemble_rom(disassembled_rom, MAX_ROM_SIZE, rom, rom_size);
}

// narg = no argument
void op_narg(char* instruction_str, const char* instruction_name) { 
  sprintf(instruction_str, "%s", instruction_name);
}

void op_rp(char* instruction_str, const char* instruction_name, uint8_t rp) { 
  char dest[8];
  strcpy(dest, rp_strings.at(rp));
  if((strcmp(instruction_name, "push") || strcmp(instruction_name, "pop")) && rp == 0b11){
    sprintf(dest, "psw");
  }
  sprintf(instruction_str, "%s %s", instruction_name, dest);
}

void op_rg(char* instruction_str, const char* instruction_name, uint8_t rg) { 
  sprintf(instruction_str, "%s %s", instruction_name, rg_strings.at(rg));
}

void op_addr(char* instruction_str, const char* instruction_name, uint16_t addr) { 
  sprintf(instruction_str, "%s 0x%04x", instruction_name, addr);
} 

void op_d8(char* instruction_str, const char* instruction_name, uint8_t data) { 
  if(strcmp(instruction_name, "rst")) {
    sprintf(instruction_str, "%s %d", instruction_name, static_cast<int>(data));
  } else {
    sprintf(instruction_str, "%s 0x%02x", instruction_name, static_cast<int>(data));
  }
}

void op_condition(char* instruction_str, const char instruction_name, uint8_t cf, const std::optional<uint16_t>& addr) { 
  sprintf(instruction_str, "%c%s", instruction_name, conditional_flags.at(cf));
  if(addr){
    sprintf(instruction_str + strlen(instruction_str), " 0x%04x", *addr);
  }
}

void op_lxi(char* instruction_str, uint8_t rp, uint16_t data){
  sprintf(instruction_str, "lxi %s, 0x%04x", rp_strings.at(rp), data);
}

void op_mvi(char* instruction_str, uint8_t rg, uint8_t data){
  sprintf(instruction_str, "mvi %s, 0x%02x", rg_strings.at(rg), static_cast<int>(data));
}

void op_mov(char* instruction_str, uint8_t r1, uint8_t r2){
  sprintf(instruction_str, "mov %s, %s", rg_strings.at(r1), rg_strings.at(r2));
}
