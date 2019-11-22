/*
 * MotorMovements.h
 *
 *  Created on: Nov 14, 2018
 *      Author: Wesley
 */

#ifndef MOTORMOVEMENTS_H_
#define MOTORMOVEMENTS_H_


#define MOTOR P4
#define IN1 BIT0
#define IN2 BIT1
#define IN3 BIT2
#define IN4 BIT3



void Motor_Initialization(void);
void wave(int dir);
void halfStep(int dir);
void fullStep(int dir);

static uint8_t state = 0b1000;

void Motor_Initialization(void)
{
    // Sets motor driver pins to be used as GPIO
    MOTOR->SEL0 &= ~(IN1 | IN2 | IN3 | IN4);
    MOTOR->SEL1 &= ~(IN1 | IN2 | IN3 | IN4);
    // Sets the direction of the pins to output
    MOTOR->DIR  |=  (IN1 | IN2 | IN3 | IN4);
}

void wave(int dir)
{
    MOTOR->OUT &= ~(IN1 | IN2 | IN3 | IN4);

	switch (state)
	{
		case 0b1000:
			if(dir == 1)
			{
				state = 0b0100;
			}else if(dir == 0)
			{
				state = 0b0001;
			}
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
		
		case 0b0100:
			if(dir == 1)
			{
				state = 0b0010;
			}else if(dir == 0)
			{
				state = 0b1000;
			}
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
		break;
		
		case 0b0010:
			if(dir == 1)
			{
				state = 0b0001;
			}else if(dir == 0)
			{
				state = 0b0100;
			}
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
		
		case 0b0001:
			if(dir == 1)
			{
				state = 0b1000;
			}else if(dir == 0)
			{
				state = 0b0010;
			}
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
		default:
			state = 0b1000;
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
	}
}

void halfStep(int dir)
{
    MOTOR->OUT &= ~(IN1 | IN2 | IN3 | IN4);
	switch (state)
	{
		case 0b1000:
			if(dir == 1)
			{
				state = 0b1100;
			}else if(dir == 0)
			{
				state = 0b1001;
			}
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
			
		case 0b1100:
			if(dir == 1)
			{
				state = 0b0100;
			}else if(dir == 0)
			{
				state = 0b1000;
			}
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
			
		case 0b0100:
			if(dir == 1)
			{
				state = 0b0110;
			}else if(dir == 0)
			{
				state = 0b1100;
			}
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
			
		case 0b0110:
			if(dir == 1)
			{
				state = 0b0010;
			}else if(dir == 0)
			{
				state = 0b0100;
			}
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
			
		case 0b0010:
			if(dir == 1)
			{
				state = 0b0011;
			}else if(dir == 0)
			{
				state = 0b0110;
			}
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
			
		case 0b0011:
			if(dir == 1)
			{
				state = 0b0001;
			}else if(dir == 0)
			{
				state = 0b0010;
			}
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
			
		case 0b0001:
			if(dir == 1)
			{
				state = 0b1001;
			}else if(dir == 0)
			{
				state = 0b0011;
			}
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
			
		case 0b1001:
			if(dir == 1)
			{
				state = 0b1000;
			}else if(dir == 0)
			{
				state = 0b0001;
			}
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
			
		default:
			state = 0b1000;
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
	}
}


void fullStep(int dir)
{
    MOTOR->OUT &= ~(IN1 | IN2 | IN3 | IN4);
	switch (state)
	{
		case 0b1100:
			if(dir == 1)
			{
				state = 0b0110;
			}else if(dir == 0)
			{
				state = 0b1001;
			}
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
		
		case 0b0110:
			if(dir == 1)
			{
				state = 0b0011;
			}else if(dir == 0)
			{
				state = 0b1100;
			}
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
		break;
		
		case 0b0011:
			if(dir == 1)
			{
				state = 0b1001;
			}else if(dir == 0)
			{
				state = 0b0110;
			}
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
		
		case 0b1001:
			if(dir == 1)
			{
				state = 0b1100;
			}else if(dir == 0)
			{
				state = 0b0011;
			}
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
		default:
			state = 0b1100;
			MOTOR->OUT |= (IN1 | IN2 | IN3 | IN4) & state;
			break;
	}
}



#endif /*MOTORMOVEMENTS_H_*/
