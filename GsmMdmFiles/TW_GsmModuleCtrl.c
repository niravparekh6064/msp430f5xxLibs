/*
 * TW_GsmModuleCtrl.c
 *
 * The source code has been developed for Turjasuzworld Products. However, for limited conditions
 * this may be used freely for any communication or non-commercial products.
 *
 * select yourModule in the header file. Supported
 * modules are M95, SIM900, SIM800, M66
 *
 *  Created on: Jun 29, 2019
 *      Author: Turjasu
 */

#include "GsmMdmFiles/TW_GsmModuleCtrl.h"
#include "BranonSpecific/BranonSpecific.h"
#include "BranonSpecific/HomeScreen.h"
#include "GenericUART/TW_GenericUART_UCA3.h"
#include <msp430.h>
#include <stdint.h>
#include <string.h>

/*
 *  Variables
 */

volatile            unsigned char           _MdmBuffer[2048],
                                            _MdmStatus[80],
                                            _MdmHTTPBuff[2048];
volatile  unsigned char                    _MdmAPN[] = "internet******";
int                 StrResult, ReplyTimeout, _MdmInitSts;
volatile  static int         _MdmBuffCnt = 0,SETAPIlength=0;
char *temp_mdm;
volatile unsigned char log_data;


/*
 * AT COMMANDS & REPLIES FOR M95
 */
const          uint8_t         EchoRply[] = "ATE0\r\r\nOK\r\n";
const          uint8_t         ShrtRply[] = "0\r";
const          uint8_t         ICCIDRply[]= "+CCID: ";
const          uint8_t         CREGRply[]= "+CREG: ";
const          uint8_t         COPSRply[]= "+COPS: ";
const          uint8_t         QINISTATRply[] = "AT+QINISTAT\r\r\n+QINISTAT: 3";
const          uint8_t         CSQRply[]= "+CSQ: ";
const          uint8_t         QLTSRply[] = "+QLTS: ";
const          uint8_t         CBCRply[] = "+CBC: ";
const          uint8_t         CGATT_ON_Rply[] = "+CGATT: 1\r\n";
const          uint8_t         CGATT_OFF_Rply[] = "+CGATT: 0\r\n";
const          uint8_t         QIFGCNTRply[] = "0\r";
const          uint8_t         QICSGPRply[] = "0\r";
const          uint8_t         QHTTPURLRply[] = "CONNECT\r\n";
const          uint8_t         URLRply[] = "0\r";
const          uint8_t         QHTTPGETRply[] = "0\r";
const          uint8_t         QHTTPREADRply[] = "CONNECT\r\n";
const          uint8_t         QIDEACTRply[] = "0\r";

///*
// *  GET / POST URL
// */
//
//const uint8_t   _MdmGETurl[] = "http://www.turjasuzworld.in/Branon/api/srv2.php?dvid=D1255";
//const uint8_t   _MdmSETurl[] = "http://www.turjasuzworld.in/Branon/api/srv.php?dvid=D1255&oprt=AIRTEL00&temp_mdm=56&dvip=182.065.004.125&dvfw=00.00.00.01&dvnc=31&L1=1&L1T=10&L2=1&L2T=21&L3=1&L3T=14&L4=0&L4T=0&L5=0&L5T=0&L6=0&L6T=0&L7=0&L7T=0&L8=0&L8T=0&VL1=0&VL1T=0&VL1P=0&VL2=1&VL2T=12&VL2P=50&RTC=18-56-26-04-02-2019&RTCSet=0&FR=0";


/* Configure the ports ONLY FOR THE USCIA0
 * P3.4 = USCIA0TXD
 * P3.5 = USCIA0RXD
 * IF THE BOARD USES OTHER USCIAx MODULE, PLEASE MODIFY THE
 * INSTRUCTIONS ACCORDINGLY
 *
 * TW PRODUCTS WILL ALWAYS USE USCIA0 = P3.4 & 3.5 FOR MODEM
 * COMMUNICATIONS
 *
 */
void Modem_PinSetup(void)
{
    _M95_PORT_DIR |= _M95_ON_OFF_PIN;   // makes the port pin P3.6.
    _M95_PORT_OUT &= ~_M95_ON_OFF_PIN;   // makes the port pin P3.6.
}

/*
 * Turn the modem on/off control
 * mode = 1 = turn on immediately
 * mode = 2 = turn on after 1 second delay
 * mode = 3 = turn off
 * mode = 4 = restart (requires 5 second delay)
 *
 * returns a 0 if everything is done properly
 */

uint8_t Modem_ON_OFF(uint8_t mode)
{
        enum ReplyCodes status = SUCCESS;
        Modem_PinSetup();       // Configures the switch on/off pins accordingly
        switch (mode)
        {
            case 1:// TURN ON IMMEDIATELY
                _M95_PORT_OUT |= _M95_ON_OFF_PIN;
                break;
            case 2:// TURN ON AFTER 1 SEC DELAY
                _M95_PORT_OUT |= _M95_ON_OFF_PIN;
                break;
            case 3:// TURN OFF IMMEDIATELY
                _M95_PORT_OUT &= ~_M95_ON_OFF_PIN;
                break;
            case 4:// RESTART
                _M95_PORT_OUT &= ~_M95_ON_OFF_PIN;
                _delay_cycles(SystemFreq/2); // this will be timer based later
                _M95_PORT_OUT |=  _M95_ON_OFF_PIN;
                _delay_cycles(SystemFreq*5); // this will be timer based later
                break;
            default:
                status = FAIL;
                break;
        }

        return  status;
}

/*
 * baudrate will be 9600 to any suitable value.
 * Oversampling is off. please read datasheet before pushing values
 */
uint8_t ConfigureMdmUART(float baudrate, uint8_t interrupt_polling)
{
    volatile float    val, val3;
    uint16_t  val2;
    volatile int val4 = 0;
     _GsmMdm_PORT_SEL = (_GsmMdm_UART_RX + _GsmMdm_UART_TX);                             // P3.4,5 = USCI_A0 TXD/RXD
     UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
     UCA0CTL1 |= UCSSEL_2;                     // SMCLK
     // Calculation of the baudrate values
     val = (SystemFreq/baudrate);
     val2 = (uint16_t)(val);
     UCA0BR0 = val2 % 256;                              // (see User's Guide)
     UCA0BR1 = val2 / 256;                              //
     val3 = (val - val2)*8;
     val4 = (int)(val3);
     switch (val4)
     {
        case 0:
            UCA0MCTL |= UCBRS_0 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
            break;
        case 1:
            UCA0MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
            break;
        case 2:
            UCA0MCTL |= UCBRS_2 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
            break;
        case 3:
            UCA0MCTL |= UCBRS_3 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
            break;
        case 4:
            UCA0MCTL |= UCBRS_4 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
            break;
        case 5:
            UCA0MCTL |= UCBRS_5 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
            break;
        case 6:
            UCA0MCTL |= UCBRS_6 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
            break;
        case 7:
            UCA0MCTL |= UCBRS_7 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
            break;
        default:
            break;
    }


     UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
     switch (interrupt_polling)
     {
        case 1:
            UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
            __bis_SR_register(GIE);       // Enter LPM0, interrupts enabled
            break;
        case 2:
             UCA0IE &= ~(UCRXIE + UCTXIE);                         // Enable USCI_A0 RX interrupt
             break;
        default:
            break;
    }

     return 0;
}

/*
 *  Reset the buffer counter
 */
void    ClrMdmBuff(void)
{
    if(_MdmBuffCnt>0)
        {
            unsigned int var;
            for (var = 0; var < _MdmBuffCnt; ++var)
            {
                _MdmBuffer[var]=0;
            }
            _MdmBuffCnt = 0;
        }
}


/*
 *  Send String of data to modem by calling this Fn.
 */
void     SendDataToMdm(const uint8_t* data)
{
//    uint8_t indx=0;
    ClrMdmBuff();
    while(*data)
    {
        while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
        UCA0TXBUF = *data;                  // TX -> RXed character
        data++;
//        _delay_cycles(SystemFreq/100);
    }
//    while(_MdmBuffCnt==0);

}

/*
 *  Send single character data to modem
 */
void      SendCharToMdm(unsigned char data)
{
    while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
    UCA0TXBUF = data;                  // TX -> RXed character
    _delay_cycles(SystemFreq/100);
}
/*
 *  Disable the Long Response and Echo,
 *  Condition is saved in _MdmInitSts
 */
void    DeEchoShrtRsp(void)
{
    _MdmInitSts = _M95_INIT_SUCCESS;
    ReplyTimeout = 5;
    do
    {
        SendDataToMdm("ATE0\r");
        _delay_cycles(SystemFreq*3);
        StrResult = strncmp(_MdmBuffer, EchoRply, 10);
        ReplyTimeout--;

    } while((StrResult != 0)&&(ReplyTimeout>0));

    if(ReplyTimeout == 0)
        {
            _MdmInitSts = _M95_HW_RETRY_TIMEOUT;
            Modem_ON_OFF(TurnOff);
        }
    else
    {
        ReplyTimeout = 5;
        do
        {
            SendDataToMdm("ATV0\r");
            _delay_cycles(SystemFreq*3);
            StrResult = strncmp(_MdmBuffer, ShrtRply, 2);
            ReplyTimeout--;

        } while((StrResult != 0)&&(ReplyTimeout>0));

        if(ReplyTimeout == 0)
            {
                _MdmInitSts = _M95_HW_RETRY_TIMEOUT;
                Modem_ON_OFF(TurnOff);
            }
    }


    //Compare reply

    _delay_cycles(SystemFreq*2);
    //Compare reply
}


/*
 *  Initializes the Modem and checks the status of init. Also
 *  Checks for the diagnostics and saves required values
 *  to an Array through a return Pointer to an Array
 *  OutPut Array Format = {
 *  ICCID[20] //+CCID: “898600220909A0206023898600220909A0206023”
 *  COPS[] //+COPS: 0,0,"Vodafone - Delhi"
 *  QINISTAT[1] //+QINISTAT: 3
 *  QLTS[]      //+QLTS: "19/07/06,17:26:27+22,0" --> ONLY ONCE ALLOWED IMMEDIATELY AFTER POWER UP & NEEDS AT+CTZU=3
 *  CREG[]      //+CREG: 0,5
 *  CSQ[]       //+CSQ: 31,0
 *  CBC[]       //+CBC: 0,82,3955
 *
 *  _MdmStatus[0-19:ICCID][20-22:0,2/0,5 CREG][OPERATER]
 *
 *  ///////////////////////
 *  STATE MACHINES
 *  //////////////////////
 *   POWER ON -> UNECHO SHRT RESPNSE -> SET NTWRK TIME SYNC -> CHK NTWRK REG -> CHECK NTWRK PWR -> CHK MODULE SUPPLY VOLTAGE-->EXIT
 *   /////////////////////
 *   example:
 *   _MdmStatus[70]=[iccid 0-19][;][netwrg status 20-22][;][operator name 23-?][;][RSSI ? +2][;][network time GMT + time zone ?+21][;][cbc mode ? +7][;]
 *   _MdmStatus[70]=[12231092012343127901][;][0,5][;][Vodafone - Delhi][;][31][;][19/07/13,08:09:53,+22][;][,03,3960]
 */

int                ReadMdmRSSI(void)
{
    ///////////// Network Strength Fetch ////////////
    uint8_t RetryTimeout = 5;
    do
    {
        SendDataToMdm("AT+CSQ\r");
        _delay_cycles(SystemFreq*3);
        StrResult = strncmp(_MdmBuffer, CSQRply, 5);
        RetryTimeout--;

    } while((StrResult != 0)&&(RetryTimeout>1));

    if (RetryTimeout == 1)
            {
                _MdmStatus[42] = 0xFA;
                _MdmStatus[43] = 0xFA;
                return -1;
            }
    else
    {
        temp_mdm = strchr(_MdmBuffer, ':'); //char *strchr(const char *str, int c) searches for the first
                                                    //occurrence of the character c (an unsigned char) in the
                                                    //string pointed to by the argument str //
        temp_mdm+= 2;
        int Charcount= 42; // DONT USE THIS BECAUSE WE DON'T\KNOW HOW MANY CHARS WILL COOME IN COPS
        while(*temp_mdm != ',')
        {

            _MdmStatus[Charcount]= *temp_mdm;
            temp_mdm++;
            Charcount++;
        }

    }

    return 0;
}


/*
 *  Get the IP Address of the instance
 */

int            ReadIPAddr(void)
{
    ///////////// Network Strength Fetch ////////////
    uint8_t RetryTimeout = 5;
    do
    {
        SendDataToMdm("AT+QISHOWRA=1\r");
 //       while(_MdmBuffCnt < sizeof(QILOCIPRply)-1);
        _delay_cycles(SystemFreq*7);
        StrResult = strncmp(_MdmBuffer, CSQRply, 5);
        RetryTimeout--;

    } while((StrResult != 0)&&(RetryTimeout>1));

    if (RetryTimeout == 1)
            {
                _MdmStatus[42] = 0xFA;
                _MdmStatus[43] = 0xFA;
                return -1;
            }
    else
    {
        temp_mdm = strchr(_MdmBuffer, ':'); //char *strchr(const char *str, int c) searches for the first
                                                    //occurrence of the character c (an unsigned char) in the
                                                    //string pointed to by the argument str //
        temp_mdm+= 2;
        int Charcount= 42; // DONT USE THIS BECAUSE WE DON'T\KNOW HOW MANY CHARS WILL COOME IN COPS
        while(*temp_mdm != ',')
        {

            _MdmStatus[Charcount]= *temp_mdm;
            temp_mdm++;
            Charcount++;
        }

    }

    return 0;
}






MdmStateMachines    MdmInitAndDiag(void)
{
    SendDataToUCA3("__Entering MdmInitAndDiag__\r");
    MdmStateMachines    MdmState;
    uint8_t                 RetryTimeout=0;
    uint8_t             CheckSts=0;
    volatile int Charcount=0;

    MdmState = _M95_STARTUP_SUCCESS;
    RetryTimeout = 5;
    do
    {
        CheckSts = Modem_ON_OFF(TurnOnImmediately);
        RetryTimeout--;
    } while ((RetryTimeout > 0)&&(CheckSts != SUCCESS));
    if ((RetryTimeout == 0)&&(CheckSts == FAIL))
    {
        MdmState = _M95_STARTUP_CMD_FAILURE;
        _MdmStatus[0] = 0xFA;                           // 0xFA = Failure Code can be read from position 0 of modem status array
        return  MdmState;
    }

    else if(MdmState==_M95_STARTUP_SUCCESS)
    {
        _delay_cycles(120000000);
        RetryTimeout = 5;
        do
        {
            SendDataToMdm("AT+QINISTAT\r");//
            _delay_cycles(SystemFreq*10);
            StrResult =strncmp(_MdmBuffer, QINISTATRply, 26);
            RetryTimeout--;
        } while ((RetryTimeout > 0)&&(StrResult != 0));

        if (RetryTimeout == 0)
                {
                    MdmState = _M95_AT_FAILURE;
                    _MdmStatus[0] = 0xFA;
                    return  MdmState;
                }


        RetryTimeout = 5;
        do
        {
            DeEchoShrtRsp();
            RetryTimeout--;
        } while ((RetryTimeout > 0)&&(_MdmInitSts != _M95_INIT_SUCCESS));

        if ((RetryTimeout == 0)&&(_MdmInitSts == _M95_HW_RETRY_TIMEOUT))
        {
            MdmState = _M95_ECHO_SHRTRSP_FAILURE;
            _MdmStatus[0] = 0xFA;
            return  MdmState;
        }

        MdmState = _M95_ECHO_SHRTRSP_OFF;   // State Machine state = _M95_ECHO_SHRTRSP_OFF

        RetryTimeout = 5;
        do
        {
            SendDataToMdm("AT+CCID\r");
            _delay_cycles(SystemFreq*3);
            StrResult = strncmp(_MdmBuffer, ICCIDRply, 7);
            RetryTimeout--;

        } while((StrResult != 0)&&(RetryTimeout>1));

        if ((RetryTimeout == 1)&&(_MdmInitSts == _M95_ECHO_SHRTRSP_OFF))
                {
                    MdmState = _M95_ICCID_FAILURE;
                    _MdmStatus[0] = 0xFA;
                    return  MdmState;
                }
        else
        {
            MdmState = _M95_ICCID_READ;   // State Machine state = ICCID has been read and placed in buffer
            temp_mdm = strchr(_MdmBuffer, '"'); //char *strchr(const char *str, int c) searches for the first
                                            //occurrence of the character c (an unsigned char) in the
                                            //string pointed to by the argument str //
            Charcount=0;
            while(Charcount < 20)
            {
                temp_mdm++;
                _MdmStatus[Charcount]= *temp_mdm;
                Charcount++;
            }

            _MdmStatus[Charcount]= ';';     //iinsert delimiter
            Charcount++;

            ///////////// Check SIM Registration ////////////
            RetryTimeout = 5;
            do
            {
                SendDataToMdm("AT+CREG?\r");
                _delay_cycles(SystemFreq*3);
                StrResult = strncmp(_MdmBuffer, CREGRply, 7);
                RetryTimeout--;

            } while((StrResult != 0)&&(RetryTimeout>0));

            if ((RetryTimeout == 1)&&(MdmState == _M95_ICCID_READ))
                    {
                        MdmState = _M95_NTWRK_REG_FAIL;
                        _MdmStatus[0] = 0xFA;
                        return  MdmState;
                    }
            else
                    {
                        MdmState = _M95_NTWRK_REG_OK;
                        temp_mdm = strchr(_MdmBuffer, ':'); //char *strchr(const char *str, int c) searches for the first
                                                                    //occurrence of the character c (an unsigned char) in the
                                                                    //string pointed to by the argument str //
                        temp_mdm+= 2;
                        Charcount= 21;
                        while(Charcount < 24)
                        {

                            _MdmStatus[Charcount]= *temp_mdm;
                            temp_mdm++;
                            Charcount++;
                        }

                        _MdmStatus[Charcount]= ';';     //iinsert delimiter
                        Charcount++;


                        ///////////// Operator Name Fetch ////////////
                        RetryTimeout = 5;
                        do
                        {
                            SendDataToMdm("AT+COPS?\r");
                            _delay_cycles(SystemFreq*3);
                            StrResult = strncmp(_MdmBuffer, COPSRply, 7);
                            RetryTimeout--;

                        } while((StrResult != 0)&&(RetryTimeout>1));
                        if ((RetryTimeout == 1)&&(MdmState == _M95_NTWRK_REG_OK))
                                {
                                    MdmState = _M95_OPER_NAME_FETCH_FAIL;
                                    _MdmStatus[0] = 0xFA;
                                    return  MdmState;
                                }
                        else
                        {
                            MdmState = _M95_OPER_NAME_FETCH_SUCCESS;
                            temp_mdm = strchr(_MdmBuffer, ':'); //char *strchr(const char *str, int c) searches for the first
                                                                        //occurrence of the character c (an unsigned char) in the
                                                                        //string pointed to by the argument str //
                            temp_mdm+= 7;
                            Charcount= 25;
                            while((*temp_mdm != '"')&&(Charcount < 41))
                            {

                                _MdmStatus[Charcount]= *temp_mdm;
                                temp_mdm++;
                                Charcount++;
                            }

                            if(Charcount < 41)
                            {
                                while(!(Charcount == 41))
                                {
                                    _MdmStatus[Charcount]= '*'; // if operator name comes < 8, fill remaining spaces with *.
                                    Charcount++;
                                }
                            }

                            _MdmStatus[Charcount]= ';';     //iinsert delimiter
                            Charcount++;


                            ///////////// Network Strength Fetch ////////////
                            RetryTimeout = 5;
                            do
                            {
                                SendDataToMdm("AT+CSQ\r");
                                _delay_cycles(SystemFreq*3);
                                StrResult = strncmp(_MdmBuffer, CSQRply, 5);
                                RetryTimeout--;

                            } while((StrResult != 0)&&(RetryTimeout>1));

                            if ((RetryTimeout == 1)&&(MdmState == _M95_OPER_NAME_FETCH_SUCCESS))
                                    {
                                        MdmState = _M95_NTWRK_RSSI_RETRIEVE_FAIL;
                                        _MdmStatus[0] = 0xFA;
                                        return  MdmState;
                                    }
                            else
                            {
                                MdmState = _M95_NTWRK_RSSI_RETRIEVE_OK;
                                temp_mdm = strchr(_MdmBuffer, ':'); //char *strchr(const char *str, int c) searches for the first
                                                                            //occurrence of the character c (an unsigned char) in the
                                                                            //string pointed to by the argument str //
                                temp_mdm+= 2;
                                //int Charcount= 23; // DONT USE THIS BECAUSE WE DON'T\KNOW HOW MANY CHARS WILL COOME IN COPS
                                while(*temp_mdm != ',')
                                {

                                    _MdmStatus[Charcount]= *temp_mdm;
                                    temp_mdm++;
                                    Charcount++;
                                }

                                SendDataToMdm("AT+CTZU=3\r");
                                _delay_cycles(SystemFreq*3);

                                _MdmStatus[Charcount]= ';';     //iinsert delimiter
                                Charcount++;


                                ///////////// Network Time Fetch ////////////
                                RetryTimeout = 5;
                                do
                                {
                                    SendDataToMdm("AT+QLTS\r");
                                    _delay_cycles(SystemFreq*3);
                                    StrResult = strncmp(_MdmBuffer, QLTSRply, 7);//+QLTS: "19/07/11,18:44:25+22,0"\r\n0\r
                                    RetryTimeout--;

                                } while((StrResult != 0)&&(RetryTimeout>1));

                                if ((RetryTimeout == 1)&&(MdmState == _M95_NTWRK_RSSI_RETRIEVE_OK))
                                        {
                                            MdmState = _M95_NTWRK_TIME_RETRIEVE_PARSE_FAIL;
                                            _MdmStatus[0] = 0xFA;
                                            return  MdmState;
                                        }

                                else
                                {
                                    MdmState = _M95_NTWRK_TIME_RETRIEVE_PARSE_OK;
                                     temp_mdm = strchr(_MdmBuffer, ':'); //char *strchr(const char *str, int c) searches for the first
                                                                                 //occurrence of the character c (an unsigned char) in the
                                                                                 //string pointed to by the argument str //
                                     temp_mdm+= 3;
                                     //int Charcount= 23; // DONT USE THIS BECAUSE WE DON'T\KNOW HOW MANY CHARS WILL COOME IN COPS
                                     while(*temp_mdm != '"')
                                     {

                                         _MdmStatus[Charcount]= *temp_mdm;
                                         temp_mdm++;
                                         Charcount++;
                                     }

                                     _MdmStatus[Charcount]= ';';     //iinsert delimiter
                                     Charcount++;


                                     ///////////// CBC Fetch ////////////
                                     RetryTimeout = 5;
                                     do
                                     {
                                         SendDataToMdm("AT+CBC\r");
                                         _delay_cycles(SystemFreq*3);
                                         StrResult = strncmp(_MdmBuffer, CBCRply, 6);// +CBC: 0,83,3960\r\n0\r
                                         RetryTimeout--;

                                     } while((StrResult != 0)&&(RetryTimeout>1));


                                     if ((RetryTimeout == 1)&&(MdmState == _M95_NTWRK_TIME_RETRIEVE_PARSE_OK))
                                             {
                                                 MdmState = _M95_CBC_RETIEVE_FAIL;
                                                 _MdmStatus[0] = 0xFA;
                                                 return  MdmState;
                                             }
                                     else
                                     {
                                         MdmState = _M95_CBC_RETIEVE_OK;
                                          temp_mdm = strchr(_MdmBuffer, ':'); //char *strchr(const char *str, int c) searches for the first
                                                                                      //occurrence of the character c (an unsigned char) in the
                                                                                      //string pointed to by the argument str //
                                          temp_mdm+= 5;                        // To make the pointer parse the Battery % val and also the mV val.

                                          while(*temp_mdm != '\r')
                                          {

                                              _MdmStatus[Charcount]= *temp_mdm;
                                              temp_mdm++;
                                              Charcount++;
                                          }

                                          _MdmStatus[Charcount]= ';';     //iinsert delimiter
                                          Charcount++;


                                     }




                                }
                            }



                        }
                    }
        }

    }

    return  MdmState;
}

MdmStateMachines    MdmMakeReady(MdmOperCommand OperCmd)
{
    /*                  === GET EXAMPLE =====
     *
    AT+QIFGCNT=0
    OK
    AT+QICSGP=1,"CMNET" //Set APN
    OK
    AT+QIREGAPP //Optional
    OK
    AT+QIACT //Optional
    OK
    AT+QHTTPURL=79,30 //Set URL
    CONNECT
    <Input data>
    //For example, input 79 bytes:
    http://api.efxnow.com/DEMOWebServices2.8/Service.asmx/Echo?Message=helloquectel.
    OK
    AT+QHTTPGET=60 //Send GET Request to HTTP server.
    OK
    AT+QHTTPREAD=30 //Read the response of HTTP server.
    CONNECT
    <Output data> //Output the response data of HTTP server to UART.
    //For example, UART outputs:
    <?xml version="1.0" encoding="utf-8"?>
    <string xmlns="https://api.efxnow.com/webservices2.3">Message='helloquectel' ASCII:104 101 108 108
    111 113 117 101 99 116 101 108 </string>.
    OK
    AT+QIDEACT //Deactivate PDP context.
    DEACT OK
     */
    MdmStateMachines RetSts = _M95_NOP;
    uint8_t          RetryTimeout=0, WaitTimeout = 0;
    unsigned char    temp[4];
    volatile int Charcount=0;
    static int apiLength=0, apnValidation=0;
///////////// Initialization Switch Cases////////////////////
    switch (OperCmd)
    {
        case _Modem_Full_Init:
            SendDataToUCA3("__case _Modem_Full_Init in MdmMakeReady__\r");
            RetSts = _M95_MAKING_READY;
            RetryTimeout=5;
            while((RetryTimeout>0)&& (RetSts ==_M95_AT_FAILURE||
                                        RetSts ==_M95_MAKING_READY||
                                        RetSts == _M95_NTWRK_REG_FAIL||
                                        RetSts == _M95_ECHO_SHRTRSP_FAILURE||
                                        RetSts == _M95_STARTUP_CMD_FAILURE))
            {
                if((RetSts == _M95_NTWRK_REG_FAIL)||(RetSts == _M95_ECHO_SHRTRSP_FAILURE)||
                        (RetSts == _M95_AT_FAILURE)||(RetSts == _M95_STARTUP_CMD_FAILURE))
                {
                    Modem_ON_OFF(Restart);
                }

                RetryTimeout--;
                RetSts = MdmInitAndDiag();
                _delay_cycles(SystemFreq);
            } //(RetryTimeout >0)&&((RetSts == _M95_NTWRK_REG_FAIL)||(RetSts == _M95_ECHO_SHRTRSP_FAILURE)||
                                                                                                             //(RetSts == _M95_AT_FAILURE)||(RetSts == _M95_STARTUP_CMD_FAILURE))
//            if(RetSts == _M95_CBC_RETIEVE_OK)
//            {
//                RetSts =  _M95_READY;
//            }
//            else
//            {
//                RetSts =  _M95_INIT_INCOMPLETE_OR_FAIL;
//            }

            break;
        case _Modem_Start_GPRS:

                RetSts = _M95_START_CGATT_SUCCESS;
                ///////////// Check CGATT STATUS ////////////
                RetryTimeout = 5;
                do
                {
                    SendDataToMdm("AT+CGATT?\r"); // CHECK IF GPRS IS ATTACHED ?
                    _delay_cycles(SystemFreq*3);
                    StrResult = strncmp(_MdmBuffer, CGATT_ON_Rply, 11);
                    if(StrResult != 0)
                    {
                        SendDataToMdm("AT+CGATT=1\r"); // if GPRS bearer detached, attach it
                        _delay_cycles(SystemFreq*3);
                    }
                    RetryTimeout--;

                } while((StrResult != 0)&&(RetryTimeout>0));

                if (RetryTimeout == 0)
                 {
                    RetSts = _M95_START_CGATT_FAIL;
                 }

            break;
        case _Modem_Stop_GPRS:
            RetSts = _M95_STOP_CGATT_SUCCESS;
            ///////////// Check CGATT STATUS ////////////
            RetryTimeout = 5;
            do
            {
                SendDataToMdm("AT+CGATT?\r"); // CHECK IF GPRS IS ATTACHED ?
                _delay_cycles(SystemFreq*3);
                StrResult = strncmp(_MdmBuffer, CGATT_OFF_Rply, 11);
                if(StrResult != 0)
                {
                    SendDataToMdm("AT+CGATT=0\r"); // if GPRS bearer detached, attach it
                    _delay_cycles(SystemFreq*3);
                }
                RetryTimeout--;

            } while((StrResult != 0)&&(RetryTimeout>0));
            if (RetryTimeout == 0)
             {
                RetSts = _M95_STOP_CGATT_FAIL;
             }
            break;
        case _Modem_GET_Request:
            SendDataToUCA3("__case _Modem_GET_Request in MdmMakeReady__\r");
            runHomeDisplay(_UpdateTxDataTransaction);

            ///////////// AT+QIFGCNT=0 ////////////
            RetSts = _M95_HTTP_GET_START;
            RetryTimeout = 5;
            WaitTimeout = 0;
            do
            {
                SendDataToMdm("AT+QIFGCNT=0\r");
                SendDataToUCA3("__AT+QIFGCNT=0\r");
                while((_MdmBuffCnt == 0)&&(WaitTimeout<10))
                {
                    _delay_cycles(SystemFreq);
                    WaitTimeout ++;
                    if(WaitTimeout == 9)
                    {
                        RetryTimeout = 1;       // To break out the loop and exit
                    }

                }
//                _delay_cycles(SystemFreq*2);
//                while(_MdmBuffCnt < sizeof(QIFGCNTRply)-1);
//                while(_MdmBuffCnt == 0);
//                _delay_cycles(SystemFreq);
                StrResult = strncmp(_MdmBuffer, QIFGCNTRply, 2);// +CBC: 0,83,3960\r\n0\r
                RetryTimeout--;

            } while((StrResult != 0)&&(RetryTimeout>1));


            if ((RetryTimeout >1)&&(RetSts == _M95_HTTP_GET_START))
            {

                ///////////// AT+QICSGP=1,"CMNET" ////////////
                RetryTimeout = 5;
                do
                {

                //    SendDataToMdm("AT+QICSGP=1,\"internet\"\r");
                    SendDataToMdm("AT+QICSGP=1,\"");
                    apnValidation = 0;
                    while(_MdmAPN[apnValidation] != '*')
                    {
                        SendCharToMdm(_MdmAPN[apnValidation]);
                        apnValidation++;
                    }
                    SendDataToMdm("\"\r");
                    SendDataToUCA3("__AT+QICSGP=1,\"APN\"\r");
//                    _delay_cycles(SystemFreq*5);///////////////////////////////////////////////////////////>>
//                    while(_MdmBuffCnt < sizeof(QICSGPRply)-1);
                    while(_MdmBuffCnt == 0);
                    _delay_cycles(SystemFreq);

                    StrResult = strncmp(_MdmBuffer,QICSGPRply,2);// +CBC: 0,83,3960\r\n0\r
                    RetryTimeout--;

                } while((StrResult != 0)&&(RetryTimeout>1));

                if (RetryTimeout >1)
                {
                    ///////////// AT+QICSGP=1,"CMNET" ////////////
                    RetSts = _M95_HTTP_URL_CONNECT_SUCCESS;

                    RetryTimeout = 5;
                    do
                    { ///////////// AT+QHTTPURL ////////////
                        SendDataToMdm("AT+QHTTPURL=");
                        SendDataToMdm("58,");//58
                        //SendDataToMdm((uint8_t)strlen(_MdmGETurl));
                        SendDataToMdm("30\r");// 30 mS duration to send the URL
                        SendDataToUCA3("__AT+QHTTPURL=58,30\r");
//                        _delay_cycles(SystemFreq*3);///////////////////////////////////////////////////////////>>
//                        while(_MdmBuffCnt < sizeof(QHTTPURLRply)-1);
                        while(_MdmBuffCnt == 0);
                        _delay_cycles(SystemFreq);
                        StrResult = strncmp(_MdmBuffer,QHTTPURLRply,9);// +CBC: 0,83,3960\r\n0\r
                        RetryTimeout--;


                    } while((StrResult != 0)&&(RetryTimeout>1));
                    if(RetryTimeout > 1)
                    {
                        RetryTimeout = 5;
                        do
                        {
                            SendDataToMdm(_MdmGETurl);
                            SendDataToMdm("\r");
                            SendDataToUCA3("http://www.turjasuzworld.in/Branon/api/srv2.php?dvid=D1255\r");
//                            _delay_cycles(SystemFreq*3);///////////////////////////////////////////////////////////>>
//                            while(_MdmBuffCnt < sizeof(URLRply)-1);
                            while(_MdmBuffCnt == 0);
                            _delay_cycles(SystemFreq);
                            StrResult = strncmp(_MdmBuffer,URLRply,2);// +CBC: 0,83,3960\r\n0\r
                            RetryTimeout--;

                        } while((StrResult != 0)&&(RetryTimeout>1));
                        if(RetryTimeout==1)
                        {
                            RetSts = _M95_HTTP_URL_CONNECT_FAIL;
                        }
                        else
                        {
                            RetryTimeout = 3;
                            WaitTimeout = 0;

                            do
                            {
                                SendDataToMdm("AT+QHTTPGET=60\r");
                                SendDataToUCA3("__AT+QHTTPGET=60\r");
                                while((_MdmBuffCnt == 0)&&(WaitTimeout<20))
                                {
                                    _delay_cycles(SystemFreq);
                                    WaitTimeout ++;
                                    if(WaitTimeout == 19)
                                    {
                                        RetryTimeout = 1;       // To break out the loop and exit
                                    }

                                }
//                                 _delay_cycles(SystemFreq*8);///////////////////////////////////////////////////////////>>
//                                while(_MdmBuffCnt < sizeof(QHTTPGETRply)-1); -----> Removed to optimise speed to server, reactivate if needed
//                                while(_MdmBuffCnt == 0);
//                                _delay_cycles(SystemFreq);
                                    StrResult = strncmp(_MdmBuffer,QHTTPGETRply,2);// +CBC: 0,83,3960\r\n0\r
                                    RetryTimeout--;


                            } while((StrResult != 0)&&(RetryTimeout>1));
                            if(RetryTimeout>1)
                            {


                                RetSts = _M95_HTTP_GET_DATA_READ_FRM_BUFF_SUCCESS;
                                runHomeDisplay(_UpdateRxDataTransaction); // Indicate on HMI that data has been received from server
                                RetryTimeout = 5;
                                do
                                {
                                    SendDataToMdm("AT+QHTTPREAD=30\r");
                                    SendDataToUCA3("__AT+QHTTPREAD=30\r");
//                                    _delay_cycles(SystemFreq*5);///////////////////////////////////////////////////////////>>
//                                    while(_MdmBuffCnt < sizeof(QHTTPREADRply)-1);  -----> Removed to optimise speed to server, reactivate if needed
//                                    while(_MdmBuffCnt == 0);
                                    _delay_cycles(SystemFreq*2);
                                    StrResult = strncmp(_MdmBuffer,QHTTPREADRply,9);// +CBC: 0,83,3960\r\n0\r
                                    RetryTimeout--;


                                } while((StrResult != 0)&&(RetryTimeout>1));
                                if(RetryTimeout==1)
                                {
                                    RetSts = _M95_HTTP_GET_DATA_READ_FRM_BUFF_FAIL;
                                }
                                else
                                {
                                    /*
                                     * Fetch the GET DATA into RAM
                                     */

                                    temp_mdm = strchr(_MdmBuffer, '{'); //char *strchr(const char *str, int c) searches for the first
                                                                                //occurrence of the character c (an unsigned char) in the
                                                                                //string pointed to by the argument str //
                                    temp_mdm+= 2;                        // To make the pointer parse the Battery % val and also the mV val.

                                    while(*temp_mdm != '}')
                                    {

                                        _MdmHTTPBuff[Charcount]= *temp_mdm;
                                        temp_mdm++;
                                        Charcount++;
                                    }

                                    RetryTimeout = 5;
                                    do
                                    {
                                        SendDataToMdm("AT+QIDEACT\r");
                                        SendDataToUCA3("__AT+QIDEACT\r");
                                        _delay_cycles(SystemFreq*3);///////////////////////////////////////////////////////////>>
//                                        while(_MdmBuffCnt < sizeof(QIDEACTRply)-1);
//                                        while(_MdmBuffCnt==0);
                                        StrResult = strncmp(_MdmBuffer,QIDEACTRply,2);//DEACT HTTP PROFILE
                                        RetryTimeout--;

                                    } while((StrResult != 0)&&(RetryTimeout>1));


                                    //_delay_cycles(SystemFreq*3);

                                    void (*ptr)() = &ParseMdmBuffer;

                                    // calling function B and passing
                                    // address of the function A as argument
                                    ParseCallback(ptr);

                                }
                            }
                            else
                            {
                                RetSts = _M95_HTTP_3818_OPEN_PORT_FAIL;
                                SendDataToMdm("AT+QIDEACT\r");
                                SendDataToUCA3("__AT+QIDEACT\r");
                                _delay_cycles(SystemFreq*3);///////////////////////////////////////////////////////////>>
                            }

                        }
                    }




                }


            }
            else
            {
                RetSts = _M95_HTTP_GET_START_FAIL;
                SendDataToMdm("AT+QIDEACT\r");
                SendDataToUCA3("_M95_HTTP_GET_START_FAIL");
                _delay_cycles(SystemFreq);///////////////////////////////////////////////////////////>>

            }

            runHomeDisplay(_UpdateIoTRxTxDataTransaction);

            break;
        case _Modem_SET_Request:

            runHomeDisplay(_UpdateSET_TxDataTransaction);
            SendDataToUCA3("__case _Modem_SET_Request in MdmMakeReady__\r");
            ///////////// AT+QIFGCNT=0 ////////////
            RetSts = _M95_HTTP_SET_START;
            RetryTimeout = 5;
            WaitTimeout = 0;
            do
            {
                SendDataToMdm("AT+QIFGCNT=0\r");
                SendDataToUCA3("__AT+QIFGCNT=0\r");
                while((_MdmBuffCnt == 0)&&(WaitTimeout<10))
                {
                    _delay_cycles(SystemFreq);
                    WaitTimeout ++;
                    if(WaitTimeout == 9)
                    {
                        RetryTimeout = 1;       // To break out the loop and exit
                    }

                }
//                _delay_cycles(SystemFreq*5);
//                while(_MdmBuffCnt < sizeof(QIFGCNTRply)-1);
//                while(_MdmBuffCnt == 0);
//                _delay_cycles(SystemFreq);
                StrResult = strncmp(_MdmBuffer, QIFGCNTRply, 2);// +CBC: 0,83,3960\r\n0\r
                RetryTimeout--;

            } while((StrResult != 0)&&(RetryTimeout>1));


            if ((RetryTimeout >1)&&(RetSts == _M95_HTTP_SET_START)) // Set and get start are same
            {

                ///////////// AT+QICSGP=1,"CMNET" ////////////
                RetryTimeout = 5;
                do
                {

                    SendDataToMdm("AT+QICSGP=1,\"");
                    apnValidation = 0;
                    while(_MdmAPN[apnValidation] != '*')
                    {
                        SendCharToMdm(_MdmAPN[apnValidation]);
                        apnValidation++;
                    }
                    SendDataToMdm("\"\r");
                    SendDataToUCA3("__AT+QICSGP=1,\"APN\"\r");
//                    _delay_cycles(SystemFreq*5);///////////////////////////////////////////////////////////>>
//                    while(_MdmBuffCnt < sizeof(QICSGPRply)-1);
                    while(_MdmBuffCnt == 0);
                    _delay_cycles(SystemFreq);
                    StrResult = strncmp(_MdmBuffer,QICSGPRply,2);// +CBC: 0,83,3960\r\n0\r
                    RetryTimeout--;

                } while((StrResult != 0)&&(RetryTimeout>1));

                if (RetryTimeout >1)
                {
                    ///////////// AT+QICSGP=1,"CMNET" ////////////
                    RetSts = _M95_HTTP_URL_CONNECT_SUCCESS;

                    RetryTimeout = 5;
                    do
                    { ///////////// AT+QHTTPURL ////////////
                        SendDataToMdm("AT+QHTTPURL=");
//                        SETAPIlength = strlen(_MdmSETurl); //=> 12 sep
//                        temp[0] = SETAPIlength/100;
//                        temp[0] += 0x30;
//                        temp[1] = ((SETAPIlength%100)/10);
//                        temp[1] += 0x30;
//                        temp[2] = ((SETAPIlength%100)%10);
//                        temp[2] += 0x30;
//                        temp[3] = 0;
//                        SendDataToMdm(temp);
                        apiLength = (int)strlen((const char*)_MdmSETurl);
//                        SendDataToMdm("713,");//296
                        SendCharToMdm((apiLength/100) + 48); // hundreds
                        SendCharToMdm(((apiLength%100)/10)+ 48); // tens
                        SendCharToMdm((apiLength%10)+ 48);// units
                        SendDataToMdm(",30\r");// 30 mS duration to send the URL
                        SendDataToUCA3("__AT+QHTTPURL=.....,30\r");
//                        _delay_cycles(SystemFreq*3);///////////////////////////////////////////////////////////>>
//                        while(_MdmBuffCnt < sizeof(QHTTPURLRply)-1);
                        while(_MdmBuffCnt == 0);
                        _delay_cycles(SystemFreq);
                        StrResult = strncmp(_MdmBuffer,QHTTPURLRply,9);// +CBC: 0,83,3960\r\n0\r
                        RetryTimeout--;


                    } while((StrResult != 0)&&(RetryTimeout>1));
                    if(RetryTimeout > 1)
                    {
                        RetryTimeout = 5;
                        do
                        {
                            SendDataToMdm((const uint8_t *)_MdmSETurl);
                            SendDataToMdm("\r");
                            SendDataToUCA3("SET API sent \r");
//                            _delay_cycles(SystemFreq*3);///////////////////////////////////////////////////////////>>
//                            while(_MdmBuffCnt < sizeof(URLRply)-1);
                            while(_MdmBuffCnt == 0);
                            _delay_cycles(SystemFreq);

                            StrResult = strncmp(_MdmBuffer,URLRply,2);// +CBC: 0,83,3960\r\n0\r
                            RetryTimeout--;

                        } while((StrResult != 0)&&(RetryTimeout>1));
                        if(RetryTimeout==1)
                        {
                            RetSts = _M95_HTTP_URL_CONNECT_FAIL;
                        }
                        else
                        {
                            RetryTimeout = 5;
                            WaitTimeout = 0;
                            do
                            {
                                SendDataToMdm("AT+QHTTPGET=60\r");
                                SendDataToUCA3("__AT+QHTTPGET=60\r");
//                                 _delay_cycles(SystemFreq*8);///////////////////////////////////////////////////////////>>
//                                while(_MdmBuffCnt < sizeof(QHTTPGETRply)); //Critical !!  -----> Removed to optimise speed to server, reactivate if needed
                                while((_MdmBuffCnt == 0)&&(WaitTimeout<10))
                                {
                                    _delay_cycles(SystemFreq);
                                    WaitTimeout ++;
                                    if(WaitTimeout == 9)
                                    {
                                        RetryTimeout = 1;       // To break out the loop and exit
                                    }

                                }


                                    StrResult = strncmp(_MdmBuffer,QHTTPGETRply,2);// +CBC: 0,83,3960\r\n0\r
                                    RetryTimeout--;

                            } while((StrResult != 0)&&(RetryTimeout>1));
                            if(RetryTimeout>1)
                            {


                                RetSts = _M95_HTTP_GET_DATA_READ_FRM_BUFF_SUCCESS;
                                runHomeDisplay(_UpdateRxDataTransaction); // Indicate on HMI that data has been received from server
                                RetryTimeout = 5;
                                do
                                {
                                    SendDataToMdm("AT+QHTTPREAD=30\r");
                                    SendDataToUCA3("__AT+QHTTPREAD=30\r");
//                                    _delay_cycles(SystemFreq*5);///////////////////////////////////////////////////////////>>
//                                    while(_MdmBuffCnt < sizeof(QHTTPREADRply)-3);  -----> Removed to optimise speed to server, reactivate if needed
                                    while(_MdmBuffCnt == 0);
                                    _delay_cycles(SystemFreq);
                                    StrResult = strncmp(_MdmBuffer,QHTTPREADRply,9);// +CBC: 0,83,3960\r\n0\r
                                    RetryTimeout--;


                                } while((StrResult != 0)&&(RetryTimeout>1));
                                if(RetryTimeout==1)
                                {
                                    RetSts = _M95_HTTP_GET_DATA_READ_FRM_BUFF_FAIL;
                                }
                                else
                                {
                                    SendDataToMdm("AT+QIDEACT\r");
                                    SendDataToUCA3("__AT+QIDEACT\r");
                                    _delay_cycles(SystemFreq*3);

                                }
                            }
                            else
                            {
                                RetSts = _M95_HTTP_3818_OPEN_PORT_FAIL;
                                SendDataToMdm("AT+QIDEACT\r");
                                SendDataToUCA3("__AT+QIDEACT\r");
                                _delay_cycles(SystemFreq*3);///////////////////////////////////////////////////////////>>
                            }

                        }

                    }
                    else //AT+QHTTPURL error handling
                    {

                    }




                }
                else // AT+QICSGP=1,\"APN\"\r error handling
                {

                }


            }
            else
            {
                    RetSts = _M95_HTTP_SET_START_FAIL;
                    SendDataToMdm("AT+QIDEACT\r");
                    SendDataToUCA3("_M95_HTTP_SET_START_FAIL");
                    _delay_cycles(SystemFreq);///////////////////////////////////////////////////////////>>

            }

            runHomeDisplay(_UpdateIoTRxTxDataTransaction);



            break;


        default:
            break;
    }





    return RetSts;
}

/*
 * callback function to parse data received in UCAx Buffer
 */
void ParseCallback(void (*ptr)())
{
    (*ptr) (); // callback to A
}

///*
// *  Parsing of the JSON data received from server
// */
//void ParseMdmBuffer(void)
//{
//    SendDataToMdm("ATD9007145415;\r");
//}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *    Interrupt Vectors for the data acquired from modem
 */

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA0IV,4))
  {
  case 0:break;                             // Vector 0 - no interrupt log_data
  case 2:                                   // Vector 2 - RXIFG
      _MdmBuffer[_MdmBuffCnt] = UCA0RXBUF;                  // TX -> RXed character
      _MdmBuffCnt++;

//      if(_MdmBuffCnt==100) _MdmBuffCnt=0;
    break;
  case 4:break;                             // Vector 4 - TXIFG
  default: break;
  }
}



