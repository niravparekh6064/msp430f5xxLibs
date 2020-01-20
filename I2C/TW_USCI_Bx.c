/*
 * TW_USCI_Bx.c
 *
 *  Created on: Dec 13, 2018
 *      Author: Turjasu
 *	last changed on 14/mar/2019
 *
 *
 *	PLEASE NOTE THAT THIS SOURCE FILE IS STILL BEING DEVELOPED. SO SOME VARIABLES MAY HAVE BEEN DECLARED OR EVEN DEFINED, BUT NEVER USED
 *
 *	LOGS:
 *	2 APR 19 = ADDING LIBRARIES FOR uSD CARD
 *	2 APR 19  = ADDED THE FUNCTIONS FOR SENDING CMD0 , CMD8 TO uSDC
 *	5 APR 19 = ADDED AND CHECHED THE uSDInit , uSD_ReadBlock SUCCESFULLY WITH 2GB SDC. CHECKING INCLUDES IT'S INTERNAL FUNCTIONS ALSO.
 *	10 APR 19 = CLOSED THE DEELOPMENT OF RAW READ WRITE ONTO upto 2 GB uSDC.... NEXT FOR HC/XC CARDS SEE THE FATFS SOURCES
 *	12 APR 2019 = SUCCESFULLY READ MULTYIPLE BLOCKS FROM SDC USING CMD 12. NEXT WILL BE THE WRITING
 */

#include "TW_USCI_Bx.h"
#include <string.h>
#include <stdlib.h>

    volatile int TxByteCounter = 0;
    volatile int RxByteCounter = 0;
    volatile unsigned char *TxData = NULL;
    volatile unsigned char *RxData = NULL;
    static unsigned char  i2cChipIntMemAddr[2] = {0,0},
                          i2cAddrCounter = 0;
    static bool    i2cStopCompleted= false,
            i2cRestart      = false,
            i2cReadMode     = false,
            AT24C02IntAddrMode = false;
//////////////////////////////I2C Routines ////////////////////////////////////////////////

void        ConfigureI2C(int commtype)              // InterruptType = 1, PollingType = 2
{
    /*
     * Please change the peripheral port numbers accordingly
     * Like PxSEL |= SDA + SCL --> also do change the SDA , SCL declarations according to the pins in the actual MCU
     * Values for UCBxBR0 and UCBxBR1 will depend on the values of SMCLK freq. So check if DCOCLK/2 is there or not
     * The 16-bit value of (UCxxBR0 + UCxxBR1 × 256) forms the prescalar value UCBRx
     */
      unsigned char slavenumber = 0x0;
      int SlaveCount = 0;
      P9SEL |= (SDA + SCL);                            // Assign I2C pins to USCI_B2
      UCB2CTL1 |= UCSWRST;                      // Enable SW reset
      UCB2CTL0 |= UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
      UCB2CTL1 |= UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
      UCB2CTL1 |= UCTR;                         // Use only for transmitter
      UCB2BR0 = 200;                             // fSCL = SMCLK/12 = ~100kHz
      UCB2BR1 = 5;

      if(commtype == 1)
          {


          }
      else if(commtype == 2)
          {
              UCB2IE &= ~(UCSTPIE + UCNACKIE + UCTXIE + UCRXIE); //Enables the Stop, Slave unacknowledge, transmit and recieve interrupts
          }
}

/*
 * Transmit the data to any I2C EEPROM
 * @ChipAddr = 7 bit I2C slave address (postfix 0 at the end.
 *
 */
int     WriteTo24CxxEEPROMMultiBytes_InterruptMode(unsigned char ChipAddr,int ChipType,unsigned short Addr,int BytesToWrite, unsigned char *TxBucket)
{
    i2cStopCompleted = false;
    i2cRestart      = false;
    i2cReadMode = false;
    i2cAddrCounter = 0;

    /*
     *  Handle Chip Type
     */
    if(ChipType == 02)
    {
        i2cChipIntMemAddr[0] = Addr & 0xff;
        AT24C02IntAddrMode = true;
    }
    else if((ChipType == 16)||(ChipType == 32)||(ChipType == 64)||(ChipType == 256)||(ChipType == 512))
    {

    i2cChipIntMemAddr[0] = Addr >> 8;
    i2cChipIntMemAddr[1] = Addr & 0xff;
    AT24C02IntAddrMode = false;
    }
    else
    {
        return  _I2C_24Cxx_CHIP_NOT_SUPPORTED;
    }
//    TxByteCounter = (volatile int *)malloc(sizeof(int));
//    if (TxByteCounter != NULL)
//    {
        TxByteCounter = BytesToWrite;
//    }
//    else
//    {
//        free((void*)TxByteCounter);
//        return  _I2C_POINTR_MEM_ALLOC_ERR;
//    }
//
//    TxData = (volatile unsigned char *)malloc(sizeof(unsigned char));
//    if (TxData != NULL)
//    {
        TxData = TxBucket;
//    }
//    else
//    {
//        free((void*)TxData);
//        return  _I2C_POINTR_MEM_ALLOC_ERR;
//    }



//    _delay_cycles(15000);                           // 10 ms delay, change this value when using XT2 or other freq.
    UCB2CTL1 |= UCSWRST;                            // Enable SW reset
    UCB2CTL1 |=  UCTR;                              // Enabling the Tx mode

    UCB2I2CSA = ChipAddr >> 1;                      // right shift once to adjust for address format in MSP
    UCB2CTL1 &= ~UCSWRST;                           // Release USCI

    UCB2IE = (UCSTPIE + UCNACKIE + UCTXIE ); //Enables the Stop, Slave unacknowledge, transmit and recieve interrupts

    __bis_SR_register(GIE);

    UCB2CTL1 |= UCTXSTT;                     // I2C TX, start condition

    while(!(i2cStopCompleted))
    {
        SendDataToUCA3("__Waiting in i2cStopCompleted WriteTo24CxxEEPROMMultiBytes_InterruptMode loop__\r");
    }



//    free((void*)TxByteCounter);
//    free((void*)TxData);
    return  _I2C_SUCCESS;
}


/*
 * Transmit the data to any I2C EEPROM
 * ONLY SUPPORTS 24C08 AND ABOVE FOR 16 BIT MEM ADDRESSES
 * FOR 24C02 0R 8 BIT MEM ADDR CHIPS DON'T USE THIS SR
 * @ChipAddr = 7 bit I2C slave address (postfix 0 at the end.
 *
 */
int     ReadFrom24CxxEEPROMMultiBytes_InterruptMode(unsigned char ChipAddr,unsigned char ChipType,unsigned short Addr,int BytesToRead, unsigned char *RxBucket)
{
    SendDataToUCA3("__Entering ReadFrom24CxxEEPROMMultiBytes_InterruptMode__\r");
    i2cStopCompleted = false;
    i2cRestart      = true;
    i2cReadMode = true;
    i2cAddrCounter = 0;

    /*
     *  Handle Chip Type
     */
    if(ChipType == 02)
    {
        i2cChipIntMemAddr[0] = Addr & 0xff;
        AT24C02IntAddrMode = true;
    }
    else if((ChipType == 16)||(ChipType == 32)||(ChipType == 64)||(ChipType == 256)||(ChipType == 512))
    {

    i2cChipIntMemAddr[0] = Addr >> 8;
    i2cChipIntMemAddr[1] = Addr & 0xff;
    AT24C02IntAddrMode = false;
    }
    else
    {
        return  _I2C_24Cxx_CHIP_NOT_SUPPORTED;
    }


        RxByteCounter = BytesToRead;

        RxData = RxBucket;

    UCB2CTL1 |= UCSWRST;                            // Enable SW reset
    UCB2CTL1 |=  UCTR;                              // Enabling the Tx mode

    UCB2I2CSA = ChipAddr >> 1;                      // right shift once to adjust for address format in MSP
    UCB2CTL1 &= ~UCSWRST;                           // Release USCI

    UCB2IE = (UCNACKIE + UCTXIE + UCRXIE); //Enables the Stop, Slave unacknowledge, transmit and recieve interrupts

    __bis_SR_register(GIE);


    UCB2CTL1 |= UCTXSTT;                     // I2C TX, start condition

    while(!(i2cStopCompleted))
    {
        SendDataToUCA3("__Waiting in i2cStopCompleted ReadFrom24CxxEEPROMMultiBytes_InterruptMode loop__\r");
    }

    return  _I2C_SUCCESS;
}




















void    WriteSingleByte(unsigned char data)                     // Uses the I2C Bus and writes a single byte of data to the chip
{

}





void    OutputToPCF8574(unsigned char ChipAddr, unsigned char data) // ChipAddr = 0 1 0 0 A2 A1 A0 0, place address with the last zero suffixed
{
    UCB1CTL1 |= UCSWRST;                      // Enable SW reset
    UCB1CTL1 |=  UCTR;                          // Enabling the Tx mode
    ChipAddr >>= 1;                                     // right shift once to adjust for address format in MSP
    UCB1I2CSA = ChipAddr;                               // Slave Address is fed
    _delay_cycles(10000);
    UCB1CTL1 &= ~UCSWRST;                           // Release USCI
    UCB1CTL1 |= UCTXSTT;                            // I2C TX, start condition
    while(UCB1IFG & UCSTTIFG);                      // Wait till the STT bit is cleared after proper ack from the slave
    _delay_cycles(10000);
    if(UCB1IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
    {
        UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
        UCB1TXBUF = data;
        _delay_cycles(10000);
        while(!(UCB1IFG & UCTXIFG));                //wait till data is written into the slave

    }
    UCB1CTL1 |= UCTXSTP;                            // I2C TX, stop condition
    while(UCB1IFG & UCSTPIFG);                      // Wait till the stop condition has been sent
}

void    OutputToPCF8574MultiBytes(unsigned char ChipAddr,int count, unsigned char *data,bool delay) // ChipAddr = 0 1 0 0 A2 A1 A0 0, place address with the last zero suffixed
{                                                                              // *data should point to the array of data to be sent
    unsigned char temp;                                                                                   // make delay true if values are to be sent with delay, but delay needs to be adjusted
    UCB1CTL1 |= UCSWRST;                      // Enable SW reset
    UCB1CTL1 |=  UCTR;                          // Enabling the Tx mode
    UCB1I2CSA = (ChipAddr >>=1);                                // Slave Address is fed
    UCB1CTL1 &= ~UCSWRST;                           // Release USCI
    UCB1CTL1 |= UCTXSTT;                            // I2C TX, start condition
    while(UCB1IFG & UCSTTIFG);                      // Wait till the STT bit is cleared after proper ack from the slave
    while(count >0)
        {
            if(UCB1IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                    UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                    temp = *data;
                    UCB1TXBUF = temp;
                    while(!(UCB1IFG & UCTXIFG));                //wait till data is written into the slave
//                  _delay_cycles(1000000);               // Only for debugging purpose
                    data++;
                    if(delay)
                    {
                        _delay_cycles(25000000);
                    }
                }
            count--;
        }
    UCB1CTL1 |= UCTXSTP;                            // I2C TX, stop condition
    while(UCB1IFG & UCSTPIFG);                      // Wait till the stop condition has been sent

}

unsigned char  ReadFromPCF8574(unsigned char ChipAddr)  // Reads the data from PCF8574 and return the data as unsigned char
{
    volatile unsigned char RxData = 0;
    UCB1CTL1 |= UCSWRST;                      // Enable SW reset
    UCB1CTL1 &= ~UCTR;                          // Disabling the Tx mode
    UCB1I2CSA = (ChipAddr >>=1);                            // Slave Address is fed for Read condition
    UCB1CTL1 &= ~UCSWRST;                           // Release USCI
    UCB1CTL1 |= UCTXSTT;                            // I2C TX, start condition
    while(UCB1IFG & UCSTTIFG);                      // Wait till the STT bit is cleared after proper ack from the slave
    while(!(UCB1IFG & UCRXIFG));            //wait till data is written into the slave
    if(UCB1IFG & UCRXIFG)                           // TX flag is made high once the slave has acknowledged the address
        {
            RxData = UCB1RXBUF;                     // RXBUF is read and the data is placed into the return variable
            UCB1IFG &= ~UCRXIFG;                    // Clear USCI_B0 TX int flag
            while(!(UCB1IFG & UCRXIFG));            //wait till data is written into the slave
        }
    UCB1CTL1 |= UCTXSTP;                            // I2C TX, stop condition
    while(UCB1IFG & UCSTPIFG);                      // Wait till the stop condition has been sent
    _delay_cycles(10000);
    return  RxData;

}

//////////////////////////////// AT24Cxx Family Sources ////////////////////////////////////////////////
void                Erase24Cxx(unsigned char ChipAddr,unsigned char ChipType,unsigned short StrtAddr,unsigned short EndAddr) // Erases the chip from startAddr to EndAddr with 0xFF
{
    volatile unsigned short addr = 0,temp;              //if volatile is removed, then R12 is used for both vars.
    volatile unsigned char  MemLoc = 0;             // 8bit memory location to be sent to the EEPROM. this will be LSB part once and MSB once
    UCB1CTL1 |= UCSWRST;                      // Enable SW reset
    UCB1CTL1 |=  UCTR;                          // Enabling the Tx mode
    addr = ChipAddr;
    addr >>= 1;                                     // right shift once to adjust for address format in MSP
    UCB1I2CSA = addr;                               // Slave Address is fed
    UCB1CTL1 &= ~UCSWRST;                           // Release USCI
    UCB1CTL1 |= UCTXSTT;                            // I2C TX, start condition
    while(UCB1IFG & UCSTTIFG);                      // Wait till the STT bit is cleared after proper ack from the slave
    if(ChipType == 02)
        {
            if(UCB1IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                    MemLoc = StrtAddr;              // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                    UCB1TXBUF = MemLoc;
                    UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                    while(!(UCB1IFG & UCTXIFG));                //wait till data is written into the slave
                }
            for (temp = StrtAddr; temp <= EndAddr; ++temp)
            {

                UCB1TXBUF = 0xFF;
                UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                while(!(UCB1IFG & UCTXIFG));                //wait till data is written into the slave

            }


        }
    else
        {
            if(UCB1IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                temp = StrtAddr;
                temp >>=8;                      // Shifts the upper part for second address cycle for 04, 08,16 to 256 flavors
                MemLoc = temp;              // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                    UCB1TXBUF = MemLoc;
                    UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                    while(!(UCB1IFG & UCTXIFG));                //wait till data is written into the slave
                }
            if(UCB1IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                MemLoc = StrtAddr;              // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                    UCB1TXBUF = MemLoc;
                    UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                    while(!(UCB1IFG & UCTXIFG));                //wait till data is written into the slave
                }

            for (temp = StrtAddr; temp <= EndAddr; ++temp)
            {

                UCB1TXBUF = 0xFF;
                UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                while(!(UCB1IFG & UCTXIFG));                //wait till data is written into the slave

            }


        }

    UCB1CTL1 |= UCTXSTP;                            // I2C TX, stop condition
    while(UCB1IFG & UCSTPIFG);                      // Wait till the stop condition has been sent
}

int     WriteTo24CxxEEPROM(unsigned char ChipAddr,unsigned char ChipType,unsigned short Addr,unsigned char Data)    // Saves a single byte to provided Addr
{

    volatile unsigned short addr = 0;               //if volatile is removed, then R12 is used for both vars.
    volatile unsigned char  MemLoc = 0;             // 8bit memory location to be sent to the EEPROM. this will be LSB part once and MSB once
    volatile int errorcode = 0;                     //errorcodes: 0 = OK, 1 = Slave address not ACKNOWLEDGED
    _delay_cycles(15000);                           // 10 ms delay, change this value when using XT2 or other freq.
    UCB2CTL1 |= UCSWRST;                            // Enable SW reset
    UCB2CTL1 |=  UCTR;                              // Enabling the Tx mode
    addr = ChipAddr;
    addr >>= 1;                                     // right shift once to adjust for address format in MSP
    UCB2I2CSA = addr;                               // Slave Address is fed
    UCB2CTL1 &= ~UCSWRST;                           // Release USCI

    UCB2CTL1 |= UCTXSTT;                            // I2C TX, start condition
//    while(UCB2IFG & UCSTTIFG);                      // Wait till the STT bit is cleared after proper ack from the slave
//    _delay_cycles(10000);                           // 10 ms delay, change this value when using XT2 or other freq.
    if((ChipType == 2)&&(!(UCB2IFG & UCNACKIFG)))
        {
            if(UCB2IFG & UCTXIFG)                   // TX flag is made high once the slave has acknowledged the address
                {
                    MemLoc = Addr;                  // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                    UCB2TXBUF = MemLoc;
                    UCB2IFG &= ~UCTXIFG;            // Clear USCI_B0 TX int flag
                    while(!(UCB2IFG & UCTXIFG));    //wait till data is written into the slave
                }

            if(UCB2IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                    UCB2TXBUF = Data;
                    UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                    while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                }

        }
//    else if(!(UCB2IFG & UCNACKIFG))
//        {
            if(UCB2IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                addr = Addr;
                addr >>=8;                      // Shifts the upper part for second address cycle for 04, 08,16 to 256 flavors
                MemLoc = addr;              // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                    UCB2TXBUF = MemLoc;
//                    UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
//                    while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                    while(UCB2IFG & UCSTTIFG);                //wait till data is written into the slave--14dec//////////14/3/19
                }
            if(UCB2IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                addr = Addr;
                addr &=0xff;                        // Shifts the upper part for second address cycle for 04, 08,16 to 256 flavors
                MemLoc = addr;              // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                    UCB2TXBUF = MemLoc;
//                    UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                    while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                }
            if(UCB2IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                    UCB2TXBUF = Data;
//                    UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                    while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                }

//        }

    UCB2CTL1 |= UCTXSTP;                            // I2C TX, stop condition
    while(UCB2IFG & UCSTPIFG);                      // Wait till the stop condition has been sent
    UCB2IFG &= ~UCTXIFG;                            // Clear any pending TX flags
    UCB2IFG &= ~UCRXIFG;                            // Clear any RX flags
    if(UCB2IFG & UCNACKIFG)                         // Clear any NACK flags
    {
        errorcode = 1;
        UCB2IFG &= ~UCNACKIFG;                              // Clear any NACK flags due to non responding slave
    }
    _delay_cycles(150000);
    return errorcode ;

}

int     WriteTo24CxxEEPROMMultiBytes(unsigned char ChipAddr,unsigned char ChipType,unsigned short Addr,int BytesToWrite, unsigned char *TxBucket)   // Writes an Array data to EEPROM, return NACK error code 1
{

   unsigned short addr = 0;               //if volatile is removed, then R12 is used for both vars.
    unsigned char  MemLoc = 0;             // 8bit memory location to be sent to the EEPROM. this will be LSB part once and MSB once
    int errorcode = 0,var;                     //errorcodes: 0 = OK, 1 = Slave address not ACKNOWLEDGED
    _delay_cycles(330000);                           // 10 ms delay, change this value when using XT2 or other freq.
    UCB2CTL1 |= UCSWRST;                            // Enable SW reset
    UCB2CTL1 |=  UCTR;                              // Enabling the Tx mode
    addr = ChipAddr;
    addr >>= 1;                                     // right shift once to adjust for address format in MSP
    UCB2I2CSA = addr;                               // Slave Address is fed
    UCB2CTL1 &= ~UCSWRST;                           // Release USCI

    // Sending Stop bit to ensure clean comm.//
    UCB2CTL1 |= UCTXSTP;                            // I2C TX, stop condition
    while(UCB2IFG & UCSTPIFG);                      // Wait till the stop condition has been sent
    UCB2CTL1 |= UCTXSTT;                            // I2C TX, start condition
    while(UCB2IFG & UCSTTIFG);                      // Wait till the STT bit is cleared after proper ack from the slave
    _delay_cycles(330000);                           // 10 ms delay, change this value when using XT2 or other freq.
    if((ChipType == 2)&&(!(UCB2IFG & UCNACKIFG)))
        {
            if(UCB2IFG & UCTXIFG)                   // TX flag is made high once the slave has acknowledged the address
                {
                    MemLoc = Addr;                  // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                    UCB2TXBUF = MemLoc;
                    UCB2IFG &= ~UCTXIFG;            // Clear USCI_B0 TX int flag
                    while(!(UCB2IFG & UCTXIFG));    //wait till data is written into the slave
                }
            while(!(UCB2IFG & UCTXIFG));                    // Wait untill the TX Flag is set high by the 24Cxx, keep waiting <-- VVVIMP line
            for (var = 0; var < BytesToWrite; ++var)
            {
                    if(UCB2IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                        {
                            UCB2TXBUF = *TxBucket;
                            UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                            while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                        }
                    TxBucket++;
            }

        }
    else if(!(UCB2IFG & UCNACKIFG))
        {
            if(UCB2IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                addr = Addr;
                addr >>=8;                      // Shifts the upper part for second address cycle for 04, 08,16 to 256 flavors
                MemLoc = addr;              // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                    UCB2TXBUF = MemLoc;
                    UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                    while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                }
            if(UCB2IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                addr = Addr;
                addr &=0xff;                        // Shifts the upper part for second address cycle for 04, 08,16 to 256 flavors
                MemLoc = addr;              // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                    UCB2TXBUF = MemLoc;
                    UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                    while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                }
            while(!(UCB2IFG & UCTXIFG));                    // Wait untill the TX Flag is set high by the 24Cxx, keep waiting <-- VVVIMP line
            for (var = 0; var < BytesToWrite; ++var)
                {
                    if(UCB2IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                        {
                            UCB2TXBUF = *TxBucket;
                            UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                            while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                        }
                    TxBucket++;
                }

        }

    UCB2CTL1 |= UCTXSTP;                            // I2C TX, stop condition
    while(UCB2IFG & UCSTPIFG);                      // Wait till the stop condition has been sent
    UCB2IFG &= ~UCTXIFG;                            // Clear any pending TX flags
    UCB2IFG &= ~UCRXIFG;                            // Clear any RX flags
    if(UCB2IFG & UCNACKIFG)                         // Clear any NACK flags
    {
        errorcode = 1;
        UCB2IFG &= ~UCNACKIFG;                              // Clear any NACK flags due to non responding slave
    }
    _delay_cycles(330000);
    return errorcode ;
}

unsigned char   ReadFrom24CxxEEPROM(unsigned char ChipAddr,unsigned char ChipType,unsigned short Addr)  // Reads a single byte to provided Addr
{

    volatile unsigned short addr = 0;               //if volatile is removed, then R12 is used for both vars.
    volatile unsigned char  MemLoc = 0,temp=0;              // 8bit memory location to be sent to the EEPROM. this will be LSB part once and MSB once
    volatile int errorcode = 0;                     //errorcodes: 0 = OK, 1 = Slave address not ACKNOWLEDGED
    _delay_cycles(20000);                           // 10 ms delay, change this value when using XT2 or other freq.
    UCB2CTL1 |= UCSWRST;                            // Enable SW reset
    UCB2CTL1 |= UCTR;                               // Enabling the Tx mode
    addr = ChipAddr;
    addr >>= 1;                                     // right shift once to adjust for address format in MSP
    UCB2I2CSA = addr;                               // Slave Address is fed
    UCB2CTL1 &= ~UCSWRST;                           // Release USCI
    UCB2CTL1 |= UCTXSTT;                            // I2C TX, start condition
    while(UCB2IFG & UCSTTIFG);                      // Wait till the STT bit is cleared after proper ack from the slave
    _delay_cycles(100000);                          // 10 ms delay, change this value when using XT2 or other freq.
    if((ChipType == 2)&&(!(UCB2IFG & UCNACKIFG)))
        {
            if(UCB2IFG & UCTXIFG)                   // TX flag is made high once the slave has acknowledged the address
                {
                    MemLoc = Addr;                  // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                    UCB2TXBUF = MemLoc;
                    UCB2IFG &= ~UCTXIFG;            // Clear USCI_B0 TX int flag
                    while(!(UCB2IFG & UCTXIFG));    //wait till data is written into the slave
                }
            UCB2CTL1 &= ~UCTR;                              // Enabling the Tx mode
            UCB2CTL1 |= UCTXSTT;                            // I2C TX, repeated start condition
            while(UCB2IFG & UCSTTIFG);                      // Wait till the STT bit is cleared after proper ack from the slave
            while(!(UCB2IFG & UCRXIFG));                    // Wait untill the RX Flag is set high by the 24Cxx, keep waiting <-- VVVIMP line
            if(UCB2IFG & UCRXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                    temp = UCB2RXBUF;
                    UCB2IFG &= ~UCRXIFG;                  // Clear USCI_B0 TX int flag
                    while(!(UCB2IFG & UCRXIFG));                //wait till data is written into the slave
                }

        }
    else if(!(UCB2IFG & UCNACKIFG))
        {
            if(UCB2IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                    UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                    addr = Addr;
                    addr >>=8;                      // Shifts the upper part for second address cycle for 04, 08,16 to 256 flavors
                    MemLoc = addr;              // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                    UCB2TXBUF = MemLoc;
                    _delay_cycles(20000);
                    while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                }
            if(UCB2IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                    UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                    addr = Addr;
                    addr &=0xff;                        // Shifts the upper part for second address cycle for 04, 08,16 to 256 flavors
                    MemLoc = addr;              // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                    UCB2TXBUF = MemLoc;
                    _delay_cycles(20000);
                    while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                }
            UCB2CTL1 &= ~UCTR;                              // Enabling the Tx mode
            UCB2CTL1 |= UCTXSTT;                            // I2C TX, repeated start condition
            while(UCB2IFG & UCSTTIFG);                      // Wait till the STT bit is cleared after proper ack from the slave
            while(!(UCB2IFG & UCRXIFG));                    // Wait untill the RX Flag is set high by the 24Cxx, keep waiting <-- VVVIMP line
            if(UCB2IFG & UCRXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                    temp = UCB2RXBUF;
                    UCB2IFG &= ~UCRXIFG;                  // Clear USCI_B0 TX int flag
                    while(!(UCB2IFG & UCRXIFG));                //wait till data is written into the slave
                }

        }

    UCB2CTL1 |= UCTXSTP;                            // I2C TX, stop condition
    while(UCB2IFG & UCSTPIFG);                      // Wait till the stop condition has been sent
    UCB2IFG &= ~UCTXIFG;                            // Clear any pending TX flags
    UCB2IFG &= ~UCRXIFG;                            // Clear any RX flags
    if(UCB2IFG & UCNACKIFG)                         // Clear any NACK flags
    {
        errorcode = 1;
        UCB2IFG &= ~UCNACKIFG;                              // Clear any NACK flags due to non responding slave
    }
    _delay_cycles(15000);
    return temp;

}

int             ReadFrom24CxxEEPROMMultiBytes(unsigned char ChipAddr,unsigned char ChipType,unsigned short Addr,int BytesToRead, unsigned char *RxBucket)   // Reads a multiple bytes to provided Array pointer and checks unack condition
{

    volatile unsigned short addr = 0;               //if volatile is removed, then R12 is used for both vars.
    volatile unsigned char  MemLoc = 0,temp=0;      // 8bit memory location to be sent to the EEPROM. this will be LSB part once and MSB once
    volatile int errorcode = 0,var;                     //errorcodes: 0 = OK, 1 = Slave address not ACKNOWLEDGED
    UCB2CTL1 |= UCSWRST;                            // Enable SW reset
    UCB2CTL1 |= UCTR;                               // Enabling the Tx mode
    addr = ChipAddr;
    addr >>= 1;                                     // right shift once to adjust for address format in MSP
    UCB2I2CSA = addr;                               // Slave Address is fed
    UCB2CTL1 &= ~UCSWRST;                           // Release USCI
    UCB2CTL1 |= UCTXSTT;                            // I2C TX, start condition
    while(UCB2IFG & UCSTTIFG);                      // Wait till the STT bit is cleared after proper ack from the slave
    _delay_cycles(10000);                           // 10 ms delay, change this value when using XT2 or other freq.
    if((ChipType == 2)&&(!(UCB2IFG & UCNACKIFG)))
        {
            if(UCB2IFG & UCTXIFG)                   // TX flag is made high once the slave has acknowledged the address
                {
                    MemLoc = Addr;                  // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                    UCB2TXBUF = MemLoc;
                    UCB2IFG &= ~UCTXIFG;            // Clear USCI_B0 TX int flag
                    while(!(UCB2IFG & UCTXIFG));    //wait till data is written into the slave
                }
            UCB2CTL1 &= ~UCTR;                              // Enabling the Tx mode
            UCB2CTL1 |= UCTXSTT;                            // I2C TX, repeated start condition
            while(UCB2IFG & UCSTTIFG);                      // Wait till the STT bit is cleared after proper ack from the slave
            while(!(UCB2IFG & UCRXIFG));                    // Wait untill the RX Flag is set high by the 24Cxx, keep waiting <-- VVVIMP line
            for (var = 0; var < BytesToRead; ++var)
            {
                    if(UCB2IFG & UCRXIFG)                           // TX flag is made high once the slave has acknowledged the address
                        {
                            *RxBucket = UCB2RXBUF;
                            UCB2IFG &= ~UCRXIFG;                  // Clear USCI_B0 TX int flag
                            while(!(UCB2IFG & UCRXIFG));                //wait till data is written into the slave
                        }
                    RxBucket++;
            }

        }
    else if(!(UCB2IFG & UCNACKIFG))
        {
            if(UCB2IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
//                  MemLoc = Addr;              // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
//                  UCB2TXBUF = MemLoc;
//                  UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
//                  while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                addr = Addr;
                addr >>=8;                      // Shifts the upper part for second address cycle for 04, 08,16 to 256 flavors
                MemLoc = addr;              // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                UCB2TXBUF = MemLoc;
                _delay_cycles(20000);
                while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                }
            if(UCB2IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
//                  Addr >>=8;                      // Shifts the upper part for second address cycle for 04, 08,16 to 256 flavors
//                  MemLoc = Addr;              // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
//                  UCB2TXBUF = MemLoc;
//                  UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
//                  while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                addr = Addr;
                addr &=0xff;                        // Shifts the upper part for second address cycle for 04, 08,16 to 256 flavors
                MemLoc = addr;              // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                UCB2TXBUF = MemLoc;
                _delay_cycles(20000);
                while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                }
            UCB2CTL1 &= ~UCTR;                              // Enabling the Tx mode
            UCB2CTL1 |= UCTXSTT;                            // I2C TX, repeated start condition
            while(UCB2IFG & UCSTTIFG);                      // Wait till the STT bit is cleared after proper ack from the slave
            while(!(UCB2IFG & UCRXIFG));                    // Wait untill the RX Flag is set high by the 24Cxx, keep waiting <-- VVVIMP line
            for (var = 0; var < BytesToRead; ++var)
            {
                    if(UCB2IFG & UCRXIFG)                           // TX flag is made high once the slave has acknowledged the address
                        {
                            *RxBucket = UCB2RXBUF;
                            UCB2IFG &= ~UCRXIFG;                  // Clear USCI_B0 TX int flag
                            while(!(UCB2IFG & UCRXIFG));                //wait till data is written into the slave
                        }
                    *RxBucket++;
            }

        }

    UCB2CTL1 |= UCTXSTP;                            // I2C TX, stop condition
    while(UCB2IFG & UCSTPIFG);                      // Wait till the stop condition has been sent
    UCB2IFG &= ~UCTXIFG;                            // Clear any pending TX flags
    UCB2IFG &= ~UCRXIFG;                            // Clear any RX flags
    if(UCB2IFG & UCNACKIFG)                         // Clear any NACK flags
    {
        errorcode = 1;
        UCB2IFG &= ~UCNACKIFG;                              // Clear any NACK flags due to non responding slave
    }

    return errorcode;
}

////////////// RTC Routines ////////////////////////////////////////////////////////////

int          SetRTC(unsigned  char Addr, const unsigned char *Values, unsigned char BytesToWrite)
{
     unsigned short addr = 0;               //if  is removed, then R12 is used for both vars.
     unsigned char  MemLoc = 0;             // 8bit memory location to be sent to the EEPROM. this will be LSB part once and MSB once
     int errorcode = 0,var;                     //errorcodes: 0 = OK, 1 = Slave address not ACKNOWLEDGED
    UCB2CTL1 |= UCSWRST;                            // Enable SW reset
    UCB2CTL1 |=  UCTR;                              // Enabling the Tx mode
    addr = 0xD0;
    addr >>= 1;                                     // right shift once to adjust for address format in MSP
    UCB2I2CSA = addr;                               // Slave Address is fed
    UCB2CTL1 &= ~UCSWRST;                           // Release USCI

    UCB2CTL1 |= UCTXSTT;                            // I2C TX, start condition
    while(UCB2IFG & UCSTTIFG);                      // Wait till the STT bit is cleared after proper ack from the slave
    _delay_cycles(330000);                           // 10 ms delay, change this value when using XT2 or other freq.

    if(!(UCB2IFG & UCNACKIFG))
        {
            if(UCB2IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                    MemLoc = Addr;              // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                    UCB2TXBUF = MemLoc;
                    UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                    while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                }
            while(!(UCB2IFG & UCTXIFG));                    // Wait untill the TX Flag is set high by the 24Cxx, keep waiting <-- VVVIMP line
            for (var = 0; var < BytesToWrite; ++var)
                {
                    if(UCB2IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                        {
                            UCB2TXBUF = *Values;
 //                           UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                            _delay_cycles(50000);
                            while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                        }
                    Values++;
                }

        }

    UCB2CTL1 |= UCTXSTP;                            // I2C TX, stop condition
    while(UCB2IFG & UCSTPIFG);                      // Wait till the stop condition has been sent
    UCB2IFG &= ~UCTXIFG;                            // Clear any pending TX flags
    UCB2IFG &= ~UCRXIFG;                            // Clear any RX flags
    if(UCB2IFG & UCNACKIFG)                         // Clear any NACK flags
    {
        errorcode = 1;
        UCB2IFG &= ~UCNACKIFG;                              // Clear any NACK flags due to non responding slave
    }

    return errorcode ;

}

int             ReadFromRTCMultiBytes(unsigned short Addr,int BytesToRead, unsigned char *ReadTime)  // Reads a multiple bytes to provided Array pointer and checks unack condition
{   /*
    *   Enter the proper RTC address. Default given is 0xD0
    */

   static unsigned short addr = 0;               //if volatile is removed, then R12 is used for both vars.
   static unsigned char  MemLoc = 0,temp=0;      // 8bit memory location to be sent to the EEPROM. this will be LSB part once and MSB once
   static int errorcode = 0,var;                     //errorcodes: 0 = OK, 1 = Slave address not ACKNOWLEDGED
    UCB2CTL1 |= UCSWRST;                            // Enable SW reset
    UCB2CTL1 |= UCTR;                               // Enabling the Tx mode
    addr = 0xD0;
    addr >>= 1;                                     // right shift once to adjust for address format in MSP
    UCB2I2CSA = addr;                               // Slave Address is fed
    UCB2CTL1 &= ~UCSWRST;                           // Release USCI
    UCB2CTL1 |= UCTXSTT;                            // I2C TX, start condition
    while(UCB2IFG & UCSTTIFG);                      // Wait till the STT bit is cleared after proper ack from the slave
//    while(UCB2CTL1&UCTXSTT);
    _delay_cycles(330000);                           // 10 ms delay, change this value when using XT2 or other freq.
    if(!(UCB2IFG & UCNACKIFG))
        {
            if(UCB2IFG & UCTXIFG)                           // TX flag is made high once the slave has acknowledged the address
                {
                    MemLoc = Addr;              // 16 bit address format, regardless of the chip type. Only lower 8 bits needed for 24C02
                    UCB2TXBUF = MemLoc;
 //                   UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
                    while(!(UCB2IFG & UCTXIFG));                //wait till data is written into the slave
                }

            UCB2CTL1 &= ~UCTR;                              // Enabling the Tx mode
            UCB2CTL1 |= UCTXSTT;                            // I2C TX, repeated start condition
            while(UCB2IFG & UCSTTIFG);                      // Wait till the STT bit is cleared after proper ack from the slave
            while(!(UCB2IFG & UCRXIFG));                    // Wait untill the RX Flag is set high by the 24Cxx, keep waiting <-- VVVIMP line
            for (var = 0; var < BytesToRead; ++var)
            {
                    if(UCB2IFG & UCRXIFG)                           // TX flag is made high once the slave has acknowledged the address
                        {
                            *ReadTime = UCB2RXBUF;
                            UCB2IFG &= ~UCRXIFG;                  // Clear USCI_B0 TX int flag
                            while(!(UCB2IFG & UCRXIFG));                //wait till data is written into the slave
                        }
                    ReadTime++;
                    _delay_cycles(330000);                           // 10 ms delay, change this value when using XT2 or other freq.
            }

        }

    UCB2CTL1 |= UCTXSTP;                            // I2C TX, stop condition
    while(UCB2IFG & UCSTPIFG);                      // Wait till the stop condition has been sent
    UCB2IFG &= ~UCTXIFG;                            // Clear any pending TX flags
    UCB2IFG &= ~UCRXIFG;                            // Clear any RX flags
    if(UCB2IFG & UCNACKIFG)                         // Clear any NACK flags
    {
        errorcode = 1;
        UCB2IFG &= ~UCNACKIFG;                              // Clear any NACK flags due to non responding slave
    }

    return errorcode;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
//          interrupt routines to be placed here
//
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
/*
 *  USCI B2         ISR     FOR     I2C READ WRITE
 */
//------------------------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B2_VECTOR
__interrupt void USCI_B2_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B2_VECTOR))) USCI_B2_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCB2IV,12))
  {
  case  0:
  break;                           // Vector  0: No interrupts
  case  2:
  break;                           // Vector  2: ALIFG
  case  4:
      _delay_cycles(1);
      UCB2CTL1 |= UCTXSTP;                  // I2C stop condition
      UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
  break;                           // Vector  4: NACKIFG
  case  6:
      _delay_cycles(1);
  break;                           // Vector  6: STTIFG
  case  8:
      _delay_cycles(1);
  break;                           // Vector  8: STPIFG
  case 10:
      *RxData = UCB2RXBUF;

      RxData++;

      RxByteCounter --;
      if(RxByteCounter==0)
      {
          UCB2CTL1 |= UCTXSTP;                  // I2C stop condition
          UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
          while((UCB2CTL1 & UCTXSTP)== UCTXSTP);
          i2cStopCompleted = true;
      }

  break;                           // Vector 10: RXIFG
  case 12:                                  // Vector 12: TXIFG         i2cAddrCounter
    if ((i2cAddrCounter < 2)&&( AT24C02IntAddrMode == false))
    {
        UCB2TXBUF = i2cChipIntMemAddr[i2cAddrCounter];
        i2cAddrCounter++;
    }
    else if ((i2cAddrCounter < 1)&&( AT24C02IntAddrMode == true))
    {
        UCB2TXBUF = i2cChipIntMemAddr[i2cAddrCounter];
        i2cAddrCounter++;
    }
    else if(i2cRestart == true)
    {
          UCB2CTL1 &= ~UCTR;
          UCB2CTL1 |=UCTXSTT;
    }
      else if ((TxByteCounter > 0)&&(i2cReadMode == false))                          // Check TX byte counter
    {
      UCB2TXBUF =  *TxData; // *PTxData;               // Load TX buffer
      TxData++;
      TxByteCounter --;                          // Decrement TX byte counter

    }
      else if(i2cReadMode == false)
      {
          UCB2CTL1 |= UCTXSTP;                  // I2C stop condition
          UCB2IFG &= ~UCTXIFG;                  // Clear USCI_B0 TX int flag
          while((UCB2CTL1 & UCTXSTP)== UCTXSTP);
          i2cStopCompleted = true;
      }
//    else
//    {
//      UCB2CTL1 &= ~UCTR;
//      UCB2CTL1 |=UCTXSTT;
//    }
  default: break;
  }
}
