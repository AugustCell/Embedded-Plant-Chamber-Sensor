#include <iom128.h>
#include <intrinsics.h>
#include <avr_macros.h> 
#include "lcd.h"

//Normal includes from the asm version
#define SCK 1
#define MISO 3
#define MOSI 2
#define SS_bar 0
#define RS 4
#define BLC 5

//The display buffers that we have for our LCD screen
char dsp_buff_1[16];
char dsp_buff_2[16];
char dsp_buff_3[16];

void lcd_spi_transmit_CMD(char comd) {
  CLEARBIT(PORTB, RS);
  CLEARBIT(PORTB, SS_bar);
  SPDR = comd;
  while(!(SPSR & (1<<SPIF))) {
    //nothing
  }
  char kill = SPSR;
  kill = SPDR;
  SETBIT(PORTB, SS_bar);
  
  __delay_cycles(480); //Delay for 30us
}

__version_1 void init_spi_lcd(void) {
  SPCR = (1 << SPE) | (1 << MSTR) | (1<<CPOL) | (1<<CPHA) | (1<<SPR1) | (1<<SPR0);
  char kill = SPSR;
  kill = SPDR;
}

void lcd_spi_transmit_DATA(char data) {
  SETBIT(PORTB, RS);
  CLEARBIT(PORTB, SS_bar);
  //Kill SPIF bit in SPSR
  char kill = SPSR;
  kill = SPDR;
  SPDR = data;
  while(! (SPSR & (1<<SPIF))) {
    //nothing
  }
  char killSPIF = SPSR;
  killSPIF = SPDR;
  SETBIT(PORTB, SS_bar);
}

__version_1 void init_lcd_dog(void) {
  //C equivalent to code in the init_spd_lcd function in asm code
  init_spi_lcd();
  
  __delay_cycles(640000); //Delay for 40ms on a 16MHz clock

  //function set 1
  char cmd = 0x39;
  lcd_spi_transmit_CMD(cmd);

  //function set 2
  lcd_spi_transmit_CMD(cmd);

  //bias set
  cmd = 0x1E;
  lcd_spi_transmit_CMD(cmd);

  //power ctrl
  cmd = 0x50;
  lcd_spi_transmit_CMD(cmd);

  //follower ctrl
  cmd = 0x6C;
  lcd_spi_transmit_CMD(cmd);

  //contract set
  cmd = 0x77;
  lcd_spi_transmit_CMD(cmd);

  //display on
  cmd = 0x0C;
  lcd_spi_transmit_CMD(cmd);

  //clear display
  cmd = 0x01;
  lcd_spi_transmit_CMD(cmd);

  //entry mode
  cmd = 0x06;
  lcd_spi_transmit_CMD(cmd);
}

__version_1 void update_lcd_dog(void) {
  init_spi_lcd();

  int charsPerLine = 16;
  
  //send DDRAM address
  char cmd = 0x80;
  lcd_spi_transmit_CMD(cmd);
  
  for(int i=0; i<charsPerLine; i++) {
    lcd_spi_transmit_DATA(dsp_buff_1[i]);
  }
  
  //send DDRAM address 2
  cmd = 0x90;
  lcd_spi_transmit_CMD(cmd);
  
  for(int i=0;i<charsPerLine; i++) {
    lcd_spi_transmit_DATA(dsp_buff_2[i]);
  }
  
  //send DDRAM address 3
  cmd = 0xA0;
  lcd_spi_transmit_CMD(cmd);
  
  for(int i=0; i<charsPerLine;i++) {
    lcd_spi_transmit_DATA(dsp_buff_3[i]);
  }
}
