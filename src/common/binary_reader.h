#ifndef BINARY_READER_H
#define BINARY_READER_H

#include<filesystem>
#include<fstream>
#include<cstring>
#include<memory>

size_t read_bin_file(uint8_t* buffer, size_t buffer_size, const std::filesystem::path& path);

#endif /* BINARY_READER_H */
