/****************************************************************************************************************/
#include "application/inc/S2_als.h"
#include "application/inc/intervalTimer.h"
#include "application/inc/app.h"
#include "application/inc/command_handler.h"
/****************************************************************************************************************/
#define  CALIBRATE(luxData)						(((1275)*luxData) - (1280)) / 10000
//*******************************************************************************************************************//
void fn_ALSprocess(void)
{
	static uint8_t alsProcess_state = INIT_ALS_SENSOR_PROCESS;
	static uint16_t alsProcess_WaitTimeStart = 0;
	static uint8_t alsFailureCount = 0;
	switch(alsProcess_state)
	{
		case INIT_ALS_SENSOR_PROCESS:
			i2c_txBuffer[0] = 0x5;
			i2c_txBuffer[1] = 0x04;
			if ( fn_I2C_MasterWriteonly(ALS_TH_I2C_HANDLE, ALS_WDEV_ADDR,i2c_txBuffer,2) )
			{
				memset(i2c_txBuffer,'\0',I2C_TXBUFFER_SIZE);
				alsProcess_state = ENABLE_ALS_CONVERSIONS;
				alsFailureCount = 0;
			}
			else
			{
				alsFailureCount++;
				if(alsFailureCount > 5){
					DBG_PRINT("ALS Sensor I2C Failure .. Disabling ALS until power recycle\n");
					alsFailureDisable = INIT_ALS_SENSOR_PROCESS_FAILED;
				}
			}
		break;
		case ENABLE_ALS_CONVERSIONS:
			i2c_txBuffer[0] = MAIN_CTRL_REG;
			i2c_txBuffer[1] = ALS_EN;
			if ( fn_I2C_MasterWriteonly(ALS_TH_I2C_HANDLE, ALS_WDEV_ADDR,i2c_txBuffer,2) )
			{
				memset(i2c_txBuffer,'\0',I2C_TXBUFFER_SIZE);
				alsProcess_state = READ_ALS_DATA;
				alsFailureCount = 0;
			}
			else
			{
				alsFailureCount++;
				if(alsFailureCount > 5){
					DBG_PRINT("ALS Sensor I2C Failure .. Disabling ALS until power recycle\n");
					alsFailureDisable = ENABLE_ALS_CONVERSIONS_FAILED;
				}
			}
		break;

		case READ_ALS_DATA:
			i2c_txBuffer[0] = ALS_DATA_REG;
			if ( fn_I2C_MasterWriteonly(ALS_TH_I2C_HANDLE, ALS_WDEV_ADDR,i2c_txBuffer,1) )
			{
				memset(i2c_txBuffer,'\0',I2C_TXBUFFER_SIZE);
				memset(i2c_rxBuffer,'\0',I2C_RXBUFFER_SIZE);
				alsProcess_state = COMPUTE_ALS_DATA;
				alsFailureCount = 0;
			}
			else
			{
				alsFailureCount++;
				if(alsFailureCount > 5){
					DBG_PRINT("ALS Sensor I2C Failure .. Disabling ALS until power recycle\n");
					alsFailureDisable = READ_ALS_DATA_FAILED;
				}
			}
		break;

		case COMPUTE_ALS_DATA:
			if (fn_I2C_MasterReadonly(ALS_TH_I2C_HANDLE, ALS_RDEV_ADDR,i2c_rxBuffer,3))
			{
				snsrCurrStatus.als_LUXvalue = (i2c_rxBuffer[1]<<8);
				snsrCurrStatus.als_LUXvalue |= i2c_rxBuffer[0];
				snsrCurrStatus.als_LUXvalue  = CALIBRATE(snsrCurrStatus.als_LUXvalue);
				snsrCurrStatus.als_LUXvalue  = (snsrCurrStatus.als_LUXvalue * snsrCfg.als_cfg.calibration_factor)/100;
				memset(i2c_rxBuffer,'\0',I2C_RXBUFFER_SIZE);
				DBG_PRINT("/*...........................................*/\n");
				MAN_PRINT("currLUX = %d\n",snsrCurrStatus.als_LUXvalue);

				snsrCurrStatus.als_LUXvalue = sAlsCalibValue.m*snsrCurrStatus.als_LUXvalue+sAlsCalibValue.c;
				MAN_PRINT("Calib Val = %d\r\n", snsrCurrStatus.als_LUXvalue);


				alsProcess_state = CHECK_STATUS_CHANGE;
				alsFailureCount = 0;
			}
			else
			{
				alsFailureCount++;
				if(alsFailureCount > 5){
					DBG_PRINT("ALS Sensor I2C Failure .. Disabling ALS until power recycle\n");
					alsFailureDisable = COMPUTE_ALS_DATA_FAILED;
				}
			}
		break;

		case CHECK_STATUS_CHANGE :
			if(abs(snsrCurrStatus.als_LUXvalue - snsrPrevStatus.als_LUXvalue) >= snsrCfg.als_cfg.luxThreshold)
			{
				DBG_PRINT("/*...........................................*/\n");
				DBG_PRINT("currLUX = %d\n",snsrCurrStatus.als_LUXvalue);
				printf("Calib Val = %d\r\n", snsrCurrStatus.als_LUXvalue);
				DBG_PRINT("/*...........................................*/\n");

				snsrPrevStatus.als_LUXvalue = snsrCurrStatus.als_LUXvalue;
				send_packet(ALS,snsrMinCfg.dest_addr,false);
			}
			alsProcess_WaitTimeStart = fn_GetSecTimerStart();
			alsProcess_state = WAIT_FOR_NEXT_READ;
		break;

		case WAIT_FOR_NEXT_READ :
			if(fn_IsSecTimerElapsed (alsProcess_WaitTimeStart , snsrCfg.als_cfg.freq_LUXmeasure_s ))
			{
				alsProcess_state = READ_ALS_DATA;
				alsFailureCount = 0;
			}
		break;
		default:
		break;
	}
	return;
}
//*******************************************************************************************************************//
