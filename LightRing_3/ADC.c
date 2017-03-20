/*****************************************************
Project : Light Ring
File    : ADC.c
Version : 1.0
Date    : 7/11/2012
Author  : Gene Breniman
Company : Young Embedded Systems, LLC.
Comments: 


Chip type               : ATtiny24
AVR Core Clock frequency: 8.000000 MHz
Memory model            : Tiny
External RAM size       : 0
Data Stack size         : 32
*****************************************************/
#include <tiny24.h>
#include "ADC.h"
#include <delay.h>

void AnaCompInit(void)
{
    // Analog Comparator initialization
    ACSR=0x80;                      // Analog Comparator: Off
}

void ADCInit(void)
{
    // ADC initialization
    DIDR0 = 0x07;                   // Digital input buffers off for ADC0, ADC1 and ADC2
    ADMUX = ADC_VREF_TYPE & 0xff;   // ADC Voltage Reference: AVCC pin
    ADCSRA = 0x87;                  // ADC Auto Trigger Source: None, ADC Clock frequency: 62.500 kHz
    ADCSRB = 0x0;                   // ADC Bipolar Input Mode: Off
}

// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input)
{
    ADMUX=adc_input | (ADC_VREF_TYPE & 0xff);
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
    ADCSRA|=0x40;
    // Wait for the AD conversion to complete
    while ((ADCSRA & 0x10)==0);
    ADCSRA|=0x10;       // clear ADC complete flag
    return ADCW;
}
