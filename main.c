
#include "msp430.h"
#include "includes/serial.h"
#include "includes/rtc.h"
#include "includes/sht20.h"
#include "includes/storage.h"
#include "includes/serial.h"
#include "includes/batteryMonitor.h"

#pragma PERSISTENT(memory)
extern union Memory memory = {0};

unsigned int temperature;
unsigned char humidity;
unsigned char cmd_ready;
unsigned char int_second = 0;
unsigned char int_hour = 0;
unsigned char getSampleFlag;
unsigned char initializedRtc = 0;
unsigned char samplingProgramed = 0;

unsigned char send_metadata();
unsigned char send_data();
unsigned char sendBatt();
void configureSampling();
void startSampling();


void initPorts();
void send_date();



unsigned char battLevel = 255;

void iniClocks();

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                 // Stop Watchdog
    initPorts();
    iniClocks();

    P1DIR |= BIT2;
    PM5CTL0 &= ~LOCKLPM5;
    P1OUT &= ~BIT2;

    //Serial module ini
    serialIni();

    //RTC ini
    RTCInit(55,59, 10, 1, 1, 2018);
    initializedRtc = 0;

   //sht20 sensor Ini();
    sht20Ini();

    battMonitorIni();
    getBattLevel(&battLevel);

//    memory.storage.metadata.active = 1;
//    memory.storage.metadata.frequency = 1;
//    memory.storage.metadata.frequency_unit = MINUTES;
//    memory.storage.metadata.number_of_samples = 0;
//    RTCsetAlarm(1, 0, UNIT_SECONDS);
    startSampling();
    while(1)
    {
        __disable_interrupt();
        if(!(int_second | cmd_ready | getSampleFlag | int_hour))
        {
            __bis_SR_register(LPM3_bits + GIE);
        }

        if(int_second)
        {
            int_second = 0;
        }

        if(getSampleFlag)
        {
            getSampleFlag = 0;
            temperature = read_temperature();
            humidity = read_humidity();
            if(!saveSample(temperature, humidity))
            {
                RTCCancelAlarm();
            }
        }

        if(cmd_ready)
        {
            switch(cmd_ready)
            {
                case CMD_REQUEST_METADATA:
                   send_metadata();
                   break;
                case CMD_CHECK_BATT:    //Read batt
                    sendBatt();
                    break;
                case CMD_READ_DATA:
                    send_data();
                    break;
                case CMD_REQUEST_DATE:
                    send_date();
                    break;
                case CMD_CONFIGURE_SAMPLING:
                    configureSampling();
                case CMD_START_SAMPLING:
                    startSampling();
                    break;
            }
            cmd_ready = 0;
            UCA0IE |= UCRXIE;
        }

        if(int_hour)
        {
            int_hour = 0;
            getBattLevel(&battLevel);
            //Check sampling start and enable alarm
            if(samplingProgramed)
            {
                if(memory.storage.metadata.initial_date.hours == RTCHOUR)
                {

                    samplingProgramed = 0;
                    //getSampleFlag = 1;
                }
            }
        }

    }//end while
}

void configureSampling(){
    samplingProgramed = 1;
}

void startSampling()
{
    if(initializedRtc)
    {
        memory.storage.metadata.active = 1;
        memory.storage.metadata.frequency = serial_buffer[1];
        memory.storage.metadata.frequency_unit = serial_buffer[2];
        memory.storage.metadata.number_of_samples = 0;
        while(! (RTCCTL01 & RTCCTL01));
        memory.storage.metadata.initial_date.seconds = RTCSEC;
        memory.storage.metadata.initial_date.minutes = RTCMIN;
        memory.storage.metadata.initial_date.hours = RTCAHOUR;
        memory.storage.metadata.initial_date.day = RTCDAY;
        memory.storage.metadata.initial_date.month = RTCMON;
        memory.storage.metadata.initial_date.year = RTCYEAR;
        unsigned char offset = 0;

        if(serial_buffer[2] == UNIT_HOURS)
                offset = memory.storage.metadata.initial_date.minutes;

        if(serial_buffer[2] == UNIT_DAYS)
                offset = memory.storage.metadata.initial_date.hours;
        RTCsetAlarm(serial_buffer[1], offset, serial_buffer[2]);
        getSampleFlag = 1;
        sendACK();
    }
    else{
        //Send NACK message
    }
}

unsigned char send_metadata()
{
    serial_buffer[0] = 0xA0;
    serial_buffer[1] = 0xA1;
    serial_buffer[2] = 0;
    serial_buffer[4] = CMD_SEND_METADATA;
    unsigned char metadata_bytes = readStorageMetadata(serial_buffer+5);
    serial_buffer[3] = metadata_bytes+2; //cmd+metadata+check
    serial_buffer[5 + metadata_bytes] = calcChecksum(serial_buffer + 4, metadata_bytes+1);
    serialWrite(serial_buffer, 6 + metadata_bytes); //Preambulo, metadataBytes y checksum
    return 0;
}

unsigned char send_data()
{
    unsigned short addr = serial_buffer[1] + serial_buffer[2]*256;
    unsigned short nSamples = serial_buffer[3] + serial_buffer[4]*256;
    unsigned short bytes_read;
    serial_buffer[0] = 0xA0;
    serial_buffer[1] = 0xA1;
    serial_buffer[2] = 0;
    serial_buffer[4] = CMD_SEND_DATA;
    bytes_read = readStorageData(serial_buffer+5, addr, nSamples);
    serial_buffer[3] = bytes_read+2;
    serial_buffer[5 + bytes_read] = calcChecksum(serial_buffer + 4, bytes_read+1);
    serialWrite(serial_buffer, 6 + bytes_read); //Preambulo, data bytes y checksum
    return 0;
}

unsigned char sendBatt()
{
    serial_buffer[0] = 0xA0;
    serial_buffer[1] = 0xA1;
    serial_buffer[2] = 0;
    serial_buffer[3] = 3;
    serial_buffer[4] = CMD_BATTERY_LEVEL;
    serial_buffer[5] = battLevel;
    serial_buffer[6] = calcChecksum(serial_buffer+4,2);
    serialWrite(serial_buffer, 7); //Preambulo, data bytes y checksum
    return 0;
}

void initPorts()
{
    P1OUT = 0;
    P1DIR = 0xFF & ~(BIT7 | BIT6);     //RXD y TXD

    P2OUT = 0;
    P2DIR = 0xFF & ~(BIT0 | BIT1);     //SDA y SCL

    P3OUT = 0;
    P3DIR = 0xFF;

    P4OUT = 0;
    P4DIR = 0xFF;

    PJOUT = 0;
    PJDIR = 0xFFFF & ~(BIT5 & BIT4);    //XOUT y XIN

    PM5CTL0 &= ~LOCKLPM5;
}


void send_date()
{
    serial_buffer[0]=0xA0;
    serial_buffer[1]=0xA1;
    serial_buffer[2]=0;
    serial_buffer[3]=9;
    serial_buffer[4]=CMD_DEVICE_DATE;
    if(initializedRtc){
        while(! (RTCCTL01 & RTCCTL01));
        serial_buffer[5]=RTCSEC;
        serial_buffer[6]=RTCMIN;
        serial_buffer[7]=RTCHOUR;
        serial_buffer[8]=RTCDAY;
        serial_buffer[9]=RTCMON;
        serial_buffer[10]=RTCYEAR_H;
        serial_buffer[11]=RTCYEAR_L;
    }
    else
    {
        serial_buffer[5]=0;
        serial_buffer[6]=0;
        serial_buffer[7]=0;
        serial_buffer[8]=0;
        serial_buffer[9]=0;
        serial_buffer[10]=0;
        serial_buffer[11]=0;
    }

    serial_buffer[12] = calcChecksum(serial_buffer+4,8);
    serialWrite(serial_buffer, 13);
}



void iniClocks()
{
    // Setup Clocks
//    PJOUT = 0x00;
//    PJSEL0 |= BIT4 | BIT5;
//
//    PM5CTL0 &= ~LOCKLPM5;
//
//     CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
//     CSCTL1 = DCOFSEL_6 ;
//     CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK; // set ACLK = XT1; MCLK = DCO
//     CSCTL3 = DIVA__1 | DIVS__1 |DIVM__1;     // Set all dividers to 1
//     CSCTL4 &= ~LFXTOFF;                       // Enable LFXT1
//     do
//     {
//       CSCTL5 &= ~LFXTOFFG;                    // Clear XT1 fault flag
//       SFRIFG1 &= ~OFIFG;
//     }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
//     CSCTL0_H = 0;


     // Setup Clocks SOLO ACLK
     PJOUT = 0x00;
     PJSEL0 |= BIT4 | BIT5;

     PM5CTL0 &= ~LOCKLPM5;

      CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
      CSCTL1 = DCOFSEL_6 ;
      CSCTL2 = SELA__LFXTCLK | SELM__DCOCLK; // set ACLK = XT1; MCLK = DCO
      CSCTL3 = DIVA__1  |DIVM__1;     // Set all dividers to 1
      CSCTL4 &= ~LFXTOFF;                       // Enable LFXT1
      do
      {
        CSCTL5 &= ~LFXTOFFG;                    // Clear XT1 fault flag
        SFRIFG1 &= ~OFIFG;
      }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
      CSCTL0_H = 0;






     // Setup Clocks
//        PJOUT = 0x00;
//        PJSEL0 |= BIT4 | BIT5;
//
//         CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
//         CSCTL1 = DCOFSEL_3 | DCORSEL;                      // Set DCO to 1MHz
//         CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK; // set ACLK = XT1; MCLK = DCO
//         CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers to 1
//         CSCTL4 &= ~LFXTOFF;                       // Enable LFXT1
//         do
//         {
//           CSCTL5 &= ~LFXTOFFG;                    // Clear XT1 fault flag
//           SFRIFG1 &= ~OFIFG;
//         }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
//         CSCTL0_H = 0;

}


