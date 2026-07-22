#include<cassert>
#include<memory>

#include "../cpu.h"
#include "../cpu_state.h"
#include "../../common/binary_reader.h"

#define TEST_MEMORY_SIZE 0x10000

class TestBus : public Bus {

public:

  TestBus() {};
  ~TestBus() = default;

  uint8_t memory_read(uint16_t addr) override {
    return m_memory[addr];
  }
  void memory_write(uint16_t addr, uint8_t data) override {
    m_memory[addr] = data;
  }

  uint8_t io_read(uint8_t) override { return 0x0; };
  void io_write(uint8_t, uint8_t) override { };

  void load_rom(uint8_t* rom_data, size_t rom_size) {
    assert((rom_size + 0x100) < TEST_MEMORY_SIZE);
    memcpy(&m_memory[0x100], rom_data, rom_size);
  }

  void reset() override {};

private:
  uint8_t m_memory[TEST_MEMORY_SIZE] {};
};

int main(int argc, char* argv[]){

  if(argc > 2){
    fprintf(stderr, "Too many arguments: %d\n", argc);
    return EXIT_FAILURE;
  }

  std::filesystem::path rom_path{};
  if(argc == 2) { 
    rom_path = argv[1];
  } else {
    fprintf(stderr, "Can you point me to the test cpm test rom pretty please?\n");
    return EXIT_FAILURE;
  }

  uint8_t test_rom_buffer[TEST_MEMORY_SIZE] {};
  size_t test_rom_size = read_bin_file(test_rom_buffer, TEST_MEMORY_SIZE, rom_path);

  std::unique_ptr<TestBus> bus = std::make_unique<TestBus>();
  CPU cpu(bus.get(), 0x100);

  bus->load_rom(test_rom_buffer, test_rom_size);

  bus->memory_write(0x0001, 0x76); // hlt
  bus->memory_write(0x0007, 0xc9); // ret

  while(!cpu.halted()){
    CpuState state = cpu.get_cpu_state();

    // Jump to CP/M BDOS entry, ie make a BDOS system call. (https://en.wikipedia.org/wiki/Zero_page_(CP/M))
    if(state.pc == 0x0005){

      if(state.rgs.c == 2) {
	printf("%c", state.rgs.e);
      }

      if(state.rgs.c == 9) {
	for(int i = 0; bus->memory_read(state.rps.de+i) != '$'; i++) {
	  printf("%c", bus->memory_read(state.rps.de+i));
	}
      }

    }

    cpu.fetch_execute_instruction();
  }

  printf("\n");
}
