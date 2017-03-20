/*****************************************************
Project : Light Control
File    : IOports.h
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

#define RED_LED        (PORTB.1)
#define GRN_LED        (PORTB.4)


extern void IOInit(void);