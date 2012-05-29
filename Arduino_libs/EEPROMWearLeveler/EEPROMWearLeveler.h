/*
  EEPROMWearLeveler.h
  Created by Nabil Tewolde, May 14, 2012.
  Released into the public domain.
*/

#ifndef EEPROMWearLeveler_h
#define EEPROMWearLeveler_h

extern int AVR_EEPROM_SIZE;

#ifndef DEBUG
#include "Arduino.h"
#else

#include <string.h>
#include <inttypes.h>
#include <iostream>
#include <iomanip>
class EEPROMClass
{
public:
  EEPROMClass()
  {
    EEPROM_MEM = new uint8_t[AVR_EEPROM_SIZE];
    for ( int i = 0; i < AVR_EEPROM_SIZE; i++ )
    {
      EEPROM_MEM[i] = 0;
    }
    //memset (EEPROM_MEM, 0, AVR_EEPROM_SIZE);
  }

  ~EEPROMClass()
  {
    delete[] EEPROM_MEM;
  }

  void change_size(int size)
  {
    delete[] EEPROM_MEM;
    EEPROM_MEM = new uint8_t[size];
    for ( int i = 0; i < size; i++ )
    {
      EEPROM_MEM[i] = 0;
    }

  }

  uint8_t read(int address)
  {
    if ( address < AVR_EEPROM_SIZE && address >= 0)     
      return EEPROM_MEM[address];
  }

  void write(int address, uint8_t value)
  {
    if ( address < AVR_EEPROM_SIZE && address >= 0)
      EEPROM_MEM[address] = value;
  }

  void print()
  {
    for ( int i = 0; i < AVR_EEPROM_SIZE; i++ )
    {
      std::cout << std::right << std::setfill(' ') << std::setw(3) << (int)EEPROM_MEM[i]<< " ";

#if 1
      int buf_len = 512;
      if ( (i+1) % buf_len == 0 )
	std::cout << std::endl;

      if ( (i+1) % (buf_len*2) == 0 )
	std::cout << std::endl << std::endl;
#else
      //std::cout << std::endl;
#endif 
    }
    std::cout << std::endl;
    std::cout << std::endl;
  }
private:
  uint8_t* EEPROM_MEM;
};

#endif

class EEPROMWearLeveler
{
 public:
  EEPROMWearLeveler( uint16_t num_of_vars );

  void clear();
  uint8_t read(uint16_t);
  void write(uint16_t, uint8_t);
 private:
  uint16_t findNextWriteAddress( int address );   
  uint16_t parameterBufferAddress( int address );
  uint16_t statusBufferAddress( int address ); 

  uint16_t _num_of_vars;
  uint16_t _buffer_len;

};

#endif
