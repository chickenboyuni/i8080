#include<memory>
#include<iostream>

#include "invaders.h"
#include "shift_register.h"
#include "../cpu.h"
#include "../../common/binary_reader.h"
#include "../../common/logging.h"
#include "../../gui/gui.h"
#include "../../disasm/disasm.h"

/*
ROM LAYOUT
invaders.h 0000-07FF
invaders.g 0800-0FFF
invaders.f 1000-17FF
invaders.e 1800-1FFF
*/

#define INVADERS_H_START_ADDRESS 0x0000
#define INVADERS_G_START_ADDRESS 0x0800
#define INVADERS_F_START_ADDRESS 0x1000
#define INVADERS_E_START_ADDRESS 0x1800

#define MAX_BREAKPOINTS 16
#define REFRESH_RATE 60 // Hz
#define VBLANK_RATE (CPU_CLOCK_RATE / REFRESH_RATE)

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

  uint8_t rom_file[INVADERS_ROM_SIZE] {};
  size_t rom_file_size = load_invaders_rom(rom_file, rom_path);

  InvadersGUI igui {};
  int gui_running = !igui.setup();

  std::unique_ptr<ShiftRegister> shft_reg = std::make_unique<ShiftRegister>();
  std::unique_ptr<InvadersBus> bus = std::make_unique<InvadersBus>(shft_reg.get());
  CPU cpu(bus.get());

  bus->load_rom(rom_file, rom_file_size);

  while(gui_running) {

#ifndef NDEBUG

    bool debug_cpu_running {false};
    bool step_through_cpu {false};

    CpuState cpu_state {};

    unsigned int breakpoints[MAX_BREAKPOINTS] = {};
#endif /* ifndef NDEBUG */

    MemoryState memory_state {};

    size_t total_cycles = 0;
    bool interrupt_requested = false;
    try {
      while(cpu.running){
        memory_state = bus->get_memory_state();

#ifndef NDEBUG
        cpu_state = cpu.get_cpu_state();
        if((total_cycles > VBLANK_RATE) || !debug_cpu_running){
          if(igui.update_debugger_window(cpu_state, memory_state, breakpoints, MAX_BREAKPOINTS, step_through_cpu, debug_cpu_running)){
            gui_running = false;
            break;
          }
        }
#endif /* ifndef NDEBUG */
        if((total_cycles > VBLANK_RATE / 2) && !interrupt_requested) {
          cpu.request_interrupt(0xcf);
          interrupt_requested = true;
        }

        if(total_cycles > VBLANK_RATE) {
          if(igui.update_game_window(memory_state)){
            gui_running = false;
            break;
          }
          cpu.request_interrupt(0xd7);
          total_cycles = 0;
          interrupt_requested = false;
        }
#ifdef NDEBUG
        if(!cpu.halted()) {
          total_cycles += cpu.fetch_execute_instruction();
        }
#else 
        if((step_through_cpu || debug_cpu_running) && !cpu.halted()) {
          total_cycles += cpu.fetch_execute_instruction();
          for(size_t k = 0; k < MAX_BREAKPOINTS; k++) {
            if(cpu.get_pc() == breakpoints[k]){
                debug_cpu_running = false;
                break;
            }
          }
          step_through_cpu = false;
        }
#endif /* ifdef NDEBUG */
      }
    } 

    catch(const panic_exception& ex) {

    }

    catch(const std::exception& ex) { 
        std::cerr << ex.what() << std::endl;
    }

    cpu.reset();
  }

  igui.destroy();

  return EXIT_SUCCESS;
}
