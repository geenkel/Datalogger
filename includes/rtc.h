#ifndef INCLUDES_RTC_H_
#define INCLUDES_RTC_H_

extern unsigned char int_second;
extern unsigned char int_hour;
extern unsigned char getSampleFlag;
extern unsigned char initializedRtc;

void RTCInit(unsigned char seconds, unsigned char minutes, unsigned char hours, unsigned char day, unsigned char month,unsigned short year);


#endif /* INCLUDES_RTC_H_ */
