/*****************************************************
Project : Light Control
File    : EEPROM.c
Version : 1.0
Date    : 12/26/2012
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
#include "EEPROM.h"
#include <delay.h>

// **************************************************************************************************
// FUNCTION NAME:		EEPROM_write()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to write data to the specified EEPROM address. 
// **************************************************************************************************
// Global(s) Accessed:
// 	NONE
// 
// **************************************************************************************************
// Input(s):
// 	char ucAddress	- EEPROM addess to write
//  char ucData		- data to write to EEPROM
// 
// Output(s):
// 	NONE
// 
// **************************************************************************************************
// Return Type:		NONE
// **************************************************************************************************
void EEPROM_write(char ucAddress, char ucData)
{
    /* Wait for completion of previous write */
    while(EECR & EEPE);
    /* Set Programming mode */
    EECR = 0;
    /* Set up address and data registers */
    EEARL = ucAddress;
    EEDR = ucData;
    /* Write logical one to EEMPE */
    EECR |= EEMPE;
    /* Start eeprom write by setting EEPE */
    EECR |= EEPE;
}

// **************************************************************************************************
// FUNCTION NAME:		EEPROM_read()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to read data fromo the specified EEPROM address. 
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
// Return Type:		char - data read from the EEPROM (at specified address)
// **************************************************************************************************
char EEPROM_read(char ucAddress)
{
    /* Wait for completion of previous write */
    while(EECR & EEPE);
    /* Set up address register */
    EEARL = ucAddress;
    /* Start eeprom read by writing EERE */
    EECR |= EERE;
    /* Return data from data register */
    return EEDR;
}