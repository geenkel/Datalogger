#ifndef I2C_H

#define I2C_H

#include <msp430.h>


#define SHT20_ADDR      0x40

void writeRegister(unsigned char address, unsigned char *data, unsigned char n);
void readRegister(unsigned char address, unsigned char *data, unsigned char n);
void i2cIni();

#endif	//I2C_H
