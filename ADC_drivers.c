#include <stdio.h>
#include <iom128.h>
#include <intrinsics.h>
#include <avr_macros.h>

//******************************************************************************
// Function : void ADC_single_conversion(void)
// Date and version : 4/22/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This will simply set the ADSC bit so that an ADC conversion can start through
// the system. This will actually help make the conversion
//
//******************************************************************************
           
void ADC_single_conversion(){
  SETBIT(ADCSRA, ADSC);         //Simply set the port that starts the conversion
}


//******************************************************************************
// Function : void ADC_config(unsigned char channel)
// Date and version : 4/22/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This will be the setup for our ADC. This system will end up setting up to use
// interrupts, delay just in case there is a conversion problem, set up the 
// MUX to read signals from the correct ADC port, and start a single conversion 
// because the first conversion after single measurmeent setup is not correct.
//
//******************************************************************************
void ADC_config(unsigned char channel){
  //This will set up the interrupt enable, clear out the interrupt
  //flag, enable the adc itself, and set up the prescalar of 128 due 
  //to the high clock frequency from the ATMega128
  ADCSRA = (1 << ADEN) | (0 << ADSC) | (0 << ADFR) | (0 << ADIF)
    |(1 << ADIE) | (1 << ADPS2) | ( 1 << ADPS1) | ( 1 << ADPS0);
  
  //Delay in case 
  __delay_cycles(4000);
  
  //This will set up the ADC7 to be the negative input for the ADC, 
  //and will help us use an external AVCC with a capacitor at AREF
  ADMUX = (0 << MUX4) | (0 << MUX3) | (1 << MUX2) | 
           (1 << MUX1) | (1 << MUX0) | (0 << REFS1) | (1 << REFS0);

ADC_single_conversion();        //Get rid of the first cycle of ADC cycles read
}


