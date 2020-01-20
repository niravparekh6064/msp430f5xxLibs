/*
 * TW_GenericUART_UCA3.h
 *
 *  Created on: Nov 18, 2019
 *      Author: Turjasu
 */

#ifndef TW_GENERICUART_UCA3_H_
#define TW_GENERICUART_UCA3_H_

#include <msp430.h>
#include <stdint.h>


/*
 *  Definitions for Port and Pins
 */
#define                 SystemFreq                      12000000.00000 // This MUST BE DEFINED PROPERLY FOR COMMUNICATION


#define                 _UCA3_PORT_DIR                    P10DIR
#define                 _UCA3_PORT_OUT                    P10OUT

// USCIA3 comm. bits
#define                 _UCA3_PORT_DIR                 P10DIR
#define                 _UCA3_PORT_SEL                 P10SEL
#define                 _UCA3_UART_RX                  BIT5
#define                 _UCA3_UART_TX                  BIT4
#define                 _IntrptBased                     1
#define                 _PollingBased                    2

/*
 * Enums
 */
//enum ReplyCodes{
//    SUCCESS,
//    FAIL,
//    _M95_INIT_SUCCESS,
//    _M95_HW_FLT,
//    _M95_SIM_FAIL,
//    _M95_HW_RETRY_TIMEOUT,
//    UNKNOWN,
//};




/*
 *  Source functions required for the control and communications
 */

extern                  uint8_t             Configure_UCA3_UART(float baudrate, uint8_t interrupt_polling);
extern                  void                SendDataToUCA3(const uint8_t* data);
extern                  void                SendCharToUCA3(unsigned char data);
extern                  void                ClrUCA3Buff(void);


/*
 *  Variables
 */

extern                  unsigned char                                   _UCA3Buff[512];


#endif /* TW_GENERICUART_UCA3_H_ */
