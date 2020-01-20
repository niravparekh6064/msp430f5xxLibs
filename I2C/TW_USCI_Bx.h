/*
 * TW_USCI_Bx.h  V2.00
 *
 *  Created on: Dec 13, 2018
 *      Author: Turjasu
 *
 *      LOGS:
 *      2 APR 19 = ADDING THE uSD CARD CONFIGS. FOR UCB3 SPI AND P9.3 AS SD DTCT & P9.0 AS CARD CS'
 *      2 APR 19 = ADDED A FEW FUNCTION DECLARATIONS AND VARIABLES TO THE uSDC PART
 */

#ifndef TW_USCI_BX_H_
#define TW_USCI_BX_H_
#include <stdbool.h>
#include <stdint.h>
#include <msp430.h>
//========================RTC DEVICE CHOICE==========================
#define             _BQ32000_         true   // Appropiately select this from above to operate the RTC IC properly
#define             _DS1307_           false   // Appropiately select this from above to operate the RTC IC properly


/*
 *   Definitions for the System Frequency
 */

#define                                     Freq                                        12000000 // in Hz
/*
 * Enums
 */
typedef enum {
    _I2C_SUCCESS,
    _I2C_FAIL,
    _I2C_SLV_ACK_ERR,
    _I2C_POINTR_MEM_ALLOC_ERR,
    _I2C_24Cxx_CHIP_NOT_SUPPORTED,
    _I2C_UNKNOWN_ERR,
}I2CErrorCodes ;
//=========================I2C========================================
#define             SDA                                                 BIT1
#define             SCL                                                  BIT2

extern                void                                                ConfigureI2C(int );
extern                void                                                OutputToPCF8574(unsigned char , unsigned char );
extern                void                                                OutputToPCF8574MultiBytes(unsigned char ,int , unsigned char *,bool );
extern                unsigned char                                       ReadFromPCF8574(unsigned char );
extern                void                                                Erase24Cxx(unsigned char ,unsigned char ,unsigned short ,unsigned short );
extern                int                                                  WriteTo24CxxEEPROM(unsigned char ,unsigned char ,unsigned short ,unsigned char ) ;
extern                int                                                 WriteTo24CxxEEPROMMultiBytes(unsigned char ,unsigned char ,unsigned short ,int , unsigned char *);
extern                unsigned char                                        ReadFrom24CxxEEPROM(unsigned char ,unsigned char ,unsigned short );
extern                int                                                 ReadFrom24CxxEEPROMMultiBytes(unsigned char ,unsigned char ,unsigned short ,int , unsigned char *);
extern                int                                                   SetRTC(unsigned  char ,const unsigned char *,unsigned char);
extern                int                                                  ReadFromRTCMultiBytes(unsigned short ,int , unsigned char *);
extern                int                                                  WriteTo24CxxEEPROMMultiBytes_InterruptMode(unsigned char ,int ,unsigned short ,int , unsigned char *);
extern                int                                                  ReadFrom24CxxEEPROMMultiBytes_InterruptMode(unsigned char ,unsigned char ,unsigned short ,int , unsigned char *);

#endif /* TW_USCI_BX_H_ */
