

#ifndef INCLUDES_BATTERYMONITOR_H_
#define INCLUDES_BATTERYMONITOR_H_

//Voltage en bits Avcc/2 , 12 bits , ref= 2V
#define MAX_BATT_VOLTAGE    3072//3072//3.0V
#define MIN_BATT_VOLTAGE    2252//2.2V
#define ADAPTER_VOLTAGE_THRESHOLD   3200

void battMonitorIni();
unsigned char getBattLevel();



#endif /* INCLUDES_BATTERYMONITOR_H_ */
