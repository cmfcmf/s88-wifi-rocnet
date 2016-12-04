#ifndef SHIFTREGISTER_H
#define SHIFTREGISTER_H

#include "Arduino.h"

class ShiftRegister {
  uint8_t m_num, m_clkPin, m_dataPin, m_loadPin;
  const uint8_t m_loadSpeed = 200;
  const uint8_t m_clkSpeed = 100;
  
  public:
    ShiftRegister();
    void begin(const uint8_t num, const uint8_t clkPin, const uint8_t dataPin, const uint8_t loadPin);
    void read(bool results[]);
};
#endif
