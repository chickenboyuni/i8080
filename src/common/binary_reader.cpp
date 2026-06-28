#include "binary_reader.h"
#include "logging.h"

size_t read_bin_file(uint8_t* buffer, size_t buffer_size, const std::filesystem::path& path){
  std::ifstream bin(path, std::ios::binary | std::ios::ate);
  if(!bin){
    std::stringstream ss;
    ss <<  path.string() << ": " << std::strerror(errno);
    throw std::runtime_error(ss.str());
  }

  size_t bin_size = bin.tellg();
  bin.seekg(0, std::ios::beg);

  if(bin_size == 0){
    return 0; /* i'll deal with you later */
  }

  if(buffer_size < bin_size) {
    PANIC("terminating program, passed in buffer is smaller than the binary file being read into it");
  }
  bin.read(reinterpret_cast<char*>(buffer), bin_size);
  bin.close();

  return bin_size;
} 

