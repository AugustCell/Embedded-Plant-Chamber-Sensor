#include <stdio.h>
#include <iom128.h>
#include <intrinsics.h>
#include <avr_macros.h>
#include "lcd.h"
#include "ADC.h"

int value;

/*
*Interrupt that is set off by the conversion end of 
*the ADC itself. This will setup the value which 
*will be updated ot the LCD screen
*/
#pragma vector = ADC_vect
__interrupt void ISR_ADC(){
  value = (int)ADCH << 8;
  value |= (int)ADCL;
  
}


void main(){
  // Configure PortA for selects of the humidicon and RTC
  DDRA = 0xFF;
  SETBIT(PORTA, 0);     //Select for humidicon (Unassert)
  CLEARBIT(PORTA, 1);   //Select for RTC (Unassert)
  
  //Configure PortB for SPI
  DDRB = 0xF7;          //SCK, MISO, MOSI setup
  SETBIT(PORTB, 0);     //Select for LCD (Unassert)
  
  // Configure PortC for keypad, initial configuration
  DDRC = 0xF0;          // High nibble outputs, low nibble inputs 
  PORTC = 0x0F;      
  
  //Configure PortD for Inerrupts
  DDRD = 0xF0;          //INT0, INT1, INT2
  PORTD = 0x0D;         //Set pullup resistors on INT0 and INT2
  
  //Configure Port F for ADC
  DDRF = 0x00;
  
  //Enable interrupt config
  MCUCR = 0X30;
  EIMSK = 0X07;
  __enable_interrupt();
  
  init_lcd_dog();
  
  //This syetm will init the spi, and then update the lcd with the hex value
  //of the potentiometer
  while(1){
    init_spi_lcd();
    clear_dsp();
    
    printf("Analog Value:\n");
    printf("0x");
    update_lcd_dog();
    
    ADC_single_conversion();    //Actually start the ADC conversion
    printf("%04x", value);      //Show the value that is received from the ADC
  }
}

