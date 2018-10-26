#include "storage.h"

unsigned short currentAddr = 0;

void initStorage()
{
    memory.storage.metadata.active = 0;
    memory.storage.metadata.number_of_samples = 0;
}

void setInitialDate(unsigned char seconds, unsigned char minutes, unsigned char hours, unsigned char day, unsigned char month, unsigned short year)
{
    memory.storage.metadata.initial_date.seconds = seconds;
    memory.storage.metadata.initial_date.minutes = minutes;
    memory.storage.metadata.initial_date.hours = hours;
    memory.storage.metadata.initial_date.day = day;
    memory.storage.metadata.initial_date.month = month;
    memory.storage.metadata.initial_date.year = year;
}

void setFreq(unsigned short frequency, unsigned short frequency_unit)
{
    memory.storage.metadata.frequency = frequency;
    memory.storage.metadata.frequency = frequency_unit;
}

void saveSample(short temperature, unsigned char humidity)
{
    if(memory.storage.metadata.active)
    {
        if(currentAddr < MAX_SAMPLES)
        {
            memory.storage.samples[currentAddr].temperature = temperature;
            memory.storage.samples[currentAddr].humidity = humidity;
            currentAddr++;
            memory.storage.metadata.number_of_samples ++;
        }
    }
}

unsigned char readStorageMetadata(unsigned char * buffer)
{
    unsigned char length  = sizeof(memory.storage.metadata);
    unsigned char i;
    for(i = 0; i < length; i++)
    {
        buffer[i] = memory.raw[i];
    }
    return length;
}


unsigned short readStorageData(unsigned char *buffer , unsigned short addr, unsigned short nSamples)
{
    unsigned short i;
    unsigned char * ptrToSamples = (unsigned char *) (&memory.storage.samples[addr]);
    unsigned short n = nSamples;
    if(nSamples+addr > memory.storage.metadata.number_of_samples)
        n -=  nSamples + addr - memory.storage.metadata.number_of_samples;

    for(i = 0 ; i < n; i ++  )
    {
        buffer[i*3] = ptrToSamples[i*4];
        buffer[i*3+1] = ptrToSamples[i*4+1];
        buffer[i*3+2] = ptrToSamples[i*4+2];
    }
    return n*3;
}
