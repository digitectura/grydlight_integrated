/****************************************************************************************************************/
#ifndef APPLICATION_ALS_ALS_H_
#define APPLICATION_ALS_ALS_H_
/****************************************************************************************************************/
#include "i2c.h"
#include "main.h"
/****************************************************************************************************************/
#define ALS_WDEV_ADDR				0x52<<1
#define ALS_RDEV_ADDR				ALS_WDEV_ADDR|0x01

#define MAIN_CTRL_REG				0x00
#define ALS_EN						0x02
#define ALS_DATA_REG				0x0D
#define ALS_PART_ID					0x06
#define ALS_GAIN_6					0x02
#define ALS_GAIN_9					0x03
#define APDS_9306_PART_ID  			0xB1
#define APDS_9306_065_PART_ID 		0xB3
/*******************************************************************************************************************/
enum
{
	INIT_ALS_SENSOR_PROCESS ,
	ENABLE_ALS_CONVERSIONS ,
	READ_ALS_DATA ,
	COMPUTE_ALS_DATA ,
	CHECK_STATUS_CHANGE ,
	CHECK_STATUS_UPDATE ,
	WAIT_FOR_NEXT_READ
};

#define INIT_ALS_SENSOR_PROCESS_FAILED		0xFFFF
#define ENABLE_ALS_CONVERSIONS_FAILED		0xFFFE
#define READ_ALS_DATA_FAILED				0xFFFD
#define COMPUTE_ALS_DATA_FAILED				0xFFFC
/*******************************************************************************************************************/
void fn_ALSprocess(void);
/*******************************************************************************************************************/
#endif /* APPLICATION_ALS_ALS_H_ */
