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


void ADC_Init()
{   
    //set input channel, set AREF
    ADMUX |= (1<<REFS0)|(1 << MUX2)|(1 << MUX0);
    //enable ADC, set prescaler so the output freq is around 50-200kHz
    ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADEN);
    //disable digital buffer of that ADC pin
    DIDR0 |= (1 << ADC5D);
}

int main(void) {
    ADC_Init();
    USART_Init(51);
    unsigned char string[] = "Temperature = ";
    //unsigned char text[] = "CIRCUIT DIAGRAM";
    unsigned char buffer[10];  

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
        strcat(buffer," .C");
        
        for(int i=0; string[i] != 0; i++){
            USART_Transmit(string[i]);}
        
        for(int i=0; buffer[i] != 0; i++){
            USART_Transmit(buffer[i]);}
        
        USART_Transmit(10);
        _delay_ms(1000);
        //send data to lcd
    }
}