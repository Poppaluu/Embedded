/*
 * uno_SPI_slave_basics.c
 *
 * Created: 4.7.2019 8:50:10
 * Author : Glutex
 */ 

/* ATmega328p (UNO) is slave */

#define F_CPU 16000000UL
#define FOSC 16000000UL // Clock Speed
#define BAUD 9600
#define MYUBRR (FOSC/16/BAUD-1)

#include <avr/io.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <stdio.h>
#include <stdbool.h>

static void USART_init(uint16_t ubrr) // unsigned int
{
    /* Set baud rate in the USART Baud Rate Registers (UBRR) */
    UBRR0H = (unsigned char) (ubrr >> 8);
    UBRR0L = (unsigned char) ubrr;
    
    /* Enable receiver and transmitter on RX0 and TX0 */
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0); //NOTE: the ATmega328p has 1 UART: 0
    
    /* Set frame format: 8 bit data, 1 stop bit */
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
    
}

static void USART_Transmit(unsigned char data, FILE *stream)
{
    /* Wait until the transmit buffer is empty*/
    while(!(UCSR0A & (1 << UDRE0)))
    {
        ;
    }
    
    /* Put the data into a buffer, then send/transmit the data */
    UDR0 = data;
}

static char USART_Receive(FILE *stream)
{
    /* Wait until the receive buffer is full*/
    while(!(UCSR0A & (1 << RXC0)))
    {
        ;
    }
    
    /* Get the received data from the buffer */
    return UDR0;
}

// Setup the stream functions for UART
FILE uart_output = FDEV_SETUP_STREAM(USART_Transmit, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, USART_Receive, _FDEV_SETUP_READ);

int main(void)
{
    /* set MISO as output, pin 12 (PB4)*/
    DDRB  = (1 << PB4);
    /* set SPI enable */
    SPCR  = (1 << 6);
    
    // initialize the UART with 9600 BAUD
    USART_init(MYUBRR);
        
    // redirect the stdin and stdout to UART functions
    stdout = &uart_output;
    stdin = &uart_input;
    
    //char spi_send_data[20] = "MOVEMENT\n";
	char spi_send_data[20];
    char spi_receive_data[20];
	
	bool movement = true;
    
    /* send message to master and receive message from master */
    while (1) 
    {
		
		 if (movement == true)
		 {
			 strcpy(spi_send_data, "MOVEMENT\n");
		 }
		 else if (movement == false)
		 {
			 strcpy(spi_send_data, "MAAAAAAA\n");
		 }
		
        
        for(int8_t spi_data_index = 0; spi_data_index < sizeof(spi_send_data); spi_data_index++)
        {
            
            SPDR = spi_send_data[spi_data_index]; // send byte using SPI data register
            
            while(!(SPSR & (1 << SPIF)))
            {
                /* wait until the transmission is complete */
                ;
            }
            spi_receive_data[spi_data_index] = SPDR; // receive byte from the SPI data register

        }
        
        spi_receive_data[sizeof(spi_send_data) - 1] = '\0'; // terminate the received string
        
        printf("%s", spi_receive_data);
    }
    
    return 0;
}
