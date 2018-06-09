#include <iom128.h>
#include <intrinsics.h>
#include <avr_macros.h>
#include "humidicon.h"

#define HUMIDICON_SELECT 0
#define SS_BAR 0

//These will be the four local bytes of the humidity and the temperature
unsigned int humidicon_byte1;
unsigned int humidicon_byte2; 
unsigned int humidicon_byte3;
unsigned int humidicon_byte4;

//These will be the local raw values of the humidity and temperature
unsigned int humidity_raw;
unsigned int temperature_raw;

float humidity;
float temperature;
 
char degree_char = 0xDF;

//******************************************************************************
// Function : void SPI_humidicon_config (void)
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author :     Augusto Celis / Michael Anderson
//
// DESCRIPTION
// This function unselects the HumidIcon and configures it for operation with
// an ATmega128A operated a 16 MHz. Pin PA0 of the ATmega128A is used to select
// the HumidIcon
//
//******************************************************************************
void SPI_humidicon_config(){
    SETBIT(PORTA, HUMIDICON_SELECT); //This will unselect the humidicon
    //This will set up the SPI for the humidicon including its frequency
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPOL) | (1 << CPHA) | (1 << SPR1) | 
      (0 <SPR0);
    SPSR = (1 << SPI2X);         
    
    char kill = SPSR;           //Clear out the SPIF flag
    kill = SPDR;                //SPIF will indicate 
}

//******************************************************************************
// Function : unsigned char read_humidicon_byte(void)
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author :     Augusto Celis / Michael Anderson
//
// DESCRIPTION
// This function reads a data byte from the HumidIcon sensor and returns it as
// an unsigned char. The function does not return until the SPI transfer is
// completed. The function determines whether the SPI transfer is complete
// by polling the appropriate SPI status flag.
//
//******************************************************************************
unsigned char read_humidicon_byte(){
    unsigned char temp = 0xFF;          //This will be a dummy value
    CLEARBIT(PORTA, HUMIDICON_SELECT);  //This will send a SS byte to humidicon
    SPDR = temp;                        //This will write to the data register 
    while(!(SPSR & 1<<SPIF)){           //This will wait forever        
      //Do nothing
    }
    return SPDR;                        //Return the byte received in SPDR
    
}

//******************************************************************************
// Function : void read_humidicon (void)
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author :     Augusto Celis / Michael Anderson

// DESCRIPTION
// This function selects the Humidicon by asserting PA0. It then calls
// read_humidicon_byte() four times to read the temperature and humidity
// information. Is assigns the values read to the global unsigned ints 
// humidicon_byte1, humidion_byte2, humidion_byte3, and humidion_byte4, 
// respectively. The function then deselects the HumidIcon.
//
// The function then extracts the fourteen bits corresponding to the 
// humidity information and stores them right justified in the global unsigned 
// int humidity_raw. Next if extracts the fourteen bits corresponding to 
// the temperature information and stores them in the global unsigned int
// temperature_raw. The function then returns
//
//******************************************************************************
void read_humidicon(){            
    
    //This will read the 4 bytes, 2 for humidity and 2 for temperature. 
    humidicon_byte1 = (int)read_humidicon_byte(); 
    __delay_cycles(16*36650);
    humidicon_byte2 = (int)read_humidicon_byte();
    humidicon_byte3 = (int)read_humidicon_byte();
    humidicon_byte4 = (int)read_humidicon_byte();
    
    //These next 2 lines will shift over the bits appropriately, mask, 
    //and combine them into one integer value
    humidity_raw = ((humidicon_byte1 & 0x3F) << 8) | (humidicon_byte2); 
    temperature_raw = (humidicon_byte3 << 6) | (humidicon_byte4 >> 2); 
    
    //This will set the values of humidity and temperature
    humidity = compute_scaled_rh(humidity_raw);
    temperature = compute_scaled_temp(temperature_raw);
    
    //This will deselect the humidicon
    SETBIT(PORTA, HUMIDICON_SELECT);
   
}

//******************************************************************************
// Function : unsigned int compute_scaled_rh(unsigned int rh)
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderson
//
// DESCRIPTION
// Computess scaled relative humidity in units of 0.01% RH from the raw 14-bit
// realtive humidity value from the Humidicon.
//
//******************************************************************************
float compute_scaled_rh(unsigned int rh){
  unsigned int denominator = 16382;     //2 ^ 14 - 2
  float change;
  float result;
  
  change = ((float) rh) * 100;
  result = ((float) change) / denominator;
  
  //(Humidity Output / denominator) * 100%
  return result;
}

//******************************************************************************
// Function : unsigned int compute_scaled_temp(unsigned int temp)
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// Computess scaled temperature in units of 0.01 degrees C from the raw 14-bit
// temperature value from the Humidicon
//
//******************************************************************************
float compute_scaled_temp(unsigned int temp){
  unsigned int denominator = 16382;     //2 ^ 14 - 2
  float change;
  float result;
  
  change = ((float) temp) * 165;
  change /= ((float) denominator);
  result = ((float) change) - 40;
    
  //(Temperature Output / denominator) * 165 - 40
  return result;
}
