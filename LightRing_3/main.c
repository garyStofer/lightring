/*****************************************************
This program was produced by the
CodeWizardAVR V2.03.9 Standard
Automatic Program Generator
© Copyright 1998-2008 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : Light Ring
Version : A
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
#include <delay.h>
#include "IOports.h"
#include "Timers.h"
#include "TWI.h"

const unsigned char LED_Map[6] = {0x02, 0x04, 0x80, 0x20, 0x08, 0x01};

void SetLED_A(unsigned char Mask, unsigned char value)
{
    char i;
    // turn off target LED
    PORTA &= ~Mask;
    MS_Wait(5);         // allow power down time
    if(value == 31)
    {
        return;
    }
    // turn on target LED
        PORTA |= Mask;
    delay_us(10);
    for(i = 0; i < value; i++)
    {
        PORTA &= ~Mask;
        PORTA |= Mask;
    }
}

void SetLED_B(unsigned char Mask, unsigned char value)
{
    char i;
    // turn off target LED
    PORTB &= ~Mask;
    MS_Wait(5);         // allow power down time
    if(value == 31)
    {
        return;
    }
    // turn on target LED
    PORTB |= Mask;
    delay_us(10);
    for(i = 0; i < value; i++)
    {
        PORTB &= ~Mask;
        PORTB |= Mask;
    }
}

void SetLED(unsigned char led, unsigned char value)
{
    unsigned char Mask;
    
    Mask = LED_Map[led];
    if(led <= 1)
    {
        SetLED_B(Mask, value);
    }
    else
    {
        SetLED_A(Mask, value);
    }
}

void main(void)
{
    // Declare your local variables here
    unsigned char Primary_Secondary;   // 0 = Primary/1 = Secondary
    unsigned char command;
    unsigned char led;
    unsigned char intensity;

    // Crystal Oscillator division factor: 1
#pragma optsize-
    CLKPR = 0x80;
    CLKPR  =0x00;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

    IOInit();           // Input/Output Ports initialization
    InitTimer0();       // Timer/Counter 0 initialization

    // Global enable interrupts
#asm("sei")

    Primary_Secondary = (PINA & 0x01) ? 0 : 1;
    // change PA0 into a digital output (set low);
    DDRA = 0xA9;            // A7=Out,A5=Out,A3=Out,A2-1=In,A0=Out
    Slave_Init(Primary_Secondary);
    VREG = 1;
    while (1)
    {
        if(Slave_Is_Data_In_Buffer())
        {
            command = Slave_Receive_Byte();
            led = command >> 5;
            intensity = command & 0x1F;
            SetLED(led, intensity);      // manage LED
        }
    };
}
