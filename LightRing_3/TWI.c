/*****************************************************
Project : Light Ring
File    : TWI.C
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
#include <delay.h>
#include "TWI.h"
 
static volatile overflowState_t overflowState;

static unsigned char rxBuf[RX_BUFFER_SIZE];
static volatile unsigned char rxHead;
static volatile unsigned char rxTail;

static unsigned char txBuf[TX_BUFFER_SIZE];
static volatile unsigned char txHead;
static volatile unsigned char txTail;

unsigned char Master_Transfer(unsigned char); 
unsigned char Master_Stop(void);
unsigned char SlaveAddr;

union state 
{ 
 	// Can reuse the state for error states due to that it will not be need if there exists an error. 
	unsigned char errorState;
	struct 
	{ 
		unsigned char addressMode : 1; 
		unsigned char masterWriteDataMode : 1; 
		unsigned char unused : 6; 
	}; 
} state; 

/*--------------------------------------------------------------- 
flushes the slave TWI buffers 
---------------------------------------------------------------*/ 
static void flushTwiBuffers(void)
{
    rxTail = 0;
    rxHead = 0;
    txTail = 0;
    txHead = 0;
}
/*--------------------------------------------------------------- 
single master initialization function 
---------------------------------------------------------------*/ 
void Master_Init(void) 
{ 
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
slave initialization function 
---------------------------------------------------------------*/ 
void Slave_Init(unsigned char pri_sec)
{
    flushTwiBuffers();
    SlaveAddr = 0x18 + pri_sec;

    // In Two Wire mode (USIWM1, USIWM0 = 1X), the slave USI will pull SCL
    // low when a start condition is detected or a counter overflow (only
    // for USIWM1, USIWM0 = 11).  This inserts a wait state.  SCL is released
    // by the ISRs (USI_START_vect and USI_OVERFLOW_vect).

    DDR_USI |= (PORT_USI_SCL | PORT_USI_SDA);	// Set SCL and SDA as output
	PULL_SDA_HIGH; 
	PULL_SCL_HIGH; 
    EN_SDA_AS_IN;

    // enable Start Condition Interrupt
    // disable Overflow Interrupt
    // set USI in Two-wire mode, no USI Counter overflow hold
    // Shift Register Clock Source = external, positive edge
    // 4-Bit Counter Source = external, both edges
    // no toggle clock-port pin
    USICR = USISIE|USIWM1|USICS1;

    // clear all interrupt flags and reset overflow counter
    USISR = USISIF|USIOIF|USIPF;
}

/*--------------------------------------------------------------- 
Use this function to get hold of the error message from the last transmission 
---------------------------------------------------------------*/ 
unsigned char Get_State_Info(void) 
{ 
	return(state.errorState); // Return error state. 
} 

/*--------------------------------------------------------------- 
USI Transmit and receive function. LSB of first byte in data 
indicates if a read or write cycles is performed. If set a read 
operation is performed. 

Function generates (Repeated) Start Condition, sends address and 
R/W, Reads/Writes Data, and verifies/sends ACK. 

Success or error code is returned. Error codes are defined in 
TWI.h 
---------------------------------------------------------------*/ 
unsigned char Master_Msg(unsigned char *msg, unsigned char msgSize) 
{ 
	unsigned char tempUSISR_8bit;
	unsigned char tempUSISR_1bit;

	// Prepare register value to: Clear flags,shift 8 bits i.e. count 16 clock edges.
	tempUSISR_8bit = USISIF|USIOIF|USIPF; 
	// Prepare register value to: Clear flags,shift 1 bit i.e. count 2 clock edges.
	tempUSISR_1bit = USISIF|USIOIF|USIPF|0xE;

	state.errorState = 0; 
	state.addressMode = TRUE; 

	 // The LSB in the address byte determines if is a masterRead or masterWrite operation. 
    if(!(*msg & READ_BIT))
	{ 
		state.masterWriteDataMode = TRUE; 
	} 

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
	/*Write address and Read/Write data */ 
	do 
	{ 
		/* If masterWrite cycle (or inital address tranmission)*/ 
		if (state.addressMode || state.masterWriteDataMode) 
		{ 
			/* Write a byte */ 
			PULL_SCL_LOW; 
			USIDR = *(msg++); // Setup data. 
			Master_Transfer(tempUSISR_8bit); // Send 8 bits on bus. 

			/* Clock and verify (N)ACK from slave */ 
			EN_SDA_AS_IN; 
			if(Master_Transfer(tempUSISR_1bit) & NACK_BIT) 
			{ 
//				state.errorState = (state.addressMode) ? NO_ACK_ON_ADDRESS : NO_ACK_ON_DATA; 
			} 
			state.addressMode = FALSE; // Only perform address transmission once. 
		} 
		else	 /* Else masterRead cycle*/ 
		{ 
			/* Read a data byte */ 
			EN_SDA_AS_IN; 
			*(msg++) = Master_Transfer(tempUSISR_8bit); 

			/* Prepare to generate ACK (or NACK in case of End Of Transmission) */ 
			USIDR = (msgSize == 1) ? 0xFF : 0x00; 
			Master_Transfer(tempUSISR_1bit); // Generate ACK/NACK. 
		} 
	} while(--msgSize) ; // Until all data sent/received. 

	Master_Stop(); // Send a STOP condition on the TWI bus. 

	/* Transmission successfully completed */ 
	return (TRUE); 
} 

/*--------------------------------------------------------------- 
Core function for shifting data in and out from the USI. 
Data to be sent has to be placed into the USIDR prior to calling 
this function. Data read, will be return'ed from the function. 
---------------------------------------------------------------*/ 
unsigned char Master_Transfer( unsigned char temp ) 
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

/*--------------------------------------------------------------- 
Function for generating a TWI Stop Condition. Used to release 
the TWI bus. 

STOP CONDITION: A SDA LOW to HIGH while SCL is HIGH 

 ____  |     ____ 
     \_|____/ 
       |   _______ 
_______|_/ 
       | 
---------------------------------------------------------------*/ 
unsigned char Master_Stop( void ) 
{ 
	PULL_SDA_LOW; 
	PULL_SCL_HIGH; 
	while(!(IS_SCL_HIGH)); 
	delay_us(T4_TWI); 
	PULL_SDA_HIGH; 
	delay_us(T2_TWI); 
	return (TRUE); 
} 

/*--------------------------------------------------------------- 
slave transmit function, put data in the transmission buffer, wait if buffer is full 
---------------------------------------------------------------*/ 
void Slave_Transmit_Byte(unsigned char data)
{
    unsigned char tmphead;

    tmphead = (txHead + 1) & TX_BUFFER_MASK;		// calculate buffer index
    while(tmphead == txTail);				// wait for free space in buffer
    txBuf[tmphead] = data;				// store data in buffer
    txHead = tmphead;					// store new index
}

/*--------------------------------------------------------------- 
slave recieve function, return a byte from the receive buffer, wait if buffer is empty 
---------------------------------------------------------------*/ 
unsigned char Slave_Receive_Byte(void)
{
    while(rxHead == rxTail);				// wait for Rx data
    rxTail = (rxTail + 1) & RX_BUFFER_MASK;		// calculate buffer index
    return rxBuf[rxTail];					// return data from the buffer.
}

/*--------------------------------------------------------------- 
slave recieve buffer check function 
---------------------------------------------------------------*/ 
char Slave_Is_Data_In_Buffer(void)
{
    return rxHead != rxTail;		// return 0 (false) if the receive buffer is empty
}

/********************************************************************************
                            USI Start Condition ISR
********************************************************************************/
interrupt [USI_STR] void usi_start_isr(void)
{
    // set default starting conditions for new TWI package
    overflowState = CHECK_ADDRESS;

    // set SDA as input
    DDR_USI &= ~PORT_USI_SDA;

    // wait for SCL to go low to ensure the Start Condition has completed (the
    // start detector will hold SCL low ) - if a Stop Condition arises then leave
    // the interrupt to prevent waiting forever - don't use USISR to test for Stop
    // Condition as in Application Note AVR312 because the Stop Condition Flag is
    // going to be set from the last TWI sequence

    // while SCL his high and SDA is low
    while((IS_SCL_HIGH) && !(IS_SDA_HIGH));

    if(!(IS_SDA_HIGH))
    {
        // a Stop Condition did not occur
        // keep Start Condition Interrupt enabled to detect RESTART
        // enable Overflow Interrupt
        // set USI in Two-wire mode, hold SCL low on USI Counter overflow
        USICR = USISIE|USIOIE|USIWM1|USIWM0|USICS1;
    }
    else
    {
        // a Stop Condition did occur
        // enable Start Condition Interrupt
        // disable Overflow Interrupt
        // set USI in Two-wire mode, no USI Counter overflow hold
        USICR = USISIE|USIWM1|USICS1;
    }

    // clear interrupt flags - resetting the Start Condition Flag will
    // release SCL
    // set USI to sample 8 bits (count 16 external SCL pin toggles)
    USISR = USISIF|USIOIF|USIPF;
}

/********************************************************************************
                                USI Overflow ISR
Handles all the communication. Only disabled when waiting for a new Start Condition.
********************************************************************************/
interrupt [USI_OVF] void usi_ovf_isr(void)
{
    switch(overflowState)
    {
        // Address mode: check address and send ACK (and next SEND_DATA) if OK,
        // else reset USI
        case CHECK_ADDRESS:
            if((USIDR == 0) || ((USIDR >> 1) == SlaveAddr))
            {
                overflowState = (USIDR & SEND_DATA) ? SEND_DATA : REQUEST_DATA;
                SET_TO_SEND_ACK();
            }
            else
            {
                SET_TO_START_CONDITION_MODE();
            }
            break;

        // Master write data mode: check reply and goto SEND_DATA if OK,
        // else reset USI
        case CHECK_REPLY_FROM_SEND_DATA:
            if(USIDR)
            {
                // if NACK, the master does not want more data
                SET_TO_START_CONDITION_MODE();
                return;
            }
            // from here we just drop straight into SEND_DATA if the
            // master sent an ACK

        // copy data from buffer to USIDR and set USI to shift byte
        // next REQUEST_REPLY_FROM_SEND_DATA
        case SEND_DATA:
            // Get data from Buffer
            if(txHead != txTail)
            {
                txTail = (txTail + 1) & TX_BUFFER_MASK;
                USIDR = txBuf[txTail];
            }
            else
            {
                // the buffer is empty
                // The buffer is only empty because the master requested data before it was available
                // We were sending a NACK here, so really all we want to do is to keep SCL low here
                // and return until data gets put in the tx buffer and a send is initiated by the call
                // in the main loop. There is probably a more elegant approach than this, but in
                // testing this appears to work much better than before.

                //SET_TO_START_CONDITION_MODE();
                return;
            }
            overflowState = REQUEST_REPLY_FROM_SEND_DATA;
            SET_TO_SEND_DATA();
            break;

        // set USI to sample reply from master
        // next CHECK_REPLY_FROM_SEND_DATA
        case REQUEST_REPLY_FROM_SEND_DATA:
            overflowState = CHECK_REPLY_FROM_SEND_DATA;
            SET_TO_READ_ACK();
            break;

        // Master read data mode: set USI to sample data from master, next
        // GET_DATA_AND_SEND_ACK
        case REQUEST_DATA:
            overflowState = GET_DATA_AND_SEND_ACK;
            SET_TO_READ_DATA();
            break;

        // copy data from USIDR and send ACK
        // next REQUEST_DATA
        case GET_DATA_AND_SEND_ACK:
            // put data into buffer. Not necessary, but prevents warnings
            rxHead = (rxHead + 1) & RX_BUFFER_MASK;
            rxBuf[rxHead] = USIDR;
            overflowState = REQUEST_DATA;
            SET_TO_SEND_ACK();
            break;
    }
}



