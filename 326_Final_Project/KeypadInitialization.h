/*
 * KeypadInitialization.h
 *
 *  Created on: Nov 14, 2018
 *      Author: Wesley
 */

#ifndef KEYPADINITIALIZATION_H_
#define KEYPADINITIALIZATION_H_

/*
KEYPAD PINS
ROW0 P4.0
ROW1 P4.1
ROW2 P4.2
ROW3 P4.3
COL0 P4.4
COL1 P4.5
COL2 P4.6
*/


/*----------------------------------------------------------------
 * void initKeypad()
 *
 * Description: initializes the keypad
 * Inputs: None
 * Outputs: None
----------------------------------------------------------------*/
void initKeypad()
{
    P4->SEL0 &= ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6);
    P4->SEL1 &= ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6);
    P4->DIR &= ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6);
    P4->REN |= (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6);
    P4->OUT |= (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6);
}


/*----------------------------------------------------------------
 * void resetPins()
 *
 * Description: resets keypad pins to inputs and internal resistor to high
 * Inputs: None
 * Outputs: None
----------------------------------------------------------------*/
void resetPins()
{
    P4->DIR &= ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6);
    P4->REN |= (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6);
    P4->OUT |= (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6);
}

/*----------------------------------------------------------------
 * void readKeypad()
 *
 * Description: reads input from the keypad
 * Inputs: None
 * Outputs: None
----------------------------------------------------------------*/
int readKeypad()
{
    int reading = 0;
    resetPins();
    //disableKeypadInterrupts();

    //set column 0 to output and set it to low
    P4->DIR |= BIT4;
    P4->OUT &= ~(BIT4);

    //check all rows to see if it's pressed
    if(!(P4->IN & BIT0)){
        reading = 1;
        while(!(P4->IN & BIT0)){}
    }else if(!(P4->IN & BIT1)){
        reading = 4;
        while(!(P4->IN & BIT1)){}
    }else if(!(P4->IN & BIT2)){
        reading = 7;
        while(!(P4->IN & BIT2)){}
    }else if(!(P4->IN & BIT3)){
        reading = 10;
        while(!(P4->IN & BIT3)){}
    }

    resetPins();

    //set column 1 to output and set it to low
    P4->DIR |= BIT5;
    P4->OUT &= ~(BIT5);

    //check all rows to see if it's pressed
    if(!(P4->IN & BIT0)){
        reading = 2;
        while(!(P4->IN & BIT0)){}
    }else if(!(P4->IN & BIT1)){
        reading = 5;
        while(!(P4->IN & BIT1)){}
    }else if(!(P4->IN & BIT2)){
        reading = 8;
        while(!(P4->IN & BIT2)){}
    }else if(!(P4->IN & BIT3)){
        reading = 11;
        while(!(P4->IN & BIT3)){}
    }

    resetPins();

    //set column 2 to output and set it to low
    P4->DIR |= BIT6;
    P4->OUT &= ~(BIT6);

    //check all rows to see if it's pressed
    if(!(P4->IN & BIT0)){
        reading = 3;
        while(!(P4->IN & BIT0)){}
    }else if(!(P4->IN & BIT1)){
        reading = 6;
        while(!(P4->IN & BIT1)){}
    }else if(!(P4->IN & BIT2)){
        reading = 9;
        while(!(P4->IN & BIT2)){}
    }else if(!(P4->IN & BIT3)){
        reading = 12;
        while(!(P4->IN & BIT3)){}
    }

    return reading;
}

#endif /*KEYPADINITIALIZATION_H_*/