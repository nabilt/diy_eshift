#include "EEPROMWearLeveler.h" 
#include <assert.h>

extern EEPROMClass EEPROM;
extern int AVR_EEPROM_SIZE;
int main()
{
  for ( int esize = 1024; esize < 4096; esize *= 2 )
  {
    //int esize = 128;
    AVR_EEPROM_SIZE = esize;
    EEPROM.change_size(AVR_EEPROM_SIZE);

    for ( int num_of_var = 1; num_of_var < esize; num_of_var++ )
    {  
      //int num_of_var = 13;
      EEPROMWearLeveler e(num_of_var);    
      e.clear();
      for ( int i = 0; i < 1000; i++ )
      {
	uint16_t write_val = i%255;
	uint16_t write_addr = i%num_of_var;

	std::cout << "----------------------------\n";
	e.write(write_addr, write_val);
	EEPROM.print();  

	int read_val = static_cast<int>(e.read(write_addr));

	std::cout << "write val: " << write_val << " at rel addr: " << write_addr << std::endl;
	std::cout << "read val: " << read_val << std::endl;

	if (read_val != write_val)
	{
	  std::cout << "ERRRROR: " << read_val << " != " << write_val << std::endl;
	  std::cout << "esize: " << esize << std::endl;
	  std::cout << "num_of_var: " << num_of_var << std::endl;
	  return 1;
	}
      }
    }
  }
}
