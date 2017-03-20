/*****************************************************
Project : Light Control
File    : EEPROM.h
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

extern void EEPROM_write(char ucAddress, char ucData);
extern char EEPROM_read(char ucAddress);

// EECR register defines
#define EERE    0x01
#define EEPE    0x02
#define EEMPE   0x04