/*****************************************************
Project : Light Ring
File    : TWI.H
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
//********** Defines **********// 
// Defines controlling timing limits 

#define T2_TWI	5	// >4,7us 
#define T4_TWI	4	// >4,0us 

// I/O register defines
// USISR - USI Status Register
#define USISIF    0x80      // USI Start condition Interrupt Flag
#define USIOIF    0x40      // USI counter Overflow Interrupt Flag
#define USIPF     0x20      // USI stoP Flag
#define USIDC     0x10      // USI Data output Collision flag
// USICR - USI Control Register
#define USISIE    0x80      // USI Start condition Interrupt Enable
#define USIOIE    0x40      // USI counter Overflow Interrupt Enable
#define USIWM1    0x20      // USI Wire Mode 1 (MSB)
#define USIWM0    0x10      // USI Wire Mode 0 (LSB)
#define USICS1    0x08      // USI Clock Source 1 (MSB)
#define USICS0    0x04      // USI Clock Source 0 (LSB)
#define USICLK    0x02      // USI CLocK strobe
#define USITC     0x01      // USI Toggle Clock port pin
/**************************************************************************** 
Bit and byte definitions 
****************************************************************************/ 
#define READ_BIT	0x01      // R/W bit in "address byte". 
#define NACK_BIT	0x01      // (N)ACK bit. 

#define NO_DATA             0x00	// Transmission buffer is empty 
#define DATA_OUT_OF_BOUND	0x01	// Transmission buffer is outside SRAM space 
#define UE_START_CON        0x02	// Unexpected Start Condition 
#define UE_STOP_CON         0x03	// Unexpected Stop Condition 
#define UE_DATA_COL         0x04	// Unexpected Data Collision (arbitration) 
#define NO_ACK_ON_DATA      0x05	// The slave did not acknowledge all data 
#define NO_ACK_ON_ADDRESS   0x06	// The slave did not acknowledge the address 
#define MISSING_START_CON   0x07	// Generated Start Condition not detected on bus 
#define MISSING_STOP_CON    0x08	// Generated Stop Condition not detected on bus 

#define DDR_USI         DDRA 
#define PORT_USI        PORTA 
#define PIN_USI		    PINA 
#define PORT_USI_SDA	0x40 
#define PORT_USI_SCL	0x10 
#define PIN_USI_SDA     0x40 
#define PIN_USI_SCL     0x10 
// General defines 
#define TRUE        1 
#define FALSE		0 

typedef enum
{
  CHECK_ADDRESS                = 0x00,
  SEND_DATA                    = 0x01,
  REQUEST_REPLY_FROM_SEND_DATA = 0x02,
  CHECK_REPLY_FROM_SEND_DATA   = 0x03,
  REQUEST_DATA                 = 0x04,
  GET_DATA_AND_SEND_ACK        = 0x05
} overflowState_t;

#define RX_BUFFER_SIZE  8
#define TX_BUFFER_SIZE  8
#define RX_BUFFER_MASK  0x07
#define TX_BUFFER_MASK  0x07

//********** Globals **********// 
extern unsigned char SlaveAddr;

//********** Prototypes **********// 
void Master_Init(void); 
void Slave_Init(unsigned char pri_sec); 
unsigned char Master_Msg(unsigned char *, unsigned char); 
unsigned char Get_State_Info(void); 
void Slave_Transmit_Byte(unsigned char data);
unsigned char Slave_Receive_Byte(void);
char Slave_Is_Data_In_Buffer(void);
 

#define PULL_SCL_HIGH   PORT_USI |= PIN_USI_SCL 
#define PULL_SCL_LOW    PORT_USI &= ~PIN_USI_SCL 
#define PULL_SDA_LOW    PORT_USI &= ~PIN_USI_SDA 
#define PULL_SDA_HIGH   PORT_USI |= PIN_USI_SDA 

#define EN_SDA_AS_OUT   DDR_USI |= PIN_USI_SDA 
#define EN_SDA_AS_IN    DDR_USI &= ~PIN_USI_SDA 
#define EN_SCL_AS_OUT   DDR_USI |= PIN_USI_SCL 

#define IS_SCL_HIGH     PIN_USI & PIN_USI_SCL
#define IS_SDA_HIGH     PIN_USI & PIN_USI_SDA
 
#define SET_TO_SEND_ACK( ) \
{ \
    USIDR = 0;			/* prepare ACK */  \
    DDR_USI |= PORT_USI_SDA;	/* set SDA as output */  \
    /* clear all interrupt flags, except Start Cond, set USI counter to shift 1 bit */ \
    USISR = USIOIF|USIPF|0x0E; \
}

#define SET_TO_READ_ACK( ) \
{ \
    DDR_USI &= ~PORT_USI_SDA;	/* set SDA as input */  \
    USIDR = 0;			/* prepare ACK */  \
    /* clear all interrupt flags, except Start Cond, set USI counter to shift 1 bit  */ \
    USISR = USIOIF|USIPF|0x0E; \
}

#define SET_TO_START_CONDITION_MODE( ) \
{ \
    /* enable Start Condition Interrupt, disable Overflow Interrupt */ \
    /* set USI in Two-wire mode, no USI Counter overflow hold */ \
    /* Shift Register Clock Source = External, positive edge */ \
    /* 4-Bit Counter Source = external, both edges */ \
    /* no toggle clock-port pin */ \
    USICR = USISIE|USIWM1|USICS1; \
    /* clear all interrupt flags, except Start Cond */ \
    USISR = USIOIF|USIPF; \
}

#define SET_TO_SEND_DATA( ) \
{ \
    DDR_USI |= PORT_USI_SDA;	/* set SDA as output */  \
    /* clear all interrupt flags, except Start Cond, set USI to shift out 8 bits  */ \
    USISR = USIOIF|USIPF; \
}

#define SET_TO_READ_DATA( ) \
{ \
    DDR_USI &= ~PORT_USI_SDA;	/* set SDA as input */  \
    /* clear all interrupt flags, except Start Cond, set USI to shift out 8 bits */ \
    USISR = USIOIF|USIPF; \
}

