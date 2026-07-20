#include "shift_register.h"

ShiftRegister::ShiftRegister() : m_shift_data(0), m_shift_offset(0) { 

}

void ShiftRegister::shift_data_w(uint8_t data) {
  m_shift_data = (m_shift_data >> 8) | (data << 8);
}

void ShiftRegister::set_shift_offset_w(uint8_t offset) {
  m_shift_offset = offset & 0x07;
}

uint8_t ShiftRegister::get_shift_result_r() {
  return (uint8_t)((m_shift_data << m_shift_offset) >> 8);
}
