/*****************************************************
Project : Light Ring
File    : IO.c
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
#include <IOports.h>

void IOInit(void)
{
    // Input/Output Ports initialization
    // Port A initialization
    DDRA=0xA8;      // A7=Out,A5=Out,A3=Out,A2-1=In,A0=In
    PORTA=0x00;     // LED 6-3 Off

    // Port B initialization
    // Func3=Out Func2=Out Func1=Out Func0=In 
    // State3=0 State2=0 State1=0 State0=T 
    DDRB=0x07;      // B3-B0=Out
    PORTB=0x00;     // LED2/1 and Vreg Off
}