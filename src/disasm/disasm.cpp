#include<cerrno>
#include<iostream>
#include<cstring>
#include<boost/format.hpp>
#include "disasm.h"

BinFile readBinFile(const std::filesystem::path& path){
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

  auto [bin, bin_size] = readBinFile(bin_path);

  unsigned int pc{};

  for(size_t i=0; i < bin_size; i++){
    std::cout << boost::format("\n0x%04x: ") % pc; 
    switch(bin[pc]) { 
      case 0x00: std::cout << "nop"; pc++; break;
      default: 
        std::cout << boost::format("%02x ") % static_cast<int>(bin[pc]); 
        pc++;
        break;
    }
  }
  std::cout << std::endl;


}
