#include <stdio.h>

/* Bluetooth stack headers */
#include <bg_types.h>
#include <native_gecko.h>

#include <board_features.h>
#include "application/inc/BLE_Queue.h"
#include <application/inc/S2_als.h>
#include <application/inc/S1_pir.h>
#include <application/inc/S3_th.h>
#include <main.h>
#include "application/inc/gslink/GS_Link.h"
#include "application/inc/intervalTimer.h"
#include "application/inc/dali/daliCommand.h"
#include "application/inc/analog/mcp45hvx1.h"
#include "em_rtcc.h"
#include "udelay.h"
//struct_DaliPercent sDaliPercent;


#ifdef ALS_TEST
uint16_t MaxLuxOnTable = 600;
uint8_t Prev_mLevel = 0;
int16_t Curr_mLevel = 1;
#endif

void als_debug_sendData(void);

void send_packet(Data_Cmd_t data_cmd_type,uint16_t destAddr,uint8_t isReq)
{
	uint8_t m_packet_size = 0;
	sensorpayload_t  m_data;
//	static uint8_t m_level = 1;

	switch (data_cmd_type)
	{
		case PIR:
		{
			if(!isReq && !(snsrAppData.WL_LIGHT_RcvdExternalCmd_flag || snsrAppData.WL_LIGHT_CONTROL_2_flag))
			{
								Curr_mLevel = ((((sAlsCalibValue.req_lux - snsrCurrStatus.als_LUXvalue)*100)/MaxLuxOnTable) + Prev_mLevel);
								Curr_mLevel = ((Curr_mLevel > 10) ? Curr_mLevel : 0);
								Curr_mLevel = ((Curr_mLevel > 100) ? 100 : Curr_mLevel);
								MAN_PRINT("PIR Before: Current ALS = %d\t Prev ALS = %d\r\n", snsrCurrStatus.als_LUXvalue, snsrPrevStatus.als_LUXvalue);
								MAN_PRINT("PIR Before: Curr_mLevel = %d\t Prev_mLevel = %d\r\n", Curr_mLevel, Prev_mLevel);
								als_debug_sendData();
#ifdef AREA
					for(int i = 0; i < sConfigAreaParameters.TotalArea_Sensors_count;)
					{
						if(sConfigArea[i].SHID == snsrMinCfg.snsrID)
						{
							sConfigArea[i].Occupancy = snsrCurrStatus.pir_State;
							break;
						}
						i++;
					}
#endif

					if(mux_control_select == 0)							//DALI
					{

#ifdef DALI_SPACE
	#ifdef AREA
					if(sConfigAreaParameters.SensorIsPartOfArea == 1)
					{
						if((snsrCurrStatus.pir_State))
						{
							DBG_PRINT("RTS high\r\n");
							GPIO_PinOutSet(TRIAC_PORT,TRIAC_PIN);
							triacCfg.triacStatus = true;
							fn_saveTriacState();
							fn_sendTriacStatus(0x16);
						}
					}
					else
					{
	#endif
						if(snsrCurrStatus.pir_State && Curr_mLevel != 0)
						{
							DBG_PRINT("TRIAC set high in PIR 1\r\n");
							DBG_PRINT("RTS high\r\n");
							GPIO_PinOutSet(TRIAC_PORT,TRIAC_PIN);
							triacCfg.triacStatus = true;
							fn_saveTriacState();
						}
						DBG_PRINT("m_level = %d\r\n", Curr_mLevel);
						if((snsrCfg.emergency_light == false) && (!(snsrCurrStatus.pir_State)))
						{
							DBG_PRINT("TRIAC timer set to low PIR\r\n");
							gecko_cmd_hardware_set_soft_timer(SECONDS(13),EMERGENCY_LIGHT_TIMER_ID,ONESHOT_TIMER);	//to drive TRIAC to low to cutt of the power supply to DALI driver
						}
	#ifdef AREA
					}
	#endif
#endif

	#ifdef AREA
						if(sConfigAreaParameters.SensorIsPartOfArea == 1)
						{
							if((snsrCurrStatus.pir_State))
							{
								fn_daliMode1_Level(0xFF, 100, 0x16);
							}
						}
						else
						{
	#endif
							fn_daliMode1_Level(0xFF, ((snsrCurrStatus.pir_State)?Curr_mLevel:0), 0x11);
							Prev_mLevel = (snsrCurrStatus.pir_State) ? Curr_mLevel : 0;
							DBG_PRINT("PIR After: Curr_mLevel = %d\t Prev_mLevel = %d\r\n", Curr_mLevel, Prev_mLevel);
	#ifdef AREA
						}
	#endif
					}
					else if(mux_control_select == 1)					//Analog
					{
						DBG_PRINT("Analog Command sent PIR\r\n");
	#ifdef AREA
						if(sConfigAreaParameters.SensorIsPartOfArea == 1)
						{
							if((snsrCurrStatus.pir_State))
							{
								fn_setAnalogIntensity(100, 0x26);
								Prev_mLevel = (snsrCurrStatus.pir_State) ? Curr_mLevel : 0;
							}
						}
						else
						{
	#endif
						fn_setAnalogIntensity(((snsrCurrStatus.pir_State)?Curr_mLevel:0), 0x21);
						Prev_mLevel = (snsrCurrStatus.pir_State) ? Curr_mLevel : 0;
						DBG_PRINT("PIR After: Curr_mLevel = %d\t Prev_mLevel = %d\r\n", Curr_mLevel, Prev_mLevel);
	#ifdef AREA
						}
	#endif
					}
				#ifdef TRIAC_FEATURE
					(snsrCurrStatus.pir_State && m_level) ? fn_switchOnTriac() : fn_switchOffTriac();
					fn_sendTriacStatus(0x11);							//TODO : confirm with Ram sir for reason wch bit to set for TRIAC status change
				#endif
			}
			snsrAppData.pirRetransmitTimerStart = fn_GetSecTimerStart();
			m_data.datacmd = PIR;
			m_data.data[0] = snsrCurrStatus.pir_State;
			m_packet_size  = CMD_STATCH_PIR;
		}
		break;

		case ALS:
		{
			if(!isReq && !(snsrAppData.WL_LIGHT_RcvdExternalCmd_flag || snsrAppData.WL_LIGHT_CONTROL_2_flag))
			{
				#ifdef PIR_FEATURE
					if(snsrCurrStatus.pir_State)
				#endif
					{
						Curr_mLevel = ((((sAlsCalibValue.req_lux - snsrCurrStatus.als_LUXvalue)*100)/MaxLuxOnTable) + Prev_mLevel);
						Curr_mLevel = ((Curr_mLevel > 10) ? Curr_mLevel : 0);
						Curr_mLevel = ((Curr_mLevel > 100) ? 100 : Curr_mLevel);
						MAN_PRINT("ALS Before: Current ALS = %d\t Prev ALS = %d\r\n", snsrCurrStatus.als_LUXvalue, snsrPrevStatus.als_LUXvalue);
						MAN_PRINT("ALS Before: Curr_mLevel = %d\t Prev_mLevel = %d\r\n", Curr_mLevel, Prev_mLevel);
						als_debug_sendData();

					#ifdef TRIAC_FEATURE
						(snsrCurrStatus.pir_State && m_level) ? fn_switchOnTriac() : fn_switchOffTriac();
						fn_sendTriacStatus(0x12);
					#endif

						if(mux_control_select == 0)
						{
	#ifdef AREA
							if(sConfigAreaParameters.SensorIsPartOfArea == 1)
							{
								//do nothing
							}
							else
							{
	#endif
								fn_daliMode1_Level(0xFF, ((snsrCurrStatus.pir_State)?Curr_mLevel:0), 0x12);
								Prev_mLevel = (snsrCurrStatus.pir_State) ? Curr_mLevel : 0;
								DBG_PRINT("ALS After: Curr_mLevel = %d\t Prev_mLevel = %d\r\n", Curr_mLevel, Prev_mLevel);
	#ifdef AREA
							}
	#endif


					#ifdef DALI_SPACE
	#ifdef AREA
							if(sConfigAreaParameters.SensorIsPartOfArea == 1)
							{
							//do nothing
							}
							else
							{
	#endif
								(snsrCurrStatus.pir_State && Curr_mLevel) ? fn_switchOnTriac() : fn_switchOffTriac();
								fn_sendTriacStatus(0x12);			//	Muruga is commented out
	#ifdef AREA
							}
	#endif
					#endif
						}
						else if(mux_control_select == 1)
						{
							DBG_PRINT("Analog Command sent ALS\r\n");
	#ifdef AREA
							if(sConfigAreaParameters.SensorIsPartOfArea == 1)
							{
							//do nothing
							}
							else
							{
	#endif
								fn_setAnalogIntensity((Curr_mLevel), 0x22);
								Prev_mLevel = (snsrCurrStatus.pir_State) ? Curr_mLevel : 0;
								DBG_PRINT("ALS After: Curr_mLevel = %d\t Prev_mLevel = %d\r\n", Curr_mLevel, Prev_mLevel);
	#ifdef AREA
							}
	#endif
						}
					}
			}

			snsrAppData.alsRetransmitTimerStart = fn_GetSecTimerStart();
			m_data.datacmd = ALS;
			m_data.data[0] = snsrCurrStatus.als_LUXvalue;
			m_data.data[1] = (snsrCurrStatus.als_LUXvalue >> 8);
			m_packet_size  = CMD_STATCH_ALS;
		 }
		break;

		case TEMP_HUMIDITY:
		{
			m_data.datacmd = TEMP_HUMIDITY;
			m_packet_size  = CMD_STATCH_TH;

			m_data.data[0] = snsrCurrStatus.temp/100;
			m_data.data[1] = snsrCurrStatus.temp%100;

			m_data.data[2] = snsrCurrStatus.humidity/100;
			m_data.data[3] = snsrCurrStatus.humidity%100;

			snsrAppData.thRetransmitTimerStart = fn_GetSecTimerStart();
		}
		break;
		case MAG_SENSOR_UPT:
			m_data.datacmd = MAG_SENSOR_UPT;
			m_packet_size  = 1;
			isReq = 1;
		break;
		case UUID_REQ:
		{
			struct gecko_msg_system_get_bt_address_rsp_t *pAddr = gecko_cmd_system_get_bt_address();
			m_data.datacmd = UUID_REQ;

			for(uint8_t m_index = 0; m_index<6; m_index++){
				m_data.data[m_index] = pAddr->address.addr[5-m_index];
			}
			m_packet_size = 7;
			isReq = 1;
		}
		break;

		case ALS_CALIB_CONF1:
		{
			m_data.datacmd = ALS_CALIB_CONF1;
			m_packet_size = sizeof(float)+1;
			memcpy(m_data.data,&sAlsCalibValue.m,sizeof(float));
		}
		break;
		case ALS_CALIB_CONF2:
		{
			m_data.datacmd = ALS_CALIB_CONF2;
			m_packet_size = sizeof(float)+1;
			memcpy(m_data.data,&sAlsCalibValue.c,sizeof(float));
		}
		break;
		case ALS_CALIB_GAIN:
		{
			m_data.datacmd = ALS_CALIB_GAIN;
			m_packet_size = 3;
			memcpy(m_data.data,&sAlsCalibValue.m_gain,2);
		}
		break;
		case ALS_CALIB_REQLUX:
		{
			m_data.datacmd = ALS_CALIB_REQLUX;
			m_packet_size = 3;
			memcpy(m_data.data,&sAlsCalibValue.req_lux,2);
		}
		break;

		default:
			return ;
		break;
	}
	if(snsrAppData.broadCastComplete)
	{
		fn_enQ_blePkt((isReq?FT_DATARES:FT_STATUSCHANGE),(m_packet_size),(uint8_t*)&m_data,destAddr);
	}
	return ;
}

void als_debug_sendData(void){
	sensorpayload_t  m_data;
	if(debugPrints == 1)
	{
		m_data.datacmd = DALI_M_LEVEL;
		m_data.data[0] = Curr_mLevel;
		m_data.data[1] = Prev_mLevel;
		uint16_t sentRspns = gecko_cmd_mesh_vendor_model_send(
												PRIMARY_ELEMENT,			\
												MY_VENDOR_ID,				\
												MY_MODEL_ID,				\
												0xFFFF,		\
												0,							\
												0,							\
												0,							\
												FT_STATUSCHANGE,			\
												0xff,						\
												3,			\
												&m_data)->result;
		DBG_PRINT("sent rspns = %d\r\n",sentRspns);
	}
}

