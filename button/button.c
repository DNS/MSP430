
#include <msp430.h>

typedef enum _BOOL {
	FALSE,
	TRUE

} BOOL;

typedef unsigned char BYTE;



void main (void) {
	WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer

	P1DIR |= BIT0;
	P1DIR &= ~BIT3;
	P1REN |= BIT3; // pull-up resistor enabled

	P1SEL &= ~BIT3;  // PISEL 0: use pin as digital IO



	//BOOL state = FALSE;

	for (;;) {
		BYTE value = ((P1IN & BIT3)!=0);

		if (value == 1) {	// high
			P1OUT |= BIT0;
		} else {						// low
			P1OUT &= ~BIT0;
		}


	}

}


/*
void main (void) {
	WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer
	P1DIR |= (BIT0 + BIT6);
	// P1.3 must stay at input
	P1OUT &= ~(BIT0 | BIT6);
	P1IE |= BIT3;		// P1.3 interrupt enabled

	P1IFG &= ~BIT3;		// P1.3 IFG cleared

	__enable_interrupt();		// enable all interrupts

	for(;;);


}


// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1 (void) {
	P1OUT ^= (BIT0 + BIT6); // P1.0 = toggle
	P1IFG &= ~BIT3; // P1.3 IFG cleared
	P1IES ^= BIT3; // toggle the interrupt edge,
	// the interrupt vector will be called
	// when P1.3 goes from HitoLow as well as
	// LowtoHigh
}
*/
