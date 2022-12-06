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
/*******************************************************************************************************************************************************************/
void fn_pir_init(void)
{
	// PIR input pin configutration
	GPIO_PinModeSet(PIR_OUT_PORT, PIR_OUT_PIN, gpioModeInput, 0);
	// interrupt call back register
	GPIOINT_CallbackRegister(PIR_INTRPT_NUM, fn_PIR_int_CallBack);
	NVIC_ClearPendingIRQ(PIR_INTRPT_IRQn);
	NVIC_EnableIRQ(PIR_INTRPT_IRQn);
	DBG_PRINT("PIR initialized\r\n");
}
/*******************************************************************************************************************************************************************/
void pir_disable(void)
{
	DBG_PRINT("PIR int disabled\r\n");
	NVIC_ClearPendingIRQ(PIR_INTRPT_IRQn);
	GPIO_ExtIntConfig(PIR_OUT_PORT, PIR_OUT_PIN, PIR_INTRPT_NUM, true, false, false);
	//g_PIRState = false;
}
/*******************************************************************************************************************************************************************/
void pir_enable(void)
{
//	if(g_PIRState == false && snsrAppData.PIR_stabilized)			//	commented out my muruga
//	{
//	if(g_PIRState == false){

//	   uint32_t interruptMask = GPIO_IntGet();

	 // if (interruptMask & (1 << PIR_OUT_PIN) == 0){
		//DBG_PRINT("PIR_INTERRUPT_ENABLE Handler\r\n");
	if(__NVIC_GetPendingIRQ(PIR_INTRPT_IRQn) == 0){
		NVIC_ClearPendingIRQ(PIR_INTRPT_IRQn);
		DBG_PRINT("PIR int enabled\r\n");
		GPIO_ExtIntConfig(PIR_OUT_PORT, PIR_OUT_PIN, PIR_INTRPT_NUM, true, false, true);	//	Rising Edge = True, Falling Edge = False, Interrupt En = True

		//g_PIRState = true;
	}
}
//*******************************************************************************************************************************************************************//
void pir_enable_disable(uint8_t pir_en_dis_state)
{
	switch(pir_en_dis_state)
	{
		case PIR_ENABLE:
			NVIC_ClearPendingIRQ(PIR_INTRPT_IRQn);
			DBG_PRINT("PIR int enabled\r\n");
			GPIO_ExtIntConfig(PIR_OUT_PORT, PIR_OUT_PIN, PIR_INTRPT_NUM, true, false, true);	//	Rising Edge = True, Falling Edge = False, Interrupt En = True
			break;

		case PIR_DISABLE:
			DBG_PRINT("PIR int disabled\r\n");
			NVIC_ClearPendingIRQ(PIR_INTRPT_IRQn);
			GPIO_ExtIntConfig(PIR_OUT_PORT, PIR_OUT_PIN, PIR_INTRPT_NUM, true, false, false);
			break;

		default:
			pir_en_dis_state = PIR_ENABLE;
			break;

	}
}
//*******************************************************************************************************************************************************************//
void fn_PIR_Process(void)
{
	static uint8_t PIRCounter = 0;
	static enum_PIR_ProcessState ePIRprocess_state = SET_PIR_CONFIG;
	switch(ePIRprocess_state)
	{
		case SET_PIR_CONFIG:																//sets the required configuration fetched by Sensor Configuration Structure
			pir_unoccupancyTime = fn_GetSecTimerStart();
			//pir_enable();
			pir_enable_disable(PIR_ENABLE);
			DBG_PRINT("pir initState\r\n");
			ePIR_trggr_state =  NO_MOTION_DETECTED;
			ePIRprocess_state = CHECK_PIR_STATUS;
		break;

		case CHECK_PIR_STATUS:
		{
			if(ePIR_trggr_state)
			{
				pir_unoccupancyTime = fn_GetSecTimerStart();
				ePIR_trggr_state = NO_MOTION_DETECTED;
				// SWITCH ON the lights when motion detected
				snsrCurrStatus.pir_State = OCCUPIED;
				ePIRprocess_state = UPDATE_PIR_STATUS_CHANGE;
			}
			else if(fn_IsSecTimerElapsed(pir_unoccupancyTime, snsrCfg.pir_cfg.unoccupancyTimer_s))
			{
				pir_unoccupancyTime = fn_GetSecTimerStart();
				ePIRprocess_state = UPDATE_PIR_STATUS_CHANGE;
				//no motion detected , switch off the lights
				snsrCurrStatus.pir_State = UNOCCUPIED;
				PIRCounter = 0;
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
			if(PIRCounter == 0){
				send_packet(PIR,snsrMinCfg.dest_addr,false);
			}
			ePIRprocess_state = DECIDE_PIR_STATE;
		break;

		case DECIDE_PIR_STATE:
			if(snsrCurrStatus.pir_State == UNOCCUPIED)
			{
				ePIRprocess_state = CHECK_PIR_STATUS;
				//pir_enable();
				break;
			}else
			//if(snsrCurrStatus.pir_State == OCCUPIED)
			{
				pir_SleepTime_Start = fn_GetSecTimerStart();
				ePIRprocess_state = SLEEP_PERIOD;
				DBG_PRINT("put pir into sleep\r\n");
			}
		break;

		case SLEEP_PERIOD:
			if(fn_IsSecTimerElapsed(pir_SleepTime_Start, (snsrCfg.pir_cfg.unoccupancyTimer_s)/PIR_DIVIDE_COUNTER))
			{
				if(++PIRCounter >= PIR_DIVIDE_COUNTER){
					PIRCounter = 0;
				}
				DBG_PRINT("wake up from pir sleep\r\n");
				//pir_enable();
				pir_enable_disable(PIR_ENABLE);
				ePIRprocess_state = CHECK_PIR_STATUS;
			}else if(pir_SleepTime_Start == 0){
				pir_SleepTime_Start = fn_GetSecTimerStart();
			}
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
	if(intNum == PIR_INTRPT_NUM)
	{
		//Use GPIO_IntClear for clearing all odd number interrupt
		//uint32_t interruptMask = GPIO_IntGet();
		//GPIO_IntClear(interruptMask);
		DBG_PRINT(".........motion detected......\r\n");
		ePIR_trggr_state = MOTION_DETECTED;
		//pir_disable();
		pir_enable_disable(PIR_DISABLE);
	}
	else
	{
		DBG_PRINT("expected %d : rcvd_intrptNum = %d\r\n",PIR_INTRPT_NUM,intNum);
	}
	return ;
}
//*******************************************************************************************************************************************************************//

