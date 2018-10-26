#include <msp430.h>
#include "rtc.h"

extern unsigned char samplingStarted = 0;
unsigned char timeCounter = 0;

void RTCInit(unsigned char seconds, unsigned char minutes, unsigned char hours, unsigned char day, unsigned char month,unsigned short year)
{
    RTCCTL01 = RTCRDYIE  + RTCHOLD;


    RTCYEAR = year;                         // Year = 0x2011
    RTCMON = month;                            // Month = 0x10 = October
    RTCDAY = day;                            // Day = 0x07 = 7th
    RTCDOW = 0x05;                            // Day of week = 0x05 = Friday
    RTCHOUR = hours;                           // Hour = 0x11
    RTCMIN = minutes;                            // Minute = 0x59
    RTCSEC = seconds;                            // Seconds = 0x45

    RTCCTL01 &= ~(RTCHOLD);                   // Start RTC calendar mode
    RTCCTL0 |= RTCRDYIE + RTCTEVIE;
    initializedRtc = 1;
}

void RTCsetAlamr(unsigned char )
{

}

#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
{
  switch (__even_in_range(RTCIV, RTCIV_RTCOFIFG)){
    case RTCIV_NONE: break;
    case RTCIV_RTCRDYIFG:
        int_second = 1;

        //__bic_SR_register_on_exit(LPM3_bits);
        break;
    case RTCIV_RTCTEVIFG:
        int_hour = 1;
        if(samplingStarted)
        {
            timeCounter++;
            if(timeCounter == numberOfTimeUnits)
            {
                getSampleFlag = 1;
                timeCounter = 0;
                __bic_SR_register_on_exit(LPM3_bits);
            }
        }
      break;
    case RTCIV_RTCAIFG: break;
    case RTCIV_RT0PSIFG: break;
    case RTCIV_RT1PSIFG: break;
    case RTCIV_RTCOFIFG: break;
  }
}
