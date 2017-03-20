/*****************************************************
Project : Light Control
Version : A
Date    : 12/16/2012
Author  : Gene Breniman
Company : Young Embedded Systems
Comments: 


Chip type               : ATtiny45
AVR Core Clock frequency: 8.000000 MHz
Memory model            : Tiny
External RAM size       : 0
Data Stack size         : 64
*****************************************************/

#include <tiny25.h>
#include <delay.h>
#include "IOports.h"
#include "ADC.h"
#include "Timers.h"
#include "TWI.h"
#include "EEPROM.h"

// Switch defines
#define MODE    0x08
#define ENTER   0x04
#define UP      0x02
#define DOWN    0x01
#define TIMOUT  0x10

// State Defines
#define GET_MODE    0
#define GET_ROTATE  1
#define SET_INTEN   2
#define SET_TIMOUT  3

// Mode defines
#define ALL     0
#define GRADI   1
#define THRFRTH 2
#define HALF    3
#define QUARTR  4
#define SINGLE  5
#define ALL_OFF 6 

// List of intensity levels (in number of pulse to achieve)
const char LED_pulses[16] = { 31, 30, 29, 28, 27, 26, 25, 23, 21, 19, 17, 14, 11, 8, 4, 0 };
// List of light patterns, indexed by mode/position.  Gradient, all on to all off (full, 3/4, 1/2, 1/4, 1/12)
const char LED_Intensity[36] = {3, 4, 5, 6, 7, 8, 8, 7, 6, 5, 4, 3,
                                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// indexes (based on mode) into LED_Intensity array
const char modeStart[7] = {12, 0, 15, 18, 21, 23, 24};
// ADC count limits for switch press decoding
const unsigned int Limit[16] = { 967, 		// > 4.72V no switches pressed
								 864,		// > 4.22V SW1 pressed
                                 782,		// > 3.82V SW2 pressed
                                 714,		// > 3.48V SW1 and SW2 pressed
                                 657,		// > 3.21V SW3 pressed
                                 608,		// > 2.97V SW1 and SW3 pressed
                                 566,		// > 2.76V SW2 and SW3 pressed
                                 529,		// > 2.58V SW1, SW2 and SW3 pressed
                                 497,		// > 2.43V SW4 pressed
                                 469,		// > 2.29V SW1 and SW4 pressed
                                 443,		// > 2.16V SW2 and SW4 pressed
                                 420,		// > 2.05V SW1, SW2 and SW4 pressed
                                 400,		// > 1.95V SW3 and SW4 pressed
                                 381,		// > 1.86V SW1, SW3 and SW4 pressed
                                 364,		// > 1.78V SW2, SW3 and SW4 pressed
                                 0 };		// > 0V all swithces pressed                                            
// buffer for outbound I2C message (single command)
char TWI_Msg[3] = { 0, 0, 0 };
unsigned int Debounce = 0;
unsigned int KeyRepeat = 0;
char KeyDownWait = 0;
char PowerOn = 1;			// powered state of slave units (master is always on)


// **************************************************************************************************
// FUNCTION NAME:		ShowMode()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to update the RED/GREEN Leds to show the current UI state (based on passed
//  state).
//  GREEN ON		- Setting Intensity
//  RED ON			- Setting Mode
//  RED & GREEN ON	- Setting rotation/position
// **************************************************************************************************
// Global(s) Accessed:
// 	NONE
// 
// **************************************************************************************************
// Input(s):
// 	char state - Represents the current state of the UI state machine (actual or faked).
// 
// Output(s):
// 	NONE
// 
// **************************************************************************************************
// Return Type:		NONE
// **************************************************************************************************
void ShowMode(char state)
{
    // if not setting Timeout value (flashing LEDs scale)
    if(state != TIMOUT)
    {
        GRN_LED = (state != GET_MODE) ? 1 : 0;          // Green Intensity
        RED_LED = (state != SET_INTEN) ? 1 : 0;         // Red Mode
    }                                                   // Red/Green rotation/position
}

// **************************************************************************************************
// FUNCTION NAME:		BumpValue()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to increment/decrement the passed value.  If nowrap is set, the min and max
//  values are used to limit the result, otherwise they are used to control the wrap around values.
// **************************************************************************************************
// Global(s) Accessed:
// 	NONE
// 
// **************************************************************************************************
// Input(s):
// 	char cval		- current value of the variable to be modified.
//  char up/down	- the direction to to adjust the variable (up - inc / down - dec)
//  char nowrap		- flag indicating if the value is to conform to limits (true) or wrap (false)
//  char maxval		- maximum allowable value on increment (or wrap to value on decrement with !nowrap)
//  char maxval		- minimum allowable value on decrement (or wrap to value on increment with !nowrap)
// 
// Output(s):
// 	NONE
// 
// **************************************************************************************************
// Return Type:		char - modified result based on increment/decrement
// **************************************************************************************************
char BumpValue(char cval, char up_down, char nowrap, char maxval, char minval)
{
    // handle UP/Down keys for current setting (mode/position/intensity/timeout
    if(up_down == UP)
    {
    	// if not at maxval increment, else either wrap or clamp depending on nowrap
        cval = (cval >= maxval) ? ((nowrap) ? maxval : minval) : (++cval);
    }
    else
    {
    	// if not at inxval decrement, else either wrap or clamp depending on nowrap
        cval = (cval <= minval) ? ((nowrap) ? minval : maxval) : (--cval);
    }
    return cval;
}

// **************************************************************************************************
// FUNCTION NAME:		SetLED_remote()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to send a single LED value to the appropriate slave module.  The send
//  consists of Address (0x30/0x32) and led number/value (2 bytes).
// **************************************************************************************************
// Global(s) Accessed:
// 	char TWI_Msg[]	- modified - used to send I2C message to slave module.
// 
// **************************************************************************************************
// Input(s):
// 	char led		- led number (used to also determine slave address).
//  char value		- led intensity value.
// 
// Output(s):
// 	NONE
// 
// **************************************************************************************************
// Return Type:		NONE
// **************************************************************************************************
void SetLED_remote(char led, char value)
{
    // is LED in first on second half of ring?
    if(led >= 6)
    {
    	// second half, set high address and adjust LED number to be relative to that half
        TWI_Msg[0] = 0x32;
        led -= 6;
    }
    else
    {
       	// second half, set low address
        TWI_Msg[0] = 0x30;
    }
    // combine LED number (high 3 bits) and requested level (low 5 bits), move to I2C buffer
    TWI_Msg[1] = (led << 5) | value;
    Master_Msg(TWI_Msg, 2);			// send LED message to appropriate ring half
}

// **************************************************************************************************
// FUNCTION NAME:		UpdateLEDs()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to send update messages for each of the LEDs (12 total).  This is done as
//  the result of a UI action requiring a change in illumination (intensity, mode or rotation).
// **************************************************************************************************
// Global(s) Accessed:
// 	NONE
// 
// **************************************************************************************************
// Input(s):
// 	char mode		- current illumination mode.
//  char position	- current position/rotation.
//  char Intensity	- current intensity bias.
// 
// Output(s):
// 	NONE
// 
// **************************************************************************************************
// Return Type:		NONE
// **************************************************************************************************
void UpdateLEDs(char mode, char position, char Intensity )
{
    char i;
    char index;
    int value;
    
    // foreach LED determine intensity setting and send to appropriate slave
    for(i = 0; i < 12; i++)
    {
        // combine counter and position to form index.
    	index = position + i;
    	// wrap index (if necessary)
        if(index >= 12)
        {
            index -= 12;
        }
        // convert relative index into absolute index (based on mode)
        index += modeStart[mode];
        // get baseline intensity value;
        value = LED_Intensity[index];
        // adjust value (if not off), based on intensity bias value
        if(value != 0)
        {
            value -= (7 - Intensity);
        }
        // lookup LED pulses (to achieve commanded intensity)
        value = LED_pulses[value];
        // send I2C command for this LED
        SetLED_remote(i, (unsigned char) value);
    }
}

// **************************************************************************************************
// FUNCTION NAME:		CheckSwitch()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is called to read the UI switches and return contact(s) based on voltage value.
// **************************************************************************************************
// Global(s) Accessed:
// 	unsigned int Debounce	- read/modify	- last 4 cycles of switch read process 
//  unsigned int KeyRepeat	- read/modify	- key repeat rate (uses accelerated timing)
//  char KeyDownWait		- read/modify	- long key hold timing (used for power off/power off timing)
//  char PowerOn			- modify		- flag indicating that slaves are powered (cleared @ powerdown)
// **************************************************************************************************
// Input(s):
// 	NONE
// 
// Output(s):
// 	NONE
// 
// **************************************************************************************************
// Return Type:		char - decode switch contact(s)
//					0000 - no switch pressed
//					0001 - DOWN pressed
//					0010 - UP pressed
//					0011 - UP & DOWN pressed - not used
//					0100 - ENTER pressed
//					0101 - no switch pressed - not used
//					0110 - no switch pressed - not used
//					0111 - no switch pressed - not used
//					1000 - MODE pressed
//					1001 - no switch pressed - not used
//					1010 - no switch pressed - not used
//					1011 - no switch pressed - not used
//					1100 - no switch pressed - not used
//					1101 - no switch pressed - not used
//					1110 - no switch pressed - not used
//					1111 - all switches pressed - not used
//                 10000 - timeout occurred
// **************************************************************************************************
char CheckSwitch(void)
{
    unsigned int ADCval;
    unsigned int change;
    unsigned int newSwitch;
    unsigned int i;
    
    // Read switch voltage
    ADCval = read_adc(SW_ADC_IN);
    change = 0;		// initialize contact value
    // loop through all possible voltage ranges until reading is greater than test value
    for(i = 0; i < 16; i++)
    {
        // match voltage range to combinations of open/close for the four switches
        if( ADCval >= Limit[i] )
        {
            // in range, break out with combination
            change = i;     
            break;
        }
    }
    
    // form pattern based on the current and past three readings
    newSwitch = Debounce << 4 | change;
    // change in pattern? If so, return with key press code if stable
    if(newSwitch != Debounce)
    {
        Debounce = newSwitch;		// update debounce pattern for next loop
        // is this Up or Down key presses?
        if(newSwitch == 0x2222 || newSwitch == 0x1111)
        {
            KeyRepeat = 1000;           // setup initial key repeat time
            SetMdelay(KeyRepeat);
            return(newSwitch & 0xf);    // return UP/DOWN code
        }
        // is this Mode or Enter key presses?
        else if(newSwitch == 0x4444 || newSwitch == 0x8888)
        {
            KeyRepeat = 0;              // disable key repeat
            KeyDownWait = 3;            // setup 3 second hold timer
            SetSdelay(KeyDownWait);
            return(newSwitch & 0xf);    // return MODE/ENTER code
        }
        // is this a key release?
        else if(newSwitch == 0x0000)
        {
            KeyRepeat = 0;              // disable key repeat
            KeyDownWait = 0;            // disable 3 second hold
            return(0);                  // return NULL code
        }
    }
    // same pattern, is key repeat enabled, if so and if time out send another code
    else if(KeyRepeat && (GetMdelay() == 0))
    {
        // is key repeat time greater than minimum time?
        if(KeyRepeat > 125)
        {
            KeyRepeat >>= 1;        // yes, reduce time by 50%
        }
        SetMdelay(KeyRepeat);       // restart time
        return(newSwitch & 0xf);    // return last keycode
    }
    // same pattern, is key hold timer enabled and timed out?
    else if(KeyDownWait && (GetSdelay() == 0))
    {
        // is this a MODE key hold (i.e. Power off)
        if(newSwitch == 0x8888)
        {
            PowerOn = 0;            // yes, clear powered flag
            GRN_LED = 0;            // clear indicator leds (step flashing green)
            RED_LED = 0;
            UpdateLEDs(ALL_OFF, 0, 0);  // command all leds to OFF
        }
        else
        {
            return TIMOUT;          // no, this is entry into setup and save mode
        }
    }
    return 0;
}

// **************************************************************************************************
// FUNCTION NAME:		main()
// **************************************************************************************************
// DESCRIPTION:
//  This routine is the main program loop. The processor is initialize and then the initial LED values
//  (either determined from shutdown state (EEPROM) or default) for all of the LED are sent to the slave
//  units. The program then enters a forever loop to process key presses and control LEDs through I2C
//  commands to the slaves.
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
void main(void)
{
    char state;                     // up/down function assignment
    char input;                     // key/EEPROM input value
    char mode;                      // current assigned LED mode
    char position;                  // LED pattern rotation index
    char intensity;                 // LED pattern intensity
    char highLimitVal;              // high (UP) limit
    char lowLimitVal;               // low (DOWN) limit
    char TimeOut;                   // current TimeOut index (0 = 30min - 7 = no time out)
    char Flashing;                  // Flashing indicator counter
    char temp;                      // temp variable (Flashing >> 1)

    // Crystal Oscillator division factor: 1
#pragma optsize-
    CLKPR = 0x80;
    CLKPR  =0x00;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

    IOInit();               // Input/Output Ports initialization
    InitTimer0();           // Timer/Counter 0 initialization
    AnaCompInit();			// turn analog comparator off
    ADCInit();				// setup ADC - make switch input an analog input
    Master_Init();          // Setup USI to Master IC2 mode
    input = EEPROM_read(0); // Read first word of EEPROM settings 
    // if settings are valid
    if(input != 0xFF)
    {
        mode = (input >> 4) & 0x07;     // extract mode (high nibble)
        if(mode > 6)                    // limit to valid range
        {
            mode = 6;
        }
        position = input & 0x0F;        // extract position (low nibble)
        if(position > 11)               // limit to valid range
        {
            position = 11;
        }
        input = EEPROM_read(1);         // read second byte of EEPROM settings
        // if settings are valid
        if(input != 0xFF)
        {
            TimeOut = (input >> 4) & 0x07;  // extract TimeOut setting (high nibble)
            intensity = input & 0x0F;       // extract intensity setting (low nibble)
        }
        else
        {
            // second byte invalid, set to defaults
            intensity = 1;
            TimeOut = 0;
        }
    }
    else
    {
        // EEPROM clear, set to defaults
        mode = 0;
        position = 0;
        intensity = 1;
        TimeOut = 0;
    }
    
    // initialize power up state to INTENSITY state
    state = SET_INTEN;
    Flashing = 0;                               // clear Flashing counter

    // Global enable interrupts
#asm("sei")
    SetPowerDownTime((TimeOut+1) * 1800);       // enable PowerDown timer to setting
    ShowMode(state);                            // update indicator LEDs (show state)
    MS_Wait(20);                                // wait for slaves to complete startup
    UpdateLEDs(mode, position, intensity);      // command LEDs to selected values
    // forever loop      
    while (1)
    {
        // get current keypad switch state
        input = CheckSwitch();
        // if a key is pressed
        if(input)
        {
            SetPowerDownTime((TimeOut+1) * 1800);		// in use - extend power down time
            // if the unit is on, perform the power on processing
            if( PowerOn )
            {
                if(input == MODE)
                {
                    state = GET_MODE;							// enter mode setting state
                }
                else if(input == ENTER)
                {
                    if(state == GET_MODE && mode != ALL)		// skip over ROTATE if all LEDs on full
                    {
                        state = GET_ROTATE;						// enter rotation selection state
                    }
                    else
                    {
                        if(state == SET_TIMOUT)					// if setting timeout/defaults
                        {
                            EEPROM_write(0, (mode << 4 | position));		// write settings to EEPROM
                            EEPROM_write(1, (TimeOut << 4 | intensity));	// write settings to EEPROM
                        }
                        state = SET_INTEN;						// enter intensity selection state
                    }
                }
                else if(input == TIMOUT)						// switch setting detected timeout?
                {
                    state = SET_TIMOUT;							// go process timeout selection    
                }
                else if(input == UP || input == DOWN)			// if up/down pressed, process according to state
                {
                    switch(state)
                    {
                        case GET_MODE:      //Mode select
                            mode = BumpValue(mode, input, 0, SINGLE, ALL);
                            // if switching into GRADI, check intensity limits
                            if(mode == GRADI)
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
                        case GET_ROTATE:    //Rotation
                            position = BumpValue(position, input, 0, 11, 0);
                            break;
                        case SET_INTEN:     //Intensity
                        	// configure intensity limits based on mode (GRADIENT uses limit range)
                            highLimitVal = (mode == GRADI) ? 14 : 15;
                            lowLimitVal = (mode == GRADI) ? 5 : 1;
                            intensity = BumpValue(intensity, input, 1, highLimitVal, lowLimitVal);
                            if(intensity == highLimitVal || intensity == lowLimitVal)
                            {
                                // at limit - flicker the GREEN LED
                                GRN_LED = 0;
                                MS_Wait(50);
                                GRN_LED = 1;
                                MS_Wait(50);
                            }
                            break;
                        case SET_TIMOUT:
                            TimeOut = BumpValue(TimeOut, input, 1, 7, 0);
                            SetPowerDownTime((TimeOut+1) * 1800);
                            break; 
                    }
                    UpdateLEDs(mode, position, intensity);  // command slave units based on LED changes
                }
                ShowMode(state);		// update UI LEDs
            }
            else
            {
            	// not powered on, wakeup slave units
                PowerOn = 1;
                ShowMode(state);
                UpdateLEDs(mode, position, intensity);
                Flashing = 0;
            }
        }
        // if the unit is in powerdown state, manage slow GREEN LED flashing
        if(!PowerOn)
        {
            Flashing++;             // bump flash counter
            GRN_LED = ((Flashing & 0x70) == 0x50) ? 1 : 0;      // command GREEN LED
        }
        
        // if in TimeOut setting state, manage RED/GREEN flashing to indicate timeout value
        if(state == SET_TIMOUT)
        {
            // bump and limit Flash counter
            if(++Flashing >= 32)        
            {
                Flashing = 0;
            }
            // create a GREEN to RED ratio portional to timeOut setting (some gap time)
            temp = Flashing >> 1;       // temp value, Flashing/2
            GRN_LED = (temp < TimeOut) ? 1 : 0;         // GREEN is on if count < timeOut
            RED_LED = ((temp >= TimeOut) && (temp < 7)) ? 1 : 0;    // RED is on remaing time
        }
        MS_Wait(20);            // Pause to pace flashing and allow switch debounce time
        // check if Power Down time is enable and expired
        if(TimeOut < 7 && IsPDtimeExpired())
        {
            // yes, power down unit
            PowerOn = 0;
            GRN_LED = 0;
            RED_LED = 0;
            UpdateLEDs(ALL_OFF, 0, 0);
        }
    };
}
