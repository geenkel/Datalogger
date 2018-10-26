#include "i2c.h"

extern unsigned char drdyFlag;
unsigned char *PTxData;
volatile unsigned char TXByteCtr;
unsigned char *PRxData;
volatile unsigned char RXByteCtr=0;
unsigned char nbytes=1;
unsigned char *RxBufferi2c;
char result[120];




void i2cIni()
{


    // Configure GPIO
      P1SEL1 |= BIT6 | BIT7;                    // I2C pins

      // Disable the GPIO power-on default high-impedance mode to activate
      // previously configured port settings
      PM5CTL0 &= ~LOCKLPM5;

      // Configure USCI_B0 for I2C mode
      UCB0CTLW0 = UCSWRST;                      // put eUSCI_B in reset state
      UCB0CTLW0 |= UCMODE_3 | UCMST | UCSSEL__SMCLK; // I2C master mode, SMCLK
      UCB0BRW = 0x8;                            // baudrate = SMCLK / 8
      UCB0CTLW0 &= ~UCSWRST;                    // clear reset register
      UCB0IE |= UCTXIE0 | UCNACKIE | UCRXIE;             // transmit and NACK interrupt enable


//      unsigned char buff[10];
//      PRxData = buff;
//      unsigned char txBuff[2] = {0b11100011, 0x10};
//      PTxData = txBuff;
//      while(1)
//      {
//        __delay_cycles(100000);                   // Delay between transmissions
//        UCB0I2CSA = SHT20_ADDR;   // configure slave address
//        TXByteCtr = 1;                          // Load TX byte counter
//        while (UCB0CTLW0 & UCTXSTP);            // Ensure stop condition got sent
//
//        UCB0CTLW0 |= UCTR | UCTXSTT;            // I2C TX, start condition
//
//        __bis_SR_register(LPM0_bits | GIE);     // Enter LPM0 w/ interrupts
//        while(1);                                        // Remain in LPM0 until all data
//                                                // is TX'
//      }

}
//***************************************************************************************************
//***************************************************************************************************
//***************************************************************************************************
void writeRegister(unsigned char address, unsigned char *data, unsigned char n)
{

    unsigned char i;
    UCB0I2CSA = SHT20_ADDR;
    UCB0CTL1 |= UCTR | UCTXSTT | UCSYNC;
    LPM0;
    UCB0TXBUF=address;
    LPM0;
    for(i=0;i<n;i++){
        UCB0TXBUF=data[i];
        LPM0;
    }
    UCB0CTL1 |= UCTXSTP;
    while (UCB0CTL1 & UCTXSTP);             // Se asegura que STOPBIT se mande
}
//***************************************************************************************************
//***************************************************************************************************
//***************************************************************************************************

unsigned char finish = 0;
unsigned char RxIndex = 0;
void readRegister(unsigned char address, unsigned char *data, unsigned char n)
{

    PRxData = data;
    PTxData = &address;
    UCB0I2CSA = SHT20_ADDR;   // configure slave address
    TXByteCtr = 1;                          // Load TX byte counter
    RXByteCtr = n;
    RxIndex = 0;
    while (UCB0CTLW0 & UCTXSTP);            // Ensure stop condition got sent
    __disable_interrupt();
    finish = 0;
    UCB0CTLW0 |= UCTR | UCTXSTT;            // I2C TX, start condition
    while(!finish)
    {
        __bis_SR_register(LPM0_bits | GIE);     // Enter LPM0 w/ interrupts
        __disable_interrupt();
    }
    __enable_interrupt();
}




#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) USCI_B0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
  {
    case USCI_NONE:          break;         // Vector 0: No interrupts
    case USCI_I2C_UCALIFG:   break;         // Vector 2: ALIFG
    case USCI_I2C_UCNACKIFG:                // Vector 4: NACKIFG
      UCB0CTLW0 |= UCTXSTT;                 // resend start if NACK
      break;
    case USCI_I2C_UCSTTIFG:  break;         // Vector 6: STTIFG
    case USCI_I2C_UCSTPIFG:  break;         // Vector 8: STPIFG
    case USCI_I2C_UCRXIFG3:  break;         // Vector 10: RXIFG3
    case USCI_I2C_UCTXIFG3:  break;         // Vector 12: TXIFG3
    case USCI_I2C_UCRXIFG2:  break;         // Vector 14: RXIFG2
    case USCI_I2C_UCTXIFG2:  break;         // Vector 16: TXIFG2
    case USCI_I2C_UCRXIFG1:  break;         // Vector 18: RXIFG1
    case USCI_I2C_UCTXIFG1:  break;         // Vector 20: TXIFG1
    case USCI_I2C_UCRXIFG0:
        PRxData[RxIndex] = UCB0RXBUF;
        RxIndex++;
        RXByteCtr--;
        if(RXByteCtr == 1)
        {
            UCB0CTLW0 |= UCTXSTP;
        }
        if(RXByteCtr == 0)
        {
            finish = 1;
            __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
        }
        break;         // Vector 22: RXIFG0
    case USCI_I2C_UCTXIFG0:                 // Vector 24: TXIFG0
      if (TXByteCtr)                        // Check TX byte counter
      {
        UCB0TXBUF = PTxData[0];      // Load TX buffer
        TXByteCtr--;                        // Decrement TX byte counter
      }
      else
      {
          UCB0CTLW0 &= ~UCTR;
          UCB0CTLW0 |=  UCTXSTT;
          UCB0IFG &= ~UCTXIFG;
//        UCB0CTLW0 |= UCTXSTP;               // I2C stop condition
//        UCB0IFG &= ~UCTXIFG;                // Clear USCI_B0 TX int flag
//        __bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
      }
      break;
    default: break;
  }
}



























//unsigned int option;
//
//#pragma vector = USCI_B0_VECTOR
//__interrupt void USCI_B0_ISR(void)
//{
//    option = __even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG);
//  switch(option)
//  {
//  case  USCI_I2C_UCNACKIFG: // Vector  4: NACKIFG
//      finish = 2;
//      break;
//                         // Vector  8: STPIFG
//  case USCI_I2C_UCRXIFG0:                            // Vector 10: RXIFG
//      PRxData[RXByteCtr] = UCB0RXBUF;
//      RXByteCtr++;
//      if(RXByteCtr==1)
//      {
//          UCB0CTL1 |= UCTXSTP;
//          finish = 1;
//          __bic_SR_register_on_exit(LPM0_bits);
//      }
//
//      break;
//  case USCI_I2C_UCTXIFG1: // Vector 12: TXIFG
//      UCB0IFG &= ~UCTXIFG;
//      if(start_send)
//      {
//          //Repeated start
//          UCB0CTL1 &= ~UCTR;
//          UCB0CTL1 |= UCTXSTT;
//      }
//      else
//      {
//          start_send = 1;
//          UCB0TXBUF = cmd;
//          start_send = 1;
//      }
//    break;
//  default: break;
//  }
//}
//
