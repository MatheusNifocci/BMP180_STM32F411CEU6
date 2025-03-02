/*
 * bmp180.h
 *
 *  Created on: Mar 2, 2025
 *      Author: Matheus Nifocci
 */

#ifndef INC_BMP180_H_
#define INC_BMP180_H_

#include "stm32f4xx_hal.h"

void BMP180_Init (void); // read the Calibration coefficients

uint16_t Read_UT_Value(void); // read the uncompensated temperature value

float Get_Temp_Value(void); // calculate the temperature value

uint32_t Read_UP_Value(int oss); // read the uncompensated pressure value

float Get_Press_value(int oss); // calculate the temperature value

float BMP180_GetAlt(int oss); // calculate the altitude value


#endif /* INC_BMP180_H_ */
