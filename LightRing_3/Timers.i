
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

extern void InitTimer0(void);
extern void InitTimer1(void);
extern void MS_Wait(unsigned int delay);
extern void S_Wait(unsigned char delay);
extern void SetMdelay(unsigned int delay);
extern unsigned int GetMdelay(void);

extern void IOInit(void);

int OneSecondPeriod;
unsigned int MSdelayValue;
unsigned char SdelayValue;

void InitTimer0(void)
{
TCCR0A=0x00;    
TCCR0B=0x03;    
TCNT0=0x82;     
OCR0A=0x00;     
OCR0B=0x00;     
TIMSK0=0x01;

OneSecondPeriod = 0;
MSdelayValue = 0;
}

void InitTimer1(void)
{
TCCR1A=0x00;    
TCCR1B=0x00;    
TIMSK1=0x00;    
}

void MS_Wait(unsigned int delay)
{
MSdelayValue = delay;
while(MSdelayValue);
}

void S_Wait(unsigned char delay)
{
SdelayValue = delay;
while(SdelayValue);
}

void SetMdelay(unsigned int delay)
{
MSdelayValue = delay;
}

unsigned int GetMdelay(void)
{
return MSdelayValue;
}

interrupt [12] void timer0_ovf_isr(void)
{

TCNT0=0x82;
if(MSdelayValue)
{
MSdelayValue--;
}
OneSecondPeriod++;
if(OneSecondPeriod >= 1000)
{
OneSecondPeriod = 0;
if(SdelayValue)
{
SdelayValue--;
}
}
}
