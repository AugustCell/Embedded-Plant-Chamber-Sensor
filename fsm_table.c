#include <iom128.h>
#include <intrinsics.h>
#include <avr_macros.h>
#include "fsm.h"

//Global variable for present state of the FSM
state present_state;

//Pointer to task function
typedef void (*task_fn_ptr) ();

//Represent a row on the state transition table
typedef struct{
  key keyval;
  state next_state;
  task_fn_ptr tf_ptr;
} transition;

//Transition table for idle_dsp state
const transition idle_dsp_transitions[] = {
//  INPUT       NEXT_STATE      TASK
  { second,     options,        dsp_options_screen},
  { help,       show_instr,     dsp_instr_screen},
  { up,         idle_dsp,       scroll_dsp_up},
  { down,       idle_dsp,       scroll_dsp_down},
  { eol,        idle_dsp,       invalid_key}
};
  
//Transition table for options state
const transition options_transitions[] = {
//  INPUT       NEXT_STATE      TASK
  { one,        set_time,       dsp_enter_time},
  { two,        show_alarm_setting,     toggle_alarm_enable},
  { eol,        idle_dsp,       invalid_key}
};

const transition set_time_transitions[] = {
  { zero,        set_time,              dsp_set_time},
  { one,        set_time,              dsp_set_time},
  { two,        set_time,              dsp_set_time},
  { three,        set_time,              dsp_set_time},
  { four,        set_time,              dsp_set_time},
  { five,        set_time,              dsp_set_time},
  { six,        set_time,              dsp_set_time},
  { seven,        set_time,              dsp_set_time},
  { eight,        set_time,              dsp_set_time},
  { nine,        set_time,              dsp_set_time},
  { enter,       choose_time_alarm,      dsp_time_alarm_choice},
  { eol,        set_time,              invalid_time_entry},
};

//Transition table for choose_time_alarm state
const transition choose_time_alarm_transitions[] = {
  //  INPUT       NEXT_STATE      TASK
  { one,        idle_dsp,       set_system_time},
  { two,        idle_dsp,       set_system_alarm},
  { eol,        choose_time_alarm,      invalid_time_alarm_choice}
};

//Transition table for show_alarm_setting state
const transition show_alarm_setting_transitions[] = {
//  INPUT       NEXT_STATE      TASK
  { eol,        idle_dsp,       dsp_time_temp_rh}
};

//Transition table for show_instr state
const transition show_instr_transitions[] = {
//  INPUT       NEXT_STATE      TASK
  { eol,        idle_dsp,       dsp_time_temp_rh}
};

//Setup the array with all of the transitions
const transition *ps_transitions_ptr [6] = {
  idle_dsp_transitions,
  options_transitions,
  set_time_transitions,
  choose_time_alarm_transitions,
  show_alarm_setting_transitions,
  show_instr_transitions
};
  
//******************************************************************************
// Function : void fsm(state ps, key keyval
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This will help to figure out where in the fsm we are. This will start off 
// checking if the present state transitions and the value of i keyval does 
// not equal the keyval. This will also check if that keyval does not equal
// eol either. If it doesn't, then continually increase the value of i. Once 
// this is done, the system will look up which transition and method to use by
// going into the transition table, and setting up the present state as the 
// next state of the FSM.
//
//******************************************************************************
void fsm(state ps, key keyval){
  int i;
  for(i = 0; (ps_transitions_ptr[ps][i].keyval != keyval)
     && (ps_transitions_ptr[ps][i].keyval != eol); i++);
  
  ps_transitions_ptr[ps][i].tf_ptr();
  
  present_state = ps_transitions_ptr[ps][i].next_state;
}