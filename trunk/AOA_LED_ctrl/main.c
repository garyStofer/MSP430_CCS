//******************************************************************************
//  Angle of attack indicator

//******************************************************************************
#include <msp430x20x3.h>
#include "stddef.h"
#include "MAX_7313.h"


#define FlapSteps 3
typedef struct tag_flash_mem 
{	unsigned short Flap_low;
	unsigned short Flap_high;
	unsigned short Vane_0deg;
	unsigned short Stall[FlapSteps];
} Persist_data;
// define some symbols at the beginning of each Flash info section 
Persist_data *p_data; 		// pointer to the current persist data segment, either info_B or info_C
Persist_data info_B;		// in flash memory 
Persist_data info_C;		// in flash memory 
Persist_data info_D;		// in flash memory
// Note: Info_A is used for the calibration values and is not to be disturbed 
#pragma DATA_SECTION (info_B , ".infoB");  
#pragma DATA_SECTION (info_C , ".infoC");  
#pragma DATA_SECTION (info_D , ".infoD"); 



static void
Flash_EraseOld_and_Reset( void) 
{
	
	FCTL1 = FWKEY + ERASE;   	// Make eraseable
	FCTL3 = FWKEY ; 		 	// undoo the lock 	
	p_data->Flap_low =0xffff;	// This starts the erase of the section, takes ~12ms 	
	
	FCTL3 = FWKEY +  LOCK;     // Lock all segments 
	FCTL1 = FWKEY ;            // remove Write and Erase bits 
	
	WDTCTL = WDTPW + 0x3; 		// This will create an Watchdog reset almost immediatly... 
}	


void
main(void)
{
	unsigned char x;
	unsigned char y;
	signed char br=5;			// init to medium brightness
	volatile unsigned int adc;

		
	WDTCTL = WDTPW + WDTHOLD;   // Stop watch-dog timer
	BCSCTL2 = DIVS_3;      		// MCLK = DCO, SMCLK = DCO/8  (== 2mhz)
	BCSCTL3 = 0;      	   		// No 32khz Xtal connected, LXT1 and VLO not used

	P1OUT= 0x1;
	P1DIR = 0x1;
	P1DIR = 0x0;

	P2SEL = 0;
	P2DIR = 0x00;  // input/tristate
	P2OUT = 0x00;  // if output drive low


/* // Could Trap CPU if DCO cal data is erase
   if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF)                                     
    	while(1);                          // If calibration constants erased not execute, trap CPU!!
*/
//	_BIS_SR(GIE);

	MAX7315_Init( br );


// Led self-test
	for (x = 0; x <= NumLEDs; x++)
	{
		MAX7315_DisplayDot(x);
		for (adc= 0x3fff; adc; adc--) 	;
	}

	for (br = 0; br <= 0xe; br++)
	{
		MAX7315_MasterBright(br);
		for (adc= 0x2fff; adc;  adc--) 	;
	}

	for (br--; br >0; br--)
	{
		MAX7315_MasterBright(br);
		for (adc= 0x2fff; adc;  adc--) 	;
	}

	for (x--; x >6; x--)
	{
		MAX7315_DisplayDot(x);
		for (adc= 0x3fff; adc; adc--) 	;
	}

	// Time spent in LED selftest allows the power rail to stabilize before switching to fast clock ...
	// CPU fails when switching under 3.3V


	// DCO set for 16Mhz  -- After sufficient delay above
	DCOCTL  = CALDCO_16MHZ;  // CAL data for DCO modulator
	BCSCTL1 = CALBC1_16MHZ;  // CAL data for DCO -- ACKL div /1

	// Flash clock setup
	FCTL2 = FWKEY + FSSEL_3 + FN2;  // SCLK/5, == 400khz,  Single write is 75us, Section erase ~= 12ms

	// find the location of the persistent data,  keying on the fact that the low point of the flap pot can never be 0xffff
//	p_data = ( info_B.Flap_low != 0xffff )? &info_B : &info_C;


	while ( 1 )
	{
		MAX7313_Read_Switches( &x, &y);
		if (y)
		{
			if (++br>=14)
				br=14;

			MAX7315_MasterBright(br);
			do
			{
				MAX7313_Read_Switches( &x, &y);
			} while (y);

		}
		else if (x)
		{
			if (--br<0)
				br=0;
			MAX7315_MasterBright(br);
			do
			{
				MAX7313_Read_Switches( &x, &y);
			} while (x);

		}

	}

	


}


