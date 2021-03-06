#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//define data and cmd port, RS, RW, E ports
#define LCD_DATA_DDR DDRD
#define LCD_DATA_PORT PORTD
#define LCD_CMD_DDR DDRB
#define LCD_CMD_PORT PORTB
#define RS PORTB0
#define E PORTB1
#define RW PORTB2

void commit_data()
{
    //pulse enable
    //enable high
    LCD_CMD_PORT |= (1<<E);
    _delay_us(1000);
    
    //enable low
    LCD_CMD_PORT &= ~(1<<E);
    _delay_us(1000);
}

void send_data(uint8_t data)
{
    LCD_DATA_PORT = (data & 0xF0);
    commit_data();
    LCD_DATA_PORT = (data << 4);
    commit_data();
}

void send_lcd_command(uint8_t command)
{
  //set to cmd mode, write mode
    LCD_CMD_PORT &= ~(1 << RS);
  //send command
    send_data(command);
}

void send_lcd_data(uint8_t data)
{
  //set to data mode, write mode
    LCD_CMD_PORT |= (1 << RS);
  //send data
    send_data(data);
}

//LCD init, call once
void lcd_init()
{
  LCD_CMD_DDR |= (1 << RS) | (1 << E) | (1 << RW);
  LCD_DATA_DDR = 0xF0;
  LCD_CMD_PORT &= ~((1 << RS) | (1 << E) | (1 << RW));
  LCD_DATA_PORT = 0x00;
  
  send_lcd_command(0x03);   //4-bit mode
  send_lcd_command(0x02);

  send_lcd_command(0x28);   //4-bit comm, 2 lines, 5x8 font
  send_lcd_command(0x0C);   //display ON, cursor OFF, blink OFF
  send_lcd_command(0x01);   //clear screen
  send_lcd_command(0x80);   //cursor go to top left corner
  _delay_ms(1);
}

void adc_init()
{
    //set input channel, set AREF
    ADMUX |= (1<<REFS0)|(1 << MUX2)|(1 << MUX0);
    //enable ADC, set prescaler so the output freq is around 50-200kHz
    ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADEN);
    //disable digital buffer of that ADC pin
    DIDR0 |= (1 << ADC5D);
}

int main(void) {
    unsigned char string[] = "TEMP : ";
    unsigned char text[] = "CIRCUIT DIAGRAM";
    unsigned char buffer[10];  
    lcd_init();
    adc_init();
    while (1) {
        //start conversion
        ADCSRA |= (1 << ADSC);
        //wait for ADIF
        while (!(ADCSRA & (1 << ADIF)));
        //copy data out
        uint16_t data = ADC;
        //reset ADIF flag by writing 1
        ADCSRA |= (1 << ADIF);
        //send data to serial
        //temp
        uint16_t COUNTA = data/5;
        dtostrf(COUNTA,3,2,buffer);
        
        for(int i=0; buffer[i] != 0; i++){
            send_lcd_data(buffer[i]);}
        send_lcd_command(0x80 + 0);
        
        for(int i=0; text[i] != 0; i++){
            send_lcd_data(text[i]);}
        send_lcd_command(0x80 + 0x40 + 0);
        
        for(int i=0; string[i] != 0; i++){
            send_lcd_data(string[i]);}
        send_lcd_command(0x80 + 0x40 + 8);
        
        
        _delay_ms(1000);
        //send data to lcd
    }
}

