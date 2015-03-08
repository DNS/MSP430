#include <msp430.h>


unsigned int currentMinutes, currentSeconds;

void main (void) {
	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	BCSCTL1 |= DIVA_3;	// ACLK/8
	BCSCTL3 |= XCAP_2;
	//BCSCTL3 |= XCAP_2 | LFXT1S_0;
	//BCSCTL3 |= LFXT1S_2;

	P1DIR = BIT0 | BIT6;	// set P1.0 (LED1) as output
	P1OUT |= BIT0;
	P1OUT &= ~BIT6;

	currentMinutes = 0;
	currentSeconds = 0;

	CCTL0 = CCIE;	// CCR0 interrupt enabled
	CCR0 = 512;	// 512 -> 1 sec, 30720 -> 1 min
	TACTL = TASSEL_1 | ID_3 | MC_1;	// ACLK, /8, upmode

	//_BIS_SR(LPM3_bits + GIE);	// Enter LPM3 w/ interrupt
	__enable_interrupt();
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void) {
	P1OUT ^= BIT0 | BIT6;	// Toggle LED
}
