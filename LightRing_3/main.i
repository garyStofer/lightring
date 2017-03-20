
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

extern void IOInit(void);

extern void InitTimer0(void);
extern void InitTimer1(void);
extern void MS_Wait(unsigned int delay);
extern void S_Wait(unsigned char delay);
extern void SetMdelay(unsigned int delay);
extern unsigned int GetMdelay(void);

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

const unsigned char LED_Map[6] = {0x02, 0x04, 0x80, 0x20, 0x08, 0x01};

void SetLED_A(unsigned char Mask, unsigned char value)
{
char i;
PORTA &= ~Mask;
MS_Wait(5);         
if(value == 31)
{
return;
}
PORTA |= Mask;
delay_us(10);
for(i = 0; i < value; i++)
{
PORTA &= ~Mask;
PORTA |= Mask;
}
}

void SetLED_B(unsigned char Mask, unsigned char value)
{
char i;
PORTB &= ~Mask;
MS_Wait(5);         
if(value == 31)
{
return;
}
PORTB |= Mask;
delay_us(10);
for(i = 0; i < value; i++)
{
PORTB &= ~Mask;
PORTB |= Mask;
}
}

void SetLED(unsigned char led, unsigned char value)
{
unsigned char Mask;

Mask = LED_Map[led];
if(led <= 1)
{
SetLED_B(Mask, value);
}
else
{
SetLED_A(Mask, value);
}
}

void main(void)
{
unsigned char Primary_Secondary;   
unsigned char command;
unsigned char led;
unsigned char intensity;

#pragma optsize-
CLKPR = 0x80;
CLKPR  =0x00;
#pragma optsize+

IOInit();           
InitTimer0();       

#asm("sei")

Primary_Secondary = (PINA & 0x01) ? 0 : 1;
DDRA = 0xA9;            
Slave_Init(Primary_Secondary);
(PORTB.0) = 1;
while (1)
{
if(Slave_Is_Data_In_Buffer())
{
command = Slave_Receive_Byte();
led = command >> 5;
intensity = command & 0x1F;
SetLED(led, intensity);      
}
};
}
