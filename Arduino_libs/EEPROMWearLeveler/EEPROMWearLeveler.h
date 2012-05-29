/*
  EEPROMWearLeveler.h
  Created by Nabil Tewolde, May 14, 2012.
  Released into the public domain.
*/

#ifndef EEPROMWearLeveler_h
#define EEPROMWearLeveler_h

#include "Arduino.h"
#include "EEPROM.h"

class EEPROMWearLeveler : public EEPROM
{
 public:
  EEPROMWearLeveler(address, value);
  uint8_t read(int);
  void write(int, uint8_t);
 private:
  int _pin;
};

#endif
