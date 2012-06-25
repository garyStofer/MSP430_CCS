#include "stddef.h"
#include "MAX_7313.h"
#include "BitBangI2C_master.h"


static unsigned char *i2c_buff = 0;          // Array for transmit data
static unsigned char buff_sz;
static unsigned char leds[NumLEDs+2];


static void //  applies least brightness to all LEDS except the blue one which is max
MAX7315_AllOff( void)
{
	if (i2c_buff == NULL)
		return;

	i2c_buff[0] = MAX7313_I2C_ADDR; // initialize with I2C address of device to talk to
	i2c_buff[1] = MAX7313_INDIVID_PWM;
	i2c_buff[2] = 0xFF;     // F == off , 0 = max , e = least
	i2c_buff[3] = 0xFF;
	i2c_buff[4] = 0xFF;
	i2c_buff[5] = 0xFF;
	i2c_buff[6] = 0xFF;
	i2c_buff[7] = 0xFF;
	i2c_buff[8] = 0xfF;
	I2c_Transmit(8);
}

static void
MAX7315_LedBlinkPhase(unsigned short leds)
{
	unsigned char *cp;

	if (i2c_buff == NULL)
		return;

	cp  = (unsigned char *) &leds;
	/* turns LEDS on/off*/
	i2c_buff[0] = MAX7313_I2C_ADDR; // initialize with I2C address of device to talk to
	i2c_buff[1] = MAX7713_WRITE_PORT_CMD;
	i2c_buff[2] = *cp++;
	i2c_buff[3] = *cp;
	I2c_Transmit(3);
}
void 
MAX7315_Init( unsigned char br )
{
	i2c_buff = I2c_Init(&buff_sz);
	i2c_buff[0] = MAX7313_I2C_ADDR; // initialize with I2C address of device to talk to
	
	// Configure LED ports as outputs
	i2c_buff[1] = MAX7313_PORT_SETUP;
	i2c_buff[2] = 0x01;        // P1-p7 outputs, P0 input
	i2c_buff[3] = 0xE0;        // P8-p12 outputs, P13,14,15 inputs
	I2c_Transmit(3);
	
	// Configure Control register
	i2c_buff[1] = MAX7313_CONTROL_REG;	
	i2c_buff[2] = 0x00;    // no global dimming 
	I2c_Transmit(2);
	MAX7315_LedBlinkPhase(0xffff);	// all LEDs static "on", visibility will be programmed with individual dim value from OFF to 15/16
	MAX7315_MasterBright(br); 		// base brightness
	MAX7315_AllOff();
}



//  MAX7315_MasterDim applies brightess value to all LEDS,   0 = dimmest, 14 = brightest, 15=all LEDS off,
void MAX7315_MasterBright(unsigned char level)
{
	unsigned char dim  = 0xf0 - (level<<4);

	if (i2c_buff == NULL)
		return;

	level &= 0xf;

	i2c_buff[0] = MAX7313_I2C_ADDR; // initialize with I2C address of device to talk to

	// All values with "F" in the low nibble produce a static Low (active)Output regardless of the high nibble
	// All values with "0" in the high nibble produce a static Low (active) Output regardless of the low nibble 
	i2c_buff[1] = MAX7313_MASTER_PWM;
	i2c_buff[2] = dim;
	I2c_Transmit(2);
}


// display a bright led at level and dimm leds below , 0 == No dot, all LEDS are off, i.e. 3 == 3rd led is bright 1&2 are dimm, rest is off
#define Led_dim 0xe
#define Led_off 0xF
#define Led_max 0
void
MAX7315_DisplayDot( signed char level)
{
	int i;
	unsigned char br;

	if (i2c_buff == NULL)
		return ;

	if ( level >  NumLEDs+1)
		level = NumLEDs+1;

	leds[NumLEDs+1] = leds[0] = 0xf;	// these are inputs on the 7313, set them to off

	br = Led_dim;

	for (i = 1; i < NumLEDs+1; i++)		// 1 based for convenience since first pin is input
	{
		if (i > TargetLED )				// the RED LEDs above the blue target
			br = Led_max;

     	if (i < level)
     		leds[i] = br;
     	else if (i == level )			// the highest led on (the dot) is brightest
     		leds[i] = Led_max;
     	else
     		leds[i] = Led_off;			// LEDs above the level are off

	}

	i2c_buff[0] = MAX7313_I2C_ADDR; 		// initialize with I2C address of device to talk to
	i2c_buff[1] = MAX7313_INDIVID_PWM;

	for (i = 0; i <= NumLEDs;i+=2 )
	{
		i2c_buff[i/2+2] = leds[i] + (leds[i+1] << 4);
	}
	I2c_Transmit(NumLEDs/2+2);
}


/* MAX7315_Receive_inputs() 
 * Abbreviated I2c communication, only a single byte can be read per transfer, no clock streching by the addressed device is 
 * allowed. 
 */
static void
MAX7315_Receive_bytes( unsigned char addr, int n)
{	
	if (i2c_buff == NULL)
		return;
		
    // first write command address , pointing to register 0 for subsequent read 
	// if n >1 the read relies on the auto increment feature of the 7313 device
	i2c_buff[0] = MAX7313_I2C_ADDR; 
	i2c_buff[1] = addr; 			// Address of input reg.
	I2c_Receive( n );				// reads data in to i2c_buff[] starting at index 1
}	

void
MAX7313_Read_Switches( unsigned char * Sw1, unsigned char *Sw2)
{
	MAX7315_Receive_bytes(MAX7313_READ_PORT0_7_CMD,2);
	*Sw1 = i2c_buff[1] & SW1_BIT;
	*Sw2 = i2c_buff[2] & SW2_BIT;
}






