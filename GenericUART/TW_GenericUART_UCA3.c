/*
 * TW_GenericUART_UCA3.c
 *
 *  Created on: Nov 18, 2019
 *      Author: Turjasu
 */

# include "GenericUART/TW_GenericUART_UCA3.h"



int                                             _UCA3BuffCnt = 0;
unsigned char                                   _UCA3Buff[512];
/*
 * baudrate will be 9600 to any suitable value.
 * Oversampling is off. please read datasheet before pushing values
 */
uint8_t Configure_UCA3_UART(float baudrate, uint8_t interrupt_polling)
{
    volatile float    val, val3;
    uint16_t  val2;
    volatile int val4 = 0;
     _UCA3_PORT_SEL = (_UCA3_UART_RX + _UCA3_UART_TX);                             // P3.4,5 = USCI_A3 TXD/RXD
     UCA3CTL1 |= UCSWRST;                      // **Put state machine in reset**
     UCA3CTL1 |= UCSSEL_2;                     // SMCLK
     // Calculation of the baudrate values
     val = (SystemFreq/baudrate);
     val2 = (uint16_t)(val);
     UCA3BR0 = val2 % 256;                              // (see User's Guide)
     UCA3BR1 = val2 / 256;                              //
     val3 = (val - val2)*8;
     val4 = (int)(val3);
     switch (val4)
     {
        case 0:
            UCA3MCTL |= UCBRS_0 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
            break;
        case 1:
            UCA3MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
            break;
        case 2:
            UCA3MCTL |= UCBRS_2 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
            break;
        case 3:
            UCA3MCTL |= UCBRS_3 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
            break;
        case 4:
            UCA3MCTL |= UCBRS_4 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
            break;
        case 5:
            UCA3MCTL |= UCBRS_5 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
            break;
        case 6:
            UCA3MCTL |= UCBRS_6 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
            break;
        case 7:
            UCA3MCTL |= UCBRS_7 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
            break;
        default:
            break;
    }


     UCA3CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
     switch (interrupt_polling)
     {
        case 1:
            UCA3IE |= UCRXIE;                         // Enable USCI_A3 RX interrupt
            __bis_SR_register(GIE);       // Enter LPM0, interrupts enabled
            break;
        case 2:
             UCA3IE &= ~(UCRXIE + UCTXIE);                         // Enable USCI_A3 RX interrupt
             break;
        default:
            break;
    }

     return 0;
}

/*
 *  Send single character data to modem
 */
void      SendCharToUCA3(unsigned char data)
{
    while (!(UCA3IFG&UCTXIFG));             // USCI_A3 TX buffer ready?
    UCA3TXBUF = data;                  // TX -> RXed character
//    _delay_cycles(SystemFreq/100);
}

/*
 *  Send String of data to modem by calling this Fn.
 */
void     SendDataToUCA3(const uint8_t* data)
{

    while(*data)
    {
        while (!(UCA3IFG&UCTXIFG));             // USCI_A3 TX buffer ready?
        UCA3TXBUF = *data;                  // TX -> RXed character
        data++;
    }

}

/*
 *  Clear the UCA3 Buffer
 */
void    ClrUCA3Buff(void)
{
    if(_UCA3BuffCnt>0)
        {
            unsigned int var;
            for (var = 0; var < _UCA3BuffCnt; ++var)
            {
                _UCA3Buff[var]=0;
            }
            _UCA3BuffCnt = 0;
        }
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *    Interrupt Vectors for the data acquired from modem
 */

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A3_VECTOR
__interrupt void USCI_A3_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A3_VECTOR))) USCI_A3_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA3IV,4))
  {
  case 0:break;                             // Vector 0 - no interrupt
  case 2:                                   // Vector 2 - RXIFG
      _UCA3Buff[_UCA3BuffCnt] = UCA3RXBUF;                  // TX -> RXed character
      _UCA3BuffCnt++;
      if(_UCA3BuffCnt == 513) _UCA3BuffCnt = 0;
    break;
  case 4:break;                             // Vector 4 - TXIFG
  default: break;
  }
}
