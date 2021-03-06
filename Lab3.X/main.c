#define F_CPU 8000000L
#include <avr/io.h>
#include <util/delay.h>
char string[] = {72, 101, 108, 108, 111, 32};
char name[50];
int i; 
int j;
void USART_Init(unsigned int ubrr) {
 /* Set baud rate */
    UBRR0 = ubrr;
 /* Double Transmission Speed*/
    UCSR0A |= (1 << U2X0);
 /* Enable receiver and transmitter */
    UCSR0B |= (1 << RXEN0)|(1 << TXEN0);
 /* Set frame format: 8data */
    UCSR0C |= (1 << UCSZ01)|(1 << UCSZ00);
}
void USART_Transmit( unsigned char data ) {
 /* Wait for empty transmit buffer */
    while ( !( UCSR0A & (1 << UDRE0)) ) ;
 /* Put data into buffer, sends the data */
    UDR0 = data;
}
unsigned char USART_Receive() {
 /* Wait for data to be received */
    while ( !(UCSR0A & (1 << RXC0)) );
 /* Get and return received data from buffer */
    return UDR0;
}
int main(void) {
    USART_Init(103);
        while (1) {
        j=0;
        for(i = 0;i<sizeof(name);i++){
            unsigned char received_char = USART_Receive();
            name[i] = received_char;
            if(name[i] == '\n'){
                break; }
            j++;
 		}
        
        for(i = 0;i<sizeof(string);i++){
            USART_Transmit(string[i]);}
        
        for(i = 0;i<j;++i){
            USART_Transmit(name[i]);}
    }
}