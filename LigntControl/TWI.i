
#pragma used+
sfrb ADCSRB=0x03;
sfrb ADCL=0x04;
sfrb ADCH=0x05;
sfrw ADCW=0x04; 
sfrb ADCSRA=0x06;
sfrb ADMUX=0x07;
sfrb ACSR=0x08;
sfrb USICR=0x0d;
sfrb USISR=0x0e;
sfrb USIDR=0x0f;
sfrb USIBR=0x10;
sfrb GPIOR0=0x11;
sfrb GPIOR1=0x12;
sfrb GPIOR2=0x13;
sfrb DIDR0=0x14;
sfrb PCMSK=0x15;
sfrb PINB=0x16;
sfrb DDRB=0x17;
sfrb PORTB=0x18;
sfrb EECR=0x1c;
sfrb EEDR=0x1d;
sfrb EEARL=0x1e;
sfrb PRR=0x20;
sfrb WDTCR=0x21;
sfrb DWDR=0x22;
sfrb DTPS1=0x23; 
sfrb DT1B=0x24;  
sfrb DT1A=0x25;  
sfrb CLKPR=0x26;
sfrb PLLCSR=0x27;
sfrb OCR0B=0x28;
sfrb OCR0A=0x29;
sfrb TCCR0A=0x2a;
sfrb OCR1B=0x2b;
sfrb GTCCR=0x2c;
sfrb OCR1C=0x2d;
sfrb OCR1A=0x2e;
sfrb TCNT1=0x2f;
sfrb TCCR1=0x30;
sfrb OSCCAL=0x31;
sfrb TCNT0=0x32;
sfrb TCCR0B=0x33;
sfrb MCUSR=0x34;
sfrb MCUCR=0x35;
sfrb SPMCSR=0x37;
sfrb TIFR=0x38;
sfrb TIMSK=0x39;
sfrb GIFR=0x3a;
sfrb GIMSK=0x3b;
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

extern void Master_Init(void); 
extern char Master_Msg(char *, char); 

char Master_Transfer(char); 
char Master_Stop(void);

void Master_Init(void) 
{ 
PORTB  |= 0x01  ; 
PORTB  |= 0x04  ; 

DDRB  |= 0x04  ; 
DDRB  |= 0x01  ; 

USIDR = 0xFF; 
USICR = 0x20      |0x08      |0x02      ;
USISR = 0x80      |0x40      |0x20      ; 
} 

char Master_Msg(char *msg, char msgSize) 
{ 
char tempUSISR_8bit;
char tempUSISR_1bit;

tempUSISR_8bit = 0x80      |0x40      |0x20      ; 
tempUSISR_1bit = 0x80      |0x40      |0x20      |0xE;

PORTB  |= 0x04  ; 
while(!(PINB  & 0x04 )); 
delay_us(5	); 

PORTB  &= ~0x01  ; 
delay_us(4	); 
PORTB  &= ~0x04  ; 
delay_us(4	); 
PORTB  |= 0x01  ; 

do 
{ 

PORTB  &= ~0x04  ; 
USIDR = *(msg++); 
Master_Transfer(tempUSISR_8bit); 

DDRB  &= ~0x01  ; 
if(Master_Transfer(tempUSISR_1bit) & 0x01      ) 
{
break; 
} 
} while(--msgSize) ; 

Master_Stop(); 

return (1 ); 
} 

char Master_Transfer( char temp ) 
{ 
USISR = temp; 
temp = 0x20      |0x08      |0x02      |0x01      ;  
do 
{ 
delay_us(5	); 
USICR = temp; 
while(!(PINB  & 0x04 )); 
delay_us(4	); 
USICR = temp; 
} while(!(USISR & 0x40      )); 

delay_us(5	); 
temp = USIDR; 
USIDR = 0xFF; 
DDRB  |= 0x01  ; 

return temp; 
} 

char Master_Stop( void ) 
{ 
PORTB  &= ~0x01  ; 
PORTB  |= 0x04  ; 
while(!(PINB  & 0x04 )); 
delay_us(4	); 
PORTB  |= 0x01  ; 
delay_us(5	); 
return (1 ); 
} 

