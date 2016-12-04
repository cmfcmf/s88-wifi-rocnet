#include "ShiftRegister.h"

ShiftRegister::ShiftRegister() { }

void ShiftRegister::begin(uint8_t num, uint8_t clkPin, uint8_t dataPin, uint8_t loadPin) {
  m_num = num;
  m_loadPin = loadPin;
  m_clkPin = clkPin;
  m_dataPin = dataPin;

  pinMode(m_loadPin, OUTPUT);
  pinMode(m_clkPin, OUTPUT);
  pinMode(m_dataPin, INPUT);

  digitalWrite(m_clkPin, LOW);
  digitalWrite(m_loadPin, HIGH);
}

void ShiftRegister::read(bool results[]) {
  digitalWrite(m_loadPin, LOW);
  delayMicroseconds(m_loadSpeed);
  digitalWrite(m_loadPin, HIGH);
  delayMicroseconds(m_loadSpeed);

  for (uint8_t r = 0; r < m_num; r++) {
    for (uint8_t i = 0; i < 8; i++) {
      uint8_t pos;
      if (r < 4) {
        if (i < 4) {
          pos = i + 16 + r * 4;
        } else {
          pos = i - 1 - (i - 4) * 2 + r * 4;
        }  
      } else {
        pos = r * 8 + i; 
      }

      results[pos] = digitalRead(m_dataPin);

      digitalWrite(m_clkPin, HIGH);
      delayMicroseconds(m_clkSpeed);
      digitalWrite(m_clkPin, LOW);
      delayMicroseconds(m_clkSpeed);
    }
  }
}

