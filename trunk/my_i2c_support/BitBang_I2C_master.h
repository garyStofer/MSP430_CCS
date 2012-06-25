
//I2C_Master.h

/*
use i2cm_out(...) and i2cm_in(...) between calls to i2cm_start(...) and i2cm_stop(...)

typical use(...) 
{
//this is an example of reading the config register of a DS1631
//in this case 'device' is a select number of 0000xxx0
//This example is incomplete in that the bus is not left with a
//stop condition if something goes wrong

	i2cm_start( );
	
	if( i2cm_out( 0x90 | device ) ) //send write control byte
		return;
	
	else if( i2cm_out( 0xac ) )//send config command byte
		return;
	
	i2cm_start( );//device wants start again to read
	
	if( i2cm_out( 0x91 | device ) ) //send read control byte
		return;
	
	i2cm_in( buf, 1 ); //input the config byte to 'buf'

	i2cm_stop( );
	//device was not busy or failed...
	//set success flag if needed before return
	
} typical use end......


with mspgcc compiled for optimum code size this is about 256 bytes

*/


//Defines the port to be used like:
#define I2C_MASTER_REN P2REN
#define I2C_MASTER_DIR P2DIR
#define I2C_MASTER_OUT P2OUT
#define I2C_MASTER_IN  P2IN
//port pins
#define I2C_MASTER_SCL BIT6
#define I2C_MASTER_SDA BIT7
//Timing for 16Mhz MSP430 CPU clock  24,7,15
#define I2C_MASTER_SDLY		24
#define I2C_MASTER_SCLK_L_DELAY  7
#define I2C_MASTER_SCLK_H_DELAY  15

#define I2C_N_BITS 8
#define I2C_BufferSz  12
static unsigned char i2cBuff[I2C_BufferSz] ;          // Array for transmit data

extern unsigned char * I2c_Init(unsigned char *buff_sz );

extern void I2c_Transmit(int n);
extern void I2c_Receive(int n);
//Declarations
//sends a start condition
//will set SDA and SCL high and delay before start
static void i2cm_start( void );

//send stop condition
//will set SDA low before delay and stop
static void i2cm_stop( void );

//Output one byte
//assumes sda and scl low and leaves sda, scl low if ack.
//returns true if no ack from device
static unsigned char i2cm_out( unsigned char* buf, int count );

//input count of bytes into buf[ ]
//Assumes scl low and leaves scl low
//sends ack to device until last byte then no ack
static void i2cm_in( unsigned char* buf, int count );

