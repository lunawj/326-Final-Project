/*
 * Seven_Segment.h
 *
 *  Created on: Nov 30, 2019
 *      Author: Adan
 */

#ifndef SEVEN_SEGMENT_H_
#define SEVEN_SEGMENT_H_

void EUSCI_Initialization(void);
void CHIP_SELECT_Initialization(void);
void set_SPI_Protocol(void);
void send16BIT_Data(uint8_t address, uint8_t value);
void setup_7Seg(void);

#endif /* SEVEN_SEGMENT_H_ */
