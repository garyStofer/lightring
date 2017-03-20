/*****************************************************
Project : Light Control
File    : ADC.c
Version : 1.0
Date    : 12/16/2012
Author  : Gene Breniman
Company : Young Embedded Systems
Comments: 


Chip type               : ATtiny45
AVR Core Clock frequency: 8.000000 MHz
Memory model            : Tiny
External RAM size       : 0
Data Stack size         : 32
*****************************************************/
#include <tiny25.h>
#include <ADC.h>
#include <delay.h>

// **************************************************************************************************
// FUNCTION NAME:		AnaCompInit()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to turn off the analog comparator. 
// **************************************************************************************************
// Global(s) Accessed:
// 	NONE
// 
// **************************************************************************************************
// Input(s):
// 	NONE
// 
// Output(s):
// 	NONE
// 
// **************************************************************************************************
// Return Type:		NONE
// **************************************************************************************************
void AnaCompInit(void)
{
    // Analog Comparator initialization
    ACSR = 0x80;                    // Analog Comparator: Off
}

// **************************************************************************************************
// FUNCTION NAME:		ADCInit()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to initialize the ADC. 
// **************************************************************************************************
// Global(s) Accessed:
// 	NONE
// 
// **************************************************************************************************
// Input(s):
// 	NONE
// 
// Output(s):
// 	NONE
// 
// **************************************************************************************************
// Return Type:		NONE
// **************************************************************************************************
void ADCInit(void)
{
    // ADC initialization
    DIDR0 = 0x08;                   // Digital input buffers off for ADC3
    ADMUX = ADC_VREF_TYPE & 0xff;   // ADC Voltage Reference: AVCC pin
    ADCSRA = 0x87;                  // ADC Auto Trigger Source: None, ADC Clock frequency: 62.500 kHz
    ADCSRB = 0x0;                   // ADC Bipolar Input Mode: Off
}

// **************************************************************************************************
// FUNCTION NAME:		ADCInit()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to read the selected ADC channel. 
// **************************************************************************************************
// Global(s) Accessed:
// 	NONE
// 
// **************************************************************************************************
// Input(s):
// 	char adc_input	- desired ADC channel to read
// 
// Output(s):
// 	NONE
// 
// **************************************************************************************************
// Return Type:		unsigned int - ADC reading
// **************************************************************************************************
unsigned int read_adc(char adc_input)
{
    ADMUX = adc_input | (ADC_VREF_TYPE & 0xff);
    // Delay needed for the stabilization of the ADC input voltage
    if( adc_input == INTERNAL_REFERENCE )
    {
        delay_us(1000);
    }
    else
    {
        delay_us(10);
    }

    // Start the AD conversion
    ADCSRA |= 0x40;
    // Wait for the AD conversion to complete
    while((ADCSRA & 0x10) == 0);
    ADCSRA |= 0x10;     // clear ADC complete flag
    return ADCW;
}
