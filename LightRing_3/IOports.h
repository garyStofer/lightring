/*****************************************************
Project : Light Ring
File    : IOports.h
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

#define LED6        (PORTA.0)
#define LED5        (PORTA.3)
#define LED4        (PORTA.5)
#define LED3        (PORTA.7)
#define LED2        (PORTB.2)
#define LED1        (PORTB.1)
#define VREG        (PORTB.0)


extern void IOInit(void);