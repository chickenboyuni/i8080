#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#include <cstdint>

class ShiftRegister {
public: 
  ShiftRegister();

  void shift_data_w(uint8_t data);
  void set_shift_offset_w(uint8_t offset);
  uint8_t get_shift_result_r();

private:
  uint16_t m_shift_data;
  uint8_t m_shift_offset;
};

#endif /* SHIFT_REGISTER_H */
