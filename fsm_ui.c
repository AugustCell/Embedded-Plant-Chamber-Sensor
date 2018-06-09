#include <stdio.h>
#include <iom128.h>
#include <intrinsics.h>
#include <avr_macros.h>
#include "humidicon.h"
#include "lcd.h"
#include "adc.h"
#include "keypad.h"
#include "DS1306_RTC.h"
#include "fsm.h"

// PAGE_COUNT needs to be updated any time a new device is connected which
// requires a new page to display the information.
#define PAGE_COUNT 2

// page_index is used to keep track of the current idle display page
int page_index = 0;

// After an ADC conversion, the value is stored in adc_value
int adc_value = 0;

// keyConversion is used to store the converted value of the keypad
unsigned char keyConversion;

//******************************************************************************
// Function : void format_display_time(unsigned char hrs, mins, secs)
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This method takes the unsigned chars for hours, minutes and seconds, and
// formats them into the tens and ones places, and then prints the time on
// a single line of the LCD. This function assumes that the ATMega has been
// configured for LCD communication, and it prints the time on one line and
// then inserts a new line character so that any printing that is done after
// this function is done on the following line of the LCD.
//
//******************************************************************************
void format_display_time(unsigned char hrs, unsigned char mins, unsigned char
secs) {
  unsigned char dsp_hr_ten, dsp_hr_one, dsp_min_ten, dsp_min_one, dsp_sec_ten, 
  dsp_sec_one;
  
  dsp_hr_ten = hrs / 10;
  dsp_hr_one = hrs % 10;
  dsp_min_ten = mins / 10;
  dsp_min_one = mins % 10;
  dsp_sec_ten = secs / 10;
  dsp_sec_one = secs % 10;
  
  //Display the time, temperature, and humidity on the lcd
  printf("Time: %d%d:%d%d:%d%d\n", dsp_hr_ten, dsp_hr_one, dsp_min_ten, 
         dsp_min_one, dsp_sec_ten, dsp_sec_one);
}


//******************************************************************************
// Function : void dsp_time_temp_rh()
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This will use methods from humidicon.c, lcd_dog_iar_driver.c, lcd_ext.c,
// lcd.h, and humidicon.h to display the time, temperature, and humidity on 
// the LCD screen. 
//
//******************************************************************************
void dsp_time_temp_rh(){
  //Setup the RTC to read the current time
  SPI_rtc_ds1306_config();
  read_time_RTC();              //read the time from our registers
  format_time();                //format time appropriately
  
  //Setup the humidicon for reading the temperature and humidity
  SPI_humidicon_config();
  read_humidicon();
  
  //Setup the lcd to display the time and temperature
  init_spi_lcd();
  clear_dsp();
  
  //Break the time values into tens/ones places
  format_display_time(hours, minutes, seconds);
  
  printf("Temp:  %2.2f%cC\n", temperature, degree_char);
  printf("RH:    %2.2f%%", humidity);
  
  update_lcd_dog();             //display values correctly
}

void dsp_time_co2() {
  //Setup the RTC to read the current time
  SPI_rtc_ds1306_config();
  read_time_RTC();              //read the time from our registers
  format_time();                //format time appropriately
  
  ADC_single_conversion();
  
  //Setup the lcd to display the time and temperature
  init_spi_lcd();
  clear_dsp();
  
  format_display_time(hours, minutes, seconds);
  printf("CO2 ppm:  %d\n", adc_value);
  
  update_lcd_dog();             //display values correctly
}

/*
*Interrupt that is set off by Alarm0 from the RTC.
*Will show a change of logic for half a second
*/
#pragma vector = INT2_vect
__interrupt void ISR_INT2(void){
  
  //Toggle a logic 1 -> logic 0
  SETBIT(PORTA, 7);
  __delay_cycles(16000000);
  CLEARBIT(PORTA, 7);
  
  //Clear out the IRQF0 status flag
  write_RTC(STAT_REG_WT, 0x00);
  
}

/*
*This will be the value that is set off by the RTC
*1Hz wave. Because idle_dsp is our initial state, we 
*will start off diaplying the time and temperature.
*Many of our states return to this idle_dsp state as well
*so this will help to show the idle_dsp value
*/
#pragma vector = INT1_vect
__interrupt void ISR_INT1(void){
  
  if(present_state == idle_dsp){
    if(page_index == 0)
      dsp_time_temp_rh();
    else if (page_index == 1)
      dsp_time_co2();
  }
  
}

/*
*Interrupt that is set off by the conversion end of 
*the ADC itself. This will setup the value which 
*will be updated ot the LCD screen
*/
#pragma vector = ADC_vect
__interrupt void ISR_ADC(){
  adc_value = (int)ADCH << 8;
  adc_value |= (int)ADCL;
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
  DDRD = 0xF8;          //INT0, INT1, INT2
  PORTD = 0x05;         //Set pullup resistors on INT0 and INT2
  
  //Config RTC clock for interrupt
  SPI_rtc_ds1306_config();
  
  //Disable WP bits and enable 1Hz output
  write_RTC(0x8F, 0x00);
  write_RTC(0x8F, 0x05);
  write_RTC(HR_ALM_WT, 0x00);
  write_RTC(DAY_ALM_WT, 0x80);
  
  //Set the intial time
  write_RTC(HR_WT, 0x00);               //Initialize hours, minutes
  write_RTC(MIN_WT, 0x00);              //and seconds to display
  write_RTC(SEC_WT, 0X00);              //00:00:00

  present_state = idle_dsp;             //Setup the intiial state of our FSM
  
  init_lcd_dog();
  
  //Enable interrupt config
  MCUCR = 0X30;
  EIMSK = 0X07;
  __enable_interrupt();
  
  __delay_cycles(20);
  
  while(1){
    
  }
}

//******************************************************************************
// Function : void dsp_options_screen()
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This will use methods from lcd_dog_iar_driver.c lcd_ext.c, and lcd.h to 
// ask the user what they will want to do at this point. The user will have 
// to choose between setting the time/alarm (1), or toggle the alarm (2). The 
// toggle alarm will simply turn the alarm on or off while the set time/alarm
// will bring you to another screen for more options.
//
//******************************************************************************
void dsp_options_screen(){
  init_spi_lcd();
  clear_dsp();
  
  __delay_cycles(1000);
  
  printf("1:Set time/alarm");
  printf("2:Toggle alarm");
  
  update_lcd_dog();          
  
}

//******************************************************************************
// Function : void dsp_time_temp_rh()
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This will use methods from lcd_dog_iar_driver.c, lcd_ext.c, and lcd.h to 
// display the time, temperature, and humidity on the LCD screen. 
//
//******************************************************************************
void dsp_instr_screen(){
  init_spi_lcd();
  clear_dsp();
  
    __delay_cycles(1000);

  printf("2nd%c1: Set time\n", ARROW);
  printf("       or Alarm\n");
  printf("2nd%c2: Alarm Y/N", ARROW);
  
  update_lcd_dog();
}

//******************************************************************************
// Function : void toggle_alarm_enable()
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This will use methods from DS1306_RTC_divrers and DS1306_RTC.h to enable 
// the bit that controls the Alarm0 enable
//
//******************************************************************************
void toggle_alarm_enable(){
  unsigned char control_reg = read_RTC(CONT_REG_RD);
  control_reg ^= 0x01;
  write_RTC(CONT_REG_WT, control_reg);
  
  
  
  init_spi_lcd();
  clear_dsp();
  
  if((control_reg & 0x01) == 0x01) {
    printf("Alarm is On\n");
  } else {
    printf("Alarm is Off\n");
  }
  
  unsigned char temp_read_hr = read_RTC(HR_ALM_RD);
  unsigned char temp_read_min = read_RTC(MIN_ALM_RD);
  
  unsigned char ones;
  unsigned char tens;
  ones = (temp_read_min & 0x0F);
  tens = (temp_read_min & 0xF0);
  tens >>= 4;
  tens *= 10;
  temp_read_min = (tens + ones);
  
  ones = (temp_read_hr & 0x0F);
  tens = (temp_read_hr & 0xF0);
  tens >>= 4;
  tens *= 10;
  temp_read_hr = (tens + ones);
  
  printf("ALM: %d:%d:00\n", temp_read_hr, temp_read_min);
  printf("Press any key");
  
  update_lcd_dog();
}

//******************************************************************************
// Function : void dsp_enter_time()
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This will use methods from lcd_dog_iar_driver.c, lcd_ext.c, and lcd.h to 
// display the enter time value. The time will start off at a default 00:00. The
// user will only be able to set hours and minutes, and seconds will start off 
// at 00
//
//******************************************************************************
unsigned char time_hr_tens =0, time_hr_ones=0, time_min_tens=0, time_min_ones=0;
int time_index = 0;

void dsp_enter_time(){
  init_spi_lcd();
  clear_dsp();
  time_hr_tens =0;
  time_hr_ones=0;
  time_min_tens=0;
  time_min_ones=0;
  time_index = 0;
  
    __delay_cycles(2000);

  printf("Enter Time/Alarm");
  printf("Enter to end\n");
  printf("00:00");
  
    __delay_cycles(1000);

  update_lcd_dog();
  
}

//******************************************************************************
// Function : void dsp_set_time()
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This will use methods from lcd_dog_iar_driver.c, lcd_ext.c, and lcd.h to
// display the enter time value. The time will display the first value that
// the user has inputted, and 0:00. The user will press the next button to set 
// the ones place of the hour value.
//
//******************************************************************************
void dsp_set_time() {
  if(time_index == 0) {
    char temp_hr_tens = time_hr_tens;
    time_hr_tens = keyConversion;
    
    if(time_hr_tens <= 2){
      if(time_hr_tens == 2){
        time_hr_ones = 0;
      }
      time_index++;
    }
    else{
        time_hr_tens = temp_hr_tens;
    }
  }
  
  else if (time_index == 1) {
    char temp_hr_ones = time_hr_ones;
    time_hr_ones = keyConversion;
    
    if(time_hr_tens == 2){
      if(time_hr_ones <= 3){
        time_index++;
      }
      else{
        time_hr_ones = temp_hr_ones; 
      }
    }
    else{
      time_index++;
    }
  } 
  
  else if (time_index == 2) {
    char temp_min_tens = time_min_tens;
    time_min_tens = keyConversion;
    if(time_min_tens <= 5){
      time_index++;
    }
    else{
      time_min_tens = temp_min_tens;
    }
  }
  
  else if (time_index == 3) {
    time_min_ones = keyConversion;
    time_index = 0;
  }
  else {
    //Should not happen
  }
  
  printf("\r");
  printf("%d%d:%d%d", time_hr_tens, time_hr_ones, time_min_tens, 
         time_min_ones);
  
  init_spi_lcd();
  update_lcd_dog();
}

//******************************************************************************
// Function : void dsp_time_alarm_choice()
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This will use methods from lcd_dog_iar_driver.c, lcd_ext.c, and lcd.h 
// to display the enter time value. The lcd will display the time that the 
// user has entered, and an option for using that value for the time or the 
// alarm. The user will then have to choose option 1 or 2 to set up the time
// or alarm.
//
//******************************************************************************
void dsp_time_alarm_choice(){
  init_spi_lcd();
  clear_dsp();
  
  minutes_ones = keyConversion;
  printf("Time entered:\n");
  printf("%d%d:%d%d\n", time_hr_tens, time_hr_ones, time_min_tens, time_min_ones);
  printf("Time(1) Alarm(2)");
  
  update_lcd_dog();  
}

//******************************************************************************
// Function : void set_system_time()
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This will use methods from DS1306_RTC_drivers.c and DS1306_RTC.h to set up 
// the time of the RTC. This will load in the values of hour and minutes as 
// the time that the user has inputted.
//
//******************************************************************************
void set_system_time(){
  SPI_rtc_ds1306_config();

  unsigned char temp_hr = (time_hr_tens << 4) | (time_hr_ones);
  unsigned char temp_mins = (time_min_tens << 4) | (time_min_ones);
  write_RTC(HR_WT, temp_hr);
  write_RTC(MIN_WT, temp_mins);
  write_RTC(SEC_WT, 0x00);
}

//******************************************************************************
// Function : void set_system_alarm()
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This will use methods from DS1306_RTC_drivers.c and DS1306_RTC.h to set up 
// the Alarm0 of the RTC. This will load in the values of hour and minutes as 
// the time that the user has inputted.
//
//******************************************************************************
void set_system_alarm(){
  SPI_rtc_ds1306_config();
  
  unsigned char temp_hr = (time_hr_tens << 4) | (time_hr_ones);
  unsigned char temp_mins = (time_min_tens << 4) | (time_min_ones);
  write_RTC(HR_ALM_WT, temp_hr);
  write_RTC(MIN_ALM_WT, temp_mins);
  write_RTC(SEC_ALM_WT, 0x00);

}

//******************************************************************************
// Function : void scroll_dsp_down()
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This method simply modifies the global variable page_index to make the dsp
// show a page lower than the current page. This method implements the checks
// to ensure that pages loop around when the page limit is reached. The page
// limit is stored in a declared value PAGE_COUNT
//
//******************************************************************************
void scroll_dsp_up() {
  //If the page_index is less than the last page
  if(page_index < PAGE_COUNT-1)
    //increment the page index to move to the next page
    page_index++;
  //if the page index is on the last page
  else if(page_index == PAGE_COUNT-1)
    //roll over to display the first page
    page_index = 0;
}

//******************************************************************************
// Function : void set_system_alarm()
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This will use methods from DS1306_RTC_drivers.c and DS1306_RTC.h to set up 
// the Alarm0 of the RTC. This will load in the values of hour and minutes as 
// the time that the user has inputted.
//
//******************************************************************************
void scroll_dsp_down() {
  //If the page index is higher than 0
  if(page_index > 0) {
    //scroll down a page
    page_index--;
  }
  //If the page index is currently the last page
  else if(page_index == 0) {
    //Roll over and go to the last page to display
    page_index = PAGE_COUNT - 1;
  }
}

//******************************************************************************
// Function : void invalid_key()
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This will use methods from lcd_dog_iar_driver.c, lcd_ext.c, and lcd.h to show
// up when the user presses an invalid key at various points through the program.
// It will simply show an invalid key value and go back to the beggining screen
// showing the time, tempertaure, and humidity of the system.
//
//******************************************************************************
void invalid_key(){
  init_spi_lcd();
  clear_dsp();
  
    __delay_cycles(2000);

  printf("Invalid key!");
  update_lcd_dog();
  
  __delay_cycles(32000000);
  
  dsp_time_temp_rh();
}

//******************************************************************************
// Function : void invalid_time_entry()
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This will use methods from lcd_dog_iar_driver.c, lcd_ext.c, and lcd.h to show
// up when the user presses an invalid key at moments when the user is setting 
// up the time of the clock. This will display an error message to the user and 
// then go back to specific screens based on the present state of the fsm.
//
//******************************************************************************
void invalid_time_entry(){
  init_spi_lcd();
  clear_dsp();
   __delay_cycles(2000);

  printf("Invalid time\n");
  printf("entry");
  update_lcd_dog();
  
  __delay_cycles(32000000);
  
  clear_dsp();
  
  __delay_cycles(100);
  
  printf("Enter Time/Alarm");
  printf("Enter to end\n");
  printf("%d%d:%d%d", time_hr_tens, time_hr_ones, time_min_tens, time_min_ones);
  
  update_lcd_dog();
  
}

//******************************************************************************
// Function : void time_alarm_choice()
// Date and version : 3/25/18 version 1.0
// Target MCU : ATmega128A @ 16MHz
// Author : Augusto Celis / Michael Anderon
//
// DESCRIPTION
// This will use methods from lcd_dog_iar_driver.c, lcd_ext.c, and lcd.h to show
// up when the user presses an invalid key when choosing between setting the 
// entered time to the alarm or time registers of the RTC. It will display an 
// error message for 2 seconds. After the error message, it will show the 
// user the time they entered and their options.
//
//******************************************************************************
void invalid_time_alarm_choice(){
  init_spi_lcd();
  clear_dsp();
  
    __delay_cycles(2000);

  printf("Invalid entry");
  update_lcd_dog();
  
  __delay_cycles(32000000);
  
  clear_dsp();
  
  printf("Time entered:\n");
  printf("%d%d:%d%d", hours_tens, hours_ones, minutes_tens, minutes_ones);
  printf("Time(1) Alarm(2)");
  
  update_lcd_dog();
}

