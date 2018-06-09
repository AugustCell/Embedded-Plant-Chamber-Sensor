//***************************************************************************
//
// File Name            : lcd.h
// Title                : Header file for LCD module
// Date                 : 02/07/10
// Version              : 1.0
// Target MCU           : ATmega128 @  MHz
// Target Hardware      ; 
// Author               : Ken Short
// DESCRIPTION
// This file includes all the declaration the compiler needs to 
// reference the functions and variables written in the files lcd_ext.c.
// lcd.asm and lcd_dog_iar_driver.asm
//
// Warnings             : none
// Restrictions         : none
// Algorithms           : none
// References           : none
//
// Revision History     : Initial version 
// 
//
//**************************************************************************

/**
 *  To use the lcd functions in lcd_dog_iar_driver.asm lcd_ext.c all that is 
 *  needed is to include this file.
 */

/**
 *  This declaration tells the compiler to look for dsp_buff_x in
 *  another module. It is used by lcd_ext.c and main.c to locate the buffers.
 */
extern char dsp_buff_1[16];
extern char dsp_buff_2[16];
extern char dsp_buff_3[16];

/**
 *  Declaratios of low level lcd functions located in lcd_dog_iar_driver.asm
 *  Note that these are external.
 */
__version_1 extern void init_lcd_dog(void);
__version_1 extern void update_lcd_dog(void);
__version_1 extern void init_spi_lcd(void);

/**
 *  These functions are located in lcd_ext.c
 */
extern void clear_dsp(void);
extern int putchar(int);



