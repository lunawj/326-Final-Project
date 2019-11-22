/*
 * Rotary_Encoder.h
 *
 *  Created on: Oct 26, 2019
 *      Author: Adan
 */

#ifndef ROTARY_ENCODER_H_
#define ROTARY_ENCODER_H_

// Public Functions
void rotary_encoder_initialization(void);
void RE_setInterrupts(void);
void PORT2_IRQHandler(void);
void get_CLK_DT_values(void);
void getCount(void);

#endif /* ROTARY_ENCODER_H_ */
