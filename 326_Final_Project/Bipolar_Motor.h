/*
 * Bipolar_Motor.h
 *
 *  Created on: Nov 8, 2019
 *      Author: Adan
 */

#ifndef BIPOLAR_MOTOR_H_
#define BIPOLAR_MOTOR_H_

#include "ST7735.h"

#define MOTOR P4
#define A BIT0
#define Aprime BIT1
#define B BIT2
#define Bprime BIT3

#define FORWARD 0
#define REVERSE 1

void BiMotor_Initialization(void);

void BiMotor_Initialization(void)
{
    // Sets motor driver pins to be used as GPIO
    MOTOR->SEL0 &= ~(A | B | Aprime | Bprime);
    MOTOR->SEL1 &= ~(A | B | Aprime | Bprime);
    // Sets the direction of the pins to output
    MOTOR->DIR  |=  (A | B | Aprime | Bprime);
}

void full_step(int direction, int steps)
{
    int i; // Used as control variable in for loop

    MOTOR->OUT &= ~(A | B | Aprime | Bprime); // Turn off motor coils

    for(i = 0; i < steps; i++)
    {
        if(direction == FORWARD)
        {
            MOTOR->OUT |= (A | B);
            MOTOR->OUT &= ~(Aprime | Bprime);
            Delay1ms(10);
            MOTOR->OUT |= (B | Aprime);
            MOTOR->OUT &= ~(A | Bprime);
            Delay1ms(10);
            MOTOR->OUT |= (Aprime | Bprime);
            MOTOR->OUT &= ~(A | B);
            Delay1ms(10);
            MOTOR->OUT |= (Bprime | A);
            MOTOR->OUT &= ~(B | Aprime);
            Delay1ms(10);
        }
        else if(direction == REVERSE)
        {
            MOTOR->OUT |= (Bprime | A);
            MOTOR->OUT &= ~(B | Aprime);
            Delay1ms(10);
            MOTOR->OUT |= (Aprime | Bprime);
            MOTOR->OUT &= ~(A | B);
            Delay1ms(10);
            MOTOR->OUT |= (B | Aprime);
            MOTOR->OUT &= ~(A | Bprime);
            Delay1ms(10);
            MOTOR->OUT |= (A | B);
            MOTOR->OUT &= ~(Aprime | Bprime);
            Delay1ms(10);
        }
    }


}



//for(i = 0; i < (cycles / 4); i++)
//{
//    if(direction == REVERSE)
//    {
//        MOTOR->OUT |= (A | B);
//        MOTOR->OUT &= ~(Aprime | Bprime);
//        delayMilli(10);
//        MOTOR->OUT |= (B | Aprime);
//        MOTOR->OUT &= ~(A | Bprime);
//        delayMilli(10);
//        MOTOR->OUT |= (Aprime | Bprime);
//        MOTOR->OUT &= ~(A | B);
//        delayMilli(10);
//        MOTOR->OUT |= (Bprime | A);
//        MOTOR->OUT &= ~(B | Aprime);
//        delayMilli(10);
//    }
//    else if(direction == FORWARD)
//    {
//        MOTOR->OUT |= (Bprime | A);
//        MOTOR->OUT &= ~(B | Aprime);
//        delayMilli(10);
//        MOTOR->OUT |= (Aprime | Bprime);
//        MOTOR->OUT &= ~(A | B);
//        delayMilli(10);
//        MOTOR->OUT |= (B | Aprime);
//        MOTOR->OUT &= ~(A | Bprime);
//        delayMilli(10);
//        MOTOR->OUT |= (A | B);
//        MOTOR->OUT &= ~(Aprime | Bprime);
//        delayMilli(10);
//    }
//}

#endif /* BIPOLAR_MOTOR_H_ */
