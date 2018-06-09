//***************************************************************************
//
// File Name            : humidicon.h
// Title                : Header file for Honeywell Humidicon module
// Date                 : 04/07/10
// Version              : 1.0
// Target MCU           : ATmega128 @  16MHz
// Author               : Augusto Celis / Michael Anderson
//
// DESCRIPTION
// This file includes all the declaration the compiler needs to 
// reference the functions and variables written in the file humidicon.c
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

//This will be the humidity and temperature extern values for 
//the driver and the main
extern float humidity;
extern float temperature;

//This will help to get external functions from out humidicon drivers
extern void SPI_humidicon_config();
extern void read_humidicon();

//These are methods from the main used to compute the actual temperature
//and humidity of the system
extern float compute_scaled_rh(unsigned int rh);
extern float compute_scaled_temp(unsigned int temp);
extern void meas_display_rh_temp();

//The hex value for the degree character to display on our LCD screen
extern char degree_char;      
