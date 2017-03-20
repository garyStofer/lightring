/*****************************************************
Project : Light Control
File    : Timers.h
Version : 1.0
Date    : 12/16/2012
Author  : Gene Breniman
Company : Young Embedded Systems, LLC.
Comments: 


Chip type               : ATtiny45
AVR Core Clock frequency: 8.000000 MHz
Memory model            : Tiny
External RAM size       : 0
Data Stack size         : 64
*****************************************************/

extern void InitTimer0(void);
extern void MS_Wait(unsigned int delay);
extern void SetMdelay(unsigned int delay);
extern unsigned int GetMdelay(void);
extern void SetSdelay(char delay);
extern char GetSdelay(void);
extern void SetPowerDownTime(unsigned int PDtime);
extern char IsPDtimeExpired(void);
