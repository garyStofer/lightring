/*****************************************************
Project : Light Control
File    : TWI.C
Version : 1.0
Date    : 12/16/2012
Author  : Gene Breniman
Company : Young Embedded Systems
Comments: This module is stripped down to support Master send
          only.  All slave and master read functionality has
          been removed to minimize memory usage.

Chip type               : ATtiny45
AVR Core Clock frequency: 8.000000 MHz
Memory model            : Tiny
External RAM size       : 0
Data Stack size         : 32
*****************************************************/
#include <tiny25.h>
#include <delay.h>
#include "TWI.h"

char Master_Transfer(char); 
char Master_Stop(void);

/*--------------------------------------------------------------- 
single master initialization function 
---------------------------------------------------------------*/ 
// **************************************************************************************************
// FUNCTION NAME:		Master_Init()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to initialize the I2C master (hardware and software) 
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
void Master_Init(void) 
{ 
	// configure bi-direction I/O for clock/data
	PULL_SDA_HIGH; 
	PULL_SCL_HIGH; 

	EN_SCL_AS_OUT; 
	EN_SDA_AS_OUT; 

	USIDR = 0xFF; // Preload dataregister with "released level" data.
	// Disable Interrupts, Two-wire mode, Software strobe as clock source
	USICR = USIWM1|USICS1|USICLK;
	// Clear flags, and reset counter.
	USISR = USISIF|USIOIF|USIPF; 
} 

/*--------------------------------------------------------------- 
USI Transmit function.  

Function generates (Repeated) Start Condition, sends address and 
W, Writes Data, and verifies ACK. 

Success is returned. 
---------------------------------------------------------------*/ 
// **************************************************************************************************
// FUNCTION NAME:		Master_Msg()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to send a message via the I2C master. Function generates (Repeated) Start
//  Condition, sends address and Write flag, Writes Data, and verifies ACK.
//  Note: this is a hardware/software implementation of the I2C write transfer (this device has a
//  general serial tansmitter and start/stop signaling is performed under software control).
// **************************************************************************************************
// Global(s) Accessed:
// 	NONE
// 
// **************************************************************************************************
// Input(s):
// 	char *msg		- pointer to transmit data buffer
//  char msgSize	- size of message to send
// 
// Output(s):
// 	NONE
// 
// **************************************************************************************************
// Return Type:		char - Returns true at completion of message (no error returned)
// **************************************************************************************************
char Master_Msg(char *msg, char msgSize) 
{ 
	char tempUSISR_8bit;
	char tempUSISR_1bit;

	// Prepare register value to: Clear flags,shift 8 bits i.e. count 16 clock edges.
	tempUSISR_8bit = USISIF|USIOIF|USIPF; 
	// Prepare register value to: Clear flags,shift 1 bit i.e. count 2 clock edges.
	tempUSISR_1bit = USISIF|USIOIF|USIPF|0xE;

	/* Release SCL to ensure that (repeated) Start can be performed */ 
	PULL_SCL_HIGH; 
	while(!(IS_SCL_HIGH)); 
	delay_us(T2_TWI); // Delay for T2TWI if STANDARD_MODE 

	/* Generate Start Condition */ 
	PULL_SDA_LOW; 
	delay_us(T4_TWI); 
	PULL_SCL_LOW; 
	delay_us(T4_TWI); 
	PULL_SDA_HIGH; 
	/*Write address and data */ 
	do 
	{ 
        /* Write a byte */ 
        PULL_SCL_LOW; 
        USIDR = *(msg++); // Setup data. 
        Master_Transfer(tempUSISR_8bit); // Send 8 bits on bus. 

        /* Clock and verify (N)ACK from slave */ 
        EN_SDA_AS_IN; 
        if(Master_Transfer(tempUSISR_1bit) & NACK_BIT) 
        {
            break; 
        } 
	} while(--msgSize) ; // Until all data sent/received. 

	Master_Stop(); // Send a STOP condition on the TWI bus. 

	/* Transmission successfully completed */ 
	return (TRUE); 
} 

// **************************************************************************************************
// FUNCTION NAME:		Master_Transfer()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is provides low level support for sending I2C data.  SCL clocking is generated in
//  software until transfer is complete. Data to be sent has to be placed into the USIDR prior to
//  calling this function. Data read, will be return'ed from the function.
// **************************************************************************************************
// Global(s) Accessed:
// 	NONE
// 
// **************************************************************************************************
// Input(s):
// 	char temp - USI status register state for sending data (overwritten in processing)
// 
// Output(s):
// 	NONE
// 
// **************************************************************************************************
// Return Type:		char - USI data register at completion of transmission (reply)
// **************************************************************************************************
char Master_Transfer( char temp ) 
{ 
	USISR = temp; // Set USISR according to temp. 
	// Prepare clocking. 
	// Interrupts disabled, Set Two-wire mode, Software clock strobe as source, Toggle Clock Port.
	temp = USIWM1|USICS1|USICLK|USITC;  
	do 
	{ 
		delay_us(T2_TWI); 
		USICR = temp; // Generate positve SCL edge.
		while(!(IS_SCL_HIGH)); 
		delay_us(T4_TWI); 
		USICR = temp; // Generate negative SCL edge. 
	} while(!(USISR & USIOIF)); // Check for transfer complete. 

	delay_us(T2_TWI); 
	temp = USIDR; // Read out data. 
	USIDR = 0xFF; // Release SDA. 
	EN_SDA_AS_OUT; 

	return temp; // Return the data from the USIDR 
} 

// **************************************************************************************************
// FUNCTION NAME:		Master_Stop()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to generate a 'STOP' condition, used to release the I2C bus 
// STOP CONDITION: A SDA LOW to HIGH while SCL is HIGH 
// 
// _____  |     _____ 	SDA
//      \_|____/ 
//        |   _______ 	SCL
// _______|_/ 
//        | 
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
// Return Type:		char - Returns true at completion of message (no error returned)
// **************************************************************************************************
char Master_Stop( void ) 
{ 
	PULL_SDA_LOW; 
	PULL_SCL_HIGH; 
	while(!(IS_SCL_HIGH)); 
	delay_us(T4_TWI); 
	PULL_SDA_HIGH; 
	delay_us(T2_TWI); 
	return (TRUE); 
} 



