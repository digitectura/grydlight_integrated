/*
 * s3_TH.h
 *
 *  Created on: 07-Jun-2020
 *      Author: BINDUSHREE M V
 */
/*********************************************************************************************************************/
#include "application/inc/S3_th.h"
#include "application/inc/app.h"
#include "application/inc/intervalTimer.h"
#include "application/inc/command_handler.h"
/*********************************************************************************************************************/
void fn_THprocess(void)
{
	static uint8_t thProcess_state = SEND_WAKUP_COMMAND;
	static uint16_t thProcess_WaitTimeStart = 0;
	static uint8_t thFailureCount = 0;
	switch(thProcess_state)
	{
		case SEND_WAKUP_COMMAND:
			i2c_txBuffer[0] = SHTC3_WAKE_CMND_MSB;
			i2c_txBuffer[1] = SHTC3_WAKE_CMND_LSB;
			if(fn_I2C_MasterWriteonly(ALS_TH_I2C_HANDLE, TH_WDEV_ADDR,i2c_txBuffer,2))
			{
				memset(i2c_txBuffer,'\0',I2C_TXBUFFER_SIZE);
				thProcess_WaitTimeStart = fn_GetmSecTimerStart();
				thProcess_state = WAIT_FOR_TH_STABLE_MODE;
				thFailureCount = 0;
			}
			else
			{
				thFailureCount++;
				if(thFailureCount > 5){
					DBG_PRINT("TH Sensor I2C Failure .. Disabling TH until power recycle\n");
					thFailureDisable = SEND_WAKUP_COMMAND_FAILED;
				}
			}
		break;
		case WAIT_FOR_TH_STABLE_MODE:
			if(fn_IsmSecTimerElapsed(thProcess_WaitTimeStart,20))
			{
				thProcess_state = ENABLE_TH_MEASUREMENT;
			}
		break;
		case ENABLE_TH_MEASUREMENT:
			i2c_txBuffer[0] = SHTC3_TH_MEASURMNT_CMND_MSB;
			i2c_txBuffer[1] = SHTC3_TH_MEASURMNT_CMND_LSB;
			if(fn_I2C_MasterWriteonly(ALS_TH_I2C_HANDLE, TH_WDEV_ADDR,i2c_txBuffer,2))
			{
				memset(i2c_txBuffer,'\0',I2C_TXBUFFER_SIZE);
				thProcess_WaitTimeStart = fn_GetmSecTimerStart();
				thProcess_state = READ_and_COMPUTE_TEMP_HUMIDITY;
				thFailureCount = 0;
			}
			else
			{
				thFailureCount++;
				if(thFailureCount > 5){
					DBG_PRINT("TH Sensor I2C Failure .. Disabling TH until power recycle\n");
					thFailureDisable = ENABLE_TH_MEASUREMENT_FAILED;
				}
			}
		break;
		case READ_and_COMPUTE_TEMP_HUMIDITY:
			if(fn_IsmSecTimerElapsed(thProcess_WaitTimeStart,50))
			{
				if(fn_I2C_MasterReadonly(ALS_TH_I2C_HANDLE, TH_RDEV_ADDR,i2c_rxBuffer,6))
				{

					snsrCurrStatus.temp 		= (i2c_rxBuffer[0] << 8) | i2c_rxBuffer[1];
					snsrCurrStatus.humidity 	= (i2c_rxBuffer[3] << 8) | i2c_rxBuffer[4];
					snsrCurrStatus.temp 		= CALC_SHTC3_TEMP(snsrCurrStatus.temp);
					snsrCurrStatus.humidity 	= CALC_SHTC3_HUMIDITY(snsrCurrStatus.humidity);
					memset(i2c_rxBuffer,'\0',I2C_RXBUFFER_SIZE);
					thProcess_state = CHECK_TH_STATUS_CHANGE;
					DBG_PRINT("/*...........................................*/\n");
					DBG_PRINT("currTmp = %d \t currHumid = %d \n",snsrCurrStatus.temp ,snsrCurrStatus.humidity);
					thFailureCount = 0;
				}
				else
				{
					thFailureCount++;
					if(thFailureCount > 5){
						DBG_PRINT("TH Sensor I2C Failure .. Disabling TH until power recycle\n");
						thFailureDisable = READ_and_COMPUTE_TEMP_HUMIDITY_FAILED;
					}
				}
			}
		break;
		case CHECK_TH_STATUS_CHANGE:
			if ( ((abs(snsrCurrStatus.temp  - snsrPrevStatus.temp)) >= snsrCfg.th_cfg.tempThreshold) ||
				 ((abs(snsrCurrStatus.humidity - snsrPrevStatus.humidity)) >= snsrCfg.th_cfg.humidityThreshold ) )
			{
				snsrPrevStatus = snsrCurrStatus;
				send_packet(TEMP_HUMIDITY,snsrMinCfg.dest_addr,false);
			}
			thProcess_WaitTimeStart = fn_GetSecTimerStart();
			thProcess_state = WAIT_FOR_NEXT_TH_READ;
		break;
		case WAIT_FOR_NEXT_TH_READ :
			if( fn_IsSecTimerElapsed ( thProcess_WaitTimeStart , snsrCfg.th_cfg.freq_THmeasure))
			{
				thProcess_state = ENABLE_TH_MEASUREMENT;
			}
		break;
		default:
		break;
	}

}
/*********************************************************************************************************************/
