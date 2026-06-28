#include<memory>
#include<iostream>

#include "../../common/binary_reader.h"
#include "../cpu.h"
#include "invaders.h"
#include "../../common/logging.h"

size_t load_invaders_rom(uint8_t* rom_buffer, const std::filesystem::path& invaders_rom_path){
  size_t rom_buffer_size{};
  rom_buffer_size += read_bin_file(&rom_buffer[INVADERS_H_START_ADDRESS], INVADERS_ROM_SIZE-rom_buffer_size, invaders_rom_path / "invaders.h");
  rom_buffer_size += read_bin_file(&rom_buffer[INVADERS_G_START_ADDRESS], INVADERS_ROM_SIZE-rom_buffer_size, invaders_rom_path / "invaders.g");
  rom_buffer_size += read_bin_file(&rom_buffer[INVADERS_F_START_ADDRESS], INVADERS_ROM_SIZE-rom_buffer_size, invaders_rom_path / "invaders.f");
  rom_buffer_size += read_bin_file(&rom_buffer[INVADERS_E_START_ADDRESS], INVADERS_ROM_SIZE-rom_buffer_size, invaders_rom_path / "invaders.e");

  if(rom_buffer_size > INVADERS_ROM_SIZE) {
    PANIC("terminating program, rom being loaded is bigger than system ROM");
  }
  return rom_buffer_size;
}

int main(int argc, char* argv[]){

  if(argc > 2){
    std::cerr << "Too many arguments: " << argc << std::endl;
    return EXIT_FAILURE;
  }

  std::filesystem::path rom_path{};
  if(argc == 2) { 
    rom_path = argv[1] / std::filesystem::path{} ;
  } else {
    std::cerr << "Can you tell me the directory that has the space invader roms pretty please?" << '\n';
    return EXIT_FAILURE;
  }

  try {
    #ifndef NDEBUG
      // TEMPORARY: just so i can test and implement instructions one by one for now
      uint8_t rom_file[INVADERS_ROM_SIZE] {0x01, 0x02, 0x02, 0xff};
      size_t rom_file_size = INVADERS_ROM_SIZE;
    #else
      uint8_t rom_file[INVADERS_ROM_SIZE] {};
      size_t rom_file_size = load_invaders_rom(rom_file, rom_path);
    #endif

    std::unique_ptr<InvadersBus> bus = std::make_unique<InvadersBus>();
    bus->load_rom(rom_file, rom_file_size);

    CPU cpu(std::move(bus));

    while(cpu.running()){
      cpu.fetch_execute_instruction();
    }
  } 
  catch(const panic_exception& ex) {
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
