#include<memory>
#include<iostream>

#include "../../common/binary_reader.h"
#include "../cpu.h"
#include "invaders.h"
#include "../../common/logging.h"
#include "../../gui/gui.h"
#include "../../disasm/disasm.h"

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

  std::unique_ptr<InvadersGUI> igui = std::make_unique<InvadersGUI>();
  int gui_running = !igui->setup();

#ifndef NDEBUG
    // TEMPORARY: just so i can test and implement instructions one by one for now
    // small test for implemented instructions 
    uint8_t rom_file[INVADERS_ROM_SIZE] {0x3e, 0x12, // mvi a, 0x12
                                         0x01, 0x56, 0x34, // lxi bc, 0x3456
                                         0x02, // stax bc
                                         0x3e, 0x00, // mvi a, 0x00
                                         0x3a, 0x56, 0x34, // lda 0x3456
                                         0xff};
    size_t rom_file_size = INVADERS_ROM_SIZE;
#else
    uint8_t rom_file[INVADERS_ROM_SIZE] {};
    size_t rom_file_size = load_invaders_rom(rom_file, rom_path);
#endif

  std::unique_ptr<InvadersBus> bus = std::make_unique<InvadersBus>();
  bus->load_rom(rom_file, rom_file_size);

  CPU cpu(std::move(bus));

  bool debug_cpu_running {false};

  while(gui_running) {

    bool step_through_cpu {false};
    try {
      CpuState cpu_state {};

      while(cpu.running()){
        cpu_state = cpu.get_cpu_state();

        // update_frame() returns 1 on exiting gui
        if(igui->update_frame(cpu_state, step_through_cpu, debug_cpu_running, rom_file, rom_file_size)){
          gui_running = false;
          break;
        }

#ifndef NDEBUG
        if(step_through_cpu || debug_cpu_running) {
          cpu.fetch_execute_instruction();
          step_through_cpu = false;
        }
#else
        cpu.fetch_execute_instruction();
#endif

      }
    } 

    catch(const panic_exception& ex) {

    }

    catch(const std::exception& ex) { 
        std::cerr << ex.what() << std::endl;
    }

    cpu.reset();
  } 
  igui->destroy();

  return EXIT_SUCCESS;
}
