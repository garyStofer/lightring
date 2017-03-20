/*****************************************************
Project : Light Control
File    : Timers.c
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
#include <Timers.h>

int OneSecondPeriod;
unsigned int MSdelayValue;
unsigned int MSwaitValue;
char SdelayValue;
unsigned int PowerDownValue;

// **************************************************************************************************
// FUNCTION NAME:		InitTimer0()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to initialize timer 0 to produce 1ms interrupts (used for timing).
//  Also clear all software timing intervals.
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
void InitTimer0(void)
{
    // Timer/Counter 0 initialization - 1ms interrupt rate
    TCCR0A=0x00;    // Mode: Normal top=FFh, OC0A & OC0B outputs: Disconnected
    TCCR0B=0x03;    // Clock source: System Clock, Clock value: 125.000 kHz
    TCNT0=0x82;     // set counter value for 1msec interrupt 
    OCR0A=0x00;     // Output compare register (not in use)
    OCR0B=0x00;     // Output compare register (not in use)
    // Timer/Counter 0 Interrupt(s) initialization
    TIMSK=0x02;     // Enable Timer0 overflow interrupt
    
    // clear timer interval variables
    OneSecondPeriod = 0;
    MSdelayValue = 0;
    MSwaitValue = 0;
    SdelayValue = 0;
    PowerDownValue = 0;
}

// **************************************************************************************************
// FUNCTION NAME:		MS_Wait()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to setup the general Msec timer interval and wait until completion. 
// **************************************************************************************************
// Global(s) Accessed:
// 	NONE
// 
// **************************************************************************************************
// Input(s):
// 	unsigned int delay	- setup up Msec interval timer to specified count
// 
// Output(s):
// 	NONE
// 
// **************************************************************************************************
// Return Type:		NONE
// **************************************************************************************************
void MS_Wait(unsigned int delay)
{
    MSwaitValue = delay;
    while(MSwaitValue);
}

// **************************************************************************************************
// FUNCTION NAME:		SetMdelay()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to setup the general Msec timer interval and leave immediately. 
// **************************************************************************************************
// Global(s) Accessed:
// 	NONE
// 
// **************************************************************************************************
// Input(s):
// 	unsigned int delay	- setup up Msec interval timer to specified count
// 
// Output(s):
// 	NONE
// 
// **************************************************************************************************
// Return Type:		NONE
// **************************************************************************************************
void SetMdelay(unsigned int delay)
{
    MSdelayValue = delay;
}

// **************************************************************************************************
// FUNCTION NAME:		GetMdelay()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to get the present value of the Msec interval timer. 
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
// Return Type:		unsigned int	- present value of the Msec interval (zero if done)
// **************************************************************************************************
unsigned int GetMdelay(void)
{
    return MSdelayValue;
}

// **************************************************************************************************
// FUNCTION NAME:		SetSdelay()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to setup the general Sec timer interval and leave immediately. 
// **************************************************************************************************
// Global(s) Accessed:
// 	NONE
// 
// **************************************************************************************************
// Input(s):
// 	char delay	- setup up Sec interval timer to specified count
// 
// Output(s):
// 	NONE
// 
// **************************************************************************************************
// Return Type:		NONE
// **************************************************************************************************
void SetSdelay(char delay)
{
    SdelayValue = delay;
}

// **************************************************************************************************
// FUNCTION NAME:		GetSdelay()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called the get the present value of the Sec interval timer. 
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
// Return Type:		char	- present value of the Sec interval (zero if done)
// **************************************************************************************************
char GetSdelay(void)
{
    return SdelayValue;
}

// **************************************************************************************************
// FUNCTION NAME:		SetPowerDownTime()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called the present value powerdown Sec interval timer. 
// **************************************************************************************************
// Global(s) Accessed:
// 	NONE
// 
// **************************************************************************************************
// Input(s):
// 	unsigned int PDtime	- Powerdown time in seconds
// 
// Output(s):
// 	NONE
// 
// **************************************************************************************************
// Return Type:		NONE
// **************************************************************************************************
void SetPowerDownTime(unsigned int PDtime)
{
    PowerDownValue = PDtime;
}

// **************************************************************************************************
// FUNCTION NAME:		IsPDtimeExpired()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to determine if the powerdown Sec interval has expired.
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
// Return Type:		char true/false based on if PowerDownValue = 0 (timer expired).
// **************************************************************************************************
char IsPDtimeExpired(void)
{
    return ((PowerDownValue == 0) ? 1 : 0);
}

// **************************************************************************************************
// FUNCTION NAME:		timer0_ovf_isr()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is timer 0 interrupt routine (called at A a 1ms rate) 
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
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
    // Reinitialize Timer 0 value
    TCNT0=0x82;				// re-establish timeout value
    // if waiting on mSec delay time, decrement count
    if(MSdelayValue)		
    {
        MSdelayValue--;
    }
    // if waiting on mSec wait time, decrement count
    if(MSwaitValue)
    {
        MSwaitValue--;
    }
    // increment 1 second counter
    OneSecondPeriod++;
    // if counter terminal (1 seconds has passed)
    if(OneSecondPeriod >= 1000)
    {
        // clear 1 seconds counter
    	OneSecondPeriod = 0;
    	// if waiting on a second delay time, decrement count
        if(SdelayValue)
        {
            SdelayValue--;
        }
    	// if waiting on the powerdown delay time, decrement count
       if(PowerDownValue)
        {
            PowerDownValue--;
        }
    }
}
