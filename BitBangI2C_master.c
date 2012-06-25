#include <msp430x20x3.h>
#include "BitBangI2C_master.h"

static unsigned char i2cBuff[I2C_BufferSz] ;          // Array for transmit data

unsigned char *
I2c_Init( unsigned char *buff_sz )
{
	//Set output drive  low -- actual signal states are generated with tri-state control, H- tristate , L = drive
	I2C_MASTER_OUT&= ~( I2C_MASTER_SDA | I2C_MASTER_SCL );

	*buff_sz = I2C_BufferSz;
	return i2cBuff;
}

void
I2c_Transmit(int n)
{
	if (n > I2C_BufferSz)
		return ;

	i2cBuff[0] &= ~0x01; 			// fix address , this is the "write" I2C address now
	i2cm_start();
// TODO: need to check for ack and return false if no ACK
	i2cm_out( i2cBuff, n+1);		// sending device address + data to slave
	i2cm_stop();					// transition bus into idle state
}


void
I2c_Receive(int n)		// Read transaction from slave
{
	if (n > I2C_BufferSz)
		return ;

	i2cBuff[0] &= ~0x01; 		// Write address first
	i2cm_start();				// transition from idle state into start condition
	i2cm_out( i2cBuff, 2);		// device address and command address in write mode
//	i2cm_stop(); // not needed
	i2cBuff[0] |= 0x01; 		// Set Read mode address bit

	i2cm_start();				// restart for read phase
	i2cm_out( i2cBuff, 1);		// Device addressed in read mode now
	i2cm_in ( i2cBuff, n);		// Reads from slave with ACK from master
	i2cm_stop();				// transition bus into idle state
	return;
}



static void
i2cm_start( void )
{

	//Make sure both pins are high, sequenced so that it can't be mistaken for a stop from anywhere
	I2C_MASTER_DIR |= I2C_MASTER_SCL;		// SCL low to  transition into start mode
	_delay_cycles( I2C_MASTER_SCLK_L_DELAY );
	I2C_MASTER_DIR &= ~ I2C_MASTER_SDA ;	// SDA high wile SCL is low
	I2C_MASTER_DIR &= ~ I2C_MASTER_SCL ;	// SCL high


	_delay_cycles( I2C_MASTER_SDLY );

	I2C_MASTER_DIR|= I2C_MASTER_SDA;		// SDA going low while SCL is high signals start condition
	_delay_cycles( I2C_MASTER_SDLY );

	I2C_MASTER_DIR|= I2C_MASTER_SCL;		// SCL Low
}


static void
i2cm_stop( void )
{
	I2C_MASTER_DIR|= I2C_MASTER_SCL;		// SCL low
	I2C_MASTER_DIR|= I2C_MASTER_SDA;		// SDA low

	_delay_cycles( I2C_MASTER_SDLY );

	I2C_MASTER_DIR&= ~I2C_MASTER_SCL;		// SCL high
	_delay_cycles( I2C_MASTER_SDLY );

	I2C_MASTER_DIR&= ~I2C_MASTER_SDA;		// SDA high -- Stop condition
}

// Must be preceded by a start
static unsigned char
i2cm_out( unsigned char* buff, int count)
{
	unsigned int i;
	unsigned char data;
	unsigned int nack ;

	for (;count;count--)	// output 'count' bytes
	{
		data = *buff++;

		for(i=0 ; i < I2C_N_BITS; ++i )//output eight bits of data per byte
		{

			I2C_MASTER_DIR|= I2C_MASTER_SCL; 	//SCL low
			_delay_cycles( I2C_MASTER_SCLK_L_DELAY );

			//send the data bit
			if( data & 0x80 )
				I2C_MASTER_DIR &= ~I2C_MASTER_SDA;
			else
				I2C_MASTER_DIR|= I2C_MASTER_SDA;

			//shift next data bit
			data<<= 1;

			// SCL_High - clock in bit
			I2C_MASTER_DIR &= ~I2C_MASTER_SCL;		//SCL high
			_delay_cycles( I2C_MASTER_SCLK_H_DELAY );

		}
		// Now read the ACK from the slave
		I2C_MASTER_DIR|= I2C_MASTER_SCL;  // Clock low
		I2C_MASTER_DIR&= ~I2C_MASTER_SDA; // SDA input
		_delay_cycles( I2C_MASTER_SCLK_L_DELAY );

		I2C_MASTER_DIR&= ~(I2C_MASTER_SCL); //SDA input, Clock high --> clock out the ACK from slave
		_delay_cycles( I2C_MASTER_SCLK_H_DELAY );

		nack = ( I2C_MASTER_IN & I2C_MASTER_SDA );  // Read ACK, Slave pulls SDA low for ack

		if ( nack)
		{
			break;
		}
	}
	I2C_MASTER_DIR|= I2C_MASTER_SCL;  // Clock low
	return nack;
}

//Must be preceded by a i2cm_out to setup the read address first
static void
i2cm_in( unsigned char* buf, int count )
{
	unsigned char data;
	unsigned int i;

	buf++; 		// start returning data at index 1 leave device address in [0] untouched

	for( ; count; count-- )
	{
		data= 0;

		I2C_MASTER_DIR|= I2C_MASTER_SCL;	// SCL Low
		_delay_cycles( I2C_MASTER_SCLK_L_DELAY );

		I2C_MASTER_DIR&= ~I2C_MASTER_SDA;	// SDA is input
		_delay_cycles( I2C_MASTER_SCLK_L_DELAY );


		for( i=0; i < I2C_N_BITS; ++i )
		{

			I2C_MASTER_DIR&= ~I2C_MASTER_SCL;	// SCL high --
			_delay_cycles( I2C_MASTER_SCLK_H_DELAY );
			
			//shift the bit over
			data = data << 1;
		
			if( I2C_MASTER_IN & I2C_MASTER_SDA )	// read data bit
				data|= 0x01;

			I2C_MASTER_DIR|= I2C_MASTER_SCL;		// SCL Low
			_delay_cycles( I2C_MASTER_SCLK_L_DELAY );


		}
		//put the input  byte into the buffer
		*buf++= data;
		
		//Ack  for all but last byte
		if( count > 1 )
		{
			I2C_MASTER_DIR|= I2C_MASTER_SDA;		// SDA low to send the ACK to the salve
			I2C_MASTER_DIR&= ~I2C_MASTER_SCL;		// SCL high --
			_delay_cycles( I2C_MASTER_SCLK_H_DELAY );


		}

	}

}


