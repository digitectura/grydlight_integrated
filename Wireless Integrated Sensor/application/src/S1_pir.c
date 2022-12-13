/*******************************************************************************************************************************************************************/
#include <application/inc/S1_pir.h>
#include "application/inc/app.h"
#include "application/inc/intervalTimer.h"
#include "application/inc/command_handler.h"
#include "application/inc/gsLink/GSLink_uart.h"
#include "application/inc/gsLink/gsLinkPktQ.h"
#include "application/inc/gsLink/GS_Link.h"
#include "gpiointerrupt.h"
#include "application/inc/dali/daliCommand.h"
#include "application/inc/analog/mcp45hvx1.h"

/*******************************************************************************************************************************************************************/
uint16_t pir_SleepTime_Start = 1;
uint16_t pir_unoccupancyTime = 0;
enum_PIR_trigger_state ePIR_trggr_state = NO_MOTION_DETECTED;
enum_PIR_ProcessState ePIR_process_state;
uint8_t g_PIRState = false;

enum_PIR_ProcessState ePIRprocess_state = SET_PIR_CONFIG;


/*******************************************************************************************************************************************************************/
void fn_pir_init(void)
{
	// PIR input pin configutration
	GPIO_PinModeSet(PIR_OUT_PORT, PIR_OUT_PIN, gpioModeInput, 0);
	// interrupt call back register
	GPIOINT_CallbackRegister(PIR_INTRPT_NUM, fn_PIR_int_CallBack);
	DBG_PRINT("PIR int disabled\r\n");
	GPIO_ExtIntConfig(PIR_OUT_PORT, PIR_OUT_PIN, PIR_INTRPT_NUM, true, false, true);
	NVIC_ClearPendingIRQ(PIR_INTRPT_IRQn);
	NVIC_EnableIRQ(PIR_INTRPT_IRQn);
	DBG_PRINT("PIR initialized\r\n");
}


//*******************************************************************************************************************************************************************//
void fn_PIR_Process(void)
{
	static uint8_t PIRCounter = 0;

	switch(ePIRprocess_state)
	{
		case SET_PIR_CONFIG:																//sets the required configuration fetched by Sensor Configuration Structure
			pir_unoccupancyTime = fn_GetSecTimerStart();
			DBG_PRINT("pir initState\r\n");
			ePIRprocess_state = CHECK_PIR_STATUS;
			snsrCurrStatus.pir_State = UNOCCUPIED;
		break;

		case CHECK_PIR_STATUS:
		{
			if(fn_IsSecTimerElapsed(pir_unoccupancyTime, snsrCfg.pir_cfg.unoccupancyTimer_s))
			{
				pir_unoccupancyTime = fn_GetSecTimerStart();
				DBG_PRINT("UnOccupied .. %d \r\n ",pir_unoccupancyTime);
				if(snsrCurrStatus.pir_State)
					ePIRprocess_state = UPDATE_PIR_STATUS_CHANGE;
				//no motion detected , switch off the lights
				snsrCurrStatus.pir_State = UNOCCUPIED;
			}else if(pir_unoccupancyTime == 0){
				pir_unoccupancyTime = fn_GetSecTimerStart();
			}
		}
		break;

		case UPDATE_PIR_STATUS_CHANGE:
			DBG_PRINT("/*...........................................*/\r\n");
			#ifdef TRIAC_FEATURE
				DBG_PRINT("PIR ---> %s ---> %s\r\n",(snsrCurrStatus.pir_State?"OCCUPIED":"UNOCCUIPED"),
													(snsrCurrStatus.pir_State?"RTS_PULL_HIGH":"RTS_PULL_LOW"));
			#else
				DBG_PRINT("PIR rs485 data ---> %s\r\n",(snsrCurrStatus.pir_State?"OCCUPIED":"UNOCCUIPED"));
			#endif

			send_packet(PIR,snsrMinCfg.dest_addr,false);

			ePIRprocess_state = DECIDE_PIR_STATE;
		break;

		case DECIDE_PIR_STATE:
			ePIRprocess_state = CHECK_PIR_STATUS;
					//	IDEAL STATE
		break;

		case SLEEP_PERIOD:			//	Redantant state
			ePIRprocess_state = CHECK_PIR_STATUS;
		break;
		default :
			ePIRprocess_state = SET_PIR_CONFIG;
		break;
	}
	return;
}
//*******************************************************************************************************************************************************************//
void fn_PIR_int_CallBack(uint8_t intNum)
{
	pir_unoccupancyTime = fn_GetSecTimerStart();
	DBG_PRINT(".........motion detected...... %d \r\n",pir_unoccupancyTime);
	if(!snsrCurrStatus.pir_State){
		ePIRprocess_state = UPDATE_PIR_STATUS_CHANGE;
	}
	snsrCurrStatus.pir_State = OCCUPIED;

	(snsrCurrStatus.pir_State && Curr_mLevel) ? fn_switchOnTriac() : fn_switchOffTriac();
	fn_sendTriacStatus(0x12);

	return ;
}
//*******************************************************************************************************************************************************************//

