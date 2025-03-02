/*
 * bmp180.c
 *
 *  Created on: Mar 2, 2025
 *      Author: Matheus Nifocci
 */

#include "stm32f4xx_hal.h"
#include "math.h"

extern I2C_HandleTypeDef hi2c1;

#define BMP180_ADDRESS 0xEE // device address

//******************* Calibration Data ***********************//
short AC1 = 0;
short AC2 = 0;
short AC3 = 0;
unsigned short AC4 = 0;
unsigned short AC5 = 0;
unsigned short AC6 = 0;
short B1 = 0;
short B2 = 0;
short MB = 0;
short MC = 0;
short MD = 0;
//***********************************************************//

long UT = 0;
short oss = 0;
long UP = 0;
long X1 = 0;
long X2 = 0;
long X3 = 0;
long B3 = 0;
long B5 = 0;
unsigned long B4 = 0;
long B6 = 0;
unsigned long B7 = 0;

long Press = 0;
long Temp = 0;

#define atmPress 101325 //Pa



void BMP180_Init (void)
{
	uint8_t call_data[22] = {0};
	uint16_t start_data = 0xAA;

	HAL_I2C_Mem_Read(&hi2c1, BMP180_ADDRESS, start_data, 1, call_data, 22, HAL_MAX_DELAY);

	AC1 = ((call_data[0]<<8)|call_data[1]);
	AC2 = ((call_data[2]<<8)|call_data[3]);
	AC3 = ((call_data[4]<<8)|call_data[5]);
	AC4 = ((call_data[6]<<8)|call_data[7]);
	AC5 = ((call_data[8]<<8)|call_data[9]);
	AC6 = ((call_data[10]<<8)|call_data[11]);
	B1 = ((call_data[12]<<8)|call_data[13]);
	B2 = ((call_data[14]<<8)|call_data[15]);
	MB = ((call_data[16]<<8)|call_data[17]);
	MC = ((call_data[18]<<8)|call_data[19]);
	MD = ((call_data[20]<<8)|call_data[21]);

}


uint16_t Read_UT_Value(void)
{
	uint8_t eddrwrite = 0x2E;
	uint8_t tempdata[2] = {0};
	HAL_I2C_Mem_Write(&hi2c1, BMP180_ADDRESS, 0xF4, 1, &eddrwrite, 1 , 1000);
	HAL_Delay(5);
	HAL_I2C_Mem_Read(&hi2c1, BMP180_ADDRESS, 0xF6, 1, tempdata, 2, 1000);
	return ((tempdata[0]<<8)+ tempdata[1]);

}

float Get_Temp_Value(void)
{
	UT = Read_UT_Value();
	X1 = ((UT-AC6) * (AC5/(pow(2,15))));
	X2 = ((MC*(pow(2,11))) / (X1+MD));
	B5 = X1+X2;
	Temp = (B5+8)/(pow(2,4));
	return Temp/10.0;

}

uint32_t Read_UP_Value(int oss)
{
	uint8_t datawrite = 0x34 +(oss<<6);
	uint8_t pressdata[3] ={0};
	HAL_I2C_Mem_Write(&hi2c1, BMP180_ADDRESS, 0x04, 1, &datawrite, 1, 1000);
	switch (oss) {
		case 0:
			HAL_Delay(5);
			break;
		case 1:
			HAL_Delay(8);
			break;
		case 2:
			HAL_Delay(14);
			break;

		case 3:
			HAL_Delay(26);
			break;

	}
	HAL_I2C_Mem_Read(&hi2c1, BMP180_ADDRESS, 0xF6, 1, pressdata, 3, 1000);
	return(((pressdata[0] <<16)+(pressdata[1]<<8)+pressdata[2])>>(8-oss));

}

float Get_Press_value(int oss)
{
	UP = Read_UP_Value(oss);
	X1 = ((UT-AC6) * (AC5/(pow(2,15))));
	X2 = ((MC*(pow(2,11))) / (X1+MD));
	B5 = X1+X2;
	B6 = B5-4000;
	X1 = (B2 * (B6*B6/(pow(2,12))))/(pow(2,11));
	X2 = AC2*B6/(pow(2,11));
	X3 = X1+X2;
	B3 = (((AC1*4+X3)<<oss)+2)/4;
	X1 = AC3*B6/pow(2,13);
	X2 = (B1 * (B6*B6/(pow(2,12))))/(pow(2,16));
	X3 = ((X1+X2)+2)/pow(2,2);
	B4 = AC4*(unsigned long)(X3+32768)/(pow(2,15));
	B7 = ((unsigned long)UP-B3)*(50000>>oss);
	if (B7<0x80000000) Press = (B7*2)/B4;
	else Press = (B7/B4)*2;
	X1 = (Press/(pow(2,8)))*(Press/(pow(2,8)));
	X1 = (X1*3038)/(pow(2,16));
	X2 = (-7357*Press)/(pow(2,16));
	Press = Press + (X1+X2+3791)/(pow(2,4));

	return Press;
}

float BMP180_GetAlt(int oss)
{
	Get_Press_value(oss);
	return 44330*(1-(pow((Press/(float)atmPress), 0.19029495718)));
}
