#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void USART_Init(unsigned int ubrr) { 
    /* Set baud rate */
    UBRR0 = ubrr;
    /* Enable receiver and transmitter */
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
    /* Set frame format: 8data*/ 
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); 
}

void USART_Transmit( unsigned char data ) { 
    /* Wait for empty transmit buffer */ 
    while ( !( UCSR0A & (1 << UDRE0)) );
    /* Put data into buffer, sends the data */ 
    UDR0 = data; 
}

void SPI_Init()
{   
    DDRB |= (1 << DDB2) | (1 << DDB3) | (1 << DDB4);
    /*Chip select high*/
    PORTB |= (1 << PB2);
    /* Enable SPI, Master mode, clk/16 */
    SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);
    
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
    USART_Init(51);
    SPI_Init();
    
    int i;
    float temp;
    unsigned char string[] = "Temperature = ";
    unsigned char buffer[10];  
    while (1) {
        uint16_t sensor = SPI_Read();
        temp = (((sensor/4095.0) * 5.0) - 0.5) * 100.0 ;
        
        dtostrf(temp,3,2,buffer);
        strcat(buffer," .C");
        
        for(i=0; string[i] != 0; i++){
            USART_Transmit(string[i]);
        }
        for(i=0; buffer[i] != 0; i++){
            USART_Transmit(buffer[i]);
        }

        USART_Transmit(10);
        _delay_ms(1000);
    }
}