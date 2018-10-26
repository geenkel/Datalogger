#include "i2c.h"

#ifndef INCLUDES_SHT20_H_
#define INCLUDES_SHT20_H_

#define TEMPERATURE_CMD     0xE3
#define HUMIDITY_CMD        0xE5


void sht20Ini();

unsigned int read_temperature();

unsigned char read_humidity();



#endif /* INCLUDES_SHT20_H_ */
