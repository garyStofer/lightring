
#pragma used+
sfrb PRR=0x00;
sfrb DIDR0=0x01;
sfrb ADCSRB=0x03;
sfrb ADCL=0x04;
sfrb ADCH=0x05;
sfrw ADCW=0x04; 
sfrb ADCSRA=0x06;
sfrb ADMUX=0x07;
sfrb ACSR=0x08;
sfrb TIFR1=0x0b;
sfrb TIMSK1=0x0c;
sfrb USICR=0x0d;
sfrb USISR=0x0e;
sfrb USIDR=0x0f;
sfrb USIBR=0x10;
sfrb PCMSK0=0x12;
sfrb GPIOR0=0x13;
sfrb GPIOR1=0x14;
sfrb GPIOR2=0x15;
sfrb PINB=0x16;
sfrb DDRB=0x17;
sfrb PORTB=0x18;
sfrb PINA=0x19;
sfrb DDRA=0x1a;
sfrb PORTA=0x1b;
sfrb EECR=0x1c;
sfrb EEDR=0x1d;
sfrb EEARL=0x1e;
sfrb PCMSK1=0x20;
sfrb WDTCSR=0x21;
sfrb TCCR1C=0x22;
sfrb GTCCR=0x23;
sfrb ICR1L=0x24;
sfrb ICR1H=0x25;
sfrb CLKPR=0x26;
sfrb DWDR=0x27;
sfrb OCR1BL=0x28;
sfrb OCR1BH=0x29;
sfrb OCR1AL=0x2a;
sfrb OCR1AH=0x2b;
sfrb TCNT1L=0x2c;
sfrb TCNT1H=0x2d;
sfrw TCNT1=0x2c; 
sfrb TCCR1B=0x2e;
sfrb TCCR1A=0x2f;
sfrb TCCR0A=0x30;
sfrb OSCCAL=0x31;
sfrb TCNT0=0x32;
sfrb TCCR0B=0x33;
sfrb MCUSR=0x34;
sfrb MCUCR=0x35;
sfrb OCR0A=0x36;
sfrb SPMCSR=0x37;
sfrb TIFR0=0x38;
sfrb TIMSK0=0x39;
sfrb GIFR=0x3a;
sfrb GIMSK=0x3b;
sfrb OCR0B=0x3c;
sfrb SPL=0x3d;
sfrb SPH=0x3e;
sfrb SREG=0x3f;
#pragma used-

#asm
	#ifndef __SLEEP_DEFINED__
	#define __SLEEP_DEFINED__
	.EQU __se_bit=0x20
	.EQU __sm_mask=0x18
	.EQU __sm_adc_noise_red=0x08
	.EQU __sm_powerdown=0x10
	.EQU __sm_standby=0x18
	.SET power_ctrl_reg=mcucr
	#endif
#endasm

#pragma used+

void delay_us(unsigned int n);
void delay_ms(unsigned int n);

#pragma used-

typedef enum
{
CHECK_ADDRESS                = 0x00,
SEND_DATA                    = 0x01,
REQUEST_REPLY_FROM_SEND_DATA = 0x02,
CHECK_REPLY_FROM_SEND_DATA   = 0x03,
REQUEST_DATA                 = 0x04,
GET_DATA_AND_SEND_ACK        = 0x05
} overflowState_t;

extern unsigned char SlaveAddr;

void Master_Init(void); 
void Slave_Init(unsigned char pri_sec); 
unsigned char Master_Msg(unsigned char *, unsigned char); 
unsigned char Get_State_Info(void); 
void Slave_Transmit_Byte(unsigned char data);
unsigned char Slave_Receive_Byte(void);
char Slave_Is_Data_In_Buffer(void);

static volatile overflowState_t overflowState;

static unsigned char rxBuf[8];
static volatile unsigned char rxHead;
static volatile unsigned char rxTail;

static unsigned char txBuf[8];
static volatile unsigned char txHead;
static volatile unsigned char txTail;

unsigned char Master_Transfer(unsigned char); 
unsigned char Master_Stop(void);
unsigned char SlaveAddr;

union state 
{ 
unsigned char errorState;
struct 
{ 
unsigned char addressMode : 1; 
unsigned char masterWriteDataMode : 1; 
unsigned char unused : 6; 
}; 
} state; 

static void flushTwiBuffers(void)
{
rxTail = 0;
rxHead = 0;
txTail = 0;
txHead = 0;
}

void Master_Init(void) 
{ 
PORTA  |= 0x40  ; 
PORTA  |= 0x10  ; 

DDRA  |= 0x10  ; 
DDRA  |= 0x40  ; 

USIDR = 0xFF; 
USICR = 0x20      |0x08      |0x02      ;
USISR = 0x80      |0x40      |0x20      ; 
} 

void Slave_Init(unsigned char pri_sec)
{
flushTwiBuffers();
SlaveAddr = 0x18 + pri_sec;

DDRA  |= (0x10  | 0x40 );	
PORTA  |= 0x40  ; 
PORTA  |= 0x10  ; 
DDRA  &= ~0x40  ;

USICR = 0x80      |0x20      |0x08      ;

USISR = 0x80      |0x40      |0x20      ;
}

unsigned char Get_State_Info(void) 
{ 
return(state.errorState); 
} 

unsigned char Master_Msg(unsigned char *msg, unsigned char msgSize) 
{ 
unsigned char tempUSISR_8bit;
unsigned char tempUSISR_1bit;

tempUSISR_8bit = 0x80      |0x40      |0x20      ; 
tempUSISR_1bit = 0x80      |0x40      |0x20      |0xE;

state.errorState = 0; 
state.addressMode = 1 ; 

if(!(*msg & 0x01      ))
{ 
state.masterWriteDataMode = 1 ; 
} 

PORTA  |= 0x10  ; 
while(!(PINA  & 0x10 )); 
delay_us(5	); 

PORTA  &= ~0x40  ; 
delay_us(4	); 
PORTA  &= ~0x10  ; 
delay_us(4	); 
PORTA  |= 0x40  ; 

do 
{ 

if (state.addressMode || state.masterWriteDataMode) 
{ 

PORTA  &= ~0x10  ; 
USIDR = *(msg++); 
Master_Transfer(tempUSISR_8bit); 

DDRA  &= ~0x40  ; 
if(Master_Transfer(tempUSISR_1bit) & 0x01      ) 
{ 
} 
state.addressMode = 0 ; 
} 
else	  
{ 

DDRA  &= ~0x40  ; 
*(msg++) = Master_Transfer(tempUSISR_8bit); 

USIDR = (msgSize == 1) ? 0xFF : 0x00; 
Master_Transfer(tempUSISR_1bit); 
} 
} while(--msgSize) ; 

Master_Stop(); 

return (1 ); 
} 

unsigned char Master_Transfer( unsigned char temp ) 
{ 
USISR = temp; 
temp = 0x20      |0x08      |0x02      |0x01      ;  
do 
{ 
delay_us(5	); 
USICR = temp; 
while(!(PINA  & 0x10 )); 
delay_us(4	); 
USICR = temp; 
} while(!(USISR & 0x40      )); 

delay_us(5	); 
temp = USIDR; 
USIDR = 0xFF; 
DDRA  |= 0x40  ; 

return temp; 
} 

unsigned char Master_Stop( void ) 
{ 
PORTA  &= ~0x40  ; 
PORTA  |= 0x10  ; 
while(!(PINA  & 0x10 )); 
delay_us(4	); 
PORTA  |= 0x40  ; 
delay_us(5	); 
return (1 ); 
} 

void Slave_Transmit_Byte(unsigned char data)
{
unsigned char tmphead;

tmphead = (txHead + 1) & 0x07;		
while(tmphead == txTail);				
txBuf[tmphead] = data;				
txHead = tmphead;					
}

unsigned char Slave_Receive_Byte(void)
{
while(rxHead == rxTail);				
rxTail = (rxTail + 1) & 0x07;		
return rxBuf[rxTail];					
}

char Slave_Is_Data_In_Buffer(void)
{
return rxHead != rxTail;		
}

interrupt [16] void usi_start_isr(void)
{
overflowState = CHECK_ADDRESS;

DDRA  &= ~0x40 ;

while((PINA  & 0x10 ) && !(PINA  & 0x40 ));

if(!(PINA  & 0x40 ))
{
USICR = 0x80      |0x40      |0x20      |0x10      |0x08      ;
}
else
{
USICR = 0x80      |0x20      |0x08      ;
}

USISR = 0x80      |0x40      |0x20      ;
}

interrupt [17] void usi_ovf_isr(void)
{
switch(overflowState)
{
case CHECK_ADDRESS:
if((USIDR == 0) || ((USIDR >> 1) == SlaveAddr))
{
overflowState = (USIDR & SEND_DATA) ? SEND_DATA : REQUEST_DATA;
{     USIDR = 0;			      DDRA  |= 0x40 ;	           USISR = 0x40      |0x20      |0x0E; };
}
else
{
{                              USICR = 0x80      |0x20      |0x08      ;          USISR = 0x40      |0x20      ; };
}
break;

case CHECK_REPLY_FROM_SEND_DATA:
if(USIDR)
{
{                              USICR = 0x80      |0x20      |0x08      ;          USISR = 0x40      |0x20      ; };
return;
}

case SEND_DATA:
if(txHead != txTail)
{
txTail = (txTail + 1) & 0x07;
USIDR = txBuf[txTail];
}
else
{

return;
}
overflowState = REQUEST_REPLY_FROM_SEND_DATA;
{     DDRA  |= 0x40 ;	           USISR = 0x40      |0x20      ; };
break;

case REQUEST_REPLY_FROM_SEND_DATA:
overflowState = CHECK_REPLY_FROM_SEND_DATA;
{     DDRA  &= ~0x40 ;	      USIDR = 0;			           USISR = 0x40      |0x20      |0x0E; };
break;

case REQUEST_DATA:
overflowState = GET_DATA_AND_SEND_ACK;
{     DDRA  &= ~0x40 ;	           USISR = 0x40      |0x20      ; };
break;

case GET_DATA_AND_SEND_ACK:
rxHead = (rxHead + 1) & 0x07;
rxBuf[rxHead] = USIDR;
overflowState = REQUEST_DATA;
{     USIDR = 0;			      DDRA  |= 0x40 ;	           USISR = 0x40      |0x20      |0x0E; };
break;
}
}

