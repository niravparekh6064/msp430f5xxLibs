/*
 * TW_ADC12.h
 *
 *  Created on: Sep 23, 2019
 *      Author: Turjasu
 */

#ifndef ADC12_TW_ADC12_H_
#define ADC12_TW_ADC12_H_

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Defines
 */

#define                                     CALADC12_15V_30C  *((unsigned int *)0x1A1A)   // Temperature Sensor Calibration-30 C
                                                      //See device datasheet for TLV table memory mapping
#define                                     CALADC12_15V_85C  *((unsigned int *)0x1A1C)   // Temperature Sensor Calibration-85 C

#define                                     Freq                                        12000000 // in Hz

/*
 *  Enums, Structs, Typedefs
 */

typedef     enum    {
            ADC12_TEMP_USE,
            ADC12_CHANNEL_USE
}           ADC12ConfigStates;

/*
 *  Variables
 */

extern          float               temperatureDegC;
extern          float               temperatureDegF;


/*
 * Functions
 */

extern          uint8_t             ConfigureADC12(ADC12ConfigStates);
extern          float               ConvertTemp(void);

#endif /* ADC12_TW_ADC12_H_ */
