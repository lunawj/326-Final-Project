/*
 * TimerA.h
 *
 *  Created on: Oct 25, 2019
 *      Author: Adan
 */

#ifndef TIMERA_H_
#define TIMERA_H_

void TimerA_Initialization(void);
void setTimerA_Period(void);
void TA0_N_IRQHandler(void);
void USS_TRIG_Initialization(void);

#endif /* TIMERA_H_ */
