//***************************************************************************
//
// File Name            : DS1306_RTC.h
// Title                : Header file for  module
// Date                 : 04/07/10
// Version              : 1.0
// Target MCU           : ATmega128 @  MHz
// Author               : Augusto Celis / Michael Anderson
// DESCRIPTION
// This file includes all the declaration the compiler needs to 
// reference the functions and variables written in the file 
// DS1306_RTC_drivers.c
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

//This is the select of the RTC
#define SS_RTC 1

//This will have the beggining values for the read and write of the RTC
#define WRITE_LOCATION 0xA0
#define READ_LOCATION  0x20

//This is the status register read and write values
#define STAT_REG_WT  0x90
#define STAT_REG_RD  0x10

//This is the contro register read and write values
#define CONT_REG_WT  0x8F
#define CONT_REG_RD  0x0F

//This will help to write to all the registers of the RTC including seconds,
//minutes, hours, date, month, and year
#define SEC_WT   0x80
#define MIN_WT   0x81
#define HR_WT    0x82
#define DATE_WT  0x84
#define MONTH_WT 0x85
#define YEAR_WT  0x86     

//This will help to read from all the registers of the RTC including seconds,
//minutes, hours, date, month, and year
#define SEC_RD   0x00
#define MIN_RD   0x01
#define HR_RD    0x02
#define DATE_RD  0x04
#define MONTH_RD 0x05
#define YEAR_RD  0x06  

//This will help to write to all the registers of the RTC alarm including 
//seconds, minutes, hours and day
#define SEC_ALM_WT  0x87
#define MIN_ALM_WT  0x88
#define HR_ALM_WT   0x89
#define DAY_ALM_WT  0x8A

//This will help to read from all the registers of the RTC alarm including 
//seconds, minutes, hours and day
#define SEC_ALM_RD  0x07
#define MIN_ALM_RD  0x08
#define HR_ALM_RD   0x09
#define DAY_ALM_RD  0x0A

//This simply defines the hex value for the RTC 
#define ARROW 0x7E

#define set_alarm_seconds 0x80

//These are the functions that we are using from DS1306_RTC_drivers.c
extern void SPI_rtc_ds1306_config();
extern void write_RTC(unsigned char reg_RTC, unsigned char data_RTC);
extern unsigned char read_RTC(unsigned char reg_RTC);
extern void write_read_RTC_test(void);
extern void block_read_RTC(volatile unsigned char *array_ptr, 
                           unsigned char start_addr, unsigned char count);
extern void block_write_RTC(volatile unsigned char *array_ptr, 
                            unsigned char start_addr, unsigned char count);

extern void read_time_RTC(void);
extern void format_time(void);

//These are variables declared that need to be read from our FSM as well.
extern volatile unsigned char RTC_time_date_write[7];
extern volatile unsigned char RTC_time_date_read[7];
extern void block_write_read_test(void);

extern unsigned char seconds;
extern unsigned char minutes;
extern unsigned char hours;

extern unsigned char alarm_seconds;
extern unsigned char alarm_minutes;
extern unsigned char alarm_hours;

extern unsigned char seconds_RTC;
extern unsigned char minutes_RTC;
extern unsigned char hours_RTC;

extern unsigned char hours_tens;
extern unsigned char hours_ones;
extern unsigned char minutes_tens;
extern unsigned char minutes_ones;

