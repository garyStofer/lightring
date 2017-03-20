/*****************************************************
Project : Light Control
File    : ADC.H
Version : 1.0
Date    : 12/16/2012
Author  : Gene Breniman
Company : Young Embedded Systems
Comments: 


Chip type               : ATtiny45
AVR Core Clock frequency: 8.000000 MHz
Memory model            : Tiny
External RAM size       : 0
Data Stack size         : 64
*****************************************************/
#define ADC_VREF_TYPE       0x00
#define INTERNAL_REFERENCE  0x21

#define SW_ADC_IN	0x3

extern void AnaCompInit(void);
extern void ADCInit(void);
extern unsigned int read_adc(char adc_input);
