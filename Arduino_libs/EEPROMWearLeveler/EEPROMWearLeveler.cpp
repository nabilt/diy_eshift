/*
 *  EEPROMWearLeveler.cpp
 *  Created by Nabil Tewolde, May 14, 2012.
 *  Released into the public domain.
 * 
 *  This library performs EEPROM wear leveling by spreading out writes across
 *  multiple address by following the techniques documented by Atmel
 *  (http://www.atmel.com/Images/doc2526.pdf). It is not memory efficient,
 *  but provides guaranteed reliability for a variable number of paramaters.
 *
 *  Facts:
 *
 *  AVR_EEPROM_SIZE/4 is the maximum number of variables that can be used with a
 *  guarantee of 200 000 erase/write cycles each. Any more than that and the 
 *  library reverts back to one variable per address space providing no benefit
 *
 *  The maximum number of writes with 1 variable is 100 000 * (AVR_EEPROM_SIZE/2)
 *
 *  If the EEPROM size is not multiple of the number of variables there will
 *  be wasted space.
 *
 *
 *  The EEPROM in AVRs are rated for 100 000 erase/write cycles. By alternating 
 *  writes across two memory address this number increase to 200 000. Three 
 *  memory addresses gives you 300 000 writes and so on. To automate this 
 *  process a status buffer is used to keep track of where the next write should
 *  be. The status buffer must also be the same length as the parameter buffer 
 *  because wear leveling must be performed on it as well. When the first value, 
 *  7, is written to the parameter buffer the corresponding element in the 
 *  status buffer is incremented as shown in the figure below.
 *
 *  Note: The status buffer must be initialized to zero by calling clear() before 
 *        any writes occur.
 *
 *      <------------------- EEPROM -------------------->  
 *      0                                               N
 *      -------------------------------------------------
 *          Parameter Buffer    |     Status Buffer     |
 *      -------------------------------------------------
 *
 *      Initial state.
 *      [ 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 ]
 *
 *      First write is a 7. This corresponding position
 *      in the status buffer is incremented.
 *      [ 7 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 ]
 *
 *      A second value, 4, is written to the parameter buffer 
 *      and the second element in the status buffer becomes
 *      the previous element, 1 in this case, plus 1.
 *      [ 7 | 4 | 0 | 0 | 0 | 0 | 1 | 2 | 0 | 0 | 0 | 0 ]
 *
 *      And so on
 *      [ 7 | 4 | 9 | 0 | 0 | 0 | 1 | 2 | 3 | 0 | 0 | 0 ]
 *
 *  
 *  If the previous element + 1 does not equal the next element then that is
 *  where the next write should occur. Take a look at some examples.
 * 
 *      Compute the differences by starting at the first
 *      element and wrapping around. 
 *      1st element: 0 + 1 = 1 = 1st element (move on)
 *      2nd element: 1 + 1 = 2 = 2nd element (move on)
 *      3rd element: 2 + 1 = 3 = 3rd element (move on)
 *      4th element: 3 + 1 = 4 != 4th element (next write occurs here)
 *
 *      [ 7 | 4 | 9 | 0 | 0 | 0 | 1 | 2 | 3 | 0 | 0 | 0 ]
 *                                            ^
 *                                            |
 *
 *      This also works on the first element because it is
 *      a circular buffer.
 *      x = don't care
 *      1st element: 6 + 1 = 7 != first element (next write occurs here)
 *
 *      [ x | x | x | x | x | x | 1 | 2 | 3 | 4 | 5 | 6 ]
 *                                ^
 *                                | 
 *
 *      Another edge case to consider is when the incrementing values
 *      wrap around at 256 because we are writing bytes. With the
 *      following buffer we know the next write is at the 3rd element 
 *      because 255 + 1 = 0 != 250 assuming we are using byte arithmetic.
 *      
 *      [ x | x | x | x | x | x | 254 | 255 | 250 | 251 | 252 | 253 ]
 *                                             ^
 *                                             | 
 *      
 *      After we write at element 3 the status buffer is incremented 
 *      to the next value using byte arithmetic and looks like this.
 *      255 + 1 = 0
 *      [ x | x | x | x | x | x | 254 | 255 |  0  | 251 | 252 | 253 ]
 *
 *      Doing the math we see that element 4 is next to be written to.
 *      1st element: 253 + 1 = 254 = 1st element (move on)
 *      2nd element: 254 + 1 = 255 = 2nd element (move on)
 *      3rd element: 255 + 1 = 0 = 3rd element (move on)
 *      4rd element: 0 + 1 = 1 != 4th element  (next write occurs here)
 *                                             ^
 *                                             |

 [ x | x | x | x ^ x | x | 1 | 2 ^ 3 | 4 | 5 | 6 ]

 */

//#include "Arduino.h"
//#include <../EEPROM/EEPROM.h>
#include "EEPROMWearLeveler.h"

#ifdef DEBUG
EEPROMClass EEPROM;
#endif

int AVR_EEPROM_SIZE = 32;

EEPROMWearLeveler::EEPROMWearLeveler( uint16_t num_of_vars )
{
  _num_of_vars = num_of_vars;

  _buffer_len = ( AVR_EEPROM_SIZE / 2 ) / _num_of_vars;

  //if ( (_buffer_len % 256) == 0 )
  //  _buffer_len -= 1;

  if ( _buffer_len <= (uint16_t)1 )
  {
    // Revert back to using the entire EEPROM since we would get
    // no benefits with a buffer length of 1
    _num_of_vars = AVR_EEPROM_SIZE;
    _buffer_len = 1;
  }

  std::cout << "_buffer_len: " << _buffer_len << std::endl;
  std::cout << "_num_of_vars: " << _num_of_vars << std::endl;
  std::cout << std::endl;
    
}

void EEPROMWearLeveler::clear()
{
  for (int i = 0; i < AVR_EEPROM_SIZE; i++)
  {
    EEPROM.write(i, 0);
  }  
}

uint8_t EEPROMWearLeveler::read( uint16_t address )
{
  if ( _num_of_vars == AVR_EEPROM_SIZE )
  {
    // Revert back to EEPROM class if the nubmer of variables is
    // greater than AVR_EEPROM_SIZE/4
    return EEPROM.read( address );
  }
  else
  {
    uint16_t write_offset = findNextWriteAddress( address );
    uint16_t pb_start_addr = parameterBufferAddress( address );

    uint16_t read_addr = pb_start_addr + write_offset;
    if ( read_addr == pb_start_addr )
    {
      std::cout << read_addr << " BBBBBBBB\n";      
      read_addr += _buffer_len - 1;
    }
    else
    {
      read_addr -= 1;
      std::cout << "AAAAAA\n";
    }
    std::cout << "read address: " << read_addr << std::endl;
    std::cout << "pb_start_addr: " << pb_start_addr << std::endl;
    std::cout << "write_offset: " << write_offset << std::endl;
    std::cout << "address: " << address << std::endl;

    return EEPROM.read( read_addr );  
  }
}

uint16_t EEPROMWearLeveler::statusBufferAddress( int address ) 
{
  return ( ( (address + 1) * _buffer_len * 2 ) - _buffer_len );  
}

uint16_t EEPROMWearLeveler::parameterBufferAddress( int address ) 
{
  return ( address * _buffer_len * 2 ) ;  
}

uint16_t EEPROMWearLeveler::findNextWriteAddress( int address )
{
  uint16_t sb_start_addr = statusBufferAddress( address );
  std::cout << "sb_start_addr: " << (int)sb_start_addr << std::endl;

  uint16_t i;
  for ( i = sb_start_addr; i < ( _buffer_len + sb_start_addr ); i++ )
  {
    uint16_t prev_index;
    // Wrap around case
    if ( i == sb_start_addr )
    {
      prev_index = sb_start_addr + _buffer_len - 1;
    }
    else
      prev_index = i - 1;

    uint8_t prev_elem = EEPROM.read( prev_index );
    uint8_t curr_elem = EEPROM.read( i );

#if 0
    std::cout << "prev_index: " << (int)prev_index << std::endl;
    std::cout << "prev_elem: " << (int)prev_elem << std::endl;
    std::cout << "curr_index: " << (int)i << std::endl;
    std::cout << "curr_elem: " << (int)curr_elem << std::endl << std::endl;
#endif

    // Must truncate the addition because the index tracking relies of wrap around
    if ( ( ( prev_elem + 1 ) & 0xFF ) != curr_elem )
    {
      // Return the relative write position
      uint16_t offset = i - sb_start_addr;

      std::cout << "prev_elem + 1: " << prev_elem + 1 << " curr_elem: " << (int)curr_elem << std::endl;
      std::cout << "prev_index: " << (int)prev_index << std::endl;
      std::cout << "prev_elem: " << (int)prev_elem << std::endl;
      std::cout << "curr_index: " << (int)i << std::endl;
      std::cout << "curr_elem: " << (int)curr_elem << std::endl ;
      
      std::cout << "offset: " << (int)offset << std::endl << std::endl;
      
      return offset;      
    }
  }

  std::cout << "ERROR!!!!!!!!!!!!!!!!!!!!" << std::endl;
  //exit(0);
  // Should never get here. Just in case return first index
  return 0;
}

void EEPROMWearLeveler::write( uint16_t address, uint8_t value )
{
  if ( _num_of_vars == AVR_EEPROM_SIZE )
  {
    // Revert back to EEPROM class if the nubmer of variables is
    // greater than AVR_EEPROM_SIZE/4
    EEPROM.write( address, value );
  }
  else
  {
    // bounds check
    if ( address >= _num_of_vars )
    {
      std::cout << " BAD address \n";           
      return;
    }
    uint16_t write_offset = findNextWriteAddress( address );
    std::cout << "write_offset: " << write_offset << std::endl;

    uint16_t pb_start_addr = parameterBufferAddress( address );
    std::cout << "pb_start_addr: " << pb_start_addr << std::endl;

    std::cout << "write address: " <<  pb_start_addr + write_offset << std::endl;

    // Write value to paramater buffer
    EEPROM.write( pb_start_addr + write_offset, value );
  
    // Update status buffer 
    uint16_t sb_start_addr = statusBufferAddress( address );
    uint16_t curr_index = sb_start_addr + write_offset;
    uint16_t prev_index;

    std::cout << "curr_index: " <<  (int)curr_index << std::endl;
    std::cout << "sb_start_addr: " <<  (int)sb_start_addr << std::endl;
    std::cout << "write_offset: " <<  (int)write_offset << std::endl;
    
    // Wrap around case
    if ( curr_index == sb_start_addr )
      prev_index = sb_start_addr + _buffer_len - 1;
    else
      prev_index = curr_index - 1;

    uint16_t sb_val = EEPROM.read(prev_index) + 1;
    std::cout << "write status buffer at: " <<  (int)curr_index << std::endl;
    std::cout << "\tstatus buffer value: " <<  (int)sb_val << std::endl;

    EEPROM.write( curr_index, sb_val );
  }
}
