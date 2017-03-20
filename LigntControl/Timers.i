
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

extern void InitTimer0(void);
extern void MS_Wait(unsigned int delay);
extern void SetMdelay(unsigned int delay);
extern unsigned int GetMdelay(void);
extern void SetSdelay(char delay);
extern char GetSdelay(void);
extern void SetPowerDownTime(unsigned int PDtime);
extern char IsPDtimeExpired(void);

int OneSecondPeriod;
unsigned int MSdelayValue;
unsigned int MSwaitValue;
char SdelayValue;
unsigned int PowerDownValue;

void InitTimer0(void)
{
TCCR0A=0x00;    
TCCR0B=0x03;    
TCNT0=0x82;     
OCR0A=0x00;     
OCR0B=0x00;     
TIMSK=0x02;     

OneSecondPeriod = 0;
MSdelayValue = 0;
MSwaitValue = 0;
SdelayValue = 0;
PowerDownValue = 0;
}

void MS_Wait(unsigned int delay)
{
MSwaitValue = delay;
while(MSwaitValue);
}

void SetMdelay(unsigned int delay)
{
MSdelayValue = delay;
}

unsigned int GetMdelay(void)
{
return MSdelayValue;
}

void SetSdelay(char delay)
{
SdelayValue = delay;
}

char GetSdelay(void)
{
return SdelayValue;
}

void SetPowerDownTime(unsigned int PDtime)
{
PowerDownValue = PDtime;
}

char IsPDtimeExpired(void)
{
return ((PowerDownValue == 0) ? 1 : 0);
}

interrupt [6] void timer0_ovf_isr(void)
{
TCNT0=0x82;				
if(MSdelayValue)		
{
MSdelayValue--;
}
if(MSwaitValue)
{
MSwaitValue--;
}
OneSecondPeriod++;
if(OneSecondPeriod >= 1000)
{
OneSecondPeriod = 0;
if(SdelayValue)
{
SdelayValue--;
}
if(PowerDownValue)
{
PowerDownValue--;
}
}
}
