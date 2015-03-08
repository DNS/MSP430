
#include <msp430.h> 
#include <time.h>

#define RED BIT0
#define GREEN BIT6

void delay (unsigned int ms) {
	unsigned int i;
	for (i=ms; i>0; i--) {
		__delay_cycles(1000);		// set for 16Mhz change it to 1000 for 1 Mhz
	}
}


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    P1OUT &= ~RED;
    P1DIR = RED;

    time_t t;
	t = time(NULL);
	struct tm *x = localtime(&t);
	int day = x->tm_mday;
	int hour = x->tm_hour;
	int min = x->tm_min;

	int i;
	for(i=0; i<day; i++) {
		P1OUT ^= RED;				// Toggle P1.0 using exclusive-OR
		delay(500);
		P1OUT ^= RED;

		delay(500);		// ms


	}

	return 0;
}
