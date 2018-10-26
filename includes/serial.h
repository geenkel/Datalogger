#ifndef INCLUDES_SERIAL_H_
#define INCLUDES_SERIAL_H_

#define MCLK_FREQ_MHZ 8    // MCLK = 8MHz
#define MAX_COMMAND_SIZE    150

//PC Commands
#define CMD_SET_TIME        0x11
#define CMD_REQUEST_SERIAL_NUMBER   0x10
#define CMD_SET_SAMPLE_RATE 0x12
#define CMD_CHECK_BATT          0x13
#define CMD_REQUEST_DATE        0x14
#define CMD_READ_DATA           0x15
#define CMD_REQUEST_METADATA    0x16

//Device TX commands
#define CMD_SERIAL_NUMBER   0x20
#define CMD_BATTERY_LEVEL   0x21
#define CMD_DEVICE_DATE     0x22
#define CMD_SEND_DATA       0x25
#define CMD_SEND_METADATA   0x26

#define CMD_ACK             0x01

unsigned char serial_buffer[MAX_COMMAND_SIZE];

void serialIni();
void serialWrite(unsigned char *buffer, unsigned int length);
unsigned char readCommand();
unsigned char calcChecksum(unsigned char* buffer, unsigned int length);
void serialPrint(char * string);

#endif /* INCLUDES_SERIAL_H_ */
