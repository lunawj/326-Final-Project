/*
 * Seven_Segment.c
 *
 *  Created on: Nov 30, 2019
 *      Author: Adan
 */
#include "msp.h"
#include "ST7735.h"

#define eUSCI_B_DISABLE BIT0
#define eUSCI_B_ENABLE  BIT0

/* --- SPI Ports and Pin ---- */
#define SS_PORT P10  // Old P1
#define DIN BIT2     // BIT6
#define CLK BIT1     // BIT5

#define CS_PORT P3  // Old P5
#define CHIP_  BIT6 // BIT0



void clear_7Seg(void);
/*
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Function: Sets up the DIN and CLK pins to be used for SPI
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *    Input: None
 *   Output: None
 *   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
void EUSCI_Initialization(void)
{
    // Sets Seven-segment to be used for SPI
    SS_PORT->SEL0 |=  (DIN | CLK);
    SS_PORT->SEL1 &= ~(DIN | CLK);

    SS_PORT->DIR |= (DIN);    // Set as output for communications
}


/*
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Function: Sets the Chip port and pin to interface with MSP
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *    Input: None
 *   Output: None
 *   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
void CHIP_SELECT_Initialization(void)
{
    // Set Chip-select port for GPIO
    CS_PORT->SEL0 &= ~(CHIP_);
    CS_PORT->SEL1 &= ~(CHIP_);

    CS_PORT->DIR |=  (CHIP_); // Set Chip-select pin as output for toggling
    CS_PORT->OUT &= ~(CHIP_);  // Default CS pin as output low
}

/*
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Function: Sets up the protocol for SPI
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *    Input: None
 *   Output: None
 *   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
void set_SPI_Protocol(void)
{
    EUSCI_B3->CTLW0 = 0x0001;     /* disable UCB0 during config */
    /* clock phase/polarity:11, MSB first, 8-bit, master, 3-pin SPI,
       synchronous mode, use SMCLK as clock source */
    EUSCI_B3->CTLW0 = 0xE9C1;
    EUSCI_B3->BRW = 30;            /* 3 MHz / 1 = 3 MHz */
    EUSCI_B3->CTLW0 &= ~0x0001;   /* enable UCB0 after config */
}


/*
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Function: Sends data to the device
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *    Input: address: address wish to write data to
 *             value: data you want to transmit
 *   Output: None
 *   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
void send16BIT_Data(uint8_t address, uint8_t value)
{
    // Set chip-select low
    CS_PORT->OUT &= ~(CHIP_);

    while(!(EUSCI_B3->IFG & 2));    // waits until ready to transmit
    EUSCI_B3->TXBUF = address;
    while(EUSCI_B3->STATW & 1) ;      /* wait for transmit done */


    while(!(EUSCI_B3->IFG & 2));    // waits until ready to transmit
    EUSCI_B3->TXBUF = value;
    while(EUSCI_B3->STATW & 1) ;      /* wait for transmit done */

    Delay1ms(1);
    CS_PORT->OUT |=  (CHIP_);
    Delay1ms(1);
    CS_PORT->OUT &= ~(CHIP_);
}


/*
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Function: Clears all 7-segment digits to 0
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *    Input: None
 *   Output: None
 *   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
void clear_7Seg(void)
{
    send16BIT_Data(0x01, 0);
    send16BIT_Data(0x02, 0);
    send16BIT_Data(0x03, 0);
    send16BIT_Data(0x04, 0);
    send16BIT_Data(0x05, 0);
    send16BIT_Data(0x06, 0);
    send16BIT_Data(0x07, 0);
    send16BIT_Data(0x08, 0);
}
/*
 *  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * Function: Initial writes to the 7-segment to allow it to work
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *    Input: None
 *   Output: None
 *   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
void setup_7Seg(void)
{
    //Decode Mode
    send16BIT_Data(0x09, 0xFF);
    // Intensity
    send16BIT_Data(0x0A, 3);
    // Scan Limit
    send16BIT_Data(0x0B, 7);
    // Disable Shutdown
    send16BIT_Data(0x0C, 1);
    // Clear 7-segment display to all 0's
    clear_7Seg();
}
