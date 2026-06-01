#ifndef DISASM_H
#define DISASM_H

#include<filesystem>
#include<memory>
#include<cstdint>
#include<fstream>

struct BinFile {
  std::unique_ptr<uint8_t[]> bin_content;
  size_t bin_size{};
};

BinFile readBinFile(const std::filesystem::path& path);

#endif /* DISASM_H */
