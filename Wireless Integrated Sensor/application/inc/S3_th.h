/*
 * s3_TH.h
 *
 *  Created on: 07-Jun-2020
 *      Author: BINDUSHREE M V
 */
/*********************************************************************************************************************/
#ifndef APPLICATION_INC_S3_TH_H_
#define APPLICATION_INC_S3_TH_H_
/*********************************************************************************************************************/
#include "i2c.h"
#include "math.h"
/*********************************************************************************************************************/
#define SHTC3_WAKE_CMND_LSB			0x17
#define SHTC3_WAKE_CMND_MSB			0x35

#define SHTC3_SLEEP_CMND_LSB			0x98
#define SHTC3_SLEEP_CMND_MSB			0xB0

#define SHTC3_REG_ID_CMND_BYTE1		0xC8
#define SHTC3_REG_ID_CMND_BYTE2		0xEF

#define SHTC3_TH_MEASURMNT_CMND_MSB		0x7C
#define SHTC3_TH_MEASURMNT_CMND_LSB		0xA2

#define TH_WDEV_ADDR		0xE0										//shtc3 I2C device address to write 0x70
#define TH_RDEV_ADDR		0xE1

#define CALC_SHTC3_TEMP(sensorTempData)			( (175 * (sensorTempData /65536.0) ) - 45 ) * 100
#define CALC_SHTC3_HUMIDITY(sensorHumidityData)   ( (100 * (sensorHumidityData /65536.0) ))*100
/*********************************************************************************************************************/
enum
{
	SEND_WAKUP_COMMAND ,
	WAIT_FOR_TH_STABLE_MODE ,
	ENABLE_TH_MEASUREMENT ,
	READ_and_COMPUTE_TEMP_HUMIDITY ,
	CHECK_TH_STATUS_CHANGE ,
	CHECK_TH_STATUS_UPDATE ,
	WAIT_FOR_NEXT_TH_READ
};
/*********************************************************************************************************************/
void fn_THprocess(void);
/*********************************************************************************************************************/
#endif /* APPLICATION_INC_S3_TH_H_ */
/*********************************************************************************************************************/
