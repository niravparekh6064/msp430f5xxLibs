/*
 * TW_GsmModuleCtrl.h
 *
 *  Created on: Jun 29, 2019
 *      Author: Turjasu
 */

#ifndef TW_GSMMODULECTRL_H_
#define TW_GSMMODULECTRL_H_

#include <stdint.h>

/*
 *  Definitions for Port and Pins
 */
#define                 SystemFreq                      12000000.00000 // This MUST BE DEFINED PROPERLY FOR COMMUNICATION
#define                 _M95_                           true
#define                 _SIM800_                        false
#define                 _M66_                           false

#define                 TurnOnImmediately               1
#define                 TurnOnAfter1Sec                 2
#define                 TurnOff                         3
#define                 Restart                         4


#define                 _M95_PORT_DIR                    P3DIR
#define                 _M95_PORT_OUT                    P3OUT
#define                 _M95_ON_OFF_PIN                  BIT6
// USCIA0 comm. bits
#define                 _GsmMdm_PORT_DIR                 P3DIR
#define                 _GsmMdm_PORT_SEL                 P3SEL
#define                 _GsmMdm_UART_RX                  BIT5
#define                 _GsmMdm_UART_TX                  BIT4
#define                 _IntrptBased                     1
#define                 _PollingBased                    2

/*
 * Enums
 */
enum ReplyCodes{
    SUCCESS,
    FAIL,
    _M95_INIT_SUCCESS,
    _M95_HW_FLT,
    _M95_SIM_FAIL,
    _M95_HW_RETRY_TIMEOUT,
    UNKNOWN,
};

typedef enum    { //POWER ON -> UNECHO SHRT RESPNSE -> SET NTWRK TIME SYNC -> CHK NTWRK REG -> CHECK NTWRK PWR -> CHK MODULE SUPPLY VOLTAGE
                          //*   --> CHK GPRS -->EXIT
        _M95_NOP,
        _M95_STARTUP_SUCCESS,                                   //0
        _M95_AT_FAILURE,
        _M95_STARTUP_CMD_FAILURE,                       //1
        _M95_ECHO_SHRTRSP_OFF,                          //2
        _M95_ECHO_SHRTRSP_FAILURE,                          //3
        _M95_ICCID_FAILURE,
        _M95_ICCID_READ,
        _M95_NTWRK_TIME_SYNC_OK,
        _M95_NTWRK_TIME_SYNC_FAIL,
        _M95_NTWRK_TIME_RETRIEVE_PARSE_OK,
        _M95_NTWRK_TIME_RETRIEVE_PARSE_FAIL,
        _M95_NTWRK_REG_OK,
        _M95_NTWRK_REG_FAIL,
        _M95_NTWRK_RSSI_RETRIEVE_OK,
        _M95_NTWRK_RSSI_RETRIEVE_FAIL,
        _M95_OPER_NAME_FETCH_SUCCESS,
        _M95_OPER_NAME_FETCH_FAIL,
        _M95_CBC_RETIEVE_OK,
        _M95_CBC_RETIEVE_FAIL,
        _M95_MAKING_READY,
        _M95_READY,
        _M95_INIT_INCOMPLETE_OR_FAIL,
        _M95_START_CGATT_SUCCESS,
        _M95_START_CGATT_FAIL,
        _M95_STOP_CGATT_SUCCESS,
        _M95_STOP_CGATT_FAIL,
        _M95_HTTP_GET_START,
        _M95_HTTP_GET_START_FAIL,
        _M95_HTTP_URL_CONNECT_SUCCESS,
        _M95_HTTP_URL_CONNECT_FAIL,
        _M95_HTTP_GET_DATA_READ_FRM_BUFF_SUCCESS,
        _M95_HTTP_GET_DATA_READ_FRM_BUFF_FAIL,
        _M95_HTTP_SET_START,
        _M95_HTTP_SET_START_FAIL,
        _M95_HTTP_3818_OPEN_PORT_FAIL,

}MdmStateMachines;

typedef     enum    {
        _Modem_Full_Init,
        _Modem_Start_GPRS,
        _Modem_Stop_GPRS,
        _Modem_GET_Request,
        _Modem_SET_Request,
        _Modem_POST_Request,
}MdmOperCommand;


/*
 *  Source functions required for the control and communications
 */

extern                  void                Modem_PinSetup(void);
extern                  uint8_t             Modem_ON_OFF(uint8_t);
extern                  uint8_t             ConfigureMdmUART(float baudrate, uint8_t interrupt_polling);
extern                  void                SendDataToMdm(const uint8_t* data);
extern                  void                SendCharToMdm(unsigned char);
extern                  void                DeEchoShrtRsp(void);
extern                  MdmStateMachines    MdmInitAndDiag(void);       // Implements StateMachine for the init and diag for M95
extern                  MdmStateMachines    MdmMakeReady(MdmOperCommand);
extern                  void                ParseCallback(void (*ptr)());
extern                  int                ReadMdmRSSI(void);
extern                  int                 ReadIPAddr(void);
extern                  void                ClrMdmBuff(void);



/*
 *  Variables
 */


extern      volatile            unsigned char                                   _MdmBuffer[2048],
                                                                       _MdmStatus[80],        // Global MBuffer to store the modem replies. Define in your required C/CPP files/classes
                                                                       _MdmHTTPBuff[2048];     //HTTP Get Buffer
extern   volatile       unsigned char                                   _MdmAPN[];           // APN name
extern                  int         StrResult, _MdmInitSts;

///*
// *  HTTP GET / POST URL STRINGS
// */
//
//extern const uint8_t   _MdmGETurl[];
//extern const uint8_t   _MdmSETETurl[];

#endif /* TW_GSMMODULECTRL_H_ */
