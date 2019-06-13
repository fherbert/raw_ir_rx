#include <avr/io.h>
#include <avr/interrupt.h>
//#include "lib/uart.h"
#include "main.h"

//#define UART_BAUD_RATE 9600
#define NUMPERIODS 65535 // how many raw periods can we capture

volatile uint16_t data[NUMPERIODS]; // store the received pulses


//main program
int main(void)
{
	//uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );
	//uart_puts("NOMATCH\r\n");
	while(1) {}
}
