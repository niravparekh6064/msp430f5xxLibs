/*
 * TW_timer.c
 *
 *  Created on: Sep 1, 2019
 *      Author: Turjasu
 */

#include "TimerAx/TW_timer.h"
#include "BranonSpecific/BranonSpecific.h"
#include <msp430.h>

volatile    uint16_t    counterVal_T1A0 = 60000 ;
volatile    uint32_t    counterRollover_T1A0 = RolloverValue_T1A0_300Sec,        //iot
                        counterRollover2_T1A0 = RolloverValue_T1A0_RSSIupdateDelay ,      // rssi
                        counterRollover3_T1A0 = RolloverValue_T1A0_7Sec ,       // Loads
                        counterRollover4_T1A0 = RolloverValue_T1A0_1Min;       // for Time display


int ConfigureTimer0A5(uint32_t countTimer)
{

  TA0CCTL0 = CCIE;                          // CCR0 interrupt enabled
  TA0CCTL1 = CCIE;                          // CCR0 interrupt enabled
  TA0CCTL2 = CCIE;                          // CCR0 interrupt enabled
  TA0CCR0 = countTimer;//50000;
  TA0CCR1 = countTimer;
  TA0CCR2 = countTimer;
  TA0CTL = TASSEL_2 + MC_2 + TACLR;         // SMCLK, contmode, clear TAR

  //__bis_SR_register(GIE);       // Enter LPM0, enable interrupts <= HANDLED IN MAIN
//  __no_operation();                         // For debugger

  return 0;
}

int ConfigureTimer1A3(uint16_t countTimer, _enableTimerIntrpts RequiredIntrpts)
{

  TA1CCTL0 = CCIE;                          // CCR0 interrupt enabled
//  TA1CCTL1 = CCIE;                          // CCR0 interrupt enabled
//  TA1CCTL2 = CCIE;                          // CCR0 interrupt enabled
  TA1CCR0 = 20000;//50000;
//  TA1CCR1 = countTimer;
//  TA1CCR2 = countTimer;
  TA1CTL = TASSEL_2 + MC_2  + TACLR;         // SMCLK, contmode, clear TAR

  //__bis_SR_register(GIE);       // Enter LPM0, enable interrupts <= HANDLED IN MAIN
//  __no_operation();                         // For debugger

  return 0;
}

int ConfigureTimer0B7(unsigned int countTimer, _enableTimerIntrpts RequiredIntrpts)
{

  TB0CCTL0 = CCIE;                          // CCR0 interrupt enabled
  TB0CCR0 = countTimer;//50000;
  TB0CCTL1 |= CCIE;
  TB0CCR1 = countTimer;
  TB0CTL = TBSSEL_2 + MC_2 + ID_1 + TBCLR;

  return 0;
}


int ConfigurePWM_Timer1A3(uint8_t* VL1Pwr, uint8_t* VL2Pwr, int PWMPeriod, _outMODE VL1, _outMODE VL2)
{

    //**************************REFERENCE*************************************
    //  MSP430F543xA Demo - Timer_A3, PWM TA1.1-2, Up/Down Mode, DCO SMCLK
    //
    //  Description: This program generates two PWM outputs on P2.2,3 using
    //  Timer1_A configured for up/down mode. The value in CCR0, 128, defines the
    //  PWM period/2 and the values in CCR1 and CCR2 the PWM duty cycles. Using
    //  ~1.045MHz SMCLK as TACLK, the timer period is ~233us with a 75% duty cycle
    //  on P2.2 and 25% on P2.3.
    //  SMCLK = MCLK = TACLK = default DCO ~1.045MHz.
    //
    //                MSP430F5438A
    //            -------------------
    //        /|\|                   |
    //         | |                   |
    //         --|RST                |
    //           |                   |
    //           |         P2.2/TA1.1|--> CCR1 - 75% PWM (value = 96)
    //           |         P2.3/TA1.2|--> CCR2 - 25% PWM (value = 32)
    //
    //   M. Morales
    //   Texas Instruments Inc.
    //   June 2009
    //   Built with CCE Version: 3.2.2 and IAR Embedded Workbench Version: 4.11B
    //******************************************************************************

//    P2DIR |= 0x06;                            // P2.2 and P2.3 output
//    P2SEL |= 0x06;                            // P2.2 and P2.3 options select
    TA1CCTL0 = CCIE ;
    TA1CCR0 = 500;                            // PWM Period/2
//    TA1CCTL1 = OUTMOD_7;                      // CCR1 toggle/set
//    TA1CCR1 = 30000;                             // CCR1 PWM duty cycle
//    TA1CCTL2 = OUTMOD_7;                      // CCR1 toggle/set
//    TA1CCR2 = 40000;                             // CCR1 PWM duty cycle

    TA1CTL = TASSEL_2 + MC_2 + ID_1 + TACLR ;         // SMCLK, up-down mode, clear TAR
    return 0;
}



//**************************REFERENCE***************************************
//   MSP430F543xA Demo - Timer_B, PWM TB1-6, Up Mode, DCO SMCLK
//
//   Description: This program generates six PWM outputs on P2/P3 using
//   Timer_B configured for up mode. The value in CCR0, 512-1, defines the PWM
//   period and the values in CCR1-6 the PWM duty cycles. Using ~1048kHz SMCLK
//   as TBCLK, the timer period is ~488us.
//   ACLK = 32kHz, SMCLK = MCLK = TBCLK = default DCO ~1048kHz.
//
//                MSP430F5438A
//             -----------------
//         /|\|              XIN|-
//          | |                 |  32kHz
//          --|RST          XOUT|-
//            |                 |
//            |         P4.1/TB1|--> CCR1 - 75% PWM
//            |         P4.2/TB2|--> CCR2 - 25% PWM
//            |         P4.3/TB3|--> CCR3 - 12.5% PWM
//            |         P4.4/TB4|--> CCR4 - 6.26% PWM
//            |         P4.5/TB5|--> CCR5 - 3.13% PWM
//            |         P4.6/TB6|--> CCR6 - 1.566% PWM
//
//   M. Morales
//   Texas Instruments Inc.
//   June 2009
//   Built with CCE Version: 3.2.2 and IAR Embedded Workbench Version: 4.11B
//******************************************************************************



int ConfigurePWM_Timer0B7(uint8_t* VL1Pwr, uint8_t* VL2Pwr, int PWMPeriod, _outMODE VL1, _outMODE VL2)
{


  P4SEL |= 0x7E;                            // P4 option select
  P4DIR |= 0x7E;                            // P4 outputs

  TBCCR0 = 512-1;                           // PWM Period
  TBCCTL1 = OUTMOD_7;                       // CCR1 reset/set
  TBCCR1 = 383;                             // CCR1 PWM Duty Cycle
  TBCCTL2 = OUTMOD_7;                       // CCR2 reset/set
  TBCCR2 = 128;                             // CCR2 PWM duty cycle
  TBCCTL3 = OUTMOD_7;                       // CCR3 reset/set
  TBCCR3 = 64;                              // CCR3 PWM duty cycle
  TBCCTL4 = OUTMOD_7;                       // CCR4 reset/set
  TBCCR4 = 32;                              // CCR4 PWM duty cycle
  TBCCTL5 = OUTMOD_7;                       // CCR5 reset/set
  TBCCR5 = 16;                              // CCR5 PWM duty cycle
  TBCCTL6 = OUTMOD_7;                       // CCR6 reset/set
  TBCCR6 = 8;                               // CCR6 PWM duty cycle
  TBCTL = TBSSEL_2 + MC_1 + TBCLR;          // SMCLK, upmode, clear TBR

  return 0;

}


/*
 * callback function to different operations at several timer interrupts
 */
void ParseCallbackFromT1A0(void (*ptr)())
{
    (*ptr) (); // callback to A
}







//////////////////////////////////////////////////////////////////////TIMER 0 INTERRUPT VECTORS //////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * TIMER A0 INTERRUPT ISR  TIMER1_A1_VECTOR FOR CCR0
 */
// Timer A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) TIMER0_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
#if RollOversNeeded_T1A0
    counterRollover_T1A0 --;
    if(counterRollover_T1A0 == 0)
    {
        counterRollover_T1A0 = RolloverValue_T1A0_300Sec;
//        void (*ptr)() = &branRSSIUpdate;

//        ParseCallbackFromT1A0(ptr); // Callback to the operation
        _BranonMasterStatus |=  BIT2;

    }

#endif

    TA0CCR0 += counterVal_T1A0;                         // Add Offset to CCR0
}

/*
 * TIMER0_A1_VECTOR INTERRUPT ISR FOR CCR1 and CCR2
 */
// Timer A1 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A1_VECTOR))) TIMER0_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
switch (TA0IV)
{
    case TA0IV_TACCR1:

        counterRollover2_T1A0 --;
        counterRollover4_T1A0 --;

        if(counterRollover2_T1A0 == 0)
        {
            counterRollover2_T1A0 = RolloverValue_T1A0_RSSIupdateDelay;
            _BranonMasterStatus |=  BIT3;       // APPEND RSSI REQUEST


        }

        if(counterRollover4_T1A0 == 0)
        {
            counterRollover4_T1A0 = RolloverValue_T1A0_1Min;
            _BranonMasterStatus |=  BIT6;       // APPEND CLOCK UPDT REQUEST


        }






        TA0CCR1 += counterVal_T1A0;

        break;

    case TA0IV_TACCR2:
        counterRollover3_T1A0 --;
        if(counterRollover3_T1A0 == 0)
        {
            counterRollover3_T1A0 = RolloverValue_T1A0_7Sec;
    //        void (*ptr)() = &branRSSIUpdate;

    //        ParseCallbackFromT1A0(ptr); // Callback to the operation
            _BranonMasterStatus |=  BIT4;       // APPEND RSSI REQUEST


        }
        TA0CCR2 += counterVal_T1A0;
        break;
    default:
        break;
}

}



///////////////////////////////////////////////////////////////////TIMER 1 INTERRUPT VECTORS ///////////////////////////////////////////////////////////////////////////////

/*
 * TIMER A0 INTERRUPT ISR  TIMER1_A1_VECTOR FOR CCR0
 */
// Timer A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER1_A0_VECTOR))) TIMER1_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    P2OUT |= BIT1;
    TA1CTL &= ~0x0030;


}



/*
 * TIMER 1 _Ax_VECTOR INTERRUPT ISR FOR CCR1 to  CCR2
 */
// Timer A1 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER1_A1_VECTOR))) TIMER1_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
switch (__even_in_range(TA1IV, 12))
{
    case 2:
//        P2OUT |= BIT1;
//        TA1CTL &= ~0x0030;
        TA1CCR1 += 10000;
        break;

    case 4:
        _delay_cycles(1);
        break;
    default:
        break;
}

}

///////////////////////////////////////////////////////////////////TIMER 0 B INTERRUPT VECTORS ///////////////////////////////////////////////////////////////////////////////

/*
 * TIMER B0 INTERRUPT ISR  TIMER1_B7_VECTOR FOR CCR0
 */
// Timer B0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_B0_VECTOR
__interrupt void TIMER0_B0_VECTOR_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_B0_VECTOR))) TIMER0_B0_VECTOR_ISR (void)
#else
#error Compiler not supported!
#endif
{
//    P1IFG &= ~BIT0;
//    P2OUT |=  BIT1;
    TB0CCR0 += _VL1PowerVal;
//    TB0CTL &= ~0x0030;


}

// Timer A1 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_B1_VECTOR
__interrupt void TIMER0_B1_VECTOR_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_B1_VECTOR))) TIMER0_B1_VECTOR_ISR (void)
#else
#error Compiler not supported!
#endif
{
switch (__even_in_range(TB0IV, 12))
{
    case 2:
        P2OUT |= BIT1;
        StrtTmrVal[1] = TB0R;
        break;

    default:
        break;
}

}
