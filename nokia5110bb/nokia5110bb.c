
#include <msp430.h>

#include "PCD8544.h"

volatile unsigned int adc_val;
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



void main(void) {
	unsigned long volts;
	unsigned long degrees;

	WDTCTL = WDTPW + WDTHOLD;

	LCD_init();
	
	while(1) {
/*	**************************************************************************
*	Write out the Temperature ADC Values
*
*/
		LCD_gotoXY(0,0);
		LCD_writeString("Temp :");


		ADC10CTL0 &= ~ENC;							// Switch off ADC.. (Can't set if enabled)

		ADC10CTL1 = INCH_10 + ADC10DIV_0;			// Select Temperature channel and Div by 1 clock
		ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
													// Range to VRef+, Max Sample and hold time
													// turn on reference, turn on the ADC and enable Interrupts
		__delay_cycles(40);							// Allow time for the reference voltage to settle
		ADC10CTL0 |= ENC + ADC10SC;					// enable the encoder and start conversion
		__bis_SR_register(CPUOFF + GIE);			// Turn off the CPU and allow interrupts (ISR turns on CPU)

		adc_val = ADC10MEM;							// Put the ADC value into var
		//LCD_writeString(convertADC(adc_val,-1));		// Write out the converted calue

		LCD_gotoXY(28,1);
		LCD_writeChar('(');
		degrees = adc_val - 673;
		degrees *= 4230;
		degrees /= 1024;
		//degrees = (adc_val - 673) * 4230 / 1024;
		LCD_writeString(convertADC(degrees,1));
		LCD_writeChar(0x7f);
		LCD_writeString("C)");

	//Write out the VCC/2 ADC Values


		LCD_gotoXY(0,2);
		LCD_writeString("VCC  :");

		ADC10CTL0 &= ~ENC;							// Switch off ADC.. (Can't set if enabled)

		ADC10CTL1 = INCH_11;						// Select VCC/2
//		ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE + REF2_5V;

		ADC10CTL0 = SREF_1 + REF2_5V + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
													// Range to 2.5V reference, Max Sample and hold time
													// turn on reference, turn on the ADC and enable Interrupts
		__delay_cycles(140);							// Allow time for the reference voltage to settle
		ADC10CTL0 |= ENC + ADC10SC;					// enable the encoder and start conversion
		__bis_SR_register(CPUOFF + GIE);			// Turn off the CPU and allow interrupts (ISR turns on CPU)

		adc_val = ADC10MEM;							// Put the ADC value into var
		LCD_writeString(convertADC(adc_val,-1));		// Write out the converted calue
		LCD_gotoXY(28,3);							// Move to next line
		LCD_writeChar('(');
		volts = adc_val;
		volts *= 5000;
		volts /= 1023;
		LCD_writeString(convertADC(volts,100));
		LCD_writeString("V)");

		//Write out the POT ADC Values (Pin 1.3)


		LCD_gotoXY(0,4);							// Move to next line

		LCD_writeString("POT  :");
		ADC10CTL0 &= ~ENC;							// Turn off ADC so we can change settings

		ADC10CTL1 = INCH_3 + ADC10DIV_0;			// Channel 3 (pin 1.3)
		ADC10CTL0 = SREF_0 + ADC10SHT_2 + ADC10ON + ADC10IE;	// Shorter sample and hold time, turn on ADC and use interrupts
		ADC10AE0 |= BIT3;							// Enable bit 3
		ADC10CTL0 |= ENC + ADC10SC;					// Start the conversion
		__bis_SR_register(CPUOFF + GIE);			// Turn off CPU and enable interrupts (ISR re-enables CPU)

		adc_val = ADC10MEM;
		LCD_writeString(convertADC(adc_val,-1));

	}
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
	__bic_SR_register_on_exit(CPUOFF);
}


