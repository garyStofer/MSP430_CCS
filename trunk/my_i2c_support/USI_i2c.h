#ifndef I2C_H_
#define I2C_H_
#define I2C_BufferSz  12

extern unsigned char * I2c_Init(unsigned char *buff_sz );
extern void I2c_Transmit(unsigned char n); 
extern unsigned char I2c_Receive(void);

#endif /*I2C_H_*/
