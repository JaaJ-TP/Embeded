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
#define RW PORTB6

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

void spi_init() {
    DDRB |= (1 << DDB2) | (1 << DDB3) | (1 << DDB5);
    
    SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR1);
}

void spi_read(uint8_t *data) {
    //chip select
    PORTB &= ~(1 << PORTB2);
    
    //send burst read command
    SPDR = 0xBF;
    
    //wait
    while(!(SPSR & (1 << SPIF)));
    
    //clock out 8 bytes
    for(int i = 0; i < 8; i++) {
        SPDR = 0x00;
        while(!(SPSR & (1 << SPIF)));
        data[i] = SPDR;
        data[i] = ((data[i] >> 4) * 10) + (data[i] & 0x0F);
    }
    
    //chip select high
    PORTB |= (1 << PORTB2);
}
uint16_t SPI_Read()
{
    uint16_t Data1;  /*high*/
    uint16_t Data2; /*low*/
    uint16_t Data_out;
    
    PORTB &= ~(1 << PB2);
    
    SPDR = 0xFF;
    while (!(SPSR & (1<< SPIF)));
    Data1 = SPDR;

    SPDR = 0xFF;
    while (!(SPSR & (1<< SPIF)));
    Data2 = SPDR;

    PORTB |= (1 << PB2);
        
    Data_out = (Data1<<8)|Data2;
    Data_out <<=3; /*chip 3 bit*/
    Data_out >>=4; /*chip 4 bit*/

    return Data_out;
}

int main(void) {
    lcd_init();
    spi_init(); 
    unsigned char string[] = "TEMP : ";
    unsigned char buffer[10]; 
    unsigned char string1[] = "C";
    float temp;

    //const char month[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    while (1){
        uint8_t RTC_data[8];
        spi_read(RTC_data);
        
        uint8_t high_byte;
        uint8_t low_byte;
        uint16_t result = (high_byte << 8) | low_byte; 
        
        uint16_t sensor = SPI_Read();    
        temp = sensor/128 ;
              
        char line1[16];
        char line2[16];   
        
        dtostrf(temp,3,2,buffer);
                
        sprintf(line1,"%.2u/%.2u/%.2u",RTC_data[3],RTC_data[4],RTC_data[6]);
        send_lcd_command(0x80);
        int i = 0;
        while(line1[i] != 0) {
            send_lcd_data(line1[i++]);
        }
         
        sprintf(line2,"  %.2u:%.2u",RTC_data[2],RTC_data[1]);   
        int j = 0;
        while(line2[j] != 0) {
            send_lcd_data(line2[j++]);
        }
        
        send_lcd_command(0xC0);
        int z = 0;
        while(string[z] != 0) {
            send_lcd_data(string[z++]);
        }
        
        int k = 0;
        while(buffer[k] != 0) {
            send_lcd_data(buffer[k++]);
        }
        send_lcd_data("C");

         _delay_ms(100);
    }

}

