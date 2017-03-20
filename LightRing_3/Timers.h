/*****************************************************
Project : Light Ring
File    : Timers.h
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

extern void InitTimer0(void);
extern void InitTimer1(void);
extern void MS_Wait(unsigned int delay);
extern void S_Wait(unsigned char delay);
extern void SetMdelay(unsigned int delay);
extern unsigned int GetMdelay(void);
