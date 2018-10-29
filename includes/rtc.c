#include <msp430.h>
#include "rtc.h"

extern unsigned char samplingStarted = 0;
unsigned char timeCounter = 0;
unsigned char period;
unsigned char unit = 0;

void RTCInit(unsigned char seconds, unsigned char minutes, unsigned char hours, unsigned char day, unsigned char month,unsigned short year)
{
    RTCCTL01 |= RTCHOLD;


    RTCYEAR = year;                         // Year = 0x2011
    RTCMON = month;                            // Month = 0x10 = October
    RTCDAY = day;                            // Day = 0x07 = 7th
    RTCDOW = 0x05;                            // Day of week = 0x05 = Friday
    RTCHOUR = hours;                           // Hour = 0x11
    RTCMIN = minutes;                            // Minute = 0x59
    RTCSEC = seconds;                            // Seconds = 0x45

    RTCCTL01 &= ~(RTCHOLD);                   // Start RTC calendar mode
    RTCCTL01 |= RTCTEV_1;            //Para revisar la batería cada hora
    RTCCTL01 |=  RTCTEVIE;           //Interrupción por evento de hora
    initializedRtc = 1;
}

void RTCCancelAlarm()
{
    unit = 0;
    timeCounter = 0;
    RTCCTL01 = (RTCCTL01 & ~RTCTEV_3) | RTCTEV_1;
    RTCCTL01 &= ~RTCAIE;
    RTCCTL01 &= ~RTCRDYIE;
    RTCADAY = 0;
    RTCADOW = 0;
    RTCAHOUR = 0;
    RTCAMIN = 0;
    RTCADOWDAY = 0;
}

void RTCsetAlarm(unsigned char times, unsigned char offset, unsigned char u)
{
    RTCCancelAlarm();
    unit = u;
    period = times;
    if(unit == UNIT_DAYS)
    {
        RTCAHOUR = offset | 0x80;
        RTCCTL01 |= RTCAIE;          //activa la alarma
    }
    else if(unit == UNIT_HOURS)
    {
        RTCAMIN = offset | 0x80;
        RTCCTL01 |= RTCAIE;          //activa la alarma
    }
    else if(unit == UNIT_MINUTES)
    {
        RTCCTL01 = (RTCCTL01 & ~RTCTEV_3) | RTCTEV_0;
    }
    else if(unit == UNIT_SECONDS)
    {
        RTCCTL01 |= RTCRDYIE;
    }
}



#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
{
  switch (__even_in_range(RTCIV, RTCIV_RTCOFIFG)){
    case RTCIV_NONE: break;
    case RTCIV_RTCRDYIFG:
        timeCounter++;
        if(timeCounter == period)
        {
            getSampleFlag = 1;
            timeCounter = 0;
            __bic_SR_register_on_exit(LPM3_bits);
        }
        break;
    case RTCIV_RTCTEVIFG:
        if(unit == UNIT_MINUTES)
        {
            timeCounter++;
            if(timeCounter == period)
            {
                getSampleFlag = 1;
                timeCounter = 0;
                __bic_SR_register_on_exit(LPM3_bits);
            }
            if(RTCMIN == 0)
            {
                int_hour = 1;
                __bic_SR_register_on_exit(LPM3_bits);
            }
        }
        else
        {
            int_hour = 1;
            __bic_SR_register_on_exit(LPM3_bits);
        }
        break;
    case RTCIV_RTCAIFG:
        timeCounter++;
        if(timeCounter == period)
        {
            getSampleFlag = 1;
            timeCounter = 0;
            __bic_SR_register_on_exit(LPM3_bits);
        }
        break;
    case RTCIV_RT0PSIFG: break;
    case RTCIV_RT1PSIFG: break;
    case RTCIV_RTCOFIFG: break;
  }
}
