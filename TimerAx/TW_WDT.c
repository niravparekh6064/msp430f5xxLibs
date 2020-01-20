/*
 * TW_WDT.c
 *
 *  Created on: Nov 16, 2019
 *      Author: Turjasu
 */

# include "TimerAx/TW_WDT.h"
# include "GenericUART/TW_GenericUART_UCA3.h"
# include <msp430.h>


int         ConfigureWDT(_wdt_Config_Run Mode, _wdt_Config_Run ClkSrc, _wdt_Config_Run IntervalSel)
{
    SendDataToUCA3("__Entering ConfigureWDT__\r");
    if(Mode == _wdt_IntervalMode)
    {
        if(ClkSrc == _wdt_Clk_ACLK)
        {
            switch (IntervalSel)
            {
                case _wdt_ClkDiv_2_31:
                    WDTCTL = WDTPW + WDTCNTCL + WDTIS_0 + WDTSSEL1;
                    break;
                case _wdt_ClkDiv_2_27:
                    WDTCTL = WDTPW + WDTCNTCL + WDTIS_1 + WDTSSEL0;
                    break;
                case _wdt_ClkDiv_2_23:
                    WDTCTL = WDTPW + WDTCNTCL + WDTIS_2 + WDTSSEL0;
                    break;
                case _wdt_ClkDiv_2_19:
                    WDTCTL = WDTPW + WDTCNTCL + WDTIS_3 + WDTSSEL0;
                    break;
                case _wdt_ClkDiv_2_15:
                    WDTCTL = WDTPW + WDTCNTCL + WDTIS_4 + WDTSSEL0;
                    break;
                case _wdt_ClkDiv_2_13:
                    WDTCTL = WDTPW + WDTCNTCL + WDTIS_5 + WDTSSEL0;
                    break;
                case _wdt_ClkDiv_2_9:
                    WDTCTL = WDTPW + WDTCNTCL + WDTIS_6 + WDTSSEL0;
                    break;
                case _wdt_ClkDiv_2_6:
                    WDTCTL = WDTPW + WDTCNTCL + WDTIS_7 + WDTSSEL0;
                    break;
                default:
                    break;
            }
        }
        else if(ClkSrc == _wdt_Clk_SMCLK)
        {
            switch (IntervalSel)
            {
            case _wdt_ClkDiv_2_31:
                WDTCTL = WDTPW + WDTCNTCL + WDTIS_0 ;
                break;
            case _wdt_ClkDiv_2_27:
                WDTCTL = WDTPW + WDTCNTCL + WDTIS_1 ;
                break;
            case _wdt_ClkDiv_2_23:
                WDTCTL = WDTPW + WDTCNTCL + WDTIS_2 ;
                break;
            case _wdt_ClkDiv_2_19:
                WDTCTL = WDTPW + WDTCNTCL + WDTIS_3 ;
                break;
            case _wdt_ClkDiv_2_15:
                WDTCTL = WDTPW + WDTCNTCL + WDTIS_4 ;
                break;
            case _wdt_ClkDiv_2_13:
                WDTCTL = WDTPW + WDTCNTCL + WDTIS_5 ;
                break;
            case _wdt_ClkDiv_2_9:
                WDTCTL = WDTPW + WDTCNTCL + WDTIS_6 ;
                break;
            case _wdt_ClkDiv_2_6:
                WDTCTL = WDTPW + WDTCNTCL + WDTIS_7 ;
                break;
            default:
                break;
            }
        }
        else
        {

        }
    }
    else if(Mode == _wdt_TimerMode)
    {

    }
    else
    {

    }

    return 0;
}

/*
 *  Clear the WDT timer
 */
void        ClearWDT(void)
{
    WDTCTL = WDTPW + WDTCNTCL ;
}
