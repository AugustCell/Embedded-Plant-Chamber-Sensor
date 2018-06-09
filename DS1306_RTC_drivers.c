#include <avr_macros.h>
#include <iom128.h>
#include <intrinsics.h>
#include "DS1306_RTC.h" 

unsigned char RTC_byte[10];
volatile unsigned char RTC_time_date_write[7];
volatile unsigned char RTC_time_date_read[7];

unsigned char seconds;
unsigned char minutes;
unsigned char hours;

unsigned char alarm_seconds;
unsigned char alarm_minutes;
unsigned char alarm_hours;

unsigned char seconds_RTC;
unsigned char minutes_RTC;
unsigned char hours_RTC;

unsigned char hours_tens;
unsigned char hours_ones;
unsigned char minutes_tens;
unsigned char minutes_ones;


//******************************************************************************
// Function : void SPI_rtc_ds1306_config (void)
// Date and version : 03/11/18, version 1.0
// Target MCU : ATmega128 @ 16MHz
// Author : Augusto Celis / Micahel Anderson
//
// DESCRIPTION
// This function unselects the ds_1306 and configures an ATmega128 operated at
// 16 MHz to communicate with the ds1306. Pin PA1 of the ATmega128 is used to
// select the ds_1306. SCLK is operated a the maximum possible frequency for
// the ds1306.
//
//******************************************************************************
void SPI_rtc_ds1306_config(){
  CLEARBIT(PORTA, 1);            //Deselect the RTC
  SPCR = (1 << SPE) | (1 << MSTR) | (0 << CPOL) | ( 1 << CPHA) | 
    (0 << SPR1) | (1 <<SPR0);
  SPSR = (1 <<SPI2X);
}

//******************************************************************************
// Function : void write_RTC (unsigned char reg_RTC, unsigned char data_RTC)
// Date and version : 03/11/18, version 1.0
// Target MCU : ATmega128 @ 16MHz
// Author : Augusto Celis / Michael Anderson
//
// DESCRIPTION
// This function writes data to a register in the RTC. To accomplish this, it
// must first write the register's address (reg_RTC) followed by writing the
// data (data_RTC). In the DS1306 data sheet this operation is called an SPI
// single-byte write.
//
//******************************************************************************
void write_RTC(unsigned char reg_RTC, unsigned char data_RTC){
  //Select the RTC
  SETBIT(PORTA, SS_RTC);
  
  //Delay for CE setup
  __delay_cycles(18);
  
  //Begin transmission
  SPDR = reg_RTC;
  
  //Wait for SPIF flag to be set
  while(!(SPSR & (1 << SPIF)));
  
  //Send the data byte
  SPDR = data_RTC;
  
  //wait for transmission
  while(!(SPSR & (1 << SPIF)));
  
  //Wait for the end of transmission
  __delay_cycles(2);
  
  //Deselect the slave
  CLEARBIT(PORTA, SS_RTC);
  
  //Wait for the slave to become inactive
  __delay_cycles(20);
  
}

//******************************************************************************
// Function Name : unsigned char read_RTC (unsigned char reg_RTC)
// Date and version : 03/11/18, version 1.0
// Target MCU : ATmega128 @ 16MHz
// Author : Augusto Celis / Micahel Anderson
//
// DESCRIPTION
// This function reads data from a register in the RTC. To accomplish this, it
// must first write the register's address (reg_RTC) followed by writing a dummy
// byte to generate the SCLKs to read the data (data_RTC). In the DS1306 data
// sheet this operaration is called am SPI single-byte read.
//
//******************************************************************************
unsigned char read_RTC(unsigned char reg_RTC){
  //SET THE RTC
  SETBIT(PORTA, SS_RTC);
  
  //clock enable setup
  __delay_cycles(17);
  
  //send a dummy value
  SPDR = reg_RTC;
  
  //wait for transmission
  while(!(SPSR & (1 << SPIF)));
  
  //send a dummy value
  SPDR = 0x77;
  
  //wait for transmission
  while(!(SPSR & (1 << SPIF)));
  
  
  //values in temp
  unsigned char temp;
  temp = SPDR;
  
  //wait for end of full transmission
  __delay_cycles(2);
  
  //unasser the slave
  CLEARBIT(PORTA, SS_RTC);
  
  //wait for full change 
  __delay_cycles(20);
  
  return temp;

}


//******************************************************************************
// Function Name : void write_read_RTC_test(void)
// Date and version : 03/11/18, version 1.0
// Target MCU : ATmega128 @ 16MHz
// Author : Augusto Celis / Michael Anderson
//
// DESCRIPTION
// This function writes a byte to the NV RAM and then it reads back the location
// just written and places the result in a global array named RTC_byte[]. The
// function repeats this write/read sequencye 10 times. The locations written 
// are 0xA0 through 0xA9 and the corresponding locations read are 0x20 
// through 0x29.
//
//******************************************************************************
void write_read_RTC_test(){
  unsigned char temp = 0x2B;
  for(int i = 0; i < 9; i++){
    write_RTC((WRITE_LOCATION + i), temp);
    RTC_byte[i] = read_RTC((READ_LOCATION + i));
  }
}


//******************************************************************************
// Function Name : "block_write_RTC"
// void block_write_RTC (volatile unsigned char *array_ptr,
// unsigned char strt_addr, unsigned char count)
// Date and version : 03/11/18, version 1.0
// Target MCU : ATmega128 @ 16MHz
// Author : Augusto Celis / Michael Anderson
//
// DESCRIPTION
// This function writes a block of data from an array to the DS1306. strt_addr
// is the starting address in the DS1306. count is the number of data bytes to
// be transferred and array_ptr is the address of the source array.
//
//******************************************************************************
void block_write_RTC(volatile unsigned char *array_ptr, unsigned char strt_addr,
                     unsigned char count){
  
  SETBIT(PORTA, SS_RTC);
  __delay_cycles(17);
  SPDR = strt_addr;
  while(!(SPSR & (1 << SPIF)));
  for(int i = 0; i < count; i++){
    SPDR = array_ptr[i];
  while(!(SPSR & (1 << SPIF)));
  }
  __delay_cycles(2);
  CLEARBIT(PORTA, SS_RTC);
  __delay_cycles(20);
}

//******************************************************************************
// Function Name : "block_read_RTC"
// void block_read_RTC (volatile unsigned char *array_ptr,
// unsigned char strt_addr, unsigned char count)
// Date and version : 03/11/18, version 1.0
// Target MCU : ATmega128 @ 16MHz
// Author : Augusto Celis / Michael Anderson
//
// DESCRIPTION
// This function reads a block of data from the DS1306 and transfers it to an
// array. strt_addr is the starting address in the DS1306. count is the number
// of data bytes to be transferred and array_ptr is the address of the
// destination array.
//
//*****************************************************************************
void block_read_RTC(volatile unsigned char *array_ptr, unsigned char start_addr,
                    unsigned char count){
  SETBIT(PORTA, SS_RTC);
  
  __delay_cycles(17);
  
  SPDR = start_addr;
  while(!(SPSR & (1 << SPIF)));
  
  for(int i=0; i<count; i++) {
    SPDR = 0xB7;
    while(!(SPSR & (1 << SPIF)));
    array_ptr[i] = SPDR;
  }
  
  __delay_cycles(2);
  CLEARBIT(PORTA, SS_RTC);
  __delay_cycles(20);
}

//******************************************************************************
// Function Name : "read_time_RTC()"
// Target MCU : ATmega128 @ 16MHz
// Author : Augusto Celis / Michael Anderson
//
// DESCRIPTION
// This function reads the hours, minutes, and seconds from the respective
// registers, and assigns them to external variables. Thexe external variables
// are hours, minutes, and seconds
//
//******************************************************************************
void read_time_RTC(void){
  hours_RTC = read_RTC(HR_RD);
  minutes_RTC = read_RTC(MIN_RD);
  seconds_RTC = read_RTC(SEC_RD);
}

//******************************************************************************
// Function Name : "format_time()"
// Target MCU : ATmega128 @ 16MHz
// Author : Augusto Celis / Michael Anderson
//
// DESCRIPTION
// This function will format the hours, minutes, and seconds before reading
// into the printf statement. We shift around each register is respective tens
// and ones places, to finalize with an integer value. 
//
//******************************************************************************
void format_time(void){
  //ones and tens will continually
  //be manipulated to get the value 
  //from the RTC registers
  unsigned char ones;
  unsigned char tens;
  
  ones = (seconds_RTC & 0x0F);
  tens = (seconds_RTC & 0xF0);
  tens >>= 4;
  tens *= 10;
  seconds = (tens + ones);
  
  ones = (minutes_RTC & 0x0F);
  tens = (minutes_RTC & 0xF0);
  tens >>= 4;
  tens *= 10;
  minutes = (tens + ones);
  
  ones = (hours_RTC & 0x0F);
  tens = (hours_RTC & 0xF0);
  tens >>= 4;
  tens *= 10;
  hours = (tens + ones);
}


void format_alarm_time(void){
  
  
}
