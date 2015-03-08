//***************************************************************************************
// MSP430 Timer Blink LED Demo - Timer A Software Toggle P1.0 & P1.6
//
// Description; Toggle P1.0 and P1.6 by xor'ing them inside of a software loop.
// Since the clock is running at 1Mhz, an overflow counter will count to 8 and then toggle
// the LED. This way the LED toggles every 0.5s.
// ACLK = n/a, MCLK = SMCLK = default DCO
//
// MSP430G2xx
// -----------------
// /|\| XIN|-
// | | |
// --|RST XOUT|-
// | P1.6|-->LED
// | P1.0|-->LED
//
// Aldo Briano
// Texas Instruments, Inc
// June 2010
// Built with Code Composer Studio v4
//***************************************************************************************
#include <msp430.h>

#define LED_0 BIT0
#define LED_1 BIT6
#define LED_OUT P1OUT
#define LED_DIR P1DIR



unsigned char timerCount = 0;


typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long long QWORD;



int main () {
	WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer


	P1DIR |= BIT0 | BIT6; // Set P1.0 and P1.6 to output direction
	//P1OUT &= ~(BIT0 | BIT6); // Set the LEDs off
	P1OUT |= BIT0;
	P1OUT &= ~BIT6;


	CCTL0 = CCIE;
	TACTL = TASSEL_2 | MC_2; // Set the timer A to SMCLCK, Continuous
	// Clear the timer and enable timer interrupt

	__enable_interrupt();

	//__bis_SR_register(LPM0 + GIE); // LPM0 with interrupts enabled

	return 0;
}




// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0 () {
	timerCount = (timerCount + 1) % 16;
	if(timerCount == 0) {

		P1OUT ^= BIT0 | BIT6;
	}
}


