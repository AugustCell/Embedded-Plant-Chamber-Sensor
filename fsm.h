//***************************************************************************
//
// File Name            : fsm.h
// Title                : Header file for  module
// Date                 : 04/07/10
// Version              : 1.0
// Target MCU           : ATmega128 @  MHz
// Target Hardware      ; 
// Author               : Augusto Celis / Michael Anderson
// DESCRIPTION
// This file includes all the declaration the compiler needs to 
// reference the functions and variables written in the files fsm_table.c
// and fsm_ui.c
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

//This will be the enum for the state variable and key variable
typedef enum { idle_dsp, options, set_time, choose_time_alarm, 
show_alarm_setting, show_instr} state;

//typedef unsigned char key;
typedef enum { one, two, three, up, four, five, six, down, seven, eight, nine,
second, clear, zero, help, enter, eol } key;

//This will extern the present state so we can continually poll that from other 
//functions in our code.
extern state present_state;

//This will extern the function to actually use.
extern void fsm(state, key);

extern unsigned char keyConversion;

//Functions called by fsm during state transitions
extern void dsp_options_screen();
extern void dsp_instr_screen();
extern void toggle_alarm_enable(); //Print and enable
extern void dsp_time_temp_rh();
extern void dsp_set_time();
extern void dsp_time_alarm_choice();
extern void set_system_time();
extern void set_system_alarm();
extern void dsp_enter_time();
extern void invalid_time_entry();
extern void invalid_time_alarm_choice();
extern void invalid_key();
extern void scroll_dsp_down();
extern void scroll_dsp_up();