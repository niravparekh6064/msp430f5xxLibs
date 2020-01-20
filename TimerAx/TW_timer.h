/*
 * TW_timer.h
 *
 *  Created on: Sep 1, 2019
 *      Author: Turjasu
 */

#ifndef TIMERAX_TW_TIMER_H_
#define TIMERAX_TW_TIMER_H_

#include <stdint.h>
/*
 *  Conditional Compilation Params
 */

#define         RollOversNeeded_T1A0                1
#define         RolloverValue_T1A0_10Sec          23992    // Mention the number of rollovers needed, depending on the Xtal/ DCO freq.
#define         RolloverValue_T1A0_7Sec           1400      //loads
#define         RolloverValue_T1A0_RSSIupdateDelay          29911     //RSSI DELAY = 300 sec = 5 min
#define         RolloverValue_T1A0_300Sec         19999 // May be needed to change to 10250 to comply with the ToD, old value 31111
#define         RolloverValue_T1A0_1Min           2999      // 1 min interval for clock display, was 10989



/*
 *   Calibration, Timer Rollver counters and other constants
 */


extern          unsigned int    StrtTmrVal[30];

/*
 *  Enum Declarations
 */

typedef             enum            {
    _TIMER0_TA0CCR0_INTRPT,
    _TIMER0_TA0IFG_TA0IV_INTRPT,

}_enableTimerIntrpts;

typedef             enum            {
    _PWM_OUTMODE0,
    _PWM_OUTMODE1,
    _PWM_OUTMODE2,
    _PWM_OUTMODE3,
    _PWM_OUTMODE4,
    _PWM_OUTMODE5,
    _PWM_OUTMODE6,
    _PWM_OUTMODE7,

}_outMODE;

/*
 *  function declarations
 */

extern                  int                 ConfigureTimer0A5(uint32_t );
extern                  int                 ConfigureTimer1A3(uint16_t countTimer, _enableTimerIntrpts RequiredIntrpts);
extern                  int                 ConfigurePWM_Timer1A3(uint8_t* , uint8_t* , int , _outMODE , _outMODE );
extern                  int                 ConfigurePWM_Timer0B7(uint8_t* , uint8_t* , int , _outMODE , _outMODE );
extern                  int                 ConfigureTimer0B7(unsigned int countTimer, _enableTimerIntrpts RequiredIntrpts);
extern                  void                ParseCallbackFromT1A0(void (*ptr)());



#endif /* TIMERAX_TW_TIMER_H_ */
