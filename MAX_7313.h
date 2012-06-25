#ifndef MAX_7313_H_
#define MAX_7313_H_
/* Defines for MAXIM 7313 I2c portexpander/LED driver */

#define MAX7313_I2C_ADDR 		0x40  // MAX7315 Address A0-A2 inputs connected to GND = 0x40 according to Table1
#define MAX7313_READ_PORT0_7_CMD   0x00  // Auto increment
#define MAX7313_READ_PORT8_15_CMD  0x01  // Auto increment
#define MAX7713_WRITE_PORT_CMD 	0x02  // Auto Increment to 0x3,Blink Phase0 port control	
#define MAX7313_PORT_SETUP 		0x06  // Auto Increment
#define MAX7313_MASTER_PWM		0x0e  // Non auto increment	
#define MAX7313_CONTROL_REG 	0x0f  // Non auto increment 
#define MAX7313_INDIVID_PWM 	0x10  // Auto Increment

#define NumLEDs 12
#define TargetLED 9	// the blue led, one based
#define SW1_BIT 0x01
#define SW2_BIT 0x20

// exported functions 
extern void MAX7315_Init( unsigned char dim );
extern void MAX7315_MasterBright(unsigned char level);
extern void MAX7315_DisplayDot( signed char level);
extern void MAX7313_Read_Switches( unsigned char * Sw1, unsigned char *Sw2);




#endif /*MAX_7313_H_*/
