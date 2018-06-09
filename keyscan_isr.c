//******************************************************************************
//
// File Name            : keyscan_isr.c
// Title                : Keypad scan interrupt service routine
// Date                 : 02/07/10
// Version              : 1.0
// Target MCU           : ATmega128 @  MHz
// Target Hardware      ; 
// Author               : Ken Short
// DESCRIPTION
// When keypad interrupt occurs, key matirx is scanned and is encoded using
// a table lookup. The keypad is connected to PORTC. See diagram in laboratory
// description. 
//
// Warnings             : none
// Restrictions         : none
// Algorithms           : none
// References           : none
//
// Revision History     : Initial version 
// 
//
//******************************************************************************

//#define debug   //this can be uncommented to remove delays for simulation

#include <iom128.h>         //Atmega128 definitions
#include <intrinsics.h>     //Intrinsic functions.
#include <avr_macros.h>     //Useful macros.
#include <stdio.h>
#include "keypad.h"
#include "fsm.h"

char keycode;

/*
* Port pin numbers for columns and rows of the keypad
*/
//PORT Pin Definitions.
#define COL1  7   //pin definitions for PortB
#define COL2  6
#define COL3  5
#define COL4  4
#define ROW1  3
#define ROW2  2
#define ROW3  1
#define ROW4  0

#define INT0  0   //pin definitions for PortD

/*
* Function declarations.
*/
void check_release(void);

/*
* Lookup table declaration
*/
const char tbl[16] = {1, 2, 3, 0, 4, 5, 6, 0, 7, 8, 9, 0, 0, 0, 0, 0};


const key key_sequence[] = { one, two, three, up, four, five, six, down,
seven, eight, nine, second, clear, zero, help, enter, eol};
/*
* Interrupt service routine
*/
#pragma vector=INT0_vect          // Declare vector location.
__interrupt void ISR_INT0(void)   // Declare interrupt function
{
  
  //Note: TESTBIT returns 0 if bit is not set and a non-zero number otherwise.
  
  if(!TESTBIT(PINC,ROW1))         //Find Row of pressed key.
    keycode = 0;
  else if(!TESTBIT(PINC,ROW2))
    keycode = 4;
  else if(!TESTBIT(PINC,ROW3))
    keycode = 8;
  else if(!TESTBIT(PINC,ROW4))
    keycode = 12;
  
  DDRC = 0x0F;                    //Reconfigure PORTC for Columns.
  PORTC = 0xF0;
  
#ifndef debug
  __delay_cycles(256);            //Let PORTC settle.
#endif
  
  
  if(!TESTBIT(PINC,COL1))         //Find Column.
    keycode += 0;
  else if(!TESTBIT(PINC,COL2))
    keycode += 1;
  else if(!TESTBIT(PINC,COL3))
    keycode += 2;
  else if(!TESTBIT(PINC,COL4))
    keycode += 3;
  
  DDRC = 0xF0;                    //Reconfigure PORTC for Rows.
  PORTC = 0x0F;
  
  check_release();                //Wait for keypad release.
 
  keyConversion = (tbl[keycode]);  
 
  key key_entered = (key_sequence[keycode]);
  
  fsm(present_state, key_entered);    //Go through the fsm to register the 
  //next state and keycode
  
}

/*
*  Check keypad is released and not bouncing.
*/
void check_release(void)
{
#ifndef debug
  while(!TESTBIT(PIND,INT0));     //Check that keypad key is released.
  
  __delay_cycles(8000000);          //Delay (.05secs) / (1 / 1MHz) cycles.
  
  while(!TESTBIT(PIND,INT0));     //Check that key has stopped bouncing.
#endif
  return;
}
