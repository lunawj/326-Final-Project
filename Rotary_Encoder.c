/*
 * Rotary_Encoder.c
 *
 *  Created on: Oct 26, 2019
 *      Author: Adan
 */
#include "msp.h"

// Macros used for Rotary Encoder
#define RE_PORT P2
#define CLK BIT4
#define DT  BIT5
#define SW  BIT6

#define TRUE  1
#define FALSE 0
#define CLEAR 0

// Global variables
int cw_temp  = 0;    // Used to temporarily store cw direction
int ccw_temp = 0;    // Used to temporarily store ccw direction
extern int cw  = 0;  // Used to store cw count
extern int ccw = 0;  // Used to store ccw count

uint8_t current;
uint8_t    last;
uint8_t CW_FLAG  = FALSE;
uint8_t CCW_FLAG = FALSE;

volatile uint8_t  clk = 0;
volatile uint8_t   dt = 0;
extern volatile uint8_t REFLAG = FALSE;


/**********************************************************************
 *   Function: Initializes pins to interface Rotary Encoder with MSP432
 *      Input: none
 *     Output: none
 *********************************************************************/
void rotary_encoder_initialization(void)
{
    // Set Rotary Encoder pins to GPIO
    RE_PORT->SEL0 &= ~(CLK | DT | SW);
    RE_PORT->SEL1 &= ~(CLK | DT | SW);
    // Set direction of pins as inputs
    RE_PORT->DIR  &= ~(CLK | DT | SW);
    // Enable pull-up resistor for switch
    RE_PORT->REN  |= SW;
    RE_PORT->OUT  |= SW;
}


/**********************************************************************
 *   Function: Enables interrupts for CLK and DT pins of Rotary Encoder
 *      Input: none
 *     Output: none
 *********************************************************************/
void RE_setInterrupts(void)
{
    // Enable interrupts for CLK and DT
    RE_PORT->IE  |=  (CLK | DT | SW);
    // Sets flag when detecting a low-to-high transition
    RE_PORT->IES &= ~(CLK | DT);
    // Clear all Rotary encoders' pins flag.
    RE_PORT->IFG  = 0;
}


/**********************************************************************
 *   Function: Gets initial conditions for CLK and DT pins
 *      Input: none
 *     Output: none
 *********************************************************************/
void get_CLK_DT_values(void)
{
    // Gets initial conditions for CLK and DT
    clk = ( (RE_PORT->IN & CLK) == CLK);
    dt  = ( (RE_PORT->IN & DT ) ==  DT);

    last = 2 * dt + clk; // saves current state in grey scale

    // Sets interrupts to look for a rising or falling edge based on initial conditions
    if(clk)
        RE_PORT->IES |=  CLK; // high-to-low transition
    else
        RE_PORT->IES &= ~CLK; // low-to-high transition

    if(dt)
        RE_PORT->IES |=  DT; // high-to-low transition
    else
        RE_PORT->IES &= ~DT; // low-to-high transition
}

/**********************************************************************
 *   Function: Port 2 interrupt handler
 *      Input: none
 *     Output: none
 *********************************************************************/
void PORT2_IRQHandler(void)
{
    // Flag will be set when detecting a button press from R.E.
    if(RE_PORT->IFG & BIT6)
        REFLAG = TRUE;

    // Gets current state of CLK and DT
    clk = ( (RE_PORT->IN & CLK) == CLK);
    dt  = ( (RE_PORT->IN & DT ) ==  DT);

    // Sets interrupts to look for a rising or falling edge based on current state
    if(clk)
        RE_PORT->IES |=  CLK;
    else
        RE_PORT->IES &= ~CLK;

    if(dt)
        RE_PORT->IES |=  DT;
    else
        RE_PORT->IES &= ~DT;
    getCount();
    RE_PORT->IFG = CLEAR;
}

/**********************************************************************
 *   Function: Gets the number of times the R.E. was turned either
 *             clockwise or counter-clockwise
 *      Input: none
 *     Output: none
 *********************************************************************/
void getCount(void)
{
    // Gets current state for grey code
    current = 2 * dt + clk;

    if((current == 1 && last == 0) || (current == 3 && last == 1) || (current == 2 && last == 3) || (current == 0 && last == 2))
    {
        cw_temp++;
        CW_FLAG = TRUE;
    }

    if((current == 2 && last == 0) || (current == 3 && last == 2) || (current == 1 && last == 3) || (current == 0 && last == 1))
    {
        ccw_temp++;
        CCW_FLAG = TRUE;
    }

    // For every 2 cw interrupts, it will increment cw by 1
    if((cw_temp % 2) == 0 && CW_FLAG)
    {
        cw++;
        CW_FLAG = FALSE;
    }

    // For every 2 ccw interrupts, it will increment cw by 1
    if((ccw_temp % 2) == 0 && CCW_FLAG)
    {
        ccw++;
        CCW_FLAG = FALSE;
    }

    // Store the 'current' state as the 'last' state
    last = current;
}
