/****************************************************************************************************************/
#include "application/inc/switch.h"
#include "application/inc/intervalTimer.h"
#include "application/inc/app.h"
#include "application/inc/gsLink/GS_Link.h"
#include "application/inc/BLE_Queue.h"
#include "application/inc/dali/dali.h"
#include "application/inc/analog/mcp45hvx1.h"
#include "application/inc/dali/daliCommand.h"
/****************************************************************************************************************/
uint8_t g_loopMap[LOOP_MAP_ITRT] = {BOTH_LOOP,LOOP1,LOOP2,NO_LOOP};
uint16_t g_switchPressed_StartTime;
uint16_t g_loopMapStratTime;

uint8_t g_jumpToLoopState = 0;
uint8_t g_loopMapIndex = 0;
uint8_t g_switchIntrptEnabled = false;
uint8_t g_loopInidcationCnt = 0;
/****************************************************************************************************************/
uint8_t fn_getCurntMappedIndx(void)
{
	uint8_t indx;
	for(indx =0 ;indx<LOOP_MAP_ITRT;indx++)
	{
		if(snsrMinCfg.lghtMpng.loopMap == g_loopMap[indx])
			return indx;
	}
	return 0;
}
/****************************************************************************************************************/
void fn_switch_intrpt_callBack(uint8_t intNum)
{
	if(intNum == 0){
		gecko_external_signal(EXT_SIGNAL_SWITCH_INTERRUPT);		//	Register the PIR interrupt and do the processing in event
	}else if(intNum == 2){
		gecko_external_signal(EXT_SIGNAL_DALI_INTERRUPT);
	}
	return ;
}
/****************************************************************************************************************/
void fn_switchInit(void)
{
	if(brdFeature.boardtype == INTEGRATED){
		GPIO_PinModeSet(SWITCH_PORT, SWITCH_PIN, gpioModeInputPull, 1);
	}

	if(brdFeature.boardtype == DALI || brdFeature.boardtype == ANALAOG || brdFeature.boardtype == TRIAC){
		GPIO_PinModeSet(SWITCH_PORT, SWITCH_PIN, gpioModeInput, 0);
	}

	GPIOINT_CallbackRegister(SWITCH_INTRPT_NUM, fn_switch_intrpt_callBack);
	NVIC_ClearPendingIRQ(SWITCH_INTRPT_IRQn);
	NVIC_EnableIRQ(SWITCH_INTRPT_IRQn);
	fn_enableSwitch_int();
	return ;
}
/****************************************************************************************************************/
void fn_enableSwitch_int(void)
{
	if(!g_switchIntrptEnabled)
	{
		GPIO_ExtIntConfig(SWITCH_PORT,SWITCH_PIN,SWITCH_INTRPT_NUM,true,true,true);
		g_switchIntrptEnabled = true;
		MAN_PRINT("/`/`/`/`/`/switch interrupt enabled/`/`/`/`/`/`/\r\n");
	}
	return ;
}
/****************************************************************************************************************/
void fn_disableSWitch_int(void)
{
	GPIO_ExtIntConfig(SWITCH_PORT,SWITCH_PIN,SWITCH_INTRPT_NUM,true,true,false);
	g_switchIntrptEnabled = false;
	return ;
}
/****************************************************************************************************************/
void fn_switchReleased(void)
{
	uint16_t rleaseTimeout = fn_GetSecTimerStart();
	uint16_t diffTime	= rleaseTimeout - g_switchPressed_StartTime;
	if(diffTime <= THREE &&  diffTime >= ONE)
	{
		send_packet(MAG_SENSOR_UPT, 0xFFFF, 1);
		snsrAppData.switchPIR_interrupt = true;
		snsrAppData.switchPIR_state = ~(snsrAppData.switchPIR_state);
		DBG_PRINT("Sent PIR %s status\r\n",(snsrAppData.switchPIR_state)?"occupied":"unoccupied");
		if(brdFeature.boardtype == TRIAC){
			fn_pirBasedTriacCntrl();
		}


		if(brdFeature.boardtype == INTEGRATED){
			fn_pirBasedTriacCntrl();
		}


		if(mux_control_select == 1)
		{
			fn_setAnalogIntensity(((snsrAppData.switchPIR_state)?100:0), 0);
		}
		else if(mux_control_select == 0)
		{
			fn_daliMode1_Level(0xFF, ((snsrAppData.switchPIR_state)?100:0), 0);
		}
		return ;
	}
	else if(diffTime >= LOOP_MAP_CHANGE_INTERVAL && diffTime <= (LOOP_MAP_CHANGE_INTERVAL+BUFFER_TIME) )
	{
			//LOOP JUMP
			if( g_loopMapIndex < (LOOP_MAP_ITRT-1) )
			{
				++g_loopMapIndex;
			}
			else
			{
				g_loopMapIndex = 0;
			}
			fn_indicate_LoopMapping_LEDstate(g_loopMap[g_loopMapIndex]);
			DBG_PRINT("no of ch mapped %d\r\n",g_loopMap[g_loopMapIndex]);
			gecko_cmd_hardware_set_soft_timer(MILLISECONDS(1),LOOP_MAP_INDICATION,ONESHOT_TIMER);
			//	Send the details to Controller
			if(snsrMinCfg.lghtMpng.loopMap != g_loopMap[g_loopMapIndex])
			{
				gecko_cmd_hardware_set_soft_timer(SECONDS(1),TIMER_ID_LIGHT_MAP_PKT,ONESHOT_TIMER);
			}
			else
			{
				DBG_PRINT("no loop map change\r\n");
			}
	}
	else if(diffTime >= PIR_CONFIG_CHANGE && diffTime <= (PIR_CONFIG_CHANGE+BUFFER_TIME))
	{
		fn_loopPIR_cfg();
	}
	else if(diffTime >= ALS_CONFIG_CHANGE && diffTime <= (ALS_CONFIG_CHANGE+BUFFER_TIME))
	{
		if(snsrMinCfg.device_type & ALS_SENSOR)
		{
			fn_loopALS_cfg();
		}
	}
	else if( diffTime >= MAGNET_SWITCH_SYSTEM_RESET_TIMEOUT && diffTime <= (MAGNET_SWITCH_SYSTEM_RESET_TIMEOUT + BUFFER_TIME))
	{
		//system_reset
		MAN_PRINT("system_reset by switch\r\n");
		gecko_cmd_hardware_set_soft_timer(SECONDS(1),FCTORY_RESET_ID,ONESHOT_TIMER);
	}
	else if( diffTime >= OTA_TIMEOUT && (diffTime <= OTA_TIMEOUT+BUFFER_TIME) )
	{
		MAN_PRINT("OTA MODE INIT\r\n");
		gecko_cmd_system_reset(OTA_DFU_RESET);
	}
	else if( diffTime >= MAGNET_SWITCH_FCATORY_RESET_TIMEOUT && (diffTime <= MAGNET_SWITCH_FCATORY_RESET_TIMEOUT+BUFFER_TIME) )
	{
		//factory reset
		MAN_PRINT("factory_reset by switch\r\n");
		fn_initiate_factory_reset(0);
	}
	else if(diffTime >= MAGNET_SWITCH_NETWORK_RESET_TIMEOUT)
	{
		MAN_PRINT("Network Reset initiated by magnetic switch\r\n");
		fn_initiate_factory_reset(1);
	}
	else
	{
		TURN_OFF_LED(APP_LED1);
		TURN_OFF_LED(APP_LED2);
	}
	g_jumpToLoopState = 0;
	g_switchPressed_StartTime = 0;
	return ;
}/****************************************************************************************************************/
void fn_sendLightMap_Pkt(void)
{
	uint8_t pir_map_pkt[15];
	uint8_t als_map_pkt[15];
	uint8_t pirLen = 0;
	uint8_t alsLen = 0;

	snsrMinCfg.lghtMpng.loopMap = g_loopMap[g_loopMapIndex];
	pir_map_pkt[0] = als_map_pkt[0] = LIGHT_SENSOR_MAP;

	switch(snsrMinCfg.lghtMpng.loopMap)
	{
		case BOTH_LOOP:
			pir_map_pkt[1] = (PIR_MAPNG | TWO_LOOP_MAPPED);
			pir_map_pkt[2] = LOOP1 + ADD_CNTRL_TYPE(sCtrlCfg.cntrl_type);
			pir_map_pkt[3] = LOOP2 + ADD_CNTRL_TYPE(sCtrlCfg.cntrl_type);

			als_map_pkt[1] = (ALS_MAPNG | TWO_LOOP_MAPPED);
			als_map_pkt[2] = LOOP1 + ADD_CNTRL_TYPE(sCtrlCfg.cntrl_type);
			als_map_pkt[3] = (uint8_t)(snsrMinCfg.lghtMpng.reqLux);
			als_map_pkt[4] = (uint8_t)(snsrMinCfg.lghtMpng.reqLux>>8);
			als_map_pkt[5] = snsrMinCfg.lghtMpng.gain;
			als_map_pkt[6] = LOOP2 + ADD_CNTRL_TYPE(sCtrlCfg.cntrl_type);
			als_map_pkt[7] = (uint8_t)(snsrMinCfg.lghtMpng.reqLux);
			als_map_pkt[8] = (uint8_t)(snsrMinCfg.lghtMpng.reqLux>>8);
			als_map_pkt[9] = snsrMinCfg.lghtMpng.gain;
			pirLen = 4;
			alsLen = 10;
		break;
		case LOOP1:
			pir_map_pkt[1] = (PIR_MAPNG | ONE_LOOP_MAPPED);
			pir_map_pkt[2] = LOOP1 + ADD_CNTRL_TYPE(sCtrlCfg.cntrl_type);
			als_map_pkt[1] = (ALS_MAPNG | ONE_LOOP_MAPPED);
			als_map_pkt[2] = LOOP1 + ADD_CNTRL_TYPE(sCtrlCfg.cntrl_type);
			als_map_pkt[3] = (uint8_t)(snsrMinCfg.lghtMpng.reqLux);
			als_map_pkt[4] = (uint8_t)(snsrMinCfg.lghtMpng.reqLux>>8);
			als_map_pkt[5] = snsrMinCfg.lghtMpng.gain;
			pirLen = 3;
			alsLen = 6;
		break;
		case LOOP2:
			pir_map_pkt[1] = (PIR_MAPNG | ONE_LOOP_MAPPED);
			pir_map_pkt[2] = LOOP2 + ADD_CNTRL_TYPE(sCtrlCfg.cntrl_type);
			als_map_pkt[1] = (ALS_MAPNG | ONE_LOOP_MAPPED);
			als_map_pkt[2] = LOOP2 + ADD_CNTRL_TYPE(sCtrlCfg.cntrl_type);
			als_map_pkt[3] = (uint8_t)(snsrMinCfg.lghtMpng.reqLux);
			als_map_pkt[4] = (uint8_t)(snsrMinCfg.lghtMpng.reqLux>>8);
			als_map_pkt[5] = snsrMinCfg.lghtMpng.gain;
			pirLen = 3;
			alsLen = 6;
		break;
		case NO_LOOP:
			pir_map_pkt[1] = (PIR_MAPNG | NO_LOOP);
			als_map_pkt[1] = (ALS_MAPNG | NO_LOOP);
			pirLen = 2;
			alsLen = 2;
		break;
	}
/*****************************************************************************************************************/
#ifdef DEBUG_MODE
	DBG_PRINT("sent the light mapping\r\n");
	DBG_PRINT("PIR_MAPPING : ");
	for(int i=0;i<pirLen;i++)
	{
		DBG_PRINT("0x%02x ",pir_map_pkt[i]);
	}
	DBG_PRINT("\r\nALS MAPPING : ");
	for(int i=0;i<alsLen;i++)
	{
		DBG_PRINT("0x%02x ",als_map_pkt[i]);
	}
	DBG_PRINT("\r\n");
#endif
/*******************send the config packet to UI through agg; only if agg is binded to that snsr******************/
//if the agg is not binded and sent on broadcast addr all other sensors will rcv and update with same mapping details
	if(!snsrMinCfg.aggBinded)
	{
		DBG_PRINT("no agg mapped\r\n");
	}
	else
	{
		sensorpayload_t  m_data;

		m_data.datacmd = LIGHT_SENSOR_MAP;
		memcpy(m_data.data,&pir_map_pkt[1],pirLen-1);
		fn_enQ_blePkt(FT_COMMAND,pirLen,(uint8_t*)&m_data, snsrMinCfg.dest_addr);

		m_data.datacmd = LIGHT_SENSOR_MAP;
		memcpy(m_data.data,&als_map_pkt[1],alsLen-1);
		fn_enQ_blePkt(FT_COMMAND,alsLen,(uint8_t*)&m_data, snsrMinCfg.dest_addr);
	}
/****************************************************************************************************************/
	return ;
}
/****************************************************************************************************************/
void fn_handleSwitchIntrpt(void)
{
	if( fn_IsSecTimerElapsed(g_loopMapStratTime,LOOP_MAP_CHANGE_INTERVAL) )
	{
		if(g_jumpToLoopState < LOOP_MAP_ITRT)
		{
			g_loopMapStratTime = fn_GetSecTimerStart();
			g_jumpToLoopState += 1;
			//LOOP JUMP
			fn_loopLightMapping();
		}
	}
	return ;
}
/****************************************************************************************************************/
void fn_indicate_LoopMapping_LEDstate(uint8_t loopMapState)
{
	switch(loopMapState)
	{
		case BOTH_LOOP:
//			TURN_ON_LED(APP_LED1);
//			TURN_ON_LED(APP_LED2);
			//blink 3 times
			g_loopInidcationCnt = 2;
		break;
		case LOOP1:
//			TURN_ON_LED(APP_LED1);
//			TURN_OFF_LED(APP_LED2);
			//blink once
			g_loopInidcationCnt = 4;
		break;
		case LOOP2:
//			TURN_OFF_LED(APP_LED1);
//			TURN_ON_LED(APP_LED2);
			//blink twice
			g_loopInidcationCnt = 6;
		break;
		case NO_LOOP:
//			TURN_OFF_LED(APP_LED1);
//			TURN_OFF_LED(APP_LED2);
			//blink four times
			g_loopInidcationCnt = 10;
		break;
		default:
		break;
	}
	gecko_cmd_hardware_set_soft_timer(MILLISECONDS(500),LOOP_MAP_INDICATION,ONESHOT_TIMER);
	return ;
}
/****************************************************************************************************************/
void fn_loopLightMapping(void)
{
	switch(g_jumpToLoopState)
	{
		case 1:
//			fn_indicate_LoopMapping_LEDstate(snsrMinCfg.lghtMpng.loopMap);
//		break;
		default:
			if( g_loopMapIndex < (LOOP_MAP_ITRT-1) )
			{
				++g_loopMapIndex;
			}
			else
			{
				g_loopMapIndex = 0;
			}
			fn_indicate_LoopMapping_LEDstate(g_loopMap[g_loopMapIndex]);
		break;
	}
	return ;
}
/****************************************************************************************************************/
