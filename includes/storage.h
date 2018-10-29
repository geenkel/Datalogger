#ifndef INCLUDES_STORAGE_H_
#define INCLUDES_STORAGE_H_

#define MINUTES     2
#define MAX_SAMPLES 10000



struct Date
{
    unsigned char seconds;
    unsigned char minutes;
    unsigned char hours;
    unsigned char day;
    unsigned char month;
    unsigned short year;
};

struct Metadata
{
    unsigned char active;
    struct Date initial_date;
    unsigned short frequency;
    unsigned char frequency_unit;
    unsigned short number_of_samples;
};

struct Sample
{
    short temperature;
    unsigned char humidity;
};

struct Storage
{
    struct Metadata metadata;
    struct Sample samples[MAX_SAMPLES];
};


union Memory
{
    unsigned char raw[sizeof(struct Storage)];
    struct Storage storage;

};

extern union Memory memory;


void initStorage();
void setInitialDate(unsigned char seconds, unsigned char minutes, unsigned char hours, unsigned char day, unsigned char month, unsigned short year);
void setFreq(unsigned short frequency, unsigned short frequency_unit);
unsigned char saveSample(short temperature, unsigned char humidity);
unsigned short readStorageData(unsigned char *buffer , unsigned short addr, unsigned short nSamples);
unsigned char readStorageMetadata(unsigned char * buffer);


#endif /* INCLUDES_STORAGE_H_ */
