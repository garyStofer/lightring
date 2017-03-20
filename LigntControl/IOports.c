/*****************************************************
Project : Light Control
File    : IO.c
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
#include <IOports.h>

// **************************************************************************************************
// FUNCTION NAME:		IOInit()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to initialize the B-port direction and values. 
// **************************************************************************************************
// Global(s) Accessed:
// 	NONE
// 
// **************************************************************************************************
// Input(s):
// 	char ucAddress	- EEPROM addess to read
// 
// Output(s):
// 	NONE
// 
// **************************************************************************************************
// Return Type:		NONE
// **************************************************************************************************
void IOInit(void)
{
    DDRB=0x12;      // B4&B1=Out
    PORTB=0x00;     // RED_LED/1 Off
}