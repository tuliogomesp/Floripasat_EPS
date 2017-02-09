#include <msp430.h> 
#include <stdlib.h>
#include "eps_onewire.h"
#include "eps_i2c.h"
#include "eps_timer.h"
#include "eps_uart.h"
#include "ADS1248.h"
#include "pid.h"



void config_msp430(void);


void main(void){

	WDTCTL = WDTPW + WDTHOLD;

	config_msp430();

	config_ADS1248(6);
	config_DS2775();
	__bis_SR_register(GIE);
	while(1);
}


/**********************************
 * Function name: config_msp430
 *
 * Arguments: void
 *
 * Returns: void
 *
 */


void config_msp430(void){

	/*Clock Configuration:
	 * MCKL = default DCO = 1.045MHz
	 * SMCKL = default DCO = 1.045MHz
	 * ACKL = REFOCLK = 32.768kHz
	 */
	UCSCTL4 |= SELA_2 + SELS_3;	// SELA_2: ACLK source is REFOCLK (32768Hz), SELS_3: SMCL source is DCOCLK (1.045MHz)

	/* UCA2 UART configuration:
	 * baud rate: 9600
	 * 	 */
	P9SEL |= 0x0C;                            // Assign P9.2 to UCA2TXD and P9.3 to UCA2RXD

	UCA2CTL1 |= UCSWRST;                      // **Put state machine in reset**
	UCA2CTL1 |= UCSSEL_1;                     // CLK = SMCLK
	UCA2BR0 = 0x03;                           // baud rate selection 32768/UCA2BRO=9600 => UCA0BRO = 3
	UCA2BR1 = 0x00;
	UCA2MCTL = UCBRS_3|UCBRF_0;               // Modulation UCBRSx=3, UCBRFx=0
	UCA2CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

	#ifdef _DEBUG
		uart_tx("system booting\r\n");
	#endif

	/* Timer A0 configuration
	 * Interrupt period: 1s
	 */

	P1DIR |= 0x01;                          // P1.0 output
	TA0CCR0 = 32768;						// timer A0 CCR0 interrupt period = 32768 * 1/32768 = 1s
	TA0CCTL0 = CCIE;                        // timer A0 CCR0 interrupt enabled
	TA0CTL = TASSEL_1 + MC_1 + TACLR;       // SMCLK, upmode, timer A interrupt enable, clear TAR

	/* Timer A1 configuration
	 * Interrupt period: 100.006ms
	 */

	P3DIR |= 0x01;							// P3.0 output
	TA1CCR0 = 3277;							// timer A1 CCR0 interrupt period = 3277 * 1/32768 = 100.006ms
	TA1CCTL0 = CCIE;						// timer A1 CCR0 interrupt enabled
	TA1CTL = TASSEL_1 + MC_1 + TACLR;       // SMCLK, upmode, timer A interrupt enable, clear TAR

	/* ADC configuration
	 *
	 */

	P6SEL = 0xff;                             			// Enable A/D channel inputs
	ADC12CTL0 = ADC12ON + ADC12MSC + ADC12SHT0_15; 		// Turn on ADC12, extend sampling time to avoid overflow of results
	ADC12CTL1 = ADC12SHP + ADC12CONSEQ_1;       		// Use sampling timer, sequence of channels
	ADC12MCTL0 = ADC12INCH_0;              				// ref+=AVcc, channel = A0
	ADC12MCTL1 = ADC12INCH_1;              				// ref+=AVcc, channel = A1
	ADC12MCTL2 = ADC12INCH_2;              				// ref+=AVcc, channel = A2
	ADC12MCTL3 = ADC12INCH_3;        					// ref+=AVcc, channel = A3
	ADC12MCTL4 = ADC12INCH_4;        					// ref+=AVcc, channel = A4
	ADC12MCTL5 = ADC12INCH_5;        					// ref+=AVcc, channel = A5
	ADC12MCTL6 = ADC12INCH_6;        					// ref+=AVcc, channel = A6
	ADC12MCTL7 = ADC12INCH_7;        					// ref+=AVcc, channel = A7
	ADC12MCTL8 = ADC12INCH_8;        					// ref+=AVcc, channel = A8
	ADC12MCTL9 = ADC12INCH_9;        					// ref+=AVcc, channel = A9
	ADC12MCTL10 = ADC12INCH_10;        					// ref+=AVcc, channel = A10
	ADC12MCTL11 = ADC12INCH_11;        					// ref+=AVcc, channel = A11
	ADC12MCTL12 = ADC12INCH_12;        					// ref+=AVcc, channel = A12
	ADC12MCTL13 = ADC12INCH_13;        					// ref+=AVcc, channel = A13
	ADC12MCTL14 = ADC12INCH_14;        					// ref+=AVcc, channel = A14
	ADC12MCTL15 = ADC12INCH_15 + ADC12EOS;   			// ref+=AVcc, channel = A15
	ADC12CTL0 |= ADC12ENC;                    			// Enable conversions



/*

	P1DIR |= BIT6;
	P1OUT ^= BIT6;
	P3DIR |= BIT6;

	*** SPI configuration ***
	UCB1CTL0 |=  UCMSB + UCMST + UCSYNC;  				// 3-pin, 8-bit SPI master
	UCB1CTL1 |= UCSSEL_2;                     			// SMCLK
	UCB1BR0 |= 0x02;                          			// BRCLK = SMCLK/2
	UCB1BR1 = 0;                              			//
	UCB1CTL1 &= ~UCSWRST;                     			// **Initialize USCI state machine**
	P5OUT &= ~BIT0;                           			// reset slave - RST - active low
	P5OUT |= BIT0;                            			// Now with SPI signals initialized,

	*** I2C Configuration ***
	P3SEL |= 0x06;                            // Assign I2C pins to USCI_B0
	UCB0CTL1 |= UCSWRST;                      // Enable SW reset
	UCB0CTL0 = UCMODE_3 + UCSYNC;             // I2C Slave, synchronous mode
	UCB0I2COA = 0x13;                         // Own Address is 0ABh
	UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
	IE2 |= UCB0TXIE;                          // Enable TX interrupt

	*** ADS1248 configuration ***
	P5DIR |= BIT0 + BIT4;
	P5OUT = BIT0;                             // Set slave reset - P3.
	P5SEL |= 0x0E;                            // P5.1,2,3 USCI_B1 option select
	P4DIR |= BIT6;
	P4OUT |= BIT6;

	*/
	__bis_SR_register(GIE);       // enable interrupts

	#ifdef _DEBUG
		uart_tx("system boot complete\r\n");
	#endif
}


