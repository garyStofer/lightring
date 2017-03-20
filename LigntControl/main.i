
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

extern void IOInit(void);

extern void AnaCompInit(void);
extern void ADCInit(void);
extern unsigned int read_adc(char adc_input);

extern void InitTimer0(void);
extern void MS_Wait(unsigned int delay);
extern void SetMdelay(unsigned int delay);
extern unsigned int GetMdelay(void);
extern void SetSdelay(char delay);
extern char GetSdelay(void);
extern void SetPowerDownTime(unsigned int PDtime);
extern char IsPDtimeExpired(void);

extern void Master_Init(void); 
extern char Master_Msg(char *, char); 

extern void EEPROM_write(char ucAddress, char ucData);
extern char EEPROM_read(char ucAddress);

const char LED_pulses[16] = { 31, 30, 29, 28, 27, 26, 25, 23, 21, 19, 17, 14, 11, 8, 4, 0 };
const char LED_Intensity[36] = {3, 4, 5, 6, 7, 8, 8, 7, 6, 5, 4, 3,
7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const char modeStart[7] = {12, 0, 15, 18, 21, 23, 24};
const unsigned int Limit[16] = { 967, 		
864,		
782,		
714,		
657,		
608,		
566,		
529,		
497,		
469,		
443,		
420,		
400,		
381,		
364,		
0 };		
char TWI_Msg[3] = { 0, 0, 0 };
unsigned int Debounce = 0;
unsigned int KeyRepeat = 0;
char KeyDownWait = 0;
char PowerOn = 1;			

void ShowMode(char state)
{
if(state != 0x10)
{
(PORTB.4) = (state != 0) ? 1 : 0;          
(PORTB.1) = (state != 2) ? 1 : 0;         
}                                                   
}

char BumpValue(char cval, char up_down, char nowrap, char maxval, char minval)
{
if(up_down == 0x02)
{
cval = (cval >= maxval) ? ((nowrap) ? maxval : minval) : (++cval);
}
else
{
cval = (cval <= minval) ? ((nowrap) ? minval : maxval) : (--cval);
}
return cval;
}

void SetLED_remote(char led, char value)
{
if(led >= 6)
{
TWI_Msg[0] = 0x32;
led -= 6;
}
else
{
TWI_Msg[0] = 0x30;
}
TWI_Msg[1] = (led << 5) | value;
Master_Msg(TWI_Msg, 2);			
}

void UpdateLEDs(char mode, char position, char Intensity )
{
char i;
char index;
int value;

for(i = 0; i < 12; i++)
{
index = position + i;
if(index >= 12)
{
index -= 12;
}
index += modeStart[mode];
value = LED_Intensity[index];
if(value != 0)
{
value -= (7 - Intensity);
}
value = LED_pulses[value];
SetLED_remote(i, (unsigned char) value);
}
}

char CheckSwitch(void)
{
unsigned int ADCval;
unsigned int change;
unsigned int newSwitch;
unsigned int i;

ADCval = read_adc(0x3);
change = 0;		
for(i = 0; i < 16; i++)
{
if( ADCval >= Limit[i] )
{
change = i;     
break;
}
}

newSwitch = Debounce << 4 | change;
if(newSwitch != Debounce)
{
Debounce = newSwitch;		
if(newSwitch == 0x2222 || newSwitch == 0x1111)
{
KeyRepeat = 1000;           
SetMdelay(KeyRepeat);
return(newSwitch & 0xf);    
}
else if(newSwitch == 0x4444 || newSwitch == 0x8888)
{
KeyRepeat = 0;              
KeyDownWait = 3;            
SetSdelay(KeyDownWait);
return(newSwitch & 0xf);    
}
else if(newSwitch == 0x0000)
{
KeyRepeat = 0;              
KeyDownWait = 0;            
return(0);                  
}
}
else if(KeyRepeat && (GetMdelay() == 0))
{
if(KeyRepeat > 125)
{
KeyRepeat >>= 1;        
}
SetMdelay(KeyRepeat);       
return(newSwitch & 0xf);    
}
else if(KeyDownWait && (GetSdelay() == 0))
{
if(newSwitch == 0x8888)
{
PowerOn = 0;            
(PORTB.4) = 0;            
(PORTB.1) = 0;
UpdateLEDs(6 , 0, 0);  
}
else
{
return 0x10;          
}
}
return 0;
}

void main(void)
{
char state;                     
char input;                     
char mode;                      
char position;                  
char intensity;                 
char highLimitVal;              
char lowLimitVal;               
char TimeOut;                   
char Flashing;                  
char temp;                      

#pragma optsize-
CLKPR = 0x80;
CLKPR  =0x00;
#pragma optsize+

IOInit();               
InitTimer0();           
AnaCompInit();			
ADCInit();				
Master_Init();          
input = EEPROM_read(0); 
if(input != 0xFF)
{
mode = (input >> 4) & 0x07;     
if(mode > 6)                    
{
mode = 6;
}
position = input & 0x0F;        
if(position > 11)               
{
position = 11;
}
input = EEPROM_read(1);         
if(input != 0xFF)
{
TimeOut = (input >> 4) & 0x07;  
intensity = input & 0x0F;       
}
else
{
intensity = 1;
TimeOut = 0;
}
}
else
{
mode = 0;
position = 0;
intensity = 1;
TimeOut = 0;
}

state = 2;
Flashing = 0;                               

#asm("sei")
SetPowerDownTime((TimeOut+1) * 1800);       
ShowMode(state);                            
MS_Wait(20);                                
UpdateLEDs(mode, position, intensity);      
while (1)
{
input = CheckSwitch();
if(input)
{
SetPowerDownTime((TimeOut+1) * 1800);		
if( PowerOn )
{
if(input == 0x08)
{
state = 0;							
}
else if(input == 0x04)
{
if(state == 0 && mode != 0)		
{
state = 1;						
}
else
{
if(state == 3)					
{
EEPROM_write(0, (mode << 4 | position));		
EEPROM_write(1, (TimeOut << 4 | intensity));	
}
state = 2;						
}
}
else if(input == 0x10)						
{
state = 3;							
}
else if(input == 0x02 || input == 0x01)			
{
switch(state)
{
case 0:      
mode = BumpValue(mode, input, 0, 5, 0);
if(mode == 1)
{
if(intensity < 5)
{
intensity = 5;
}
if(intensity > 14)
{
intensity = 14;
}
}
break;
case 1:    
position = BumpValue(position, input, 0, 11, 0);
break;
case 2:     
highLimitVal = (mode == 1) ? 14 : 15;
lowLimitVal = (mode == 1) ? 5 : 1;
intensity = BumpValue(intensity, input, 1, highLimitVal, lowLimitVal);
if(intensity == highLimitVal || intensity == lowLimitVal)
{
(PORTB.4) = 0;
MS_Wait(50);
(PORTB.4) = 1;
MS_Wait(50);
}
break;
case 3:
TimeOut = BumpValue(TimeOut, input, 1, 7, 0);
SetPowerDownTime((TimeOut+1) * 1800);
break; 
}
UpdateLEDs(mode, position, intensity);  
}
ShowMode(state);		
}
else
{
PowerOn = 1;
ShowMode(state);
UpdateLEDs(mode, position, intensity);
Flashing = 0;
}
}
if(!PowerOn)
{
Flashing++;             
(PORTB.4) = ((Flashing & 0x70) == 0x50) ? 1 : 0;      
}

if(state == 3)
{
if(++Flashing >= 32)        
{
Flashing = 0;
}
temp = Flashing >> 1;       
(PORTB.4) = (temp < TimeOut) ? 1 : 0;         
(PORTB.1) = ((temp >= TimeOut) && (temp < 7)) ? 1 : 0;    
}
MS_Wait(20);            
if(TimeOut < 7 && IsPDtimeExpired())
{
PowerOn = 0;
(PORTB.4) = 0;
(PORTB.1) = 0;
UpdateLEDs(6 , 0, 0);
}
};
}
