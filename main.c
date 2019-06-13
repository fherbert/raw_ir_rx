#include <avr/io.h>
#include <avr/interrupt.h>
//#include "lib/uart.h"
#include "main.h"

//#define UART_BAUD_RATE 9600
#define NUMPULSES 520 // how many raw periods can we capture

volatile uint16_t data[NUMPULSES]; // store the received pulses
volatile uint16_t pulsecount = 0;
volatile uint16_t frequency = 0;
volatile uint16_t curr = 0;
volatile uint16_t prev = 0;

void startLearn (void)
{
	pulsecount = 0;
	frequency = 0;
	curr = 0;
	prev = 0;
}

ISR(TIMER1_CAPT_vect)
{
	curr = ICR1;
	if (pulsecount++ == 0)
		prev = curr;

	if ((pulsecount > 1) && (frequency = 0)) {
		// calc frequency on first pulse
		curr = ICR1;
		capture_period = curr - prev;
		prev = curr;
	}
}

//main program
int main(void)
{
	// Setup timer_counter1 for input capture
	// Disable noise cancellation for now (ICNC1=0)
	// trigger on a falling edge (ICES1=0)
	// No prescaler (CS12=0, CS11=0, CS10=1)
	TCCR1B |= (1<<CS10);
	/*
	 *
	 * At 8Mhz, the counter takes 0.000000125 seconds to tick
	 * 16bit timer (65536 ticks to overflow)
	 * Total time for one overflow: 0.008192 seconds
	 * So slowest frequency measurement possible is 122Hz impossible
	 * Fastest is dependent of time taken with interrupt routines.
	 *
	 */

	// Timeout timer needs to be set every time a pulse is received
	// What timeout value to use?

	//uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );
	//uart_puts("NOMATCH\r\n");
	while(1) {}
}
