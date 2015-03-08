

/*
 * RTC – Real Time Clock
 *
 * Accuracy: 1 second every 30 years (with 32,768 kHz crystal)
 * on pin 12 & 13 (XIN & XOUT)
 *
 * MSP430G2553, MSP430G2452

#define XCAP_0                 (0x00)         // XIN/XOUT Cap : 0 pF
#define XCAP_1                 (0x04)         // XIN/XOUT Cap : 6 pF
#define XCAP_2                 (0x08)         // XIN/XOUT Cap : 10 pF
#define XCAP_3                 (0x0C)         // XIN/XOUT Cap : 12.5 pF

 */


#include <msp430.h> 

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long long QWORD;

typedef enum {
	TRUE = 1,
	FALSE = 0
} BOOL;

BOOL tick = FALSE;

BYTE timerCount = 0;





typedef struct _CLOCK {
	BYTE hour;	// 24 hour mode
	BYTE min;
	BYTE sec;
} CLOCK, *PCLOCK;


CLOCK my_clock;


void sync_clock ();

void main (void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    P1DIR |= BIT0 | BIT6;
    P1OUT |= BIT0;
    P1OUT &= ~BIT6;

	CCTL0 = CCIE;
	//TACTL = TASSEL_1 | MC_1 | TACLR;	// ACLK (from external 32 kHz crystal)
	//TACTL = TASSEL_1 | MC_1 | TACLR;
	TACTL = TASSEL_2 | MC_2; // Set the timer A to SMCLCK, Continuous (internal oscilator)
	// Clear the timer and enable timer interrupt

	my_clock.hour = 0;
	my_clock.min = 0;
	my_clock.sec = 0;

	sync_clock();

    __enable_interrupt();

    //__bis_SR_register(LPM0 + GIE); // LPM0 with interrupts enabled


}

void sync_clock () {
	/* read serial input from:
	 * P1.2 Software interrupt to update clock
	 * P1.3 = hour
	 * P1.4 = min
	 * P1.5 = sec
	 */
	/*
	 * P1.2 input: high
	 * P1.3 input format (little endian): [CLOCK]
	 * 		size 3 bytes
	 *
	 */
	//BYTE hour;
	//BYTE min;
	//BYTE sec;
	//my_clock.hour = hour;
	//my_clock.min = min;
	//my_clock.sec = sec;

}


// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0 () {
	timerCount = (timerCount + 1) % 16;

	if(timerCount == 0) {

		my_clock.sec++;
		P1OUT ^= BIT0 | BIT6;

		if (my_clock.sec == 60) {
			my_clock.sec = 0;
			my_clock.min++;
			if (my_clock.min == 60) {
				my_clock.min = 0;
				my_clock.hour++;

				if (my_clock.hour == 24) {
					my_clock.hour = 0;
				}
			}

		}



	}
}


/*
#pragma vector = WDT_VECTOR
__interrupt void WATCHDOG_INTERVAL_TIMER_ISR () {
	tick = TRUE;

}
*/

