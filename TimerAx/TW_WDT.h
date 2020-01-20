/*
 * TW_WDT.h
 *
 *  Created on: Nov 16, 2019
 *      Author: Turjasu
 */

#ifndef TIMERAX_TW_WDT_H_
#define TIMERAX_TW_WDT_H_

# include <stdint.h>
# include <stdbool.h>

typedef     enum
{
        _wdt_IntervalMode,
        _wdt_TimerMode,
        _wdt_Clk_SMCLK,
        _wdt_Clk_ACLK,
        _wdt_ClkDiv_2_31,
        _wdt_ClkDiv_2_27,
        _wdt_ClkDiv_2_23,
        _wdt_ClkDiv_2_19,
        _wdt_ClkDiv_2_15,
        _wdt_ClkDiv_2_13,
        _wdt_ClkDiv_2_9,
        _wdt_ClkDiv_2_6,


}           _wdt_Config_Run;


extern      int         ConfigureWDT(_wdt_Config_Run , _wdt_Config_Run , _wdt_Config_Run ); // Configures the WDT in the MSP430 to run accordingly
extern      void        ClearWDT(void); // Clear the WDT Counter

#endif /* TIMERAX_TW_WDT_H_ */
