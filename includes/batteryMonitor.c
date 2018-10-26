#include "batteryMonitor.h"
#include "msp430.h"

unsigned char sampleFinish = 0;
unsigned int sample;

void battMonitorIni()
{
    while(REFCTL0 & REFGENBUSY);
    REFCTL0 |= REFVSEL_1 + REFON;             // Enable internal 1.2V reference

    /* Initialize ADC12_A */
    ADC12CTL0 &= ~ADC12ENC;                   // Disable ADC12
    ADC12CTL0 = ADC12SHT0_8 + ADC12ON;        // Set sample time
    ADC12CTL1 = ADC12SHP;                     // Enable sample timer
    ADC12CTL3 = ADC12BATMAP;                   // Enable internal temperature sensor
    ADC12MCTL0 = ADC12VRSEL_1 + ADC12INCH_31; // ADC input ch A30 => Avcc/2 sense
    ADC12IER0 = 0x001;                        // ADC_IFG upon conv result-ADCMEMO
    while(!(REFCTL0 & REFGENRDY));

}


unsigned char getBattLevel(unsigned char * battLevel)
{
    ADC12CTL0 |= ADC12ENC;
    ADC12CTL0 |= ADC12SC;
    __disable_interrupt();
    while(!sampleFinish)
    {
        __bis_SR_register(LPM0_bits + GIE);
        __disable_interrupt();
    }
    ADC12CTL0 &= ~ADC12ENC;
    __enable_interrupt();
    sampleFinish = 0;

    if(sample < MIN_BATT_VOLTAGE)
    {
        battLevel[0] = 1;
        return 1;
    }
    if(sample > MAX_BATT_VOLTAGE)
    {
        if(sample > ADAPTER_VOLTAGE_THRESHOLD)
            return 0;
        else
        {
            battLevel[0] = 100;
            return 1;
        }
    }
    float battLevelF = sample;
    battLevelF = (battLevelF - MIN_BATT_VOLTAGE) / (MAX_BATT_VOLTAGE - MIN_BATT_VOLTAGE) * 100;
    battLevel[0] = (unsigned char)battLevelF;
    return 1;
}


#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR (void)
{
  switch(__even_in_range(ADC12IV, ADC12IV_ADC12RDYIFG))
  {
    case ADC12IV_NONE:        break;        // Vector  0:  No interrupt
    case ADC12IV_ADC12OVIFG:  break;        // Vector  2:  ADC12MEMx Overflow
    case ADC12IV_ADC12TOVIFG: break;        // Vector  4:  Conversion time overflow
    case ADC12IV_ADC12HIIFG:  break;        // Vector  6:  ADC12BHI
    case ADC12IV_ADC12LOIFG:  break;        // Vector  8:  ADC12BLO
    case ADC12IV_ADC12INIFG:  break;        // Vector 10:  ADC12BIN
    case ADC12IV_ADC12IFG0:                 // Vector 12:  ADC12MEM0 Interrupt
      sample = ADC12MEM0;                     // Move results, IFG is cleared
      sampleFinish = 1;
      __bic_SR_register_on_exit(LPM0_bits); // Exit active CPU
      break;
    case ADC12IV_ADC12IFG1:   break;        // Vector 14:  ADC12MEM1
    case ADC12IV_ADC12IFG2:   break;        // Vector 16:  ADC12MEM2
    case ADC12IV_ADC12IFG3:   break;        // Vector 18:  ADC12MEM3
    case ADC12IV_ADC12IFG4:   break;        // Vector 20:  ADC12MEM4
    case ADC12IV_ADC12IFG5:   break;        // Vector 22:  ADC12MEM5
    case ADC12IV_ADC12IFG6:   break;        // Vector 24:  ADC12MEM6
    case ADC12IV_ADC12IFG7:   break;        // Vector 26:  ADC12MEM7
    case ADC12IV_ADC12IFG8:   break;        // Vector 28:  ADC12MEM8
    case ADC12IV_ADC12IFG9:   break;        // Vector 30:  ADC12MEM9
    case ADC12IV_ADC12IFG10:  break;        // Vector 32:  ADC12MEM10
    case ADC12IV_ADC12IFG11:  break;        // Vector 34:  ADC12MEM11
    case ADC12IV_ADC12IFG12:  break;        // Vector 36:  ADC12MEM12
    case ADC12IV_ADC12IFG13:  break;        // Vector 38:  ADC12MEM13
    case ADC12IV_ADC12IFG14:  break;        // Vector 40:  ADC12MEM14
    case ADC12IV_ADC12IFG15:  break;        // Vector 42:  ADC12MEM15
    case ADC12IV_ADC12IFG16:  break;        // Vector 44:  ADC12MEM16
    case ADC12IV_ADC12IFG17:  break;        // Vector 46:  ADC12MEM17
    case ADC12IV_ADC12IFG18:  break;        // Vector 48:  ADC12MEM18
    case ADC12IV_ADC12IFG19:  break;        // Vector 50:  ADC12MEM19
    case ADC12IV_ADC12IFG20:  break;        // Vector 52:  ADC12MEM20
    case ADC12IV_ADC12IFG21:  break;        // Vector 54:  ADC12MEM21
    case ADC12IV_ADC12IFG22:  break;        // Vector 56:  ADC12MEM22
    case ADC12IV_ADC12IFG23:  break;        // Vector 58:  ADC12MEM23
    case ADC12IV_ADC12IFG24:  break;        // Vector 60:  ADC12MEM24
    case ADC12IV_ADC12IFG25:  break;        // Vector 62:  ADC12MEM25
    case ADC12IV_ADC12IFG26:  break;        // Vector 64:  ADC12MEM26
    case ADC12IV_ADC12IFG27:  break;        // Vector 66:  ADC12MEM27
    case ADC12IV_ADC12IFG28:  break;        // Vector 68:  ADC12MEM28
    case ADC12IV_ADC12IFG29:  break;        // Vector 70:  ADC12MEM29
    case ADC12IV_ADC12IFG30:  break;        // Vector 72:  ADC12MEM30
    case ADC12IV_ADC12IFG31:  break;        // Vector 74:  ADC12MEM31
    case ADC12IV_ADC12RDYIFG: break;        // Vector 76:  ADC12RDY
    default: break;
  }
}
