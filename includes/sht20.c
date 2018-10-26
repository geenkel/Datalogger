#include "sht20.h"


unsigned char data[2];

void sht20Ini(){
    i2cIni();
}

unsigned int read_temperature()
{
    readRegister(TEMPERATURE_CMD,data,2);
    //return sht20measure.measure;
    //unsigned int temperature = (data[0]<<8 & 0xFF00) & (data[1] & 0x00FF);
    unsigned int temperature = data[0]*256 +(data[1] & 0xFC);
    float temperatureF = -46.85f + 175.72f * (float)temperature / 65536;
    return (unsigned int)(temperatureF * 100.0f);
}

unsigned char read_humidity()
{
    readRegister(HUMIDITY_CMD,data,2);
    //return sht20measure.measure;
    //unsigned int temperature = (data[0]<<8 & 0xFF00) & (data[1] & 0x00FF);
    unsigned int humidity = data[0]*256 +(data[1] & 0xFC);
    float humidityF = -6.0F + 125.0F * humidity/65536;
    return (unsigned char)(humidityF);
}
