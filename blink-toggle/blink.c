//***************************************************************************************
//  MSP430 Blink the LED Demo - Software Toggle P1.0
//
//  Description; Toggle P1.0 by xor'ing P1.0 inside of a software loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430x5xx
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.0|-->LED
//
//  J. Stevenson
//  Texas Instruments, Inc
//  July 2011
//  Built with Code Composer Studio v5
//***************************************************************************************

#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define RED BIT0
#define GREEN BIT6


void delay (unsigned int ms) {
	unsigned int i;
	for (i=ms; i>0; i--) {
		__delay_cycles(1000);		// set for 16Mhz change it to 1000 for 1 Mhz
	}
}


void main (void) {
	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer
	P1DIR = RED | GREEN;					// Set P1.0 to output direction
	P1OUT |= RED;
	P1OUT &= ~GREEN;

	/*
	time_t t;
	t = time(NULL);
	struct tm *x = localtime(&t);
	volatile int day = x->tm_mday;
	volatile int hour = x->tm_hour;
	volatile int min = x->tm_min;
*/

	//volatile char t[100] = "abc\n";
	delay(500);

	for(;;) {
		volatile unsigned int i;	// volatile to prevent optimization

		P1OUT ^= RED | GREEN;				// Toggle P1.0 using exclusive-OR


		delay(500);		// ms

	}


}




/*
 Author: Alan Barr
* Created: May 2012
* Compiler: mspgcc
* Small program to demonstrate using the switch on the launchpad.
* Each time the switch is pressed the launchpad LEDs should toggle between
* values. The switch is polled for a change.


#include "msp430g2553.h"

#define SWITCH BIT3
#define SWITCH_PRESSED 0x00

#define RED BIT0
#define GREEN BIT6

int main(void)
{
    // Disabling dog, setting up clock
    WDTCTL = WDTPW + WDTHOLD;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    // Setting up Switch
    P1DIR &= ~SWITCH; // Set the switch pin to input
    P1REN |= SWITCH; // Use an internal resistor
    P1OUT |= SWITCH; // The internal resistor is pullup

    // Setting up Launchpad LEDs
    P1DIR |= RED + GREEN; // Launchpad LEDs for output
    P1OUT |= GREEN; // Green LED lit at startup
    P1OUT &= ~RED; // Red LED off at startup


    while (1)
    {
        if ((P1IN & SWITCH) == SWITCH_PRESSED)
        {
            // Hang until the switch is released.
            while ((P1IN & SWITCH) == SWITCH_PRESSED);

            P1OUT ^= GREEN | RED; // Toggle Launchpad LEDs
        }
    }

    return 0;
}
 */


