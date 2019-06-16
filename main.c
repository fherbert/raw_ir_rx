#include <avr/io.h>
#include <avr/interrupt.h>
#include "lib/uart.h"
#include "main.h"
#include <stdbool.h>
#include <string.h>

#define UART_BAUD_RATE 9600
#define NUMPULSES 520 // how many raw periods can we capture

volatile uint16_t pulsecount = 0, nopulsecount = 0;
volatile uint16_t capture_period = 0;
typedef enum { waitforStart, waitingIR, timeout, rxPulses, rxNoPulses } state_t;
volatile state_t state = waitforStart;

void initTimer1 (void)
{
	//  Input Capture Interrupt Enable
	TIMSK = (1<<TICIE1);
}

void startTimer1 (void)
{
	// Setup timer_counter1 for input capture
	// Disable noise cancellation for now (ICNC1=0)
	// trigger on a falling edge (ICES1=0)
	// Prescaler 1 (CS12=0, CS11=0, CS10=1)
	TCCR1B |= (1<<CS10);
}

void stopTimer1 (void)
{
	TCCR1B = 0;
	TIMSK = 0;;
}

void initTimer3 (void)
{
	// Timer 3 used to count nopulses
	// Setup Clear timer on compare match 
	TCCR3B |= (1<<WGM32);
	// enable output compare match A
	ETIMSK |= (1<<OCIE3A);
}

void stopTimer3 (void)
{
	// stop timer3
	TCCR3B = 0;
	ETIMSK = 0;
}


ISR(TIMER3_COMPA_vect)
{
	// Use this interrupt to count the nopulses 
	// might need to check if the input pin is still high?
	nopulsecount++;
	state = rxNoPulses;
}

ISR(TIMER1_CAPT_vect)
{
	uint16_t curr, prev;

	curr = ICR1;
	TCNT3 = 0; // reset timer 3 (nopulse counter)
	state = rxPulses;
	if (pulsecount++ == 0) {
		prev = curr;
		return;
	}
	capture_period = curr - prev;
	OCR3A = capture_period;
	// start timer 3 no prescaler
	TCCR3B |= (1<<CS30);
}

//main program
int main(void)
{
	uint16_t irdata[NUMPULSES]; // store the received pulses
	uint16_t datacount = 0;
	uint16_t frequency = 0;
	/*
	 *
	 * At 8Mhz, the counter takes 0.000000125 seconds to tick
	 * 16bit timer (65536 ticks to overflow)
	 * Total time for one overflow: 0.008192 seconds
	 * So slowest frequency measurement possible is 122Hz impossible
	 * Fastest is dependent of time taken with interrupt routines.
	 *
	 */
	initTimer1();
	initTimer3();
	// configure ICP1 pin (PD4) as input
	DDRD &= ~(1<<PD4);
	// enable pullup
	PORTD |= (1<<PD4);
	sei(); // enable global interrupts
	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );
	uart_puts("Start\r\n");
	while (1) {
		switch (state) {
		case waitforStart:
			// poll i2c and wait for command to start capture
			if (true) {
			 	state = waitingIR;
				memset(irdata, '\0', sizeof(irdata));		
				datacount = 0;
				pulsecount = 0;
				nopulsecount = 0;
				frequency = 0;
				stopTimer3();
				startTimer1();
			};
			break;
		case waitingIR:
			// Poll i2c and wait for cancel command		
			break;
		case rxPulses:
			if ((capture_period > 0) && (frequency = 0)) {
				// we have got a frequency measurement and need to calc
				frequency = 0;// TODO work out how to calc frequency
			} else {
				// load our no pulse count into data
				if (nopulsecount > 0) {
					irdata[datacount++] = nopulsecount;
					nopulsecount = 0;
				}
				// do we need to check for a timeout here?
				if (pulsecount > 65534) {
					state = timeout;
				}
			}
			if (datacount > NUMPULSES)
				state = timeout;
			break;
		case rxNoPulses:
			if (nopulsecount > 65534) {
				//Timeout reached, send data back
				irdata[datacount++] = 9999;	
				state = timeout;
			} else if (pulsecount > 0) {
				irdata[datacount++] = pulsecount;
				pulsecount = 0;
			}
			if (datacount > NUMPULSES)
				state = timeout;
			break;
		case timeout:
			// Send result back to i2c
			state = waitforStart;
			break;
		}
	}
}
