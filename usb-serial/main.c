#include <msp430.h> 


#include	"stdbool.h"

#define		TXD			BIT1    // TXD on P1.1
#define		RXD			BIT2	// RXD on P1.2

#define		Bit_time		104	// 9600 Baud, SMCLK=1MHz (1MHz/9600)=104
#define		Bit_time_5		52	// Time for half a bit.



unsigned char BitCnt;		// Bit count, used when transmitting byte
unsigned int TXByte;		// Value sent over UART when Transmit() is called
unsigned int RXByte;		// Value recieved once hasRecieved is set

unsigned int i;			// for loop variable

bool isReceiving;		// Status for when the device is receiving
bool hasReceived;		// Lets the program know when a byte is received

void Transmit(void);
void Receive(void);


unsigned int mystate = 0;
void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;

	if (CALBC1_1MHZ ==0xFF || CALDCO_1MHZ == 0xFF)
	{
		while(1); // If cal constants erased, trap CPU!!
	}
	BCSCTL1 = CALBC1_1MHZ; // Set range
	DCOCTL = CALDCO_1MHZ; // Set DCO step + modulation



	P1SEL |= TXD;				// Connected TXD to timer
	P1DIR |= TXD;

	P1IES |= RXD;				// RXD Hi/lo edge interrupt
	P1IFG &= ~RXD;				// Clear RXD (flag) before enabling interrupt
	P1IE |= RXD;

	P1DIR = 0X41;               // set both leds to output
	P1OUT = 0;                  // and set them off initially

	isReceiving = false;			// Set initial values
	hasReceived = false;

	P1SEL &= ~0x08;					// Select Port 1 P1.3 (push button)
	P1IE |= 0x08;					// Port 1 Interrupt Enable P1.3 (push button)
	__enable_interrupt();				// Enable interrupts

	__bis_SR_register(GIE);			// interrupts enabled\


	while(1)
	{
		if (hasReceived)		// If the device has recieved a value
		{
			Receive();
		}
		if (mystate == 0)
		{
			P1OUT = 0x01; // LED on
			_delay_cycles(800000);
			P1OUT = 0x00;
			_delay_cycles(800000);


		}
		else if (mystate == 1)
		{
			P1OUT = 0x01; // LED on
			_delay_cycles(500000);
			P1OUT = 0x00;
			_delay_cycles(500000);

		}
		else if (mystate == 2)
		{
			P1OUT = 0x40; // LED on
			_delay_cycles(500000);
			P1OUT = 0x00;
			_delay_cycles(50000);
		}
		else if (mystate == 3)
		{
			P1OUT = 0x40; // LED on
			_delay_cycles(200000);
			P1OUT = 0x01; // LED on
			_delay_cycles(200000);
		}
		else if (mystate == 4)
		{
			P1OUT = 0x40; // LED on
			_delay_cycles(200000);
			P1OUT = 0x00;
			_delay_cycles(50000);
			P1OUT = 0x40; // LED on
			_delay_cycles(200000);
			P1OUT = 0x00;
			_delay_cycles(50000);
			P1OUT = 0x01; // LED on
			_delay_cycles(200000);
			P1OUT = 0x00;
			_delay_cycles(50000);
			P1OUT = 0x01; // LED on
			_delay_cycles(200000);
			P1OUT = 0x00;
			_delay_cycles(50000);

		}





	}
}
/**
 * Handles the received byte and calls the needed functions.\
 **/
void Receive()
{
	hasReceived = false;	// Clear the flag
	switch(RXByte)		// Switch depending on command value received
	{

		case '1':
			mystate = 1;
			break;

		case '2':
			mystate = 2;
			break;

		case '3':
			mystate = 3;
			break;

		case '4':
			mystate = 4;
			break;


		default:;
    }
}
/**
 * Transmits the value currently in TXByte. The function waits till it is
 *   finished transmiting before it returns.
 **/
void Transmit()
{
	while(isReceiving);				// Wait for RX completion

  	TXByte |= 0x100;				// Add stop bit to TXByte (which is logical 1)
  	TXByte = TXByte << 1;				// Add start bit (which is logical 0)
  	BitCnt = 0xA;					// Load Bit counter, 8 bits + ST/SP

  	CCTL0 = OUT;					// TXD Idle as Mark
  	TACTL = TASSEL_2 + MC_2;			// SMCLK, continuous mode
  	CCR0 = TAR;					// Initialize compare register
  	CCR0 += Bit_time;				// Set time till first bit
  	CCTL0 =  CCIS0 + OUTMOD0 + CCIE;		// Set signal, intial value, enable interrupts
  	while ( CCTL0 & CCIE );				// Wait for previous TX completion
}




#pragma vector=PORT1_VECTOR
__interrupt void Port_1 (void) {
	if (P1IFG & 0x08) {
		P1IFG &= ~0x08;
		if (mystate < 3)
		{
			mystate++;
		}
		else
		{
			mystate = 0;
		}
	} else {
		isReceiving = true;

		P1IE &= ~RXD;			// Disable RXD interrupt
		P1IFG &= ~RXD;			// Clear RXD IFG (interrupt flag)

		TACTL = TASSEL_2 + MC_2;	// SMCLK, continuous mode
		CCR0 = TAR;			// Initialize compare register
		CCR0 += Bit_time_5;		// Set time till first bit
		CCTL0 = OUTMOD1 + CCIE;		// Dissable TX and enable interrupts

		RXByte = 0;			// Initialize RXByte
		BitCnt = 0x9;			// Load Bit counter, 8 bits + ST
	}
}

/**
 * Timer interrupt routine. This handles transmiting and receiving bytes.
 **/
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	if(!isReceiving)
	{
		CCR0 += Bit_time;				// Add Offset to CCR0
		if ( BitCnt == 0)				// If all bits TXed
		{
  			TACTL = TASSEL_2;			// SMCLK, timer off (for power consumption)
			CCTL0 &= ~ CCIE ;			// Disable interrupt
		}
		else
		{
			CCTL0 |=  OUTMOD2;			// Set TX bit to 0
			if (TXByte & 0x01)
				CCTL0 &= ~ OUTMOD2;		// If it should be 1, set it to 1
			TXByte = TXByte >> 1;
			BitCnt --;
		}
	}
	else
	{
		CCR0 += Bit_time;						// Add Offset to CCR0
		if ( BitCnt == 0)
		{
  			TACTL = TASSEL_2;					// SMCLK, timer off (for power consumption)
			CCTL0 &= ~ CCIE ;					// Disable interrupt

			isReceiving = false;

			P1IFG &= ~RXD;						// clear RXD IFG (interrupt flag)
			P1IE |= RXD;						// enabled RXD interrupt

			if ( (RXByte & 0x201) == 0x200)		// Validate the start and stop bits are correct
			{
				RXByte = RXByte >> 1;			// Remove start bit
				RXByte &= 0xFF;					// Remove stop bit
				hasReceived = true;
			}
  			__bic_SR_register_on_exit(CPUOFF);	// Enable CPU so the main while loop continues
		}
		else
		{
			if ( (P1IN & RXD) == RXD)			// If bit is set?
				RXByte |= 0x400;			// Set the value in the RXByte
			RXByte = RXByte >> 1;				// Shift the bits down
			BitCnt --;
		}
	}
}
