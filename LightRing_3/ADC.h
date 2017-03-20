/*****************************************************
Project : Light Ring
File    : ADC.H
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
#define ADC_VREF_TYPE       0x00
#define INTERNAL_REFERENCE  0x21

extern void AnaCompInit(void);
extern void ADCInit(void);
unsigned int read_adc(unsigned char adc_input);
