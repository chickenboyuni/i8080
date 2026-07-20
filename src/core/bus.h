#ifndef BUS_H
#define BUS_H

#include <cstdint>

class Bus {

public:

  virtual ~Bus() = default;

  virtual uint8_t memory_read(uint16_t addr) = 0;
  virtual void memory_write(uint16_t addr, uint8_t data) = 0;

  virtual uint8_t io_read(uint8_t port) = 0;
  virtual void io_write(uint8_t port, uint8_t data) = 0;

  virtual void reset() = 0;

};

#endif /* BUS_H */
