#include <msp430.h>
#include "serial.h"
#include "rtc.h"

unsigned char reading_command=0;

unsigned int command_size=0;
unsigned int serial_count=0;
unsigned char serial_countCMD=0;
extern unsigned char int_serial;
unsigned char serial_cmd_available = 0;
extern unsigned char cmd_ready;


unsigned char calcChecksum(unsigned char* buffer, unsigned int length);
void getSerialNumber(unsigned char* serial_number);
void sendACK();

unsigned char readCommand()
{
    unsigned char command_id = serial_buffer[0];
    switch(command_id)
    {
        case CMD_SET_TIME:
            RTCInit(serial_buffer[1],serial_buffer[2],serial_buffer[3],serial_buffer[4],serial_buffer[5],serial_buffer[6]*256 + serial_buffer[7]);
            sendACK();
            break;
        case CMD_REQUEST_SERIAL_NUMBER:
            serial_buffer[0]=0xA0;
            serial_buffer[1]=0xA1;
            serial_buffer[2]=0;
            serial_buffer[3]=10;
            serial_buffer[4]=0x20;

            getSerialNumber(serial_buffer+5);
            serial_buffer[13]=calcChecksum(serial_buffer+4,9);
            serialWrite(serial_buffer, 14);

            break;
        case CMD_SET_SAMPLE_RATE:
            //setSampleRate();
            __no_operation();
            break;
        case CMD_CHECK_BATT:
//            serial_buffer[0]=0xA0;
//            serial_buffer[1]=0xA1;
//            serial_buffer[2]=0;
//            serial_buffer[3]=3;
//            serial_buffer[4]=CMD_BATTERY_LEVEL;
//            serial_buffer[5]=87;
//            serial_buffer[6] = calcChecksum(serial_buffer+4,2);
//            serialWrite(serial_buffer, 7);
            break;
        case CMD_REQUEST_DATE:
            serial_buffer[0]=0xA0;
            serial_buffer[1]=0xA1;
            serial_buffer[2]=0;
            serial_buffer[3]=9;
            serial_buffer[4]=CMD_DEVICE_DATE;
            RTCCTL0 &= ~RTCRDYIFG;
            while(! (RTCCTL0 & RTCRDYIFG));
            serial_buffer[5]=RTCSEC;
            serial_buffer[6]=RTCMIN;
            serial_buffer[7]=RTCHOUR;
            serial_buffer[8]=RTCDAY;
            serial_buffer[9]=RTCMON;
            serial_buffer[10]=RTCYEAR_H;
            serial_buffer[11]=RTCYEAR_L;
            serial_buffer[12] = calcChecksum(serial_buffer+4,8);
            serialWrite(serial_buffer, 13);
            break;
        case CMD_REQUEST_METADATA:
            cmd_ready =CMD_REQUEST_METADATA;
            break;
        case CMD_READ_DATA:

            cmd_ready = CMD_READ_DATA;
            break;

    }
    return 1;
}

void sendACK()
{
    serial_buffer[0] = 0xA0;
    serial_buffer[1] = 0xA1;
    serial_buffer[2] = 0;
    serial_buffer[3] = 0x02;
    serial_buffer[4] = 0x01;
    serial_buffer[5] = 0xFE;
    serialWrite(serial_buffer, 6);

}

unsigned char calcChecksum(unsigned char* buffer, unsigned int length)
{
    unsigned int i;
    unsigned char checksum = 0;
    for(i=0;i<length;i++)
    {
        checksum += buffer[i];
    }
    checksum = 0xFF - checksum;
    return checksum;
}

void getSerialNumber(unsigned char* serial_number)
{
    serial_number[0]='1';
    serial_number[1]='2';
    serial_number[2]='3';
    serial_number[3]='4';
    serial_number[4]='5';
    serial_number[5]='6';
    serial_number[6]='7';
    serial_number[7]='8';
}

void serialWrite(unsigned char *buffer, unsigned int length)
{
    unsigned int i;
    for(i = 0; i< length; i++)
    {
        while(!(UCA0IFG&UCTXIFG));
        UCA0TXBUF = buffer[i];
    }
}

void serialPrint(char * string)
{
    unsigned int i=0;
    while(1)
    {
        while(!(UCA0IFG&UCTXIFG));
        if(string[i]==0)
        {
            return;
        }
        UCA0TXBUF = string[i];
        i++;
    }
}

void serialIni()
{
    //Initialize serial buffer
    unsigned char i;
    for(i=0;i<MAX_COMMAND_SIZE;i++)
    {
        serial_buffer[i]=0;
    }
    // Configure GPIO
    P2SEL1 |= BIT0 | BIT1;                    // USCI_A0 UART operation
    P2SEL0 &= ~(BIT0 | BIT1);
    PM5CTL0 &= ~LOCKLPM5;

    //Con SMCLK
//    UCA0CTLW0 = UCSWRST;                      // Put eUSCI in reset
//    UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK
//    // Baud Rate calculation
//    // 8000000/(16*9600) = 52.083
//    // Fractional portion = 0.083
//    // User's Guide Table 21-4: UCBRSx = 0x04
//    // UCBRFx = int ( (52.083-52)*16) = 1
//    UCA0BR0 = 52;                             // 8000000/16/9600
//    UCA0BR1 = 0x00;
//    UCA0MCTLW |= UCOS16 | UCBRF_1;
//    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
//    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt


    //Con ACLK
    // Configure USCI_A0 for UART mode
    UCA0CTLW0 = UCSWRST;                      // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__ACLK;                // CLK = ACLK
    UCA0BR0 = 3;                              // 9600 baud
    UCA0MCTLW |= 0x5300;                      // 32768/9600 - INT(32768/9600)=0.41
                                            // UCBRSx value = 0x53 (See UG)
    UCA0BR1 = 0;
    UCA0CTL1 &= ~UCSWRST;                     // Initialize eUSCI
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt


}

unsigned char checksum(unsigned int length)
{
    unsigned int i;
    unsigned char sum = 0;
    for(i = 0; i < length; i++)
    {
        sum += serial_buffer[i];
    }
    if(sum == 0xFF)
        return 1;
    else
        return 0;
}

unsigned char rxByte;
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
   switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
        rxByte = UCA0RXBUF;
        if(reading_command)
        {
            if(serial_count==0 | serial_count==1)
            {
                if(serial_count == 0){
                    command_size = rxByte;
                    command_size = command_size << 8 & 0xFF00;
                }
                else
                {
                    command_size = (command_size << 8) & 0xFF00;
                    command_size |= rxByte & 0x00FF;
                }
                if(command_size > MAX_COMMAND_SIZE)
                {
                    reading_command=0;
                    return;
                }
            }
            else
            {
                serial_buffer[serial_count-2] = rxByte;
                if(serial_count == command_size +1 )
                {
                    //Chek cheksum
                    if(checksum(command_size))
                    {
                        serial_cmd_available = 1;                          //serial command available.
                        reading_command = 0;
                        serial_count = 0;
                        cmd_ready = serial_buffer[0];
                        readCommand();
                        __bic_SR_register_on_exit(LPM0_bits);
                    }
                    else
                    {
                        reading_command = 0;
                        serial_count = 0;
                    }
                }
            }
            serial_count++;
        }
        else
        {
            if((serial_countCMD == 0 && rxByte == 0xA0) | (serial_countCMD == 1 && rxByte == 0xA1))
            {
                serial_countCMD++;
                if(serial_countCMD == 2)
                {

                    reading_command = 1;
                    serial_countCMD = 0;
                    serial_count = 0;
                }
            }
            else
            {
                serial_countCMD = 0;
            }
        }
      __no_operation();
      break;
    case 4: break;
    default: break;
  }
}
