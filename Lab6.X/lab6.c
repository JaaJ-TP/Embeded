#define F_CPU 8000000L
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#define LCD_DATA_DDR DDRD
#define LCD_DATA_PORT PORTD
#define LCD_CMD_DDR DDRB
#define LCD_CMD_PORT PORTB
#define RS PORTB0
#define RW PORTB2
#define E PORTB1

void commit_data(){
    LCD_CMD_PORT |=(1 << E);
    _delay_us(1000);

    LCD_CMD_PORT &= ~(1 << E);
    _delay_us(1000);
}
void send_data(uint8_t data){
    LCD_DATA_PORT = (data & 0xF0);
    commit_data();
    LCD_DATA_PORT = (data << 4);
    commit_data();
}
void send_lcd_data(uint8_t data){

    LCD_CMD_PORT |= (1 << RS);
    send_data(data);
}
void send_lcd_command(uint8_t command){

    LCD_CMD_PORT &= ~(1 << RS);
    send_data(command);
}
void lcd_init(){
    LCD_CMD_DDR |= (1 << RS) | (1 << RW) | (1 << E);
    LCD_DATA_DDR = 0xF0;
    LCD_CMD_PORT &= ~((1 << RS) | (1 << RW) |(1 << E));
    LCD_DATA_PORT = 0x00;

    send_lcd_command(0x03);
    send_lcd_command(0x02);

    send_lcd_command(0x28);
    send_lcd_command(0x0C);
    send_lcd_command(0x01);
    send_lcd_command(0x80);
    _delay_ms(1);


}

uint8_t readDS1307(uint8_t address, uint8_t* data)
{
    //send S
    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTA);
    //wait complete then check status
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xF8) != 0x08)
    return 1;
    
    //send SLA + W
    TWDR = 0b11010000;
    TWCR = (1 << TWEN) | (1 << TWINT);
    //wait complete then check status
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xF8) != 0x18)
    return 2;
    
    //send register address
    TWDR = address;
    TWCR = (1 << TWEN) | (1 << TWINT);
    //wait complete then check status
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xF8) != 0x28)
    return 3;

    for(int n = 0;n < 7;++n){
        //send SR
        TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTA);
        //wait complete then check status
        while(!(TWCR & (1 << TWINT)));
        if((TWSR & 0xF8) != 0x10)
            return 4;
    
        //send SLA + R
        TWDR = 0b11010001;
        TWCR = (1 << TWEN) | (1 << TWINT);
        //wait complete then check status
        while(!(TWCR & (1 << TWINT)));
        if((TWSR & 0xF8) != 0x40)
            return 5;
    
        //wait for data
        TWCR = (1 << TWEN) | (1 << TWINT);
        while(!(TWCR & (1 << TWINT)));
        data[n] = TWDR;
        if((TWSR & 0xF8) != 0x58)
            return 6;
    }
    
    //send P
    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTO);
    return 0;
}
uint8_t writeDS1307(uint8_t address, uint8_t data)
{
    //send S
    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTA);
    //wait complete then check status
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xF8) != 0x08)
    return 1;
    
    //send SLA + W
    TWDR = 0b11010000;
    TWCR = (1 << TWEN) | (1 << TWINT);
    //wait complete then check status
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xF8) != 0x18)
    return 2;
 
    //send register address
    TWDR = address;
    TWCR = (1 << TWEN) | (1 << TWINT);
    //wait complete then check status
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xF8) != 0x28)
    return 3;
 
    //send data
    TWDR = data;
    TWCR = (1 << TWEN) | (1 << TWINT);
    //wait complete then check status
    while(!(TWCR & (1 << TWINT)));
    if((TWSR & 0xF8) != 0x28)
    return 5;
    
    //send P
    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTO);
    _delay_ms(10);
    return 0;
}
void twi_init()
{
 //SCL, SDA as output
    DDRC |= (1 << DDC4) | (1 << DDC5);
    //init I2C
    //100kHz @ prescaler /4
    TWBR = 8;
    TWSR |= (1 << TWPS0);
    //enable I2C
    TWCR |= (1 << TWEN);
}


int main(void) {
    lcd_init();
    twi_init();
    // 02/08/2021 Mon 12:00
    int i = 0;
    
    const char* week[7] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
    char time_msg[16] = {};
    char day_msg[16] = {};
    
    writeDS1307(0x00, 0x00);
    writeDS1307(0x01, 0x11);
    writeDS1307(0x02, 0x11);
    writeDS1307(0x03, 0x03);
    writeDS1307(0x04, 0x01);
    writeDS1307(0x05, 0x04);
    writeDS1307(0x06, 0x21);
    
    while (1) {
        uint8_t timestamp[7]={};
        uint8_t error = readDS1307(0x00, timestamp);
       
        uint8_t sec1 = ((timestamp[0]&0b01110000) >> 4) ;
        uint8_t sec = (timestamp[0]&0b00001111);
        
        uint8_t min1 = ((timestamp[1]&0b01110000) >> 4) ;
        uint8_t min = (timestamp[1]&0b00001111);
        
        uint8_t hour1 = ((timestamp[2]&0b11110000) >> 4) ;
        uint8_t hour = (timestamp[2]&0b00001111);
        
        uint8_t date1 = ((timestamp[4]&0b00110000) >> 4) ;
        uint8_t date = (timestamp[4]&0b00001111);
        
        uint8_t month1 = ((timestamp[5]&0b00010000) >> 4) ;
        uint8_t month = (timestamp[5]&0b00001111);
        
        uint8_t year1 = ((timestamp[6]&0b11110000) >> 4) ;
        uint8_t year = (timestamp[6]&0b00001111);
        
        sprintf(time_msg, "%s %u%u:%u%u:%u%u",week[timestamp[3]-1],hour1,hour,min1,min,sec1,sec);
        i = 0;
        send_lcd_command(0x80);
        while(time_msg[i] != 0)
        {
            send_lcd_data(time_msg[i++]);
        }
        
        sprintf(day_msg, "%u%u/%u%u/%u%u",date1, date, month1,month, year1, year);
        i = 0;
        send_lcd_command(0xC0);
        while(day_msg[i] != 0)
        {
            send_lcd_data(day_msg[i++]);
        }
       
       _delay_ms(1); 
    }
    
}
