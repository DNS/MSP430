
#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "PCD8544.h"

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long long QWORD;


QWORD tick = 0;

BYTE timerCount = 0;





typedef struct _CLOCK {
	BYTE hour;	// 24 hour mode
	BYTE min;
	BYTE sec;
} CLOCK, *PCLOCK;


CLOCK my_clock;


void syncClock ();
char ADC_Str[8];

char *convertADC(int val,int dec_place) {
	int digit;
	int cVal;
	char *ptr;
	int tens;


	ptr = ADC_Str;
	cVal = val;
	digit = 1000;
	while(digit>=1) {
		if(dec_place == digit) {
			*(ptr++) = '.';				// Add a decimal place if given
		}
		if(digit > cVal) {
			tens = 0;
		} else {
			tens = cVal / digit;
		}
		*(ptr++) = '0' + tens;
		cVal -= (tens * digit);
		digit /= 10;
	}
	*ptr = 0;							// Terminate the string
	return ADC_Str;
}

static char *i2a(unsigned i, char *a, unsigned r) {
	if (i/r > 0) a = i2a(i/r,a,r);
	*a = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i%r];
	return a+1;
}

char *itoa(int i, char *a, int r) {

	if ((r < 2) || (r > 36)) r = 10;
	if (i < 0) {
		*a = '-';
		*i2a(-(unsigned)i,a+1,r) = 0;
	} else {
		*i2a(i,a,r) = 0;
	}
	return a;
}

unsigned long adc_val;
unsigned long degrees;

void putTemperature () {

	LCD_gotoXY(0,1);
	ADC10CTL0 &= ~ENC;							// Switch off ADC.. (Can't set if enabled)

	ADC10CTL1 = INCH_10 + ADC10DIV_0;			// Select Temperature channel and Div by 1 clock
	ADC10CTL0 |= SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
														// Range to VRef+, Max Sample and hold time
														// turn on reference, turn on the ADC and enable Interrupts
	//__delay_cycles(40);							// Allow time for the reference voltage to settle
	ADC10CTL0 |= ENC + ADC10SC;					// enable the encoder and start conversion
	//__bis_SR_register(CPUOFF + GIE);			// Turn off the CPU and allow interrupts (ISR turns on CPU)

	adc_val = ADC10MEM;							// Put the ADC value into var
	//LCD_writeString(convertADC(adc_val,-1));		// Write out the converted value

	LCD_gotoXY(0,1);
	//LCD_writeChar('(');
	LCD_writeString("TMP ");
	degrees = adc_val - 673;
	degrees *= 4230;
	degrees /= 1024;
	//	degrees = ((adc_val - 673) * 4230) / 1024;
	LCD_writeString(convertADC(degrees,1));
	LCD_writeChar(0x7f);	// degree
	LCD_writeString("C");

}



void send_receive_8bit_serial_data (BYTE data) {
	BYTE i;
	BYTE recv = '\0';

	// send bits 7..0
	for (i = 0; i < 8; i++) {
		// consider leftmost bit
		// set line high if bit is 1, low if bit is 0
		if (data & 0x80)
			P1OUT |= BIT0;
		else
			P1OUT &= ~BIT0;

		BYTE value = ((P1IN & BIT3)!=0);
		if (value == 1) recv |= 1;


		if (i != 7) recv <<= 1;



		// shift byte left so next bit will be leftmost
		data <<= 1;
	}

	//LCD_writeChar(recv);

}



void send_byte (BYTE data) {
	BYTE i;
	for (i = 0; i < 8; i++) {
		if (data & 0x80)
			P1OUT |= BIT0;
		else
			P1OUT &= ~BIT0;

		data <<= 1;
	}
}


void recv_byte (BYTE *recv) {
	BYTE i;
	//BYTE data = '\0';
	recv = '\0';	// set to 0000 0000

	for (i = 0; i < 8; i++) {

		BYTE value = ((P1IN & BIT3)!=0);
		if (value == 1) *recv |= 1;


		if (i != 7) *recv <<= 1;


	}

}


void main (void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	P1DIR |= BIT0 | BIT6;
	P1OUT |= BIT0;
	P1OUT &= ~BIT6;

	//__delay_cycles(10000);
	//BCSCTL1 |= CALBC1_1MHZ; // Sets DCO range
	//DCOCTL = CALDCO_1MHZ;
	//BCSCTL1 |= DIVA_3;	// ACLK/8
	//BCSCTL3 |= LFXT1S_0 | XCAP_0;

	CCTL0 = CCIE;
	//CCR0 = 512;
	//TACTL = TASSEL_1 | MC_1 | TACLR;	// ACLK (from external 32 kHz crystal)
	//TACTL = TASSEL_1 | MC_1 | ID_3;
	TACTL = TASSEL_2 | MC_2; // Set the timer A to SMCLCK, Continuous (internal oscilator)
	// Clear the timer and enable timer interrupt

	my_clock.hour = 17;
	my_clock.min = 17;
	my_clock.sec = 0;

	//sync_clock();

//////////// NOKIA 5110 LCD ROUTINE
	P1DIR &= ~BIT3;
	P1REN |= BIT3; // pull-up resistor enabled (P1REN 	P2REN 	P3REN)
	P1SEL &= ~BIT3;
	P1OUT |= BIT0;	// red LED

	__enable_interrupt();

	LCD_init();
	char time_buf[25] = "";
	char tmp[25];
	QWORD max_rate = 0;
	QWORD byte_per_s = 0;
	int tick_old = 0;

	while (1) {
		//char *b = itoa(my_clock.hour);

		LCD_gotoXY(0, 0);
		/*strcpy(time_buf, "CLK ");

		// 24 hour mode
		if (my_clock.hour < 10) {
			itoa(my_clock.hour, tmp, 10);
			strcat(time_buf, " ");
			strcat(time_buf, tmp);
			strcat(time_buf, ":");
		} else {
			itoa(my_clock.hour, tmp, 10);
			strcat(time_buf, tmp);
			strcat(time_buf, ":");
		}

		if (my_clock.min < 10) {
			itoa(my_clock.min, tmp, 10);
			strcat(time_buf, "0");
			strcat(time_buf, tmp);
			strcat(time_buf, ":");
		} else {
			itoa(my_clock.min, tmp, 10);
			strcat(time_buf, tmp);
			strcat(time_buf, ":");
		}

		if (my_clock.sec < 10) {
			itoa(my_clock.sec, tmp, 10);
			strcat(time_buf, "0");
			strcat(time_buf, tmp);
		} else {
			itoa(my_clock.sec, tmp, 10);
			strcat(time_buf, tmp);
		}

		//my_clock.hour, my_clock.min, my_clock.sec);


		LCD_writeString(time_buf);

		putTemperature();
*/
		/*int value = ((P1IN & BIT3)!=0);
		itoa(value, tmp, 10);

		if (value == 1) {	// high
			P1OUT |= BIT0;
		} else {						// low
			P1OUT &= ~BIT0;
		}
		LCD_writeString(tmp);
		*/



		//send_receive_8bit_serial_data('x');
		if (tick_old == tick) {
			byte_per_s++;
		} else {
			max_rate = byte_per_s;
			byte_per_s = 0;
			tick_old = tick;
		}


		itoa(tick, tmp, 10);
		strcpy(time_buf, "s: ");
		strcat(time_buf, tmp);
		LCD_writeString(time_buf);

		LCD_gotoXY(0, 1);
		strcpy(tmp, "");
		itoa(max_rate, tmp, 10);
		strcpy(time_buf, "m: ");
		strcat(time_buf, tmp);
		LCD_writeString(time_buf);

		//BYTE a = '\0';

		//recv_byte(&a);
		//LCD_writeChar(a);

		//LCD_clear();

	}


	//__bis_SR_register(LPM0 + GIE); // LPM0 with interrupts enabled


}

void syncClock () {
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
		tick++;
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


#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
	__bic_SR_register_on_exit(CPUOFF);
}
