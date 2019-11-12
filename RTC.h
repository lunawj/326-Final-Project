/*
 * RTC.h
 *
 *  Created on: Oct 18, 2019
 *      Author: Adan
 */

#ifndef RTC_H_
#define RTC_H_

void I2C1_Initialization(void);
int I2C1_burstWrite(int slaveAddr, unsigned char memAddr, int byteCount, unsigned char* data);
int I2C1_burstRead(int slaveAddr, unsigned char memAddr, int byteCount, unsigned char* data);

#endif /* RTC_H_ */
