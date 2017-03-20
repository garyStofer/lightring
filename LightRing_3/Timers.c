/*****************************************************
Project : Light Ring
File    : Timers.c
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
#include "Timers.h"
#include "IOports.h"

int OneSecondPeriod;
unsigned int MSdelayValue;
unsigned char SdelayValue;

void InitTimer0(void)
{
    // Timer/Counter 0 initialization
    TCCR0A=0x00;    // Mode: Normal top=FFh, OC0A & OC0B outputs: Disconnected
    TCCR0B=0x03;    // Clock source: System Clock, Clock value: 125.000 kHz
    TCNT0=0x82;     // set counter value for 1msec interrupt 
    OCR0A=0x00;     // Output compare register (not in use)
    OCR0B=0x00;     // Output compare register (not in use)
    // Timer/Counter 0 Interrupt(s) initialization
    TIMSK0=0x01;
    
    OneSecondPeriod = 0;
    MSdelayValue = 0;
}

void InitTimer1(void)
{
    // Timer/Counter 1 initialization
    TCCR1A=0x00;    // Mode: Normal top=FFFFh, OC1A & OC1B outputs: Disconnected
    TCCR1B=0x00;    // Clock source: System Clock, Timer 1 Stopped, Noise Canceler: Off
    // Timer/Counter 1 Interrupt(s) initialization
    TIMSK1=0x00;    // Timer 1 Overflow Interrupt Off, Input Capture Interrupt: Off, Compare A&B Match Interrupt: Off
}

void MS_Wait(unsigned int delay)
{
    MSdelayValue = delay;
    while(MSdelayValue);
}

void S_Wait(unsigned char delay)
{
    SdelayValue = delay;
    while(SdelayValue);
}

void SetMdelay(unsigned int delay)
{
    MSdelayValue = delay;
}

unsigned int GetMdelay(void)
{
    return MSdelayValue;
}

// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
//    unsigned int ADCval;
    
    // Reinitialize Timer 0 value
    TCNT0=0x82;
    if(MSdelayValue)
    {
        MSdelayValue--;
    }
    OneSecondPeriod++;
    if(OneSecondPeriod >= 1000)
    {
        OneSecondPeriod = 0;
        if(SdelayValue)
        {
            SdelayValue--;
        }
    }
}
