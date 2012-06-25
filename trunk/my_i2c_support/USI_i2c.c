#include <msp430x20x3.h>
#include "stddef.h"
#include "USI_i2c.h"

// Globals for I2C ISR
typedef enum  I2Cstate {StartTX,StartRX,TX,RX,RX_end,Stop} t_I2Cstate;
static t_I2Cstate I2C_State ; 
static unsigned char n_tx;				// Var for number of bytes to transmit
static unsigned char i2cBuff[I2C_BufferSz] ;          // Array for transmit data

unsigned char *
I2c_Init( unsigned char *buff_sz )
{
	// USI setup for I2C
	USICTL0 = USIPE6+USIPE7+USIMST+USISWRST;	// Port & USI mode setup
	USICTL1 = USII2C;                			// Enable I2C mode 
// TODO change clock to faster i2C speed	
	USICKCTL = USIDIV_3+USISSEL_2+USICKPL;		// Setup USI clocks: SCL = SMCLK/5 (~250kHz)
	USICTL0 &= ~USISWRST;                		// Enable USI
	USICTL1 &= ~USIIFG;							// Clear Interrupt Flag
	*buff_sz = I2C_BufferSz;
	return i2cBuff;
}



void 
I2c_Transmit(unsigned char n) 
{
	n_tx = ( n <= I2C_BufferSz) ? n : I2C_BufferSz; // limit the number of bytes to be sent or received to max buffer size
	I2C_State = StartTX;    			// Entry into Interrupt SM 
	USICTL1 |= (USIIE + USIIFG);	// Enable interrupt and set interrupt flag to start IC2 ISR
	LPM0;                   		// CPU off, await USI transmit complete -- (exits with LPM0_EXIT)
}

 
unsigned char
I2c_Receive(void)		// single byte read 
{
	I2c_Transmit(1);
	i2cBuff[0] |= 0x1; 			// fix address , this is the "read" I2C address now
	I2C_State = StartRX;    		// Entry into Interrupt SM 
	USICTL1 |= (USIIE + USIIFG);	// Enable interrupt and set interrupt flag to start IC2 ISR
	LPM0;
	return  i2cBuff[1];			// this is where the interrupt placed the byte
}


// Interrupt service routines 
#pragma vector = USI_VECTOR
interrupt void 
ISR_USI_TXRX (void)
{
	  static unsigned char tx_ndx;	
	  
	  switch(I2C_State)
	  {
		  case StartTX:
		  case StartRX:
		  	tx_ndx = 0;					// Point to address in Data  
		  	USISRL = 0x00;           	// Generate I2c Start Condition...SDA going low while SCK is high
			USICTL0 |= USIGE+USIOE;
			USICTL0 &= ~USIGE;
			USISRL = i2cBuff[tx_ndx++]; 	// Set Slave address to be sent, bit0 indicates read or write  
			USICNT = 8; 					// Bit counter = 8, TX Address
#ifdef I2C_RX								// To conserve space, only enable TX if no RX needed 
			if (I2C_State == StartTX)
				I2C_State = TX;
			else
				I2C_State = RX;
			break;
	  	
	  case RX: // setup to receive Address Ack/Nack for address write next
	          USICTL0 &= ~USIOE;      // SDA = input
	          USICNT = 0x01;          // Bit counter = 1, receive one (N)Ack bit
	         
	          while (! (USICTL1&USIIFG)) // wait for ACK/NACK bit to be clocked in
	          ;
	          USICNT=8;					// Clock in 8 data bits from device -- 
	          I2C_State = RX_end;
	          break;
	          
	  case RX_end:
	  		  i2cBuff[1] = USISRL;		// store the read device's resposne somewhere
	  		  USISRL = 0;				// send Ack & primne for stop 
	  		  USICTL0 |= USIOE;        	// SDA = output again
	  		  USICNT = 1;      			// Bit counter = 1, SCL high, SDA low
	          I2C_State=Stop;			// terminate with STOP
	          break;
#else
			I2C_State = TX;
			break;
#endif 	  		       	  
	    		
	  case TX: // setup to receive Address Ack/Nack bit next
	          USICTL0 &= ~USIOE;      // SDA = input
	          USICNT = 0x01;          // Bit counter = 1, receive (N)Ack bit
	         
	          while (! (USICTL1&USIIFG)) // wait for ACK/NACK bit to be clocked in
	          ;
	         
	          
	          if ((USISRL & 0x01) || tx_ndx > n_tx )  // If NACK or nothing else to transmit
	          {
	          		USISRL = 0x00;		// Prime for stop, clock out a Low, so the stop condition can be generated 	
	          		USICTL0 |= USIOE;   // SDA = output again
	          		USICNT = 1;      	// Bit counter = 1, SCL high, SDA low
	          		I2C_State=Stop;		// terminate with STOP
	          }
	          else 
	          {
	          	USISRL = i2cBuff[tx_ndx++];  	// Load next data byte
	          	USICTL0 |= USIOE;        		// SDA = output again
	            USICNT = 8;       				// Bit counter = 8, start TX of next byte
	            // Stay at current state, check ACk again and trsnsmit next byte
	          }

	          break;
	
	   
	      case Stop:// Generate Stop Condition
	          USISRL = 0x0FF;          		// USISRL = 1 to release SDA
	          USICTL0 |= USIGE;        		// Transparent latch enabled
	          USICTL0 &= ~(USIGE+USIOE);	// Latch/SDA output disabled
	          USICTL1 &= ~USIIE;			// Disable Interrupts
	          LPM0_EXIT;               		// Exit active for next transfer
	          break;
	}

}
