/*
 * RTC.c
 *
 *  Created on: Oct 18, 2019
 *      Author: Adan
 */

#include "msp.h"
#include <stdio.h>

/* Macros */
// #define SLAVE_ADDR 0x68     // 1101 000.    DS1337

/* Global Variables */

/*                                         sec   min   hr    day   date  mon  yr          */
extern unsigned char timeDateToSet[15] = {0x00, 0x01, 0x09, 0x02, 0x25, 0x10, 0x19, 0};
extern unsigned char timeDateReadback[7] = {0, 0, 0, 0, 0, 0, 0};
unsigned char tempReadback[2];

/* Function: configure UCB1 as I2C
 *    Input: none
 *   Output: none
 *   Source: TI MSP432 Arm programming for Embedded Systems
 */
void I2C1_Initialization(void)
{
    EUSCI_B1->CTLW0 |= 1;            /* disable UCB1 during config                           */
    EUSCI_B1->CTLW0 = 0x0F81;        /* 7-bit slave addr, master, I2C, synch mode, use SMCLK */
    EUSCI_B1->BRW = 30;              /* set clock prescaler 3MHz / 30 = 100kHz               */
    P6->SEL0 |= 0x30;                /* P6.5, P6.4 for UCB1                                  */
    P6->SEL1 &= ~0x30;
    EUSCI_B1->CTLW0 &= ~1;           /* enable UCB1 after config                             */
}

/* Function: Use burst write to write multiple bytes to consecutive locations
 *           burst write: S-(slaveAddr+w)-ACK-memAddr-ACK-data-ACK...-data-ACK-P
 *    Input: none
 *   Output: none
 *   Source: TI MSP432 Arm programming for Embedded Systems
 */
int I2C1_burstWrite(int slaveAddr, unsigned char memAddr, int byteCount, unsigned char* data)
{
    if (byteCount <= 0)
        return -1;              /* no write was performed */

    EUSCI_B1->I2CSA = slaveAddr;      /* setup slave address                   */
    EUSCI_B1->CTLW0 |= 0x0010;        /* enable transmitter                    */
    EUSCI_B1->CTLW0 |= 0x0002;        /* generate START and send slave address */
    while((EUSCI_B1->CTLW0 & 2));     /* wait until slave address is sent      */
    EUSCI_B1->TXBUF = memAddr;        /* send memory address to slave          */

    /* send data one byte at a time */
    do {
        while(!(EUSCI_B1->IFG & 2));  /* wait till it's ready to transmit      */
        EUSCI_B1->TXBUF = *data++;    /* send data to slave                    */
        byteCount--;
     } while (byteCount > 0);

    while(!(EUSCI_B1->IFG & 2));      /* wait till last transmit is done       */
    EUSCI_B1->CTLW0 |= 0x0004;        /* send STOP                             */
    while(EUSCI_B1->CTLW0 & 4) ;      /* wait until STOP is sent               */

    return 0;                         /* no error                              */
}
/* Function: Use burst read to read multiple bytes from consecutive locations
 *           read: S-(slaveAddr+w)-ACK-memAddr-ACK-R-(slaveAddr+r)-ACK-data-ACK-...-data-NACK-P
 *    Input: none
 *   Output: none
 *   Source: TI MSP432 Arm programming for Embedded Systems
 */
int I2C1_burstRead(int slaveAddr, unsigned char memAddr, int byteCount, unsigned char* data)
{
    if (byteCount <= 0)
        return -1;              /* no read was performed */

    EUSCI_B1->I2CSA = slaveAddr;       /* setup slave address                     */
    EUSCI_B1->CTLW0 |= 0x0010;         /* enable transmitter                      */
    EUSCI_B1->CTLW0 |= 0x0002;         /* generate START and send slave address   */
    while((EUSCI_B1->CTLW0 & 2));      /* wait until slave address is sent        */
    EUSCI_B1->TXBUF = memAddr;         /* send memory address to slave            */
    while(!(EUSCI_B1->IFG & 2));       /* wait till last transmit is done         */
    EUSCI_B1->CTLW0 &= ~0x0010;        /* enable receiver                         */
    EUSCI_B1->CTLW0 |= 0x0002;         /* generate RESTART and send slave address */
    while(EUSCI_B1->CTLW0 & 2);        /* wait till RESTART is finished           */

    /* receive data one byte at a time */
    do {
        if (byteCount == 1)            /* when only one byte of data is left      */
            EUSCI_B1->CTLW0 |= 0x0004; /* setup to send STOP after the last byte is received */

        while(!(EUSCI_B1->IFG & 1));   /* wait till data is received              */
        *data++ = EUSCI_B1->RXBUF;     /* read the received data                  */
        byteCount--;
    } while (byteCount);

    while(EUSCI_B1->CTLW0 & 4) ;       /* wait until STOP is sent                 */

    return 0;                          /* no error                                */
}
