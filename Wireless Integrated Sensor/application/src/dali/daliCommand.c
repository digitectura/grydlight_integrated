/*
 * daliCommand.c
 *
 *  Created on: Aug 7, 2019
 *      Author: Rakshit
 */
#include "main.h"
#include <application/inc/dali/daliCommand.h>
#include "application/inc/intervalTimer.h"
#include "application/inc/dali/daliApplication.h"
#include "application/inc/app.h"
#include "application/inc/switch.h"
#include "application/inc/analog/mcp45hvx1.h"
//struct_DaliCommissionInfo sDaliComInfo[2][65];


//#define ENABLE_QUERY
#define CHECK_INTENSITY(a,b,c) (a>=b) ? b : (a<=c) ? c : a
#define LEVEL_PERCENT(num) pow(10, (float)((((float)num-1)/(253.0/3.0))-1))
#define GET_LOOP(x) (((x & 0x0200)==0x200) ? 1 : 0)
#define GET_SADD(x) (uint8_t)(x & 0x00FF)
#define GET_GRP(x)  (x & 0x400)

uint8_t g_daliLightStat = 0;
bool criticalOpnLock = false;
struct_DaliAction sDaliAction = { 0 };
uint16_t flag = 0x100;
uint32_t count = 0;

enum {
	QUEUE_SEGREGATION_1,
	QUEUE_SEGREGATION_2,
	BALLAST_LEVEL_SET,
	IDENTIFY_DALI,
	GROUP_LEVEL_SET,
	SCENE_SET,
	QUERY_BALLAST_STATUS,
	QUERY_BALLAST_CUR,
	BALLAST_TC_SET,
	QUERY_WAIT_STATUS,
	QUERY_WAIT_STATUS_2,
};

struct_DaliRetention g_sDaliRetention         = {
                                                .g_RetentionFlag = 0,
                                                .DaliQueryWaitTimer = 0        ,                    //for giving a delay equal to fade rate after ballast level set
                                                .Dali_RetentionStroageTimeout = 10,
                                                .m_LevelThreshold = DALI_M_LEVEL_THRESHOLD
                                              };

struct_GPentity g_sGpEntity;
struct_SNentity g_sSNEntity;
struct_gpSnAction g_sGpSnAction_obj;
//struct_gpLookup sGpLookUp[TOTAL_ENTITIES_GP];
//struct_SNLookup sSNLookUp[TOTAL_SCENES_CTRL];
//struct_DaliFlashLookUp sDaliFlashLookUp[NUM_OF_DALI_LOOPS][TOTAL_DALI_BALLASTS];

uint8_t daliResponse = 0;
uint8_t g_shortAddress = 0;
//uint8_t tempAddress_temp = 45;
//bool g_queueByPass = false;

struct_DaliSceneAction m_sDaliSceneAct;
enum_daliConfigStatus edaliConfigStatus;
enum_daliProcessStatus edaliProcessStatus;
uint16_t g_ballastCount[NUM_OF_DALI_LOOPS];
//struct_DALIStatus sDALIStatus = { 0 };
//struct_GPStatus m_sGpStatus[TOTAL_ENTITIES_GP] = { 0 };
uint8_t g_randomizeDali = 0;
struct_Dim s_DimEntity;                                  // Srikanth 01-6-22

//void fn_assignAddressbyte(uint32_t*, uint32_t, uint32_t);
//uint16_t fn_isDaliShidAvailable(uint8_t m_loop, uint32_t m_randId);
//bool fn_isDaliRndIDAvailable(uint8_t m_loop, uint32_t m_randId);
//bool fn_isSHIDAvailable(uint8_t m_shid, daliLoop_typedef m_loop);
//int8_t fn_findFree_id(daliLoop_typedef m_loop);
/*		========= ballastOn ==========
 *
 * @brief 	This function is called to switch on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballastOn(daliLoop_typedef m_daliLoop, uint8_t m_address, bool m_repeat) {
	if (fn_commandTransmit(m_address, LUM_ON, m_repeat, m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	return false;
}

/*		========= ballastOff ==========
 *
 * @brief 	This function is called to switch off the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballastOff(daliLoop_typedef m_daliLoop, uint8_t m_address, bool m_repeat) {
	if (fn_commandTransmit(m_address, LUM_OFF, m_repeat, m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	return false;
}

bool ballastFadeUp(daliLoop_typedef m_daliLoop, uint8_t m_address,bool m_repeat) {
	if (fn_commandTransmit(m_address, LUM_ON, m_repeat, m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	return false;
}

bool ballastFadeDown(daliLoop_typedef m_daliLoop, uint8_t m_address,bool m_repeat) {
	if (fn_commandTransmit(m_address, LUM_DOWN, m_repeat, m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	return false;
}
/*		========= ballastStepUp ==========
 *
 * @brief 	This function is called to step up the intensity of the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballastStepUp(daliLoop_typedef m_daliLoop, uint8_t m_address,bool m_repeat) {
	if (fn_commandTransmit(m_address, LUM_STEPUP, false, m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	//UNUSED(m_repeat);
	return false;
}

/*		========= ballastStepDown ==========
 *
 * @brief 	This function is called to step down the intensity of the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballastStepDown(daliLoop_typedef m_daliLoop, uint8_t m_address,
bool m_repeat) {
	if (fn_commandTransmit(m_address, LUM_STEPDOWN, false, m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	//UNUSED(m_repeat);
	return false;
}

/*		========= ballastRecallMax ==========
 *
 * @brief 	This function is called to step down the intensity of the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballastRecallMax(daliLoop_typedef m_daliLoop, uint8_t m_address,
bool m_repeat) {
	if (fn_commandTransmit(m_address, LUM_RECALL_MAX, false, m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	//UNUSED(m_repeat);
	return false;
}

/*		========= ballastRecallMin ==========
 *
 * @brief 	This function is called to step down the intensity of the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballastRecallMin(daliLoop_typedef m_daliLoop, uint8_t m_address,
bool m_repeat) {
	if (fn_commandTransmit(m_address, LUM_RECALL_MIN, false, m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	//UNUSED(m_repeat);
	return false;
}

/*		========= ballastStepDOff ==========
 *
 * @brief 	This function is called to step down the intensity and switch off
 * 		the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballastStepDOff(daliLoop_typedef m_daliLoop, uint8_t m_address,
bool m_repeat) {
	if (fn_commandTransmit(m_address, LUM_STEPDOWN_OFF, false,
			m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	//UNUSED(m_repeat);
	return false;
}

/*		========= ballastStepUOn ==========
 *
 * @brief 	This function is called to step down the intensity and switch off
 * 		the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballastStepUOn(daliLoop_typedef m_daliLoop, uint8_t m_address,
bool m_repeat) {
	if (fn_commandTransmit(m_address, LUM_STEPUP_ON, false, m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	//UNUSED(m_repeat);
	return false;
}

bool ballastLastActLev(daliLoop_typedef m_daliLoop, uint8_t m_address,
bool m_repeat) {
	if (fn_commandTransmit(m_address, LUM_GOTO_LAST_ACTIVE_LEV, false,
			m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	//UNUSED(m_repeat);
	return false;
}

/*		========= ballast_gotoScene ==========
 *
 * @brief 	This function is called to go to then scene set on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @params (m_sceneNumber) The scene number 0-15
 *
 * @retval true or false
 *
 */
bool ballast_gotoScene(daliLoop_typedef m_daliLoop, uint8_t m_address,bool m_repeat, uint8_t m_sceneNumber)
{
	if (fn_commandTransmit(m_address, LUM_GOTO_SCENE | m_sceneNumber, false, m_daliLoop) == true)
	{
		fn_discardPacket(m_daliLoop);
		return true;
	}
	//UNUSED(m_repeat);
	return false;
}

/*		========= ballast_reset ==========
 *
 * @brief 	This function is called to reset the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballast_reset(daliLoop_typedef m_daliLoop, uint8_t m_address) {
	if (fn_commandTransmit(m_address, LUM_RESET, true, m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	return false;
}

bool ballast_identify(daliLoop_typedef m_daliLoop, uint8_t m_address) {
	if (fn_commandTransmit(m_address, LUM_IDENTIFY, true, m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	return false;
}

/*		========= ballast_storeActLevel ==========
 *
 * @brief 	This function is called to reset the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballast_storeActLevel(daliLoop_typedef m_daliLoop, uint8_t m_address,
bool m_repeat) {
	if (fn_commandTransmit(m_address, LUM_STORE_ACT_LEVEL, true,
			m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	//UNUSED(m_repeat);
	return false;
}

/*		========= ballast_storeMaxLevel ==========
 *
 * @brief 	This function is called to set max level on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballast_storeMaxLevel(daliLoop_typedef m_daliLoop, uint8_t m_address,
		uint8_t maxLev,
		bool m_repeat) {
//	if(fn_commandTransmit(m_address, LUM_STORE_MAX_LEVEL, m_repeat, m_daliLoop) == true){
//		fn_discardPacket(m_daliLoop);
//		return true;
//	}

	static uint8_t m_setMaxStatus = 0;
	static uint8_t m_daliResp = 0;

	switch (m_setMaxStatus) {
	case 0:
		if (ballastSplCommand(m_daliLoop, LUM_SP_DTR, maxLev, false, false,
				&m_daliResp)) {
			m_setMaxStatus = 2;

		}
		break;
	case 1:
		if (ballastQuery(m_daliLoop, m_address, LUM_QDTR, &daliResponse)) {
			m_setMaxStatus = 2;
		}
		break;
	case 2:
		if (fn_commandTransmit(m_address, LUM_STORE_MAX_LEVEL, true,
				m_daliLoop) == true) {
			fn_discardPacket(m_daliLoop);
			m_setMaxStatus = 0;
			return true;
		}
		break;
	}
	//UNUSED(m_repeat);

	return false;
}

/*		========= ballast_storeMinLevel ==========
 *
 * @brief 	This function is called to set min level on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballast_storeMinLevel(daliLoop_typedef m_daliLoop, uint8_t m_address,
		uint8_t minLev,
		bool m_repeat) {
//	if(fn_commandTransmit(m_address, LUM_STORE_MIN_LEVEL, m_repeat, m_daliLoop) == true){
//		fn_discardPacket(m_daliLoop);
//		return true;
//	}
	static uint8_t m_setMinStatus = 0;
	static uint8_t m_daliResp = 0;

	switch (m_setMinStatus) {
	case 0:
		if (ballastSplCommand(m_daliLoop, LUM_SP_DTR, minLev, false, false,
				&m_daliResp)) {
			m_setMinStatus = 2;
		}
		break;
	case 1:
		if (ballastQuery(m_daliLoop, m_address, LUM_QDTR, &daliResponse)) {
			m_setMinStatus = 2;

		}
		break;
	case 2:
		if (fn_commandTransmit(m_address, LUM_STORE_MIN_LEVEL, true,
				m_daliLoop) == true) {
			fn_discardPacket(m_daliLoop);
			m_setMinStatus = 0;
			return true;
		}
		break;
	}
	//UNUSED(m_repeat);
	return false;
}

/*		========= ballast_storeFailLevel ==========
 *
 * @brief 	This function is called to set system fail level on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballast_storeFailLevel(daliLoop_typedef m_daliLoop, uint8_t m_address,
		uint8_t sysFailLev,
		bool m_repeat) {
//	if(fn_commandTransmit(m_address, LUM_STORE_SYS_FAIL, true, m_daliLoop) == true){
//		fn_discardPacket(m_daliLoop);
//		return true;
//	}
	static uint8_t m_setSysFailStatus = 0;
	static uint8_t m_daliResp = 0;
//	static uint8_t m_confAddress = 0;

	switch (m_setSysFailStatus) {
	case 0:
		if (ballastSplCommand(m_daliLoop, LUM_SP_DTR, sysFailLev, false, false,
				&m_daliResp)) {
			m_setSysFailStatus = 1;
		}
//		if(ballast_storeActLevel(m_daliLoop, m_address, true)){
//			m_setSysFailStatus = 1;
//		}
		break;
	case 1:
		if (fn_commandTransmit(m_address, LUM_STORE_SYS_FAIL, true,
				m_daliLoop) == true) {
			fn_discardPacket(m_daliLoop);
			m_setSysFailStatus = 0;
			return true;
		}
		break;
	case 2:
//		if(fn_commandTransmit(m_address, LUM_STORE_SYS_FAIL, true, m_daliLoop) == true){
//			fn_discardPacket(m_daliLoop);
//			m_setSysFailStatus = 0;
//			return true;
//		}
		break;
	}

	//UNUSED(m_repeat);
	return false;
}

/*		========= ballast_storePowOnLevel ==========
 *
 * @brief 	This function is called to set power on level on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballast_storePowOnLevel(daliLoop_typedef m_daliLoop, uint8_t m_address,
bool m_repeat) {
//	if(fn_commandTransmit(m_address, LUM_STORE_POWON_LEVEL, true, m_daliLoop) == true){
//		fn_discardPacket(m_daliLoop);
//		return true;
//	}
	static uint8_t m_setPowOnStatus = 0;
	static uint16_t m_timeout = 0;
//	static uint8_t m_daliResp = 0;
//	static uint8_t m_confAddress = 0;
//	static uint8_t powOnLev = 0;

	switch (m_setPowOnStatus) {
	case 0:
//		 if(ballastSplCommand(m_daliLoop, LUM_SP_DTR, powOnLev, false, false, &m_daliResp)){
//			 if(m_address == (0xFF)){
//				 m_confAddress = (1<<1)|0x01;
//			 }
//			 else{
//				 m_confAddress = m_address;
//			 }
//			 m_setPowOnStatus = 1;
//		 }
//		if(m_address == 0xFF){
		m_timeout = fn_GetSecTimerStart();
		m_setPowOnStatus = 1;
//		}
//		else{
//			m_setPowOnStatus = 2;
//		}
		break;
	case 1:
		if (fn_IsSecTimerElapsed(m_timeout, 1)) {
			m_setPowOnStatus = 2;
		}
//		if(ballastQuery( m_daliLoop, m_confAddress, LUM_QDTR, &daliResponse)){
////			if(powOnLev == daliResponse){
//				m_setPowOnStatus = 2;
////			}
////			else{
////				m_setPowOnStatus = 0;
////			}
//		}
//		m_setPowOnStatus = 3;
		break;
	case 2:
		if (ballast_storeActLevel(m_daliLoop, m_address, true)) {
			m_setPowOnStatus = 3;
		}
		break;
	case 3:
		if (fn_commandTransmit(m_address, LUM_STORE_POWON_LEVEL, true,
				m_daliLoop) == true) {
			fn_discardPacket(m_daliLoop);
			m_setPowOnStatus = 0;
#ifdef DEVICE_DEBUG
//		printf("Power on Set\r\n");
#endif
			return true;
		}
		break;
	default:
//		if(ballastQuery(m_daliLoop, m_address, LUM_QDTR, &daliResponse)){
//			powOnLev = daliResponse;
//		}
		m_setPowOnStatus = 0;
		break;
	}

	//UNUSED(m_repeat);
	return false;
}

/*		========= ballast_storeFadeTime ==========
 *
 * @brief 	This function is called to set Fade time on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballast_storeFadeTime(daliLoop_typedef m_daliLoop, uint8_t m_address,
		daliFadeTime_typedef m_fdTime, bool m_repeat) {
	static uint8_t m_setFdTimeStatus = 0;
	static uint8_t m_daliResp = 0;

	switch (m_setFdTimeStatus) {
	case 0:
		if (ballastSplCommand(m_daliLoop, LUM_SP_DTR, m_fdTime, false, false,
				&m_daliResp)) {
			m_setFdTimeStatus = 2;
		}
		break;
	case 1:
		if (ballastQuery(m_daliLoop, m_address, LUM_QDTR, &daliResponse)) {
			m_setFdTimeStatus = 2;
		}
		break;
	case 2:
		if (fn_commandTransmit(m_address, LUM_STORE_FADE_TIME, m_repeat,
				m_daliLoop) == true) {
			fn_discardPacket(m_daliLoop);
			m_setFdTimeStatus = 0;
			return true;
		}
		break;
	}

	return false;
}

/*		========= ballast_storeFadeRate ==========
 *
 * @brief 	This function is called to set Fade rate on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballast_storeFadeRate(daliLoop_typedef m_daliLoop, uint8_t m_address,
		daliFadeRate_typedef m_fdRate, bool m_repeat) {
	static uint8_t m_setFdRateStatus = 0;
	static uint8_t m_daliResp = 0;
	static uint8_t m_confAddress = 0;

	switch (m_setFdRateStatus) {
	case 0:
		if (ballastSplCommand(m_daliLoop, LUM_SP_DTR, m_fdRate, false, false,
				&m_daliResp)) {
			m_setFdRateStatus = 2;
		}
		break;
	case 1:
		if (ballastQuery(m_daliLoop, m_confAddress, LUM_QDTR, &daliResponse)) {
			m_setFdRateStatus = 2;
		}
		break;
	case 2:
		if (fn_commandTransmit(m_address, LUM_STORE_FADE_RATE, m_repeat,
				m_daliLoop) == true) {
			fn_discardPacket(m_daliLoop);
			m_setFdRateStatus = 0;
			return true;
		}
		break;
	}
	return false;
}

/*		========= ballast_storeScene ==========
 *
 * @brief 	This function is called to store the scene set on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @params (m_sceneNumber) The scene number 0-15
 *
 * @retval true or false
 *
 */
bool ballast_storeScene(daliLoop_typedef m_daliLoop, uint8_t m_address,
bool m_repeat, uint8_t m_sceneNumber, uint8_t m_level) {
	static uint8_t m_setSceneStatus = 0;
	static uint8_t m_daliResp = 0;

	switch (m_setSceneStatus) {
	case 0:
		if (ballastSplCommand(m_daliLoop, LUM_SP_DTR, m_level, false, false,
				&m_daliResp)) {
			m_setSceneStatus = 2;
		}
		break;
	case 1:
		if (ballastQuery(m_daliLoop, m_address, LUM_QDTR, &daliResponse)) {
			m_setSceneStatus = 2;
		}
		break;
	case 2:
		if (fn_commandTransmit(m_address, LUM_STORE_SCENE | m_sceneNumber, true,
				m_daliLoop) == true) {
			fn_discardPacket(m_daliLoop);
			m_setSceneStatus = 0;
			return true;
		}
		break;

	}

	//UNUSED(m_repeat);
	return false;
}

/*		========= ballast_removeFromScene ==========
 *
 * @brief 	This function is called to remove the luminaries on the DALI bus from the scene.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @params (m_sceneNumber) The scene number 0-15
 *
 * @retval true or false
 *
 */
bool ballast_removeFromScene(daliLoop_typedef m_daliLoop, uint8_t m_address,
bool m_repeat, uint8_t m_sceneNumber) {
	if (fn_commandTransmit(m_address, LUM_REMOVE_SCENE | m_sceneNumber, true,
			m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	//UNUSED(m_repeat);
	return false;
}

/*		========= ballast_addToGroup ==========
 *
 * @brief 	This function is called to add the luminaries on the DALI bus to the group.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @params (m_groupNumber) The group number 0-15
 *
 * @retval true or false
 *
 */
bool ballast_addToGroup(daliLoop_typedef m_daliLoop, uint8_t m_address,
bool m_repeat, uint8_t m_groupNumber) {
	if (fn_commandTransmit(m_address, LUM_ADD_TO_GROUP | m_groupNumber, true,
			m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	//UNUSED(m_repeat);
	return false;
}

/*		========= ballast_removeFromGroup ==========
 *
 * @brief 	This function is called to remove the luminaries on the DALI bus from the Group.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @params (m_GroupNumber) The Group number 0-15
 *
 * @retval true or false
 *
 */
bool ballast_removeFromGroup(daliLoop_typedef m_daliLoop, uint8_t m_address,
bool m_repeat, uint8_t m_groupNumber) {
	if (fn_commandTransmit(m_address, LUM_REMOVE_FROM_GROUP | m_groupNumber,
	true, m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	//UNUSED(m_repeat);
	return false;
}

/*		========= ballast_storeShAdd ==========
 *
 * @brief 	This function is called to store the ShAdd set on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 *
 * @retval true or false
 *
 */
bool ballast_storeShAdd(daliLoop_typedef m_daliLoop, uint8_t m_address,
bool m_repeat) {
	if (fn_commandTransmit(m_address, LUM_STORE_SHORTADD, false,
			m_daliLoop) == true) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
//	//UNUSED(m_repeat);
	return false;
}

/*		========= ballast_selectDimmingCurve ==========
 *	DT6 -DT8 Only
 * @brief 	This function is called to store the dimming curve  on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @params (m_isLinear) Linear dimming curve selection
 * 1 - Linear
 * 0 - Logarithmic (default)
 *
 * @retval true or false
 *
 */
bool ballast_selectDimmingCurve(daliLoop_typedef m_daliLoop, uint8_t m_address,
bool m_repeat, uint8_t m_isLinear) {
	static uint8_t m_setDCStat = 0;
	static uint8_t m_daliResp = 0;

	switch (m_setDCStat) {
	case 0:
		if (ballastSplCommand(m_daliLoop, LUM_SP_DTR, m_isLinear, false, false,
				&m_daliResp)) {
			m_setDCStat = 2;
		}
		break;
	case 1:
		if (ballastQuery(m_daliLoop, m_address, LUM_QDTR, &daliResponse)) {
			if (m_isLinear == daliResponse) {
				m_setDCStat = 2;
			} else {
				fn_discardPacket(m_daliLoop);
				m_setDCStat = 0;
				return true;
			}
		}
		break;
	case 2:
		if (fn_commandTransmit(m_address, LUM_DIMMING_CV, true,
				m_daliLoop) == true) {
			fn_discardPacket(m_daliLoop);
			m_setDCStat = 0;
			return true;
		}
		break;

	}

	//UNUSED(m_repeat);
	return false;
}
/*		========= ballastQuery ==========
 *
 * @brief 	This function is called to execute the query commands 144-255 on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_command) Query Command
 *
 * @params (m_response) The response received
 *
 * @retval true or false
 *
 */
bool ballastQuery(daliLoop_typedef m_daliLoop, uint8_t m_address,
		uint8_t m_command, uint8_t *m_response) {
	if (fn_commandTransmit_receive(m_address, m_command, m_response,
			m_daliLoop) != false) {
		fn_discardPacket(m_daliLoop);
		return true;
	}
	return false;
}

/*		========= ballastSplCommand ==========
 *
 * @brief 	This function is called to execute the special commands on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_command1) Command 1
 *
 * @params (m_command2) Command 2
 *
 * @params (m_repeat) true or false (if the commands must be sent repeatedly)
 *
 * @params (m_response) The response received
 *
 * @retval true or false
 *
 */
bool ballastSplCommand(daliLoop_typedef m_daliLoop, uint8_t m_command1,
		uint8_t m_command2,
		bool m_repeat,
		bool m_isresponseAvailable, uint8_t *m_response) {

	static uint8_t daliSC_state = COMMAND_INIT_STATE;
	bool daliSC_retval = false;
	static uint8_t m_repeatLoc = 0;
//	uint8_t m_responseByte = 0;
	static uint16_t m_waitTime = 0;

	switch (daliSC_state) {
	case COMMAND_INIT_STATE:
		fn_discardPacket(m_daliLoop);
		m_repeatLoc = REPEAT_NUMBER - 1;
		daliSC_state = COMMAND_EXECUTION_STATE;
		break;
	case COMMAND_EXECUTION_STATE:
		if (fn_daliTransmit(m_command1, m_command2, m_daliLoop)) {
			daliSC_state = COMPLETION_STATE;
		}
		break;
	case COMPLETION_STATE:
		if (transmitDataComplete[m_daliLoop] == 1) {
//				if(m_isresponseAvailable == true){
			daliSC_state = REPEAT_STATE;
			m_waitTime = fn_GetmSecTimerStart();
//					HAL_Delay(50);
//				}
//				else{
//					daliSC_state = REPEAT_STATE;
//					m_waitTime = fn_GetmSecTimerStart();
//				}
//				daliOff_retval = true;
		}
		break;
	case RESPONSE_STATE:
		if (processResponseFrame(m_response, m_daliLoop)) {
			daliSC_state = COMMAND_INIT_STATE;
			daliSC_retval = true;
		}
		break;
	case REPEAT_STATE:
		if ((m_repeat != false) && (m_repeatLoc != 0)) {
			daliSC_state = WAIT_STATE1;
			m_repeatLoc--;
//				HAL_Delay(20);
//				m_waitTime = fn_GetuSecTimerStart();
		} else {
			daliSC_state = WAIT_STATE2;
//				HAL_Delay(20);
			m_waitTime = fn_GetmSecTimerStart();
		}
		break;
	case WAIT_STATE1:
		if (fn_IsmSecTimerElapsed(m_waitTime, 20) == true) {
//				HAL_Delay(20);
			daliSC_state = COMMAND_EXECUTION_STATE;
		}
		break;
	case WAIT_STATE2:
		if (fn_IsmSecTimerElapsed(m_waitTime, 20) == true) {
			if (m_isresponseAvailable == true) {
				daliSC_state = RESPONSE_STATE;
			} else {
				daliSC_state = COMMAND_INIT_STATE;
				daliSC_retval = true;
			}
		}
		break;
	}
	return daliSC_retval;
}

bool ballastStoreGearFeatures(daliLoop_typedef m_daliLoop, uint8_t m_address,
		uint8_t m_command) {
	static uint8_t m_setSGStat = 0;
	static uint8_t m_daliResp = 0;
	bool m_retval = false;
	static uint8_t m_repeat = 0;
	static uint16_t m_waitTime = 0;

	switch (m_setSGStat) {
	case 0:
		if (ballastSplCommand(m_daliLoop, LUM_SP_DTR, m_command, false, false,
				&m_daliResp)) {
			m_setSGStat = 1;
		}
		break;
	case 1:
		if (fn_commandTransmit(LUM_ESP_EN_DT, 8, false, m_daliLoop)) {
			fn_discardPacket(m_daliLoop);
			m_setSGStat = 2;
		}
		break;
	case 2:
		if (fn_commandTransmit((m_address << 1) | 0x01, LUM_STORE_GEAR_FEATURES,
		false, m_daliLoop) == true) {
			fn_discardPacket(m_daliLoop);
			m_repeat++;
			if (m_repeat == 2) {
				m_repeat = 0;
				m_setSGStat = 0;
				m_retval = true;
				m_waitTime = fn_GetmSecTimerStart();
			} else {
				m_setSGStat = 1;
			}
		}
		break;
	case 3:
		if (fn_IsmSecTimerElapsed(m_waitTime, 20)) {
			m_setSGStat = 4;
		}
		break;
	case 4:
		if (ballastSplCommand(m_daliLoop, LUM_SP_DTR, m_command, false, false,
				&m_daliResp)) {
			fn_discardPacket(m_daliLoop);
			m_setSGStat = 5;
		}
		break;
	case 5:
		if (fn_commandTransmit((m_address << 1) | 0x01, LUM_STORE_GEAR_FEATURES,
		true, m_daliLoop) == true) {
			fn_discardPacket(m_daliLoop);
			m_setSGStat = 0;
			m_retval = true;
		}
		break;
	}
	return m_retval;
}

bool ballastQueryRandomAddress(daliLoop_typedef m_daliLoop, uint8_t m_address,
		uint32_t *m_randomAddress) {
	static uint8_t m_qrandAdd_state = 1;
	static uint8_t m_qrandAdd_retval = false;

	switch (m_qrandAdd_state) {
	case 0:
		if (ballastBlink(m_daliLoop, m_address, true, 10) != false) {
			m_qrandAdd_state = 1;
		}
		break;
	case 1:
		if (ballastQuery(m_daliLoop, (uint8_t) ((m_address << 1) | 0x01),
				LUM_QRAND_ADD_L, &daliResponse) == true) {
			*m_randomAddress |= daliResponse;
			*m_randomAddress <<= 8;
			m_qrandAdd_state = 2;
		}
		break;
	case 2:
		if (ballastQuery(m_daliLoop, (uint8_t) (m_address << 1) | 0x01,
				LUM_QRAND_ADD_L, &daliResponse) == true) {
			*m_randomAddress |= daliResponse;
			*m_randomAddress <<= 8;
			m_qrandAdd_state = 3;
		}
		break;
	case 3:
		if (ballastQuery(m_daliLoop, (uint8_t) (m_address << 1) | 0x01,
				LUM_QRAND_ADD_L, &daliResponse) == true) {
			*m_randomAddress |= daliResponse;
			m_qrandAdd_state = 1;
			m_qrandAdd_retval = true;
		}
		break;
	}
	return m_qrandAdd_retval;
}
#ifdef DALI_LIGHT
bool ballastAddressSearch(daliLoop_typedef m_daliLoop,
		uint8_t *m_IsaddressingComplete, uint8_t *m_selection) {
//	static uint8_t addressingComplete = 0;
//	static uint8_t randomizeComplete = 1;
	static uint32_t max_Address = 0xFFFFFF;
	static uint32_t min_Address = 0;
	static uint32_t tempAddress = 0;
	static uint8_t ballastSearch_state = TERMINATE_STATE;
	static uint8_t compareLoop = 0;
	static int8_t ballastCount = 0;
	static uint8_t m_daliNoResponseFlag = 0;
//	static uint8_t m_index =0;
	bool ballastSearchRetval = false;
	static uint8_t queryRndAdd_state = 0;
	static uint32_t temp_randomAdd = 0;
	static uint16_t timeoutDelay = 0;
	static uint8_t m_previousState = 0;
	static bool m_identifyKnownBallasts = false;
	static struct_DaliBallastCfg sDaliBallastCfgLoc = { 0 };

//	if(*m_IsaddressingComplete == 0){
////		ballastSearch_state = QUERY_RDM_ADD;
////		queryRndAdd_state = 0;
//		selectAll_flag[m_daliLoop] = 0;
//	}
	if (*m_selection == 0) {
		switch (ballastSearch_state) {
		case TERMINATE_STATE:
			if (m_daliNoResponseFlag < 20) {
				if (ballastSplCommand(m_daliLoop, LUM_SP_TERMINATE, 0x00, false,
				false, &daliResponse)) {
					ballastSearch_state = QUERY_BALLAST_RESET;
					g_daliCommissionCplt = DALI_COMMISSION_INCPLT;
//					g_ballastCount[m_daliLoop] = 0;
				}
			} else {
				if ((m_identifyKnownBallasts == false)) {
					if (g_ballastCount[m_daliLoop] != 0) {
						if (ballastSplCommand(m_daliLoop, LUM_SP_TERMINATE,
								0x00, false, false, &daliResponse)) {
							ballastSearch_state = QUERY_RDM_ADD;
							ballastCount = (int8_t) g_ballastCount[m_daliLoop];
							m_identifyKnownBallasts = false;
							m_daliNoResponseFlag = 0;
						}
					} else {
						ballastSearch_state = QUERY_RDM_ADD;
						queryRndAdd_state = 3;
						m_identifyKnownBallasts = false;
						m_daliNoResponseFlag = 0;
					}
				} else {
					// IF g_randomizeDali == 0 , this snippet will be called if re-addressing flasg is true
					// this holds good for adding new ballasts with previously existing ones where the random address
					// is compromised in-order to occupy the new ballasts
					// also this holds good for adding fresh ballasts where the first iteration of ballastAddressSearch()
					// will return no ballasts and fn_queryBallastInfo() will have queued packets with random address field
					// as 0xFFFFFF
					if (g_randomizeDali == 0) {
						if (ballastSplCommand(m_daliLoop, LUM_SP_TERMINATE,
								0x00, false, false, &daliResponse)) {
							ballastSearch_state = QUERY_BALLAST_RESET;
							m_daliNoResponseFlag = 0;
						}
					} else {
						// re-addressed ballasts.. short IDs have changed by sequential assignment
						// New ballasts can be added.
						// Removed ballasts will not show up on the UI
						if (ballastSplCommand(m_daliLoop, LUM_SP_TERMINATE,
								0x00, false, false, &daliResponse)) {
							ballastSearch_state = QUERY_RDM_ADD;
							ballastCount = (int8_t) g_ballastCount[m_daliLoop];
							m_identifyKnownBallasts = false;
							m_daliNoResponseFlag = 0;
						}
					}

				}
			}
			break;
		case QUERY_RDM_ADD:
			switch (queryRndAdd_state) {
			case 0:
//				if(ballastQuery(m_daliLoop, (sDaliBallastCfg[m_daliLoop]
//					 .sDaliBallastCtrl[ballastCount-1].ballastShortID<<1)|0x01, LUM_QRAND_ADD_H,  &daliResponse)){
//					temp_randomAdd = daliResponse;
//					queryRndAdd_state  = 1;
//				}
				if (ballastQuery(m_daliLoop,
						(uint8_t) (sDaliLookUp[m_daliLoop][ballastCount - 1].dnodePtr->sDaliCfg.ballastShortID
								<< 1) | 0x01, LUM_QRAND_ADD_H, &daliResponse)) {
					temp_randomAdd = daliResponse;
					queryRndAdd_state = 1;
				}
				break;
			case 1:
				if (ballastQuery(m_daliLoop,
						(uint8_t) (sDaliLookUp[m_daliLoop][ballastCount - 1].dnodePtr->sDaliCfg.ballastShortID
								<< 1) | 0x01, LUM_QRAND_ADD_M, &daliResponse)) {
					temp_randomAdd = (temp_randomAdd << 8) | daliResponse;
					queryRndAdd_state = 2;
				}
				break;
			case 2:
				if (ballastQuery(m_daliLoop,
						(uint8_t) (sDaliLookUp[m_daliLoop][ballastCount - 1].dnodePtr->sDaliCfg.ballastShortID
								<< 1) | 0x01, LUM_QRAND_ADD_L, &daliResponse)) {
					temp_randomAdd = (temp_randomAdd << 8) | daliResponse;
					queryRndAdd_state = 3;

				}
				break;
			case 3:
				/*if((((tempAddress>>16)&0xFF) == ((temp_randomAdd>>16)&0xFF))
				 &&	(((tempAddress>>0)&0xFF) == ((temp_randomAdd>>0)&0xFF))
				 && ballastCount!=0){ */
				if (ballastCount == 0) {
					ballastCount++;
				}
				sDaliLookUp[m_daliLoop][ballastCount - 1].dnodePtr->sDaliCfg.ballastRandomID =
						temp_randomAdd;
				ballastCount = (int8_t) (ballastCount - 1);
				temp_randomAdd = 0;
				if (ballastCount <= 0) {
					ballastSearch_state = TERMINATE_STATE;
					*m_IsaddressingComplete = 1;
					*m_selection = 1;
					ballastCount = 0;
					m_daliNoResponseFlag = 0;
					ballastSearchRetval = true;
					g_shortAddress = 1;
					queryRndAdd_state = 0;
					//				selectAll_flag[m_daliLoop] = 0;
				} else {
					queryRndAdd_state = 0;
				}
				break;
			}
			break;
		case QUERY_BALLAST_RESET:
			timeoutDelay = fn_GetSecTimerStart();
			m_previousState = INIT_ADDRESSING_WINDOW;
			ballastSearch_state = DALI_COM_WAIT_STATE;

			break;
		case INIT_ADDRESSING_WINDOW:
			if (ballastSplCommand(m_daliLoop, LUM_SP_INITIALIZE_ADDRESSING,
					0x00, true, false, &daliResponse)) {
				if ((g_randomizeDali == 1)
						&& (m_identifyKnownBallasts == false)) {
					m_previousState = RANDOMIZE;
				} else if ((g_randomizeDali == 0)
						&& (m_identifyKnownBallasts == true)) {
					m_previousState = RANDOMIZE;
				} else {
					m_previousState = INPUT_ADDRESS;
				}
				timeoutDelay = fn_GetSecTimerStart();
				ballastSearch_state = DALI_COM_WAIT_STATE;

			}
			break;
		case RANDOMIZE:
			if (ballastSplCommand(m_daliLoop, LUM_SP_RANDOMIZE, 0x00, true,
			false, &daliResponse) == true) {
				tempAddress = min_Address + (max_Address - min_Address) / 2;
				timeoutDelay = fn_GetSecTimerStart();
				m_previousState = INPUT_ADDRESS;
				ballastSearch_state = DALI_COM_WAIT_STATE;
			}
			break;
		case INPUT_ADDRESS:
			if (ballastInputAddress(m_daliLoop, 0, (tempAddress >> 16) & 0xFF,
					(tempAddress >> 8) & 0xFF, (tempAddress) & 0xFF) == true) {
				ballastSearch_state = SEARCH_ADDRESS;
			}
			break;
		case SEARCH_ADDRESS:
			ballastSearch_state = COMPARE_ADDRESS;
			break;
		case COMPARE_ADDRESS:
			if (ballastSplCommand(m_daliLoop, LUM_SP_COMPARE, 0x00, false, true,
					&daliResponse) == true) {
				switch (compareLoop) {
				case 0:
					if (daliResponse == 0xFF) {
						fn_assignAddressbyte(&max_Address, max_Address,
								min_Address);
						m_daliNoResponseFlag = 0;
						if ((max_Address - min_Address) > 1) {
							tempAddress = min_Address
									+ (max_Address - min_Address) / 2;
							ballastSearch_state = INPUT_ADDRESS;
						} else {
							tempAddress = min_Address;
							ballastSearch_state = INPUT_ADDRESS;
							compareLoop = 1;
						}
					} else if (daliResponse == 0) {
						fn_assignAddressbyte(&min_Address, max_Address,
								min_Address);
						m_daliNoResponseFlag++;
						if ((max_Address - min_Address) > 1) {
							tempAddress = min_Address
									+ (max_Address - min_Address) / 2;
							ballastSearch_state = INPUT_ADDRESS;
						} else {
							tempAddress = min_Address;
							ballastSearch_state = INPUT_ADDRESS;
							compareLoop = 1;
						}
					} else {

						ballastSearch_state = INPUT_ADDRESS;
						max_Address = 0xffffffff;
						min_Address = 0;
						tempAddress = max_Address;
//						g_shortAddress = 1;
//						m_daliNoResponseFlag = 0;
//						ballastCount = 0;
//						m_errCnt++;
					}

					break;
				case 1:
					if (daliResponse == 0xFF) {
						// One of the addresses

						// Check if the other address matches the ballast
						tempAddress = max_Address;
						ballastSearch_state = INPUT_ADDRESS;
						compareLoop = 2;
					} else {
						tempAddress = max_Address;
						ballastSearch_state = INPUT_ADDRESS;
						compareLoop = 2;
					}
					break;
				case 2:
					if (daliResponse == 0xFF) {
						/*						if(m_identifyRstBallasts == true){
						 compareLoop=0;
						 ballastSearch_state = WITHDRAW_2;
						 }
						 else{
						 // This matches the address as well
						 g_shortAddress = (uint8_t)fn_isDaliShidAvailable(m_daliLoop, tempAddress);
						 ballastSearch_state = PROGRAM_SHORT_ADDRESS;
						 compareLoop=0;
						 }
						 */
						// This matches the address as well
//						g_shortAddress = (uint8_t)fn_isDaliShidAvailable(m_daliLoop, tempAddress);
						if (tempAddress < 0xFFFF00 && tempAddress != 0x800000
								&& tempAddress != 0) {
							if (g_randomizeDali == 1) {
//								g_shortAddress += fn_findFree_id(m_daliLoop);
								ballastSearch_state = PROGRAM_SHORT_ADDRESS;
								m_identifyKnownBallasts = true;
							} else {
								ballastSearch_state = QUERY_SHORTADD;
							}

						} else {
							max_Address = 0xFFFFFF;
							min_Address = 0;
							tempAddress = min_Address
									+ (max_Address - min_Address) / 2;
							ballastSearch_state = INPUT_ADDRESS;
						}

						compareLoop = 0;
					} else {
						m_daliNoResponseFlag++;
					}
					break;
				}
			}
			if (m_daliNoResponseFlag > 20) {
				ballastSearch_state = TERMINATE_STATE;
				if (m_identifyKnownBallasts == false) {
					m_identifyKnownBallasts = true;
				} else {
					m_identifyKnownBallasts = false;
				}
			}
			break;
		case QUERY_SHORTADD:
			if (ballastSplCommand(m_daliLoop, LUM_SP_QSADD, 0, false, true,
					&daliResponse) == true) {
				if (m_identifyKnownBallasts == false) {
					if (daliResponse != (0xFF)) {
						if (fn_isSHIDAvailable((uint8_t) (daliResponse >> 1),
								m_daliLoop)) {

							if (sDaliFlashLookUp[m_daliLoop][(uint8_t) (daliResponse
									>> 1) - 1].daliRandomId == tempAddress) {
								g_shortAddress = (uint8_t) (daliResponse >> 1);
								ballastSearch_state = WITHDRAW_MATCHED_ADDRESS;
							} else {
								g_shortAddress = fn_findFree_id(m_daliLoop);
								sDaliFlashLookUp[m_daliLoop][(uint8_t) g_shortAddress
										- 1].daliShID = g_shortAddress;
								ballastSearch_state = PROGRAM_SHORT_ADDRESS;
							}
						} else {
							g_shortAddress = fn_findFree_id(m_daliLoop);
							sDaliFlashLookUp[m_daliLoop][(uint8_t) g_shortAddress
									- 1].daliShID = g_shortAddress;
							ballastSearch_state = PROGRAM_SHORT_ADDRESS;
						}
					} else {
						g_shortAddress = (uint8_t) (0xFF);
						ballastSearch_state = WITHDRAW_MATCHED_ADDRESS;
					}
				} else {
					if (daliResponse == 0xFF) {
						g_shortAddress = fn_findFree_id(m_daliLoop);
						sDaliFlashLookUp[m_daliLoop][(uint8_t) g_shortAddress
								- 1].daliShID = g_shortAddress;
						ballastSearch_state = PROGRAM_SHORT_ADDRESS;
					} else {
						g_shortAddress = (uint8_t) (daliResponse >> 1);
						ballastSearch_state = WITHDRAW_MATCHED_ADDRESS;
					}

				}

			}
			break;
		case PROGRAM_SHORT_ADDRESS:
			if (tempAddress < 0xFFFF00 && tempAddress != 0x800000
					&& tempAddress != 0) {
				if (ballastSplCommand(m_daliLoop, LUM_SP_PROGRAM_SADD,
						(uint8_t) ((g_shortAddress << 1) | 0x01), false, false,
						&daliResponse) != false) {
					ballastSearch_state = VERIFY_SHORTADD;
				}
			} else {
				max_Address = 0xFFFFFF;
				min_Address = 0;
				tempAddress = min_Address + (max_Address - min_Address) / 2;
				ballastSearch_state = INPUT_ADDRESS;
			}
			break;
		case VERIFY_SHORTADD:
			if (ballastSplCommand(m_daliLoop, LUM_SP_VERIFY_SADD,
					(uint8_t) ((g_shortAddress << 1) | 0x01), false, true,
					&daliResponse) == true) {
				if (g_shortAddress == 0xFF) {
					ballastSearch_state = WITHDRAW_MATCHED_ADDRESS;
				} else {
					if (daliResponse == 0xFF /*&& tempAddress<0xFFFF00*/) {
						ballastSearch_state = WITHDRAW_MATCHED_ADDRESS;
					} else {
						max_Address = 0xFFFFFF;
						min_Address = 0;
						tempAddress = min_Address
								+ (max_Address - min_Address) / 2;
						ballastSearch_state = INPUT_ADDRESS;
					}
				}

			}
			break;
		case WITHDRAW_MATCHED_ADDRESS:
			if (ballastSplCommand(m_daliLoop, LUM_SP_WITHDRAW, (0x00), true,
			false, &daliResponse) != false) {
//				sDaliBallastCfg[m_daliLoop].sDaliBallastCtrl[ballastCount].ballastRandomID = tempAddress;
//				sDaliBallastCfg[m_daliLoop].sDaliBallastCtrl[ballastCount].ballastShortID = g_shortAddress;

				if (g_shortAddress != 0xFF) {
					memset(&sDaliBallastCfgLoc, '\0',
							sizeof(sDaliBallastCfgLoc));
					sDaliBallastCfgLoc.ballastRandomID = tempAddress;
					sDaliBallastCfgLoc.ballastShortID = g_shortAddress;
					sDaliBallastCfgLoc.ballastloop = m_daliLoop;

					if (sDaliLookUp[m_daliLoop][g_shortAddress - 1].dnodePtr
							== NULL) {
						if (fn_enQueueDC(&sDaliCfgQueue, &sDaliBallastCfgLoc,
								0)) {
							sDaliLookUp[m_daliLoop][g_shortAddress - 1].bShID =
									g_shortAddress;
							sDaliLookUp[m_daliLoop][g_shortAddress - 1].dnodePtr =
									sDaliCfgQueue.drear;

							ballastCount++;

							if (g_shortAddress
									>= (g_ballastCount[m_daliLoop])) {
								g_ballastCount[m_daliLoop] =
										(uint8_t) g_shortAddress;
							}
						} else {
#ifdef DEVICE_DEBUG
							printf(" DALI Memory allocation fault\r\n");
#endif
						}
					} else {
						sDaliLookUp[m_daliLoop][g_shortAddress - 1].dnodePtr->sDaliCfg.ballastRandomID =
								tempAddress;
						ballastCount++;

						if (g_shortAddress >= (g_ballastCount[m_daliLoop])) {
							g_ballastCount[m_daliLoop] =
									(uint8_t) g_shortAddress;
						}
					}
					if (g_randomizeDali == 1) {
						g_shortAddress++;
					}
				}
				max_Address = 0xFFFFFF;
				min_Address = 0;
				tempAddress = min_Address + (max_Address - min_Address) / 2;
				ballastSearch_state = INPUT_ADDRESS;
				m_daliNoResponseFlag = 0;
			}
			break;
		case DALI_COM_WAIT_STATE:
			if (fn_IsSecTimerElapsed(timeoutDelay, 3)) {
				ballastSearch_state = m_previousState;
			}
			break;
		}
	} else {
		ballastSearchRetval = true;
	}
//}

	return ballastSearchRetval;

}

void fn_assignAddressbyte(uint32_t *resAddress, uint32_t maxAddress,
		uint32_t minAddress) {
	*resAddress = minAddress + (maxAddress - minAddress) / 2;
}

bool ballastInputAddress(daliLoop_typedef m_daliLoop, uint8_t m_addressFeed,
		uint8_t addH, uint8_t addM, uint8_t addL) {
	static uint8_t ballastInputAddress_status = 0;
	bool ballastInputAddress_retval = false;

	switch (ballastInputAddress_status) {
	case 0:
		switch (m_addressFeed) {
		case 0:
			ballastInputAddress_status = 1;
			break;
		case 1:
			ballastInputAddress_status = 1;
			break;
		case 2:
			ballastInputAddress_status = 2;
			break;
		case 3:
			ballastInputAddress_status = 3;
			break;
		}
		break;
	case 1:
		if (ballastSplCommand(m_daliLoop, LUM_SP_SEARCH_ADDH, addH, false,
		false, &daliResponse) == true) {
			ballastInputAddress_status = 2;
		}
		break;
	case 2:
		if (ballastSplCommand(m_daliLoop, LUM_SP_SEARCH_ADDM, addM, false,
		false, &daliResponse) == true) {
			switch (m_addressFeed) {
			case 0:
				ballastInputAddress_status = 3;
				break;
			case 1:
				ballastInputAddress_retval = true;
				break;
			case 2:
				ballastInputAddress_status = 3;
				break;
			}
		}
		break;
	case 3:
		if (ballastSplCommand(m_daliLoop, LUM_SP_SEARCH_ADDL, addL, false,
		false, &daliResponse) == true) {
			ballastInputAddress_status = 0;
			ballastInputAddress_retval = true;

		}
		break;
	}
	return ballastInputAddress_retval;
}
#endif
/*		========= ballastBlink ==========
 *
 * @brief 	This function is called to toggle the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) true or false (if the commands must be sent repeatedly)
 *
 * @retval true or false
 *
 */
uint8_t ballastBlink(daliLoop_typedef m_daliLoop, uint8_t m_address,
bool m_repeat, uint16_t delay) {
	static uint8_t daliBlk_state = COMMAND_INIT_STATE;
	static uint16_t blk_delayTime = 0;

	uint8_t daliBlk_retval = false;
	static uint8_t completionState = 0;
	static int8_t m_repeatLoc = 0;

	switch (daliBlk_state) {
	case COMMAND_INIT_STATE:
		if (m_repeat) {
			m_repeatLoc = BLINK_REPEAT_NUMBER;
		}
		daliBlk_state = COMMAND_EXECUTION_STATE;
		break;
	case COMMAND_EXECUTION_STATE:
		daliBlk_state = COMPLETION_STATE;
		break;
	case COMPLETION_STATE:

		switch (completionState) {
		case 0:
//				fn_commandTransmit((m_address<<1), 254, true)
			if (fn_commandTransmit((uint8_t) (m_address << 1), 254, true,
					m_daliLoop)) {
				blk_delayTime = fn_GetmSecTimerStart();
				completionState = 1;
			}
			break;
		case 1:
			if (fn_IsmSecTimerElapsed(blk_delayTime, delay)) {
				completionState = 2;
			}
			break;
		case 2:
//				if(ballastOff(m_address<<1, false)){
			if (fn_commandTransmit((uint8_t) (m_address << 1), 0, true,
					m_daliLoop)) {
				completionState = 3;
				blk_delayTime = fn_GetmSecTimerStart();
			}
			break;
		case 3:
			if (fn_IsmSecTimerElapsed(blk_delayTime, delay)) {
				completionState = 0;
//					if(m_repeat){
				m_repeatLoc--;
				daliBlk_state = REPEAT_STATE;
//					}
//					else{
//						daliBlk_retval = true;
//						daliBlk_state = COMMAND_INIT_STATE;
//					}
			}
			break;
		}
		break;
	case REPEAT_STATE:
		if (m_repeatLoc > 0) {
			daliBlk_state = COMMAND_EXECUTION_STATE;
		} else {
			daliBlk_retval = true;
			daliBlk_state = COMMAND_INIT_STATE;

		}
		break;
	default:
		break;
	}
	return daliBlk_retval;
}

bool fn_activateColourType(daliLoop_typedef m_daliLoop, uint8_t m_address) {
	static uint8_t m_colourTypeState = STATE_ONE;
	bool m_retval = false;
	static uint8_t m_resp = 0;
	static uint16_t m_retResp = 0;

	switch (m_colourTypeState) {
	case STATE_ONE:
		if (fn_Set16bitValue(m_daliLoop, 255, &m_retResp)) {
			m_colourTypeState = STATE_TWO;
		}
		break;
	case STATE_TWO:
		if (fn_commandTransmit(LUM_ESP_EN_DT, 8, false, m_daliLoop)) {
			fn_discardPacket(m_daliLoop);
			m_colourTypeState = STATE_THREE;
		}
		break;
	case STATE_THREE:
		if (fn_commandTransmit((m_address << 1) | 0x01, LUM_SET_TEMP_TC, true,
				m_daliLoop) == true) {
			m_colourTypeState = STATE_FOUR;
		}
		break;
	case STATE_FOUR:
		if (fn_commandTransmit(LUM_ESP_EN_DT, 8, false, m_daliLoop)) {
			fn_discardPacket(m_daliLoop);
			m_colourTypeState = STATE_FIVE;
		}
		break;
	case STATE_FIVE:
		if (fn_commandTransmit((m_address << 1) | 0x01, LUM_ACTIVATE, true,
				m_daliLoop)) {
			m_colourTypeState = STATE_SIX;
		}
		break;
	case STATE_SIX:
		if (fn_commandTransmit(LUM_ESP_EN_DT, 8, false, m_daliLoop)) {
			fn_discardPacket(m_daliLoop);
			if (m_address == 0xFF) {
				m_colourTypeState = STATE_ONE;
				m_retval = true;
			} else {
				m_colourTypeState = STATE_SEVEN;
			}

		}
		break;
	case STATE_SEVEN:
		if (ballastQuery(m_daliLoop, (m_address << 1) | 0x01, LUM_QCOLOUR_STAT,
				&m_resp)) {
			if (((m_resp & 0xF0) >> 4) == 2) {
			__NOP();
		}
		fn_discardPacket(m_daliLoop);
		m_colourTypeState = STATE_ONE;
		m_retval = true;
	}
	break;
	}
	return m_retval;
}

bool fn_setTempColourTemp_Tc(uint16_t m_temp, uint8_t m_address,
		daliLoop_typedef m_daliLoop) {
	static uint8_t m_tempState = STATE_ONE;
	bool m_retval = false;

	switch (m_tempState) {
	case STATE_ONE:
		if (fn_SetSpecific16bitValue(m_daliLoop, m_temp)) {
			fn_discardPacket(m_daliLoop);
			m_tempState = STATE_TWO;
		}
		break;
	case STATE_TWO:
		if (fn_commandTransmit(LUM_ESP_EN_DT, 8, false, m_daliLoop)) {
			fn_discardPacket(m_daliLoop);
			m_tempState = STATE_THREE;
		}
		break;
	case STATE_THREE:
		if (fn_commandTransmit((m_address << 1) | 0x01, LUM_SET_TEMP_TC, true,
				m_daliLoop) == true) {
			m_tempState = STATE_FOUR;
		}
		break;
	case STATE_FOUR:
		if (fn_commandTransmit(LUM_ESP_EN_DT, 8, false, m_daliLoop)) {
			fn_discardPacket(m_daliLoop);
			m_tempState = STATE_FIVE;
		}
		break;
	case STATE_FIVE:
		if (fn_commandTransmit((m_address << 1) | 0x01, LUM_ACTIVATE, true,
				m_daliLoop) == true) {
			m_retval = true;
			m_tempState = STATE_ONE;
		}
		break;

	}
	return m_retval;
}

bool fn_findValidTcValue(daliLoop_typedef m_daliLoop, uint8_t m_address) {
	static uint8_t m_validTcState = STATE_ONE;
	bool m_retval = false;
	static uint16_t m_timeout = 0;
	static uint8_t m_resp = 0;
	static uint16_t m_respVal = 0;

	switch (m_validTcState) {
	case STATE_ONE:
		if (ballast_reset(m_daliLoop, (m_address << 1) | 0x01)) {
			m_timeout = fn_GetmSecTimerStart();
			m_validTcState = STATE_TWO;
		}
		break;
	case STATE_TWO:
		if (fn_IsmSecTimerElapsed(m_timeout, 300)) {
			m_validTcState = STATE_THREE;
		}
		break;
	case STATE_THREE:
		if (ballastSplCommand(m_daliLoop, LUM_SP_DTR, 129, false, false,
				&m_resp)) {
			m_validTcState = STATE_FOUR;
		}
		break;
	case STATE_FOUR:
		if (fn_commandTransmit(LUM_ESP_EN_DT, 8, false, m_daliLoop)) {
			fn_discardPacket(m_daliLoop);
			m_validTcState = STATE_FIVE;
		}
		break;
	case STATE_FIVE:
		if (ballastQuery(m_daliLoop, (m_address << 1) | 0x01, LUM_QCOLOUR_VAL,
				&m_resp)) {
			m_validTcState = STATE_SIX;
		}
		break;
	case STATE_SIX:
		if (fn_Get16bitValue(m_daliLoop, m_address, &m_respVal)) {
			m_validTcState = STATE_SEVEN;
		}
		break;
	case STATE_SEVEN:
		if (ballastSplCommand(m_daliLoop, LUM_SP_DTR, 131, false, false,
				&m_resp)) {
			m_validTcState = STATE_EIGHT;
		}
		break;
	case STATE_EIGHT:
		if (fn_commandTransmit(LUM_ESP_EN_DT, 8, false, m_daliLoop)) {
			fn_discardPacket(m_daliLoop);
			m_validTcState = STATE_NINE;
		}
		break;
	case STATE_NINE:
		if (ballastQuery(m_daliLoop, (m_address << 1) | 0x01, LUM_QCOLOUR_VAL,
				&m_resp)) {
			m_validTcState = STATE_TEN;
		}
		break;
	case STATE_TEN:
		if (fn_Get16bitValue(m_daliLoop, m_address, &m_respVal)) {
			m_validTcState = STATE_ONE;
			m_retval = true;
		}
		break;
	}
	return m_retval;
}

bool fn_Set16bitValue(daliLoop_typedef m_loop, uint8_t m_val, uint16_t *m_retvl) {
	static uint8_t m_set16State = STATE_ONE;
	bool m_retval = false;
	uint8_t m_resp = 0;
	static uint8_t val_msb = 0;
	static uint8_t val_lsb = 0;

	switch (m_set16State) {
	case STATE_ONE:
		switch (m_val) {
		case 0:
			val_msb = 0;
			val_lsb = 0;
			break;
		case 255:
			val_msb = 255;
			val_lsb = 255;
			break;
		case 254:
			val_msb = 255;
			val_lsb = 254;
			break;
		default:
			val_msb = m_val;
			val_lsb = 255 - m_val;
			break;
		}
		m_set16State = STATE_TWO;
		break;
	case STATE_TWO:
		if (ballastSplCommand(m_loop, LUM_SP_DTR, val_lsb, false, false,
				&m_resp)) {
			m_set16State = STATE_THREE;
		}
		break;
	case STATE_THREE:
		if (ballastSplCommand(m_loop, LUM_ESP_DTR1, val_msb, false, false,
				&m_resp)) {
			m_set16State = STATE_FOUR;
		}
		break;
	case STATE_FOUR:
		*m_retvl = val_msb;
		*m_retvl <<= 8;
		*m_retvl |= val_lsb;
		m_set16State = STATE_ONE;
		m_retval = true;
		break;
	}
	return m_retval;
}

bool fn_SetSpecific16bitValue(daliLoop_typedef m_loop, uint16_t m_val) {
	static uint8_t m_set16State = STATE_ONE;
	bool m_retval = false;
	uint8_t m_resp = 0;
	static uint8_t val_msb = 0;
	static uint8_t val_lsb = 0;

	switch (m_set16State) {
	case STATE_ONE:
		/*		switch(m_val){
		 case 0:
		 val_msb = 0;
		 val_lsb = 0;
		 break;
		 case 255:
		 val_msb = 255;
		 val_lsb = 255;
		 break;
		 case 254:
		 val_msb = 255;
		 val_lsb = 254;
		 break;
		 default:
		 val_msb = m_val;
		 val_lsb = 255-m_val;
		 break;
		 }*/
		val_msb = (m_val & 0xFF00) >> 8;
		val_lsb = (m_val & 0x00FF);
		m_set16State = STATE_TWO;
		break;
	case STATE_TWO:
		if (ballastSplCommand(m_loop, LUM_SP_DTR, val_lsb, false, false,
				&m_resp)) {
			m_set16State = STATE_THREE;
		}
		break;
	case STATE_THREE:
		if (ballastSplCommand(m_loop, LUM_ESP_DTR1, val_msb, false, false,
				&m_resp)) {
			m_set16State = STATE_FOUR;
		}
		break;
	case STATE_FOUR:
		m_set16State = STATE_ONE;
		m_retval = true;
		break;
	}
	return m_retval;
}

bool fn_Get16bitValue(daliLoop_typedef m_loop, uint8_t m_address,
		uint16_t *m_retvl) {
	static uint8_t m_get16State = STATE_ONE;
	bool m_retval = false;
	static uint16_t m_resp = 0;

	switch (m_get16State) {
	case STATE_ONE:
		if (ballastQuery(m_loop, (m_address << 1) | 0x01, LUM_QDTR,
				(uint8_t*) &m_resp)) {
			*m_retvl = m_resp;
			m_get16State = STATE_TWO;
		}
		break;
	case STATE_TWO:
		if (ballastQuery(m_loop, (m_address << 1) | 0x01, LUM_QDTR1,
				(uint8_t*) &m_resp)) {
			*m_retvl |= m_resp << 8;
			m_retval = true;
			m_get16State = STATE_ONE;
		}
		break;
	}
	return m_retval;
}

void fn_daliProcess()
{
	static uint8_t m_DaliState = QUEUE_SEGREGATION_1;
//	static uint8_t m_dresponse = 0;
	static struct_DaliAction *sPtr;
	static uint16_t m_waitTimeout = 0;
	static uint16_t m_curWaitTimeout = 0;
//	static struct_DaliAction sDaliAction_loc = {0};
#ifdef DALI_FEATURE
	static bool isWaiting = false;
#endif
	switch (m_DaliState)
	{
	case QUEUE_SEGREGATION_1:

#ifdef DALI_FEATURE
		if (!fn_isDActQEmpty())
		{

			if (criticalOpnLock == false)
			{
#ifndef RELAY_EN
				criticalOpnLock = true;
				m_DaliState = QUEUE_SEGREGATION_2;
				edaliProcessStatus = DALI_PROCESSING;
#else
				fn_deQueueAC(&sDaliActQueue);
#endif
			}
		}
//#ifdef ENABLE_QUERY
		else if (sDaliActWaitQueue.dfront != NULL)
		{
			fn_deQueueAC(&sDaliActWaitQueue);

		}
//#endif
		else if (sDaliSceneActQueue.d_scnActFront != NULL)
		{
			if (criticalOpnLock == false)
			{
#ifndef RELAY_EN
				criticalOpnLock = true;
				/*
				 memset(criticalOpnMonitor, '\0', sizeof(criticalOpnMonitor));
				 sprintf(criticalOpnMonitor, "Func: %s Line: %d", __func__, __LINE__);
				 */
				m_DaliState = SCENE_SET;
				edaliProcessStatus = DALI_PROCESSING;
#else
				fn_deQueueScnAct(&sDaliSceneActQueue);
#endif
			}
		}
		else if (sDaliActWaitQueue.dfront != NULL)
		{
#ifndef RELAY_EN

			if (GET_GRP(sDaliActWaitQueue.dfront->sDaliAct.d_add))
			{
				fn_deQueueAC(&sDaliActWaitQueue);
			}
			else
			{
				m_DaliState = QUERY_WAIT_STATUS;
				if (isWaiting == false)
				{
					m_waitTimeout = fn_GetmSecTimerStart();
					isWaiting = true;
				}
			}
			edaliProcessStatus = DALI_PROCESSING;

#else
			fn_deQueueAC(&sDaliActWaitQueue);
#endif
		}
		else if ((fn_isDActQEmpty())
				&& (sDaliActWaitQueue.dfront == NULL)
				&& (sDaliSceneActQueue.d_scnActFront == NULL))
		{
			edaliProcessStatus = DALI_PROCESS_FREE;
		}
#else
		fn_deQueueAC(&sDaliActWaitQueue);
		fn_deQueueAC(&sDaliActQueue);
		fn_deQueueScnAct(&sDaliSceneActQueue);
#endif

		break;
	case QUEUE_SEGREGATION_2:
		/*
		 * 		| G | L | Id | Address: 0xXX |
		 */
		sPtr = (struct_DaliAction*) &sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead];
		if (sPtr != NULL)
		{
			if (GET_GRP(sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_add))
			{// Group Command ?
				if (sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_add & 0x100)
				{	// Identify ?
					m_DaliState = IDENTIFY_DALI;
				}
				else if (sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_colourType)
				{
					switch (sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_colourType)
					{
					case 1:
						m_DaliState = BALLAST_TC_SET;
						break;
					default:

						break;
					}
				}
				else
				{
					m_DaliState = GROUP_LEVEL_SET;
				}
			}
			else
			{
				if (GET_SADD(sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_add) != 0)
				{// Valid ?
					if (sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_add & 0x100)
					{	// Identify ?
						m_DaliState = IDENTIFY_DALI;
					}

					else if (sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_colourType)
					{
						switch (sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_colourType)
						{
						case 1:
							m_DaliState = BALLAST_TC_SET;
							break;
						default:

							break;
						}
					}
					else
					{
						m_DaliState = BALLAST_LEVEL_SET;
					}
				}
				else
				{
					fn_deQueueAC(&sDaliActQueue);
					m_DaliState = QUEUE_SEGREGATION_1;
					criticalOpnLock = false;
				}
			}
		}
		else
		{
			fn_deQueueAC(&sDaliActQueue);
			m_DaliState = QUEUE_SEGREGATION_1;
			criticalOpnLock = false;
		}
		break;
	case BALLAST_LEVEL_SET:
		if (fn_commandTransmit(
				(uint8_t) (GET_SADD(sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_add) << 1),
				sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_Intensity, true,
				(daliLoop_typedef) GET_LOOP(
						sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_add)))
		{
#ifdef ENABLE_QUERY
			fn_enQueueAC(&sDaliActWaitQueue, &sDaliActQueue.dfront->sDaliAct);
#endif
			if (sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_report != 0)
			{
				if (GET_SADD(sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_add) != 0xFF)
				{
//					fn_enQueueAC(&sDaliActWaitQueue,
//							&sDaliActQueue.dfront->sDaliAct);
				}
				else
				{

					fn_queueLightStat(
							(uint8_t) (GET_SADD(
									sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_add)),
							((uint8_t) (
									(sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_Intensity
											== 0) ?
											0 :
											LEVEL_PERCENT(
													sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_Intensity))),
							(uint8_t) (sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_statFrom));


					/*
					 fn_enqueueBLChange(RECEIVED_ON_CLIENT,
					 ((uint8_t) (
					 (sDaliActQueue.dfront->sDaliAct.d_Intensity == 0) ?
					 0 :
					 LEVEL_PERCENT(
					 sDaliActQueue.dfront->sDaliAct.d_Intensity))),
					 (daliLoop_typedef) GET_LOOP(sDaliActQueue.dfront->sDaliAct.d_add),
					 GET_SADD(sDaliActQueue.dfront->sDaliAct.d_add), DALI,
					 sDaliActQueue.dfront->sDaliAct.d_statFrom);
					 */
					printf("Ballast level set %d from %02x\r\n", ((uint8_t) (
							(sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_Intensity
									== 0) ?
									0 :
									LEVEL_PERCENT(
											sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_Intensity))), sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_statFrom);

					switch(sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_statFrom){
					case 0x11:
						DBG_PRINT("PIR TRIGGERED \r\n");
						break;
					case 0x12:		//	ALS
						DBG_PRINT("ALS TRIGGERED \r\n");
						break;
					case 0x13:

						break;
					case 0x14:

						break;
					case 0x15:

						break;
					case 0x16:
						DBG_PRINT("AREA TRIGGERED \r\n");
						break;

					}

/*
					if(fn_IsSecTimerElapsed(DaliQueryWaitTimer, 10))
					{

					}
*/
					g_sDaliRetention.m_intensity = sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_Intensity;
					fn_deQDAct();
					m_DaliState = QUEUE_SEGREGATION_1;
					criticalOpnLock = false;
				}
			}

		}
		break;

//	case POWER_ON_LEVEL_SET:
//		if(ballast_storePowOnLevel(DALI_LOOP1, (uint8_t) ((0xFF << 1) | 0x01), true)){
//			printf("Power On level set\r\n");
////			m_DaliState = POWER_FAIL_LEVEL_SET;
//
//			fn_deQDAct();
//			m_DaliState = QUEUE_SEGREGATION_1;
//			criticalOpnLock = false;
//
//		}
//		break;
//
//	case POWER_FAIL_LEVEL_SET:
//		if(ballast_storeFailLevel(DALI_LOOP1, (uint8_t)((0xFF << 1) | 0x01), ((uint8_t) (
//				(sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_Intensity == 0) ? 0 :
//						sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_Intensity)), true))
//		{
//			fn_deQDAct();
//			m_DaliState = QUEUE_SEGREGATION_1;
//			printf("Power fail level set to \r\n");
//			criticalOpnLock = false;
//		}
//		break;

	case IDENTIFY_DALI:
		if (ballast_identify(
				(daliLoop_typedef) GET_LOOP(
						sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_add),
				(uint8_t) ((
				GET_GRP(sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_add) ? (0x80) : 0)
						| (GET_SADD(sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead].d_add) << 1)
						| 0x01)))
		{
//			sDaliBallastCfg[m_Daliloop].sDaliBallastCtrl[m_shID].ballastIdentify = false;
//			fn_deQueueAC(&sDaliActQueue);
			fn_deQDAct();
			m_DaliState = QUEUE_SEGREGATION_1;
			criticalOpnLock = false;
		}
		break;
	case GROUP_LEVEL_SET:
/*
		if (sDaliActQueue.dfront != NULL) {
			if (fn_commandTransmit(((uint8_t) (0x80) | ((uint8_t) (GET_SADD(
					sDaliActQueue.dfront->sDaliAct.d_add) << 1))),
					sDaliActQueue.dfront->sDaliAct.d_Intensity, true,
					(daliLoop_typedef) GET_LOOP(
							sDaliActQueue.dfront->sDaliAct.d_add))) {
#ifdef DEVICE_DEBUG
				printf("dali shid %d, of loop %d, intensity %d\r\n",
						GET_SADD(sDaliActQueue.dfront->sDaliAct.d_add),
						(daliLoop_typedef) GET_LOOP(sDaliActQueue.dfront->sDaliAct.d_add),
						sDaliActQueue.dfront->sDaliAct.d_Intensity);
#endif
				uint8_t m_gpids = (uint8_t) ((GET_SADD(
						sDaliActQueue.dfront->sDaliAct.d_add) << 4));
				m_gpids |= (uint8_t) (
						((daliLoop_typedef) GET_LOOP(
								sDaliActQueue.dfront->sDaliAct.d_add)
								== DALI_LOOP2) ? 0x04 : 0x02);

#ifdef ENABLE_QUERY
				fn_enQueueAC(&sDaliActWaitQueue, &sDaliActQueue.dfront->sDaliAct);
#endif
				fn_deQueueAC(&sDaliActQueue);
				m_DaliState = QUEUE_SEGREGATION_1;
				criticalOpnLock = false;
			}
		} else {
			m_DaliState = QUEUE_SEGREGATION_1;
			criticalOpnLock = false;
		}
*/
		break;
	case SCENE_SET:
		if (sDaliSceneActQueue.d_scnActFront != NULL) {
			if (ballast_gotoScene(
					(daliLoop_typedef) GET_LOOP(
							sDaliSceneActQueue.d_scnActFront->sDaliScnAct.d_ballastshId),
					(uint8_t) ((
					GET_GRP(sDaliSceneActQueue.d_scnActFront->sDaliScnAct.d_ballastshId) ?
							(0x80) : 0)
							| ((GET_SADD(
									sDaliSceneActQueue.d_scnActFront->sDaliScnAct.d_ballastshId)
									<< 1) | 0x01)),
					false,
					sDaliSceneActQueue.d_scnActFront->sDaliScnAct.d_scn)) {

#ifdef ENABLE_QUERY
				if (!GET_GRP(
								sDaliSceneActQueue.d_scnActFront->sDaliScnAct.d_ballastshId)) {	// Not a group

					memset(&g_sDaliAction, '\0', sizeof(g_sDaliAction));
					g_sDaliAction.d_statFrom = SCENE_STAT;
					g_sDaliAction.d_add =
					sDaliSceneActQueue.d_scnActFront->sDaliScnAct.d_ballastshId;
					g_sDaliAction.d_report = 0x01;
					if (!fn_enQueueAC(&sDaliActWaitQueue,
									&sDaliActQueue.dfront->sDaliAct)) {
#ifdef DEVICE_DEBUG
						printf("Memory allocation fault Line Func: %s Line: %d", __func__,
								__LINE__);
#endif

					}
				}

				uint8_t m_gpids =
				(uint8_t) ((GET_SADD(
										sDaliSceneActQueue.d_scnActFront->sDaliScnAct.d_ballastshId)
								<< 4));
				m_gpids |=
				(uint8_t) (
						((daliLoop_typedef) GET_LOOP(
										sDaliSceneActQueue.d_scnActFront->sDaliScnAct.d_ballastshId)
								== DALI_LOOP2) ? 0x04 : 0x02);
#endif
				/*
				 fn_resetEntityReport(
				 &fnSearchLSRLNode(sLSRSceneRootNode,
				 sDaliSceneActQueue.d_scnActFront->sDaliScnAct.m_dataByte)->sLightStatReport,
				 m_gpids,
				 ((daliLoop_typedef) GET_LOOP(sDaliSceneActQueue.d_scnActFront->sDaliScnAct.d_ballastshId)
				 == DALI_LOOP2) ? GP_DL2 : GP_DL1);
				 */

				printf("dali scene shid/gpId %d, of loop %d, scene %d\r\n",
						GET_SADD(
								sDaliSceneActQueue.d_scnActFront->sDaliScnAct.d_ballastshId),
						(daliLoop_typedef) GET_LOOP(
								sDaliSceneActQueue.d_scnActFront->sDaliScnAct.d_ballastshId),
						sDaliSceneActQueue.d_scnActFront->sDaliScnAct.d_scn);
				fn_deQueueScnAct(&sDaliSceneActQueue);
				m_DaliState = QUEUE_SEGREGATION_1;
				criticalOpnLock = false;
			}
		}
		break;
	case QUERY_BALLAST_STATUS:
/*
		if (GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add) != 0xFF) {
			if (ballastQuery(
					(daliLoop_typedef) GET_LOOP(
							sDaliActWaitQueue.dfront->sDaliAct.d_add),
					(uint8_t) (GET_SADD(
							sDaliActWaitQueue.dfront->sDaliAct.d_add) << 1)
							| 0x01, LUM_QSTATUS, &m_dresponse)) {
				if ((m_dresponse & 0x10)) {
					m_DaliState = QUEUE_SEGREGATION_1;		// FADE RUNNING
					criticalOpnLock = false;
				} else if (m_dresponse & 0x01) {		// CONTROL GEAR FAILURE
#ifdef DEVICE_DEBUG
						printf("CG failure shID: %d for loop %d\r\n",
								GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add),
								(daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add));


					 fn_enqueueDaliError(GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add),
					 (daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add),
					 (m_dresponse & 0x01));

#endif
					fn_deQueueAC(&sDaliActWaitQueue);
					m_DaliState = QUEUE_SEGREGATION_1;
					criticalOpnLock = false;
				} else if (m_dresponse & 0x02) {				// LAMP FAILURE
#ifdef DEVICE_DEBUG
						printf("Lamp failure shID: %d for loop %d\r\n",
								GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add),
								(daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add));


					 fn_enqueueDaliError(GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add),
					 (daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add),
					 (m_dresponse & 0x02));

#endif
					fn_deQueueAC(&sDaliActWaitQueue);
					m_DaliState = QUEUE_SEGREGATION_1;
					criticalOpnLock = false;
				} else {
					if (m_dresponse & 0x08) {
#ifdef DEVICE_DEBUG
						printf("Limit error shID: %d for loop %d\r\n",
								GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add),
								(daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add));


						 fn_enqueueDaliError(GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add),
						 (daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add),
						 (m_dresponse & 0x08));
#endif
					}
					if (m_dresponse & 0x80) {
#ifdef DEVICE_DEBUG
						printf("Power Cycle shID: %d for loop %d\r\n",
								GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add),
								(daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add));


						 fn_enqueueDaliError(GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add),
						 (daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add),
						 (m_dresponse & 0x80));

#endif
					}
					if (m_dresponse == 0) {
						if (sDaliActWaitQueue.dfront->sDaliAct.d_Intensity
								!= 0) {
#ifdef DEVICE_DEBUG
							printf("Non-responsive shID: %d for loop %d\r\n",
									GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add),
									(daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add));


							 fn_enqueueDaliError(GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add),
							 (daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add), 0);
#endif
						}
					}
					m_DaliState = QUERY_BALLAST_CUR;
				}
			}
		} else {
			m_DaliState = QUEUE_SEGREGATION_1;
			fn_deQueueAC(&sDaliActWaitQueue);
			criticalOpnLock = false;
		}
*/
		break;
	case QUERY_BALLAST_CUR:
/*
		if (ballastQuery(
				(sDaliActWaitQueue.dfront->sDaliAct.d_add & 0x0200) ?
						DALI_LOOP2 : DALI_LOOP1,
				(uint8_t) (GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add)
						<< 1) | 0x01, LUM_QCUR_LEVEL, &m_dresponse)) {
			if (m_dresponse != 0xFF) {

				 fn_enqueueBLChange(RECEIVED_ON_CLIENT,
				 ((uint8_t) ((m_dresponse == 0) ? 0 : LEVEL_PERCENT(m_dresponse))),
				 (daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add),
				 GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add), DALI,
				 sDaliActWaitQueue.dfront->sDaliAct.d_statFrom);

				fn_queueLightStat(
						(uint8_t) (GET_SADD(
								sDaliActWaitQueue.dfront->sDaliAct.d_add)),
						((uint8_t) (
								(m_dresponse == 0) ?
										0 : LEVEL_PERCENT(m_dresponse))),
						(uint8_t) (sDaliActWaitQueue.dfront->sDaliAct.d_statFrom));

				sDaliActWaitQueue.dfront->sDaliAct.d_report &=
						(uint8_t) ~(0x01);
				fn_deQueueAC(&sDaliActWaitQueue);
				m_DaliState = QUEUE_SEGREGATION_1;
				criticalOpnLock = false;
			} else {
//				sprintf(m_daliHangState, "shid: %d\r\n", sDaliActWaitQueue.dfront->sDaliAct.d_add);
				m_curWaitTimeout = fn_GetmSecTimerStart();
				m_DaliState = QUERY_WAIT_STATUS_2;
			}
		}
*/

		break;

	case BALLAST_TC_SET:

		if (fn_setTempColourTemp_Tc(sDaliActQueue.dfront->sDaliAct.d_Tc,
				(uint8_t) ((
				GET_GRP(sDaliActQueue.dfront->sDaliAct.d_add) ? (0x40) : 0)
						| (GET_SADD(sDaliActQueue.dfront->sDaliAct.d_add))),
				(daliLoop_typedef) GET_LOOP(
						sDaliActQueue.dfront->sDaliAct.d_add))) {
			// 0x40 because the short id will be left shift in the function
			if (GET_GRP(sDaliActQueue.dfront->sDaliAct.d_add)) {
		/*				uint8_t m_gpids = (uint8_t) ((GET_SADD(
						sDaliActQueue.dfront->sDaliAct.d_add) << 4));

				m_gpids |= (uint8_t) (
						((daliLoop_typedef) GET_LOOP(
								sDaliActQueue.dfront->sDaliAct.d_add)
								== DALI_LOOP2) ? 0x04 : 0x02);

				 | ((daliLoop_typedef) GET_LOOP(sDaliActQueue.dfront->sDaliAct.d_add) == DALI_LOOP1) ? 0x02 : 0x04)

				 fn_resetEntityReport(
				 &fnSearchLSRLNode(sLSRrootNode, sDaliActQueue.dfront->sDaliAct.d_dataByte)->sLightStatReport,
				 m_gpids,
				 ((daliLoop_typedef) GET_LOOP(sDaliActQueue.dfront->sDaliAct.d_add) == DALI_LOOP2) ?
				 GP_DL2 : GP_DL1);
*/

			} else {
				if (GET_SADD(sDaliActQueue.dfront->sDaliAct.d_add) != 0xFF) {

/*
					 fn_enqueueBLChange(RECEIVED_ON_CLIENT,
					 fn_mirekToKelvin(sDaliActQueue.dfront->sDaliAct.d_Tc),
					 (daliLoop_typedef) GET_LOOP(sDaliActQueue.dfront->sDaliAct.d_add),
					 GET_SADD(sDaliActQueue.dfront->sDaliAct.d_add), DALI,
					 sDaliActQueue.dfront->sDaliAct.d_statFrom);
*/

				}
			}
			fn_deQueueAC(&sDaliActQueue);
			m_DaliState = QUEUE_SEGREGATION_1;
			criticalOpnLock = false;
		}

		break;
	case QUERY_WAIT_STATUS:
/*
		if (fn_IsmSecTimerElapsed(m_waitTimeout, 1000)) {
			if (criticalOpnLock == false) {

				criticalOpnLock = true;

				 memset(criticalOpnMonitor, '\0', sizeof(criticalOpnMonitor));
				 sprintf(criticalOpnMonitor, "Func: %s Line: %d", __func__, __LINE__);

				m_DaliState = QUERY_BALLAST_STATUS;
#ifdef DALI_FEATURE
				isWaiting = false;
#endif
			} else {
				m_DaliState = QUEUE_SEGREGATION_1;
			}
		} else {
			m_DaliState = QUEUE_SEGREGATION_1;
		}
*/
		break;
	case QUERY_WAIT_STATUS_2:
		if (fn_IsmSecTimerElapsed(m_curWaitTimeout, 200)) {
			m_DaliState = QUERY_BALLAST_CUR;
		}
		break;
#ifdef ENABLE_QUERY
		case 2:
//		if(GET_SADD(sDaliActQueue.dfront->sDaliAct.d_add) != 0xFF){
// 			ENABLE THE BELOW TO SET UP QUERY

		if (ballastQuery((daliLoop_typedef) GET_LOOP(sDaliActQueue.dfront->sDaliAct.d_add),
						(GET_SADD(sDaliActQueue.dfront->sDaliAct.d_add) << 1) | 0x01, LUM_QSTATUS,
						&m_dresponse))
		{
			if ((m_dresponse & 0x10) == 0x10)
			{
				m_DaliState = 0;
				criticalOpnLock = false;
			}
			else if ((m_dresponse & 0x02) == 0x2)
			{
				fn_deQueueAC(&sDaliActWaitQueue);
				m_DaliState = 0;
				criticalOpnLock = false;
			}
			else
			{
				m_DaliState = 11;
			}
		}

		m_DaliState = BALLAST_LEVEL_SET;

		break;
		case 4:

		if (GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add) != 0xFF)
		{
//			if(sDaliBallastCfg[m_Daliloop].sDaliBallastCtrl[m_shID].loopStatus_reportEnd != DO_NOT_REPORT){
			if (ballastQuery((daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add),
							(GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add) << 1) | 0x01, LUM_QSTATUS,
							&m_dresponse))
			{
				if ((m_dresponse & 0x10) == 0x10)
				{
					m_DaliState = 0;
					criticalOpnLock = false;
				}
				else if (m_dresponse & 0x02)
				{				// LAMP FAILURE
					printf("Lamp failure shID: %d for loop %d\r\n",
							GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add),
							(daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add));
					fn_deQueueAC(&sDaliActWaitQueue);
					m_DaliState = 0;
					criticalOpnLock = false;
				}
				else
				{
					m_DaliState = 5;
				}
			}
//			}
//			else{
//				m_DaliState = 0;
//				criticalOpnLock = false;
//			}
		}
		else
		{
//			if(sDaliBallastCfg[m_Daliloop].sDaliBallastCtrl[m_shID].loopStatus_reportEnd != DO_NOT_REPORT){
//				fn_enqueueBLChange(sDaliBallastCfg[m_Daliloop].sDaliBallastCtrl[m_shID].loopStatus_reportEnd
//						, fn_roundtoNearestTen((uint8_t)((sDaliBallastCfg[m_Daliloop].sDaliBallastCtrl[m_shID].ballastIntensity == 0) ?
//								 0: sDaliBallastCfg[m_Daliloop].sDaliBallastCtrl[m_shID].ballastIntensity/2.54)), m_Daliloop,
//						sDaliBallastCfg[m_Daliloop].sDaliBallastCtrl[m_shID].ballastShortID, DALI);

			fn_enqueueBLChange(RECEIVED_ON_CLIENT,
					(LEVEL_PERCENT(sDaliActWaitQueue.dfront->sDaliAct.d_Intensity) == 0) ?
					0 : LEVEL_PERCENT(sDaliActWaitQueue.dfront->sDaliAct.d_Intensity),
					(daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add),
					GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add), DALI);

//				sDaliBallastCfg[m_Daliloop].sDaliBallastCtrl[m_shID].loopStatus_reportEnd = DO_NOT_REPORT;
			m_DaliState = 7;
			m_dresponse =
			(sDaliActWaitQueue.dfront->sDaliAct.d_Intensity == 0) ?
			0 : sDaliActWaitQueue.dfront->sDaliAct.d_Intensity;
//			}
//			else{
//				m_DaliState = 0;
//				criticalOpnLock = false;
//			}

		}

		break;
		case 5:

		switch (sDaliActWaitQueue.dfront->sDaliAct.d_report)
		{
			case 0:
			fn_deQueueAC(&sDaliActWaitQueue);
			m_DaliState = 0;
			criticalOpnLock = false;
			break;
			case 1:
			if (ballastQuery(
							(sDaliActWaitQueue.dfront->sDaliAct.d_add & 0x0200) ? DALI_LOOP2 : DALI_LOOP1,
							(GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add) << 1) | 0x01,
							LUM_QCUR_LEVEL, &m_dresponse))
			{
				//			printf("Dali level: %d\r\n", fn_roundtoNearestTen((uint8_t)(m_dresponse/2.55)));
				if (m_dresponse != 0xFF)
				{
					fn_enqueueBLChange(RECEIVED_ON_CLIENT,
							((uint8_t) ((m_dresponse == 0) ? 0 : LEVEL_PERCENT(m_dresponse))),
							(daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add),
							GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add), DALI);
					sDaliActWaitQueue.dfront->sDaliAct.d_report &= ~(0x01);
					//			sDaliBallastCfg[m_Daliloop].sDaliBallastCtrl[m_shID].loopStatus_reportEnd = DO_NOT_REPORT;
					m_DaliState = 7;
				}
				else
				{
					m_DaliState = 4;
				}

			}
			break;
			default:
			fn_deQueueAC(&sDaliActWaitQueue);
			m_DaliState = 0;
			criticalOpnLock = false;
			break;
		}

		break;
		case 7:
		/*	if(ballast_storePowOnLevel((daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add),
		 (GET_GRP(sDaliActWaitQueue.dfront->sDaliAct.d_add) ? (0x80) : 0 ) |
		 (GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add)<<1)|0x01, true)){
		 //			fn_deQueueAC(&sDaliActWaitQueue);
		 m_DaliState = 13;
		 //			criticalOpnLock = false;
		 }*/
		fn_deQueueAC(&sDaliActWaitQueue);
		m_DaliState = QUEUE_SEGREGATION_1;
		criticalOpnLock = false;
		break;

		case 8:
//		if(sDaliActQueue.dfront != NULL){
//			 if(ballastQuery( (daliLoop_typedef) GET_LOOP(sDaliActQueue.dfront->sDaliAct.d_add),
//					 ((0x80)|((GET_SADD(sDaliActQueue.dfront->sDaliAct.d_add)<<1))), LUM_QSTATUS, &m_dresponse)){
//				 if(!(m_dresponse & 0x10)){
//					 m_DaliState = 9;
//				 }
//				 else{
//					fn_enQueueAC(&sDaliActWaitQueue, &sDaliActQueue.dfront->sDaliAct);
//					fn_deQueueAC(&sDaliActQueue);
//					m_DaliState = 0;
//					criticalOpnLock = false;
//				 }
//			 }
//		}
		/*else*/
		if (sDaliActWaitQueue.dfront != NULL)
		{
			if (ballastQuery((daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add),
							((0x80) | ((GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add) << 1))) | 0x01,
							LUM_QSTATUS, &m_dresponse))
			{
				if (!(m_dresponse & 0x10))
				{
					m_DaliState = 7;
				}
				else
				{
					m_DaliState = QUEUE_SEGREGATION_1;
					criticalOpnLock = false;
				}
			}
		}
		else
		{
			m_DaliState = QUEUE_SEGREGATION_1;
			criticalOpnLock = false;
		}

		break;

		case 11:

		if (ballastQuery((daliLoop_typedef) GET_LOOP(sDaliActQueue.dfront->sDaliAct.d_add),
						(GET_SADD(sDaliActQueue.dfront->sDaliAct.d_add) << 1) | 0x01, LUM_QLAMP_ON,
						&m_dresponse))
		{
			if (m_dresponse == 0xFF)
			{
				m_DaliState = 3;
			}
			else
			{
				m_DaliState = 3;
			}

		}
		m_DaliState = 3;

		break;
		case 12:
		if (ballastOn((daliLoop_typedef) GET_LOOP(sDaliActQueue.dfront->sDaliAct.d_add),
						(GET_SADD(sDaliActQueue.dfront->sDaliAct.d_add) << 1) | 0x01, true))
		{
			m_DaliState = 3;
		}
		break;
		case 13:
		if (ballast_storeFailLevel((daliLoop_typedef) GET_LOOP(sDaliActWaitQueue.dfront->sDaliAct.d_add),
						(GET_GRP(sDaliActWaitQueue.dfront->sDaliAct.d_add) ? (0x80) : 0)
						| (GET_SADD(sDaliActWaitQueue.dfront->sDaliAct.d_add) << 1) | 0x01, true))
		{
			fn_deQueueAC(&sDaliActWaitQueue);
			m_DaliState = QUEUE_SEGREGATION_1;
			criticalOpnLock = false;
		}
		break;
#endif
	}
}
#ifdef DALI_LIGHT
bool fn_queryBallastInfo() {
	static uint8_t m_shid = 1;
	static uint32_t m_randId = 0;
	static uint8_t stage = 2;
	static uint8_t m_status = STATE_ONE;
	bool m_retval = false;
	static uint16_t m_timeoutDaliQuery = 0;
	static daliLoop_typedef m_loop = DALI_LOOP1;
	static struct_DaliBallastCfg sDaliBallastCfgLoc = { 0 };

	switch (m_status) {
	case STATE_ONE:
		/*
		 if(m_shid<=g_ballastCount[m_loop]){

		 m_status = STATE_TWO;
		 }
		 else{
		 if(m_loop!=DALI_LOOP2){
		 m_loop = DALI_LOOP2;
		 }
		 else{
		 m_loop = DALI_LOOP1;
		 m_retval = true;
		 }
		 m_shid = 1;
		 }
		 */
		m_status = STATE_TWO;
		break;
	case STATE_TWO:
		if (ballastQuery(m_loop, (m_shid << 1) | 1, LUM_QRAND_ADD_H,
				((uint8_t*) &m_randId) + stage)) {
			m_status = STATE_THREE;
			stage--;
		}
		break;
	case STATE_THREE:
		if (ballastQuery(m_loop, (m_shid << 1) | 1, LUM_QRAND_ADD_M,
				((uint8_t*) &m_randId) + stage)) {
			m_status = STATE_FOUR;
			stage--;
		}
		break;
	case STATE_FOUR:
		if (ballastQuery(m_loop, (m_shid << 1) | 1, LUM_QRAND_ADD_L,
				((uint8_t*) &m_randId) + stage)) {
			m_status = STATE_FIVE;
			stage = 2;
			/*			sDaliComInfo[m_loop][m_shid].balRandomID = m_randId;
			 sDaliComInfo[m_loop][m_shid].shortID = m_shid;*/

			if (m_randId < 0xFFFF00 && m_randId != 0x800000 && m_randId != 0) {
				memset(&sDaliBallastCfgLoc, '\0', sizeof(sDaliBallastCfgLoc));
				sDaliBallastCfgLoc.ballastRandomID = m_randId;
				sDaliBallastCfgLoc.ballastShortID = m_shid;
				sDaliBallastCfgLoc.ballastloop = m_loop;

				if (fn_enQueueDC(&sDaliCfgQueue, &sDaliBallastCfgLoc, 0)) {
					sDaliLookUp[m_loop][m_shid - 1].bShID = m_shid;
					sDaliLookUp[m_loop][m_shid - 1].dnodePtr =
							sDaliCfgQueue.drear;

				} else {
#ifdef DEVICE_DEBUG
					printf(" DALI Memory allocation fault\r\n");
#endif
				}
			} else {
				if ((m_randId == 0)
						&& (sDaliFlashLookUp[m_loop][m_shid - 1].daliShID
								== m_shid)) {
					memset(&sDaliBallastCfgLoc, '\0',
							sizeof(sDaliBallastCfgLoc));
					sDaliBallastCfgLoc.ballastRandomID = m_randId;
					sDaliBallastCfgLoc.ballastShortID = m_shid;
					sDaliBallastCfgLoc.ballastloop = m_loop;

					if (fn_enQueueDC(&sDaliCfgQueue, &sDaliBallastCfgLoc, 0)) {
						sDaliLookUp[m_loop][m_shid - 1].bShID = m_shid;
						sDaliLookUp[m_loop][m_shid - 1].dnodePtr =
								sDaliCfgQueue.drear;

					} else {
#ifdef DEVICE_DEBUG
						printf(" DALI Memory allocation fault\r\n");
#endif
					}
				}
			}
		}
		break;
	case STATE_FIVE:
		m_shid++;

		if (m_shid > 63) {
			if (m_loop != DALI_LOOP2) {
				m_loop = DALI_LOOP2;
			} else {
				m_loop = DALI_LOOP1;
				m_retval = true;
			}
			m_shid = 1;
		} else {
			/*			if(g_ballastCount[m_loop] != 0){
			 if(m_shid>g_ballastCount[m_loop]){
			 if(m_loop!=DALI_LOOP2){
			 m_loop = DALI_LOOP2;
			 }
			 else{
			 m_loop = DALI_LOOP1;
			 m_retval = true;
			 }
			 m_shid = 1;
			 }
			 }*/

		}
		m_status = STATE_SIX;
		m_timeoutDaliQuery = fn_GetmSecTimerStart();
		break;
	case STATE_SIX:
		if (fn_IsmSecTimerElapsed(m_timeoutDaliQuery, 200)) {
			m_status = STATE_ONE;
		}
		break;
	}
	return m_retval;
}

bool fn_queryDT() {
	static uint8_t m_shid = 1;
	static uint8_t m_status = STATE_ONE;
	bool m_retval = false;
	static uint8_t m_response = 0;
//	static uint16_t m_timeoutDaliQuery = 0;
	static daliLoop_typedef m_loop = DALI_LOOP1;

	switch (m_status) {
	case STATE_ONE:
		if (m_shid <= g_ballastCount[m_loop]) {
			m_status = STATE_TWO;
		} else {
			if (m_loop != DALI_LOOP2) {
				m_loop = DALI_LOOP2;
			} else {
				m_loop = DALI_LOOP1;
				m_status = STATE_THREE;
//				m_retval = true;
			}
			m_shid = 1;
		}
		break;
	case STATE_TWO:
		if (ballastQuery(m_loop, (m_shid << 1) | 1, LUM_QDEVICE_TYPE,
				&m_response)) {
			if (m_response != 0xFF) {
				if (m_response == 6) {
					sDaliFlashLookUp[m_loop][m_shid - 1].isDT8 = 2;
				} else if (m_response == 1) {
					sDaliFlashLookUp[m_loop][m_shid - 1].isDT8 = 3;
				} else if (m_response == 8) {
					sDaliFlashLookUp[m_loop][m_shid - 1].isDT8 = 1;
				} else if (m_response == 2) {
					sDaliFlashLookUp[m_loop][m_shid - 1].isDT8 = 0;
				}

			} else {
				sDaliFlashLookUp[m_loop][m_shid - 1].isDT8 = 0;
			}
			m_shid++;
			m_status = STATE_ONE;
		}
		break;
	case STATE_THREE:
		/*
		 if (fn_Erase(DALI_LOOKUP_SECTOR * FLASH_SECTOR_SIZE, WT_SECTOR) == FLASH_OK)
		 {
		 m_status = STATE_FOUR;
		 }
		 */
		m_status = STATE_FOUR;
		break;
	case STATE_FOUR:
		/*
		 updatedConfig = 0;
		 if ((fn_writeSector(DALI_LOOKUP_SECTOR, (uint8_t*) &sDaliFlashLookUp,
		 sizeof(sDaliFlashLookUp), sizeof(updatedConfig))))
		 {
		 if (fn_writeSector(DALI_LOOKUP_SECTOR, (uint8_t*) &updatedConfig, sizeof(updatedConfig),
		 0))
		 {
		 m_status = STATE_ONE;
		 #ifdef DEVICE_DEBUG
		 printf("Dali Flash Updated\r\n");
		 #endif
		 m_retval = true;
		 }
		 }
		 */
		m_status = STATE_ONE;
		break;
	}
	return m_retval;
}
#endif
void fn_updateDaliConfig() {
	static uint8_t m_daliConfigState = 0;
//	static bool m_FailLevelSet = false;

	switch (m_daliConfigState) {
	case 0:

		if ((sDaliCfgQueue.dfront != NULL)) {
			if ((/*sDaliCfgQueue.dfront->cfgFlashUpdateFlag == false &&*/sDaliCfgQueue.dfront->cfgChangeFlag
					!= 0)) {
				if (criticalOpnLock == false) {
					criticalOpnLock = true;
					/*
					 memset(criticalOpnMonitor, '\0', sizeof(criticalOpnMonitor));
					 sprintf(criticalOpnMonitor, "Func: %s Line: %d", __func__, __LINE__);
					 */
					m_daliConfigState = 1;
				}
			}
		} else if ((sDaliGrpQueue.dfront != NULL)
		/*&& (sConfigRcvFlags.ballast_grouping_rcv != true)
		 && (sConfigRcvFlags.ballast_groupingLookup != true)*/) {
			if (criticalOpnLock == false) {
				criticalOpnLock = true;
				/*
				 memset(criticalOpnMonitor, '\0', sizeof(criticalOpnMonitor));
				 sprintf(criticalOpnMonitor, "Func: %s Line: %d", __func__, __LINE__);
				 */
				m_daliConfigState = 1;
			}
		} else if ((sDaliSceneCfgQueue.d_scnCfgFront != NULL)
				&& (sDaliSceneCfgQueue.d_scnCfgFront->d_ScFlag != 0)
				/*&& (sConfigRcvFlags.sceneCfg_rcv != true)
				 && (sConfigRcvFlags.sceneCfg_Lookup != true)*/) {
			if (criticalOpnLock == false) {
				criticalOpnLock = true;
				/*
				 memset(criticalOpnMonitor, '\0', sizeof(criticalOpnMonitor));
				 sprintf(criticalOpnMonitor, "Func: %s Line: %d", __func__, __LINE__);
				 */
				m_daliConfigState = 1;
			}
		} else if (sDaliCfgQueue.dfront == NULL && sDaliGrpQueue.dfront == NULL
				&& sDaliSceneCfgQueue.d_scnCfgFront == NULL) {
			edaliConfigStatus = DALI_FREE;
		}

		break;
	case 1:

		if (sDaliGrpQueue.dfront->d_Flg == GRP_ADDITION)
		{
			m_daliConfigState = 8;
		}
		else if (sDaliGrpQueue.dfront->d_Flg == GRP_DELETION)
		{
			m_daliConfigState = 9;
		}
		else if (sDaliSceneCfgQueue.d_scnCfgFront->d_ScFlag == SCN_ADDITION)
		{
			m_daliConfigState = 10;
		}
		else if (sDaliSceneCfgQueue.d_scnCfgFront->d_ScFlag == SCN_DELETION)
		{
			m_daliConfigState = 11;
		}
		else
		{
			if (sDaliCfgQueue.dfront->cfgChangeFlag == 0)
			{
				m_daliConfigState = 0;
				fn_deQueueDC(&sDaliCfgQueue);
				criticalOpnLock = false;
				/*
				 if(m_FailLevelSet == false){
				 m_FailLevelSet = true;
				 m_daliConfigState = 6;

				 }
				 else{
				 m_FailLevelSet = false;
				 m_daliConfigState = 0;
				 fn_deQueueDC(&sDaliCfgQueue);
				 criticalOpnLock = false;
				 }*/
			}
			else
			{

				if (sDaliCfgQueue.dfront->cfgChangeFlag & 0x01)
				{
					m_daliConfigState = 2;
				}
				else if (sDaliCfgQueue.dfront->cfgChangeFlag & 0x02)
				{
					m_daliConfigState = 3;
				}
				else if (sDaliCfgQueue.dfront->cfgChangeFlag & 0x04)
				{
					m_daliConfigState = 4;
				}
				else if (sDaliCfgQueue.dfront->cfgChangeFlag & 0x08)
				{
					m_daliConfigState = 5;
				}
				else if (sDaliCfgQueue.dfront->cfgChangeFlag & 0x10)
				{
					m_daliConfigState = 6;
				}
				else if (sDaliCfgQueue.dfront->cfgChangeFlag & 0x20)
				{
					m_daliConfigState = 7;
				}
			}
		}

		break;
	case 2:
		if (ballast_storeFadeTime(sDaliCfgQueue.dfront->sDaliCfg.ballastloop,
				(uint8_t) (sDaliCfgQueue.dfront->sDaliCfg.ballastShortID << 1
						| 0x01),
				(daliFadeTime_typedef) sDaliCfgQueue.dfront->sDaliCfg.sLightConfig.fadeTime,
				true)) {
			sDaliCfgQueue.dfront->cfgChangeFlag &= (uint8_t) ~(0x01);
//			sDaliCfgQueue.dfront->cfgChangeFlag = 0;
			m_daliConfigState = 1;
		}
		break;
	case 3:
		if (ballast_storeFadeRate(sDaliCfgQueue.dfront->sDaliCfg.ballastloop,
				(uint8_t) (sDaliCfgQueue.dfront->sDaliCfg.ballastShortID << 1
						| 0x01),
				(daliFadeRate_typedef) sDaliCfgQueue.dfront->sDaliCfg.sLightConfig.fadeRate,
				true)) {
			sDaliCfgQueue.dfront->cfgChangeFlag &= (uint8_t) ~(0x02);
			m_daliConfigState = 1;
		}
//		sDaliCfgQueue.dfront->cfgChangeFlag = 0;
//		m_daliConfigState = 0;
		break;
	case 4:
		if (ballast_storeMaxLevel(sDaliCfgQueue.dfront->sDaliCfg.ballastloop,
				(uint8_t) (sDaliCfgQueue.dfront->sDaliCfg.ballastShortID << 1
						| 0x01),
				sDaliCfgQueue.dfront->sDaliCfg.sLightConfig.presetMax, true)) {
			sDaliCfgQueue.dfront->cfgChangeFlag &= (uint8_t) ~(0x04);
			m_daliConfigState = 1;
		}
//		sDaliCfgQueue.dfront->cfgChangeFlag = 0;
//		m_daliConfigState = 0;
		break;
	case 5:
		if (ballast_storeMinLevel(sDaliCfgQueue.dfront->sDaliCfg.ballastloop,
				(sDaliCfgQueue.dfront->sDaliCfg.ballastShortID << 1 | 0x01),
				sDaliCfgQueue.dfront->sDaliCfg.sLightConfig.presetMin, true)) {
			sDaliCfgQueue.dfront->cfgChangeFlag &= ~(0x08);
			m_daliConfigState = 1;
		}
//		sDaliCfgQueue.dfront->cfgChangeFlag = 0;
//		m_daliConfigState = 1;
		break;
	case 6:
		if (ballast_storeFailLevel(sDaliCfgQueue.dfront->sDaliCfg.ballastloop,
				(sDaliCfgQueue.dfront->sDaliCfg.ballastShortID << 1 | 0x01),
				sDaliCfgQueue.dfront->sDaliCfg.sLightConfig.sysFailLevel,
				true)) {
			sDaliCfgQueue.dfront->cfgChangeFlag &= ~(0x10);
#ifdef DEVICE_DEBUG
			printf("power fail level %d set %d\r\n",
					sDaliCfgQueue.dfront->sDaliCfg.sLightConfig.sysFailLevel,
					sDaliCfgQueue.dfront->sDaliCfg.ballastShortID);
#endif
			sDaliCfgQueue.dfront->cfgChangeFlag = 0;
			m_daliConfigState = 1;
		}
//		sDaliCfgQueue.dfront->cfgChangeFlag = 0;
//		m_daliConfigState = 1;
		break;
	case 7:
//		if(ballast_storePowOnLevel(sDaliCfgQueue.dfront->sDaliCfg.ballastloop,
//			(sDaliCfgQueue.dfront->sDaliCfg.ballastShortID<<1 | 0x01),
//				sDaliCfgQueue.dfront->sDaliCfg.sLightConfig.powFailLevel, true)){
//			sDaliCfgQueue.dfront->cfgChangeFlag &= ~(0x20);
//			m_daliConfigState = 0;
//		}
		sDaliCfgQueue.dfront->cfgChangeFlag = 0;
		m_daliConfigState = 1;
		break;
	case 8:
		/*if(ballast_addToGroup(m_Daliloop, (sDaliBallastCfg[m_Daliloop].sDaliBallastCtrl[m_shID].ballastShortID<<1 | 0x01),
		 true, sDaliBallastCfg[m_Daliloop].sLightConfig[m_shID].sGroupDet.DaliGroups[sdaliGroupStatus[m_Daliloop].d_ModificationIndex[m_shID]])){
		 sdaliGroupStatus[m_Daliloop].d_ModificationIndex[m_shID]++;
		 if(sdaliGroupStatus[m_Daliloop].d_ModificationIndex[m_shID]>sDaliBallastCfg[m_Daliloop].sLightConfig[m_shID].sGroupDet.totalNumberofGroups-1){
		 sdaliGroupStatus[m_Daliloop].d_groupConfig_StatChange[m_shID] = false;
		 sdaliGroupStatus[m_Daliloop].d_ModificationIndex[m_shID] = 0;
		 #ifdef DEVICE_DEBUG
		 printf("DALI Grouping complete for Ballast: %d from Loop: %d\r\n",
		 sDaliBallastCfg[m_Daliloop].sDaliBallastCtrl[m_shID].ballastShortID, m_Daliloop);
		 #endif
		 }
		 m_daliConfigState = 0;
		 }*/
		if (ballast_addToGroup(
				(sDaliGrpQueue.dfront->sDaliGrp.d_ballastshID & 0x80) ?
						DALI_LOOP2 : DALI_LOOP1,
				(uint8_t) (sDaliGrpQueue.dfront->sDaliGrp.d_ballastshID << 1)
						| 0x01, true, sDaliGrpQueue.dfront->sDaliGrp.d_grp)) {

			/*
			 #ifdef DEVICE_DEBUG
			 printf("DALI Ballast: %d from Loop: %d added to group %d\r\n",
			 sDaliGrpQueue.dfront->sDaliGrp.d_ballastshID,
			 (sDaliGrpQueue.dfront->sDaliGrp.d_ballastshID & 0x80) ? DALI_LOOP2 : DALI_LOOP1,
			 sDaliGrpQueue.dfront->sDaliGrp.d_grp);
			 #endif
			 */

			fn_deQueueGP(&sDaliGrpQueue);
			m_daliConfigState = 0;
			criticalOpnLock = false;
		}
		break;
	case 9:
		/*if(ballast_removeFromGroup(m_Daliloop, (sDaliBallastCfg[m_Daliloop].sDaliBallastCtrl[m_shID].ballastShortID<<1 | 0x01),
		 true, sdaliGroupStatus[m_Daliloop].d_groupInf[sdaliGroupStatus[m_Daliloop].d_ModificationIndex[m_shID]-1])){

		 if(sDaliBallastCfg[m_Daliloop].sDaliBallastCtrl[m_shID].ballastShortID == 255){
		 m_grpID = sdaliGroupStatus[m_Daliloop].d_groupInf[sdaliGroupStatus[m_Daliloop].d_ModificationIndex[m_shID]-1];
		 }
		 sdaliGroupStatus[m_Daliloop].d_ModificationIndex[m_shID]--;
		 if(sdaliGroupStatus[m_Daliloop].d_ModificationIndex[m_shID]<=0){
		 sdaliGroupStatus[m_Daliloop].d_groupConfig_StatChange[m_shID] = false;
		 sdaliGroupStatus[m_Daliloop].d_ModificationIndex[m_shID] = 0;

		 #ifdef DEVICE_DEBUG
		 printf("DALI Group removed for Ballast: %d from Loop: %d\r\n",
		 sDaliBallastCfg[m_Daliloop].sDaliBallastCtrl[m_shID].ballastShortID, m_Daliloop);
		 #endif
		 }
		 if(sDaliBallastCfg[m_Daliloop].sDaliBallastCtrl[m_shID].ballastShortID == 255){
		 m_daliConfigState = 10;
		 }
		 else{
		 m_daliConfigState = 0;
		 }

		 }*/
		if (ballast_removeFromGroup(
				(sDaliGrpQueue.dfront->sDaliGrp.d_ballastshID & 0x80) ?
						DALI_LOOP2 : DALI_LOOP1,
				(uint8_t) (sDaliGrpQueue.dfront->sDaliGrp.d_ballastshID << 1)
						| 0x01, true, sDaliGrpQueue.dfront->sDaliGrp.d_grp)) {
#ifdef DEVICE_DEBUG
			printf("DALI Ballast: %d from Loop: %d removed from group %d\r\n",
					(sDaliGrpQueue.dfront->sDaliGrp.d_ballastshID == 0x7F) ?
					0xFF : sDaliGrpQueue.dfront->sDaliGrp.d_ballastshID,
					(sDaliGrpQueue.dfront->sDaliGrp.d_ballastshID & 0x80) ? DALI_LOOP2 : DALI_LOOP1,
					sDaliGrpQueue.dfront->sDaliGrp.d_grp);
#endif
			fn_deQueueGP(&sDaliGrpQueue);
			m_daliConfigState = 0;
			criticalOpnLock = false;
		}
		break;
	case 10:
		if (ballast_storeScene(
				(daliLoop_typedef) GET_LOOP(
						sDaliSceneCfgQueue.d_scnCfgFront->sDaliScnCfg.d_ballastshId),
				(uint8_t) ((
				GET_GRP(sDaliSceneCfgQueue.d_scnCfgFront->sDaliScnCfg.d_ballastshId) ?
						(0x80) : 0)
						| ((GET_SADD(
								sDaliSceneCfgQueue.d_scnCfgFront->sDaliScnCfg.d_ballastshId)
								<< 1) | 0x01)),
				true, sDaliSceneCfgQueue.d_scnCfgFront->sDaliScnCfg.d_scn,
				sDaliSceneCfgQueue.d_scnCfgFront->sDaliScnCfg.d_level)) {
//#ifdef DEVICE_DEBUG
//				printf("")
			printf(
					"DALI Ballast/Group: %d from Loop: %d stored as scene %d\r\n",
					GET_SADD(
							sDaliSceneCfgQueue.d_scnCfgFront->sDaliScnCfg.d_ballastshId),
					(daliLoop_typedef) GET_LOOP(
							sDaliSceneCfgQueue.d_scnCfgFront->sDaliScnCfg.d_ballastshId),
					sDaliSceneCfgQueue.d_scnCfgFront->sDaliScnCfg.d_scn);
//#endif
			fn_deQueueScnCfg(&sDaliSceneCfgQueue);
			m_daliConfigState = 0;
			criticalOpnLock = false;
		}
		break;
	case 11:
		if (ballast_removeFromScene(
				(daliLoop_typedef) GET_LOOP(
						sDaliSceneCfgQueue.d_scnCfgFront->sDaliScnCfg.d_ballastshId),
				(uint8_t) ((
				GET_GRP(sDaliSceneCfgQueue.d_scnCfgFront->sDaliScnCfg.d_ballastshId) ?
						(0x80) : 0)
						| ((GET_SADD(
								sDaliSceneCfgQueue.d_scnCfgFront->sDaliScnCfg.d_ballastshId)
								<< 1) | 0x01)),
				true, sDaliSceneCfgQueue.d_scnCfgFront->sDaliScnCfg.d_scn)) {
//#ifdef DEVICE_DEBUG
			printf(
					"DALI Ballast/ Group: %d from Loop: %d removed from scene %d\r\n",
					GET_SADD(
							sDaliSceneCfgQueue.d_scnCfgFront->sDaliScnCfg.d_ballastshId),
					(daliLoop_typedef) GET_LOOP(
							sDaliSceneCfgQueue.d_scnCfgFront->sDaliScnCfg.d_ballastshId),
					sDaliSceneCfgQueue.d_scnCfgFront->sDaliScnCfg.d_scn);
//#endif
			fn_deQueueScnCfg(&sDaliSceneCfgQueue);
			m_daliConfigState = 0;
			criticalOpnLock = false;
		}
		break;
	case 12:

		break;
	}
}
#ifdef DALI_LIGHT
void fn_dali_ind(uint8_t m_LoopID, uint8_t m_ballastID, uint8_t m_intensity,
		recvEnd_typedef m_rcvEnd) {
//	sDaliBallastCfg[m_LoopID].sDaliBallastCtrl[m_ballastID].ballastStatusChange = true;
//	sDaliBallastCfg[m_LoopID].sDaliBallastCtrl[m_ballastID].ballastIntensity = m_intensity;
//	sDaliBallastCfg[m_LoopID].sDaliBallastCtrl[m_ballastID].loopStatus_reportEnd = m_rcvEnd;
//	sDaliBallastCfg[m_LoopID].sLightConfig[m_ballastID].presetMax = 254;
	struct_DaliAction sDaliActionLoc = { 0 };
	sDaliActionLoc.d_Intensity = m_intensity;
	sDaliActionLoc.d_add = m_ballastID | (m_LoopID == DALI_LOOP2) ? 0x200 : 0;
	sDaliActionLoc.d_report = 0x01;
	if (!fn_enQueueAC(&sDaliActQueue, &sDaliActionLoc)) {
#ifdef DEVICE_DEBUG
		printf("Memory allocation falut in 1802 daliCommand.c\r\n");
#endif
	}
//	//UNUSED(m_rcvEnd);

}
/*

 uint8_t fn_writeDaliCfg_flashWhole(uint8_t isAddressingCplt)
 {
 static uint8_t daliCfgFlash_stat = 0;
 static uint8_t daliIndex = 0;
 static uint8_t daliLoop = 0;
 bool daliCfgFlash_retval = false;
 static uint8_t *m_comReadBuffer;

 switch (daliCfgFlash_stat)
 {

 case 0:
 if (sDaliCfgQueue.dfront == NULL)
 {
 daliCfgFlash_stat = 9;
 }
 else
 {
 if (isAddressingCplt == 1)
 {
 m_comReadBuffer = (uint8_t*) calloc(FLASH_SECTOR_SIZE, sizeof(uint8_t));
 if (m_comReadBuffer != NULL)
 {
 daliCfgFlash_stat = 3;
 daliLoop = sDaliCfgQueue.dfront->sDaliCfg.ballastloop;
 daliIndex = (uint8_t) (sDaliCfgQueue.dfront->sDaliCfg.ballastShortID - 1);
 }
 }
 else
 {
 daliCfgFlash_stat = 1;
 }
 }
 //		daliCfgFlash_stat = 5;
 break;
 case 1:
 if (fn_Erase(2 * FLASH_BLOCK_SIZE, WT_BLOCK) == FLASH_OK)
 {
 daliCfgFlash_stat = 2;
 }
 break;
 case 2:
 if (g_ballastCount[daliLoop] != 0)
 {
 if (sDaliLookUp[daliLoop][(daliLoop == 0) ? daliIndex : (daliIndex - g_ballastCount[0])].dnodePtr
 != NULL)
 {
 if (fn_writeSector(
 DALI_START_SECTOR
 + ((daliIndex) * sizeof(struct_DaliBallastCfg)) / FLASH_SECTOR_SIZE,
 (uint8_t*) (&sDaliLookUp[daliLoop][
 (daliLoop == 0) ? daliIndex : (daliIndex - g_ballastCount[0])].dnodePtr->sDaliCfg),
 sizeof(struct_DaliBallastCfg),
 ((daliIndex) * sizeof(struct_DaliBallastCfg)) % FLASH_SECTOR_SIZE))
 {
 sDaliFlashLookUp[daliLoop][
 (daliLoop == 0) ? daliIndex : (daliIndex - g_ballastCount[0])].sectorNumber =
 DALI_START_SECTOR
 + ((daliIndex) * sizeof(struct_DaliBallastCfg)) / FLASH_SECTOR_SIZE;
 sDaliFlashLookUp[daliLoop][
 (daliLoop == 0) ? daliIndex : (daliIndex - g_ballastCount[0])].sectorOffset =
 ((daliIndex) * sizeof(struct_DaliBallastCfg)) % FLASH_SECTOR_SIZE;
 sDaliFlashLookUp[daliLoop][
 (daliLoop == 0) ? daliIndex : (daliIndex - g_ballastCount[0])].daliShID =
 sDaliLookUp[daliLoop][
 (daliLoop == 0) ? daliIndex : (daliIndex - g_ballastCount[0])].bShID;
 sDaliFlashLookUp[daliLoop][
 (daliLoop == 0) ? daliIndex : (daliIndex - g_ballastCount[0])].daliRandomId =
 sDaliLookUp[daliLoop][
 (daliLoop == 0) ? daliIndex : (daliIndex - g_ballastCount[0])].dnodePtr->sDaliCfg.ballastRandomID;

 fn_deQueueDC(&sDaliCfgQueue);
 sDaliLookUp[daliLoop][
 (daliLoop == 0) ? daliIndex : (daliIndex - g_ballastCount[0])].dnodePtr =
 NULL;
 sDaliLookUp[daliLoop][
 (daliLoop == 0) ? daliIndex : (daliIndex - g_ballastCount[0])].bShID =
 0;
 }
 }
 else{
 sDaliFlashLookUp[daliLoop][(daliLoop==0)?daliIndex:(daliIndex-g_ballastCount[0])].daliShID = 0xFF;
 sDaliFlashLookUp[daliLoop][(daliLoop==0)?daliIndex:(daliIndex-g_ballastCount[0])].daliRandomId = 0;
 }
 daliIndex++;

 if (daliLoop == 0)
 {
 if ((daliIndex >= g_ballastCount[daliLoop])
 && (sDaliLookUp[daliLoop][daliIndex].dnodePtr == NULL))
 {
 daliLoop = 1;
 }
 }
 else
 {
 if (((daliIndex - g_ballastCount[0]) >= g_ballastCount[daliLoop])
 && (sDaliLookUp[daliLoop][daliIndex - g_ballastCount[0]].dnodePtr == NULL))
 {
 daliCfgFlash_stat = 7;
 daliIndex = 0;
 daliLoop = 0;
 }
 }

 }
 else
 {
 if (daliLoop == 0)
 {
 daliLoop = 1;
 //				daliIndex = 0;
 }
 else
 {
 daliCfgFlash_stat = 7;
 daliIndex = 0;
 daliLoop = 0;
 }
 }

 break;
 case 3:
 if (fn_readSector(
 (DALI_START_SECTOR
 + ((daliIndex) * sizeof(struct_DaliBallastCfg)) / FLASH_SECTOR_SIZE),
 m_comReadBuffer, FLASH_SECTOR_SIZE, 0))
 {
 memcpy(
 (uint8_t*) (m_comReadBuffer
 + ((daliIndex) * sizeof(struct_DaliBallastCfg)) % FLASH_SECTOR_SIZE),
 (uint8_t*) (&sDaliLookUp[daliLoop][daliIndex].dnodePtr->sDaliCfg),
 sizeof(struct_DaliBallastCfg));
 daliCfgFlash_stat = 4;
 }
 break;
 case 4:
 if (fn_Erase(
 (DALI_START_SECTOR
 + ((daliIndex) * sizeof(struct_DaliBallastCfg)) / FLASH_SECTOR_SIZE),
 WT_SECTOR) == FLASH_OK)
 {
 daliCfgFlash_stat = 5;
 }
 break;
 case 5:
 if (fn_writeSector(
 (DALI_START_SECTOR
 + ((daliIndex) * sizeof(struct_DaliBallastCfg)) / FLASH_SECTOR_SIZE),
 m_comReadBuffer, FLASH_SECTOR_SIZE, 0))
 {

 sDaliFlashLookUp[daliLoop][daliIndex].sectorNumber = DALI_START_SECTOR
 + ((daliIndex) * sizeof(struct_DaliBallastCfg)) / FLASH_SECTOR_SIZE;
 sDaliFlashLookUp[daliLoop][daliIndex].sectorOffset = ((daliIndex)
 * sizeof(struct_DaliBallastCfg)) % FLASH_SECTOR_SIZE;
 sDaliFlashLookUp[daliLoop][daliIndex].daliShID = sDaliLookUp[daliLoop][daliIndex].bShID;

 fn_deQueueDC(&sDaliCfgQueue);
 sDaliLookUp[daliLoop][daliIndex].dnodePtr = NULL;
 sDaliLookUp[daliLoop][daliIndex].bShID = 0;

 free(m_comReadBuffer);
 if (sDaliCfgQueue.dfront == NULL)
 {
 daliCfgFlash_stat = 7;
 }
 else
 {
 daliCfgFlash_stat = 0;
 }

 }
 break;
 case 7:
 daliCfgFlash_stat = 9;
 //		DONE ON fn_queryDT()
 //		if(fn_Erase(1*FLASH_SECTOR_SIZE, WT_SECTOR) == FLASH_OK){
 //			daliCfgFlash_stat = 8;
 //		}
 break;
 case 8:
 //		updatedConfig = 0;
 //		if((fn_writeSector(1, (uint8_t *)&sDaliFlashLookUp, sizeof(sDaliFlashLookUp), sizeof(updatedConfig)))){
 //			if(fn_writeSector(1, (uint8_t *)&updatedConfig, sizeof(updatedConfig), 0)){
 //				daliCfgFlash_stat = 9;
 //#ifdef DEVICE_DEBUG
 //				printf("Dali Flash Updated\r\n");
 //#endif
 //			}
 //		}
 break;
 case 9:
 daliCfgFlash_stat = 0;
 daliCfgFlash_retval = true;
 #ifdef DEVICE_DEBUG
 printf("Dali Commission ended\r\n");
 #endif
 break;
 }
 return daliCfgFlash_retval;
 }

 */
/*
 void fn_GroupSceneAction()
 {
 static uint8_t grpScnAct = 0;
 static int16_t m_entityIndex = -1;
 static uint8_t m_grpID = 0;
 static uint8_t m_grpIndex = 0;
 //	uint8_t m_loopID = 0;
 static char grpId[8] =
 { 0 };
 static char scnID[8] =
 { 0 };
 //	static char scnID2[8] =
 //	{ 0 };
 static struct_lightStatusReport m_sSceneStatReport =
 { 0 };
 //	uint8_t m_scnIndex = 0;
 //	uint8_t m_scnTrackID = 0;

 switch (grpScnAct)
 {
 case 0:
 if (sGroupSceneActionQueue.gpSnActFront != NULL)
 {
 if (criticalOpnLock == false)
 {

 criticalOpnLock = true;
 memset(criticalOpnMonitor, '\0', sizeof(criticalOpnMonitor));
 sprintf(criticalOpnMonitor, "Func: %s Line: %d", __func__, __LINE__);

 memset(grpId, '\0', sizeof(grpId));
 strcpy(grpId, sGroupSceneActionQueue.gpSnActFront->sGpSnAct.grpID);
 m_grpID = fn_extractGroup(
 strtok(sGroupSceneActionQueue.gpSnActFront->sGpSnAct.grpID, "-"));
 m_grpIndex = (uint8_t) atoi(strtok(NULL, "#"));
 memset(scnID, '\0', sizeof(scnID));
 //				memset(scnID2, '\0', sizeof(scnID2));
 strcpy(scnID,
 sGroupSceneActionQueue.gpSnActFront->sGpSnAct.GpSnEntity.sSNEntity.gpEntityLabel);

 //				strcpy(scnID2, scnID);
 //				m_scnTrackID = fn_extractDaliScene(strtok(scnID2, "-"));
 //				m_scnIndex = (uint8_t) atoi(strtok(NULL, "#")); // "#" is a dummy delimiter

 g_snStatProcessID += 1;

 memset(&m_sSceneStatReport, 0, sizeof(m_sSceneStatReport));
 m_sSceneStatReport.gpId = (uint16_t) ((m_grpID << 4) | (m_grpIndex & 0x0F));
 m_sSceneStatReport.procId = (uint16_t) g_snStatProcessID;

 sGroupSceneActionQueue.gpSnActFront->sGpSnAct.m_dataByte =
 m_sSceneStatReport.procId;

 if (m_grpIndex & 0x02)
 {
 m_sSceneStatReport.entityReportStat = m_sSceneStatReport.entityReportStat
 | (1 << GP_DL1);
 }
 if (m_grpIndex & 0x04)
 {
 m_sSceneStatReport.entityReportStat |= (1 << GP_DL2);
 }
 if (!(m_grpIndex & 0x06))
 {
 m_sSceneStatReport.entityReportStat = 0;
 }
 m_sSceneStatReport.m_intSet = sGroupSceneActionQueue.gpSnActFront->sGpSnAct.scnUqID;

 m_sSceneStatReport.lightStatEn =
 sGroupSceneActionQueue.gpSnActFront->sGpSnAct.scnFrom;

 if (sLSRSceneRootNode == NULL)
 {
 sLSRSceneRootNode = fn_insertLSRLData(sLSRSceneRootNode, &m_sSceneStatReport);
 if (sLSRSceneRootNode == NULL)
 {
 #ifdef		DEBUG_485CMD_PACKETS
 printf("Memory allocation error in Group Status report queue\r\n");
 #endif
 }
 }
 else
 {
 if (!fn_insertLSRLData(sLSRSceneRootNode, &m_sSceneStatReport))
 {
 #ifdef		DEBUG_485CMD_PACKETS
 printf("Memory allocation error in Group Status report queue\r\n");
 #endif
 }
 }

 grpScnAct = 1;

 }
 }
 break;
 case 1:
 memset(&g_sGpEntity, '\0', sizeof(g_sGpEntity));
 m_entityIndex = fn_extractEntityIndex(grpId);
 if (m_entityIndex != -1)
 {
 if (fn_readSector(sGpLookUp[m_entityIndex].sectorNumber_GP, (uint8_t*) &g_sGpEntity,
 sizeof(g_sGpEntity), sGpLookUp[m_entityIndex].sectorOffset_GP))
 {
 grpScnAct = 2;
 m_entityIndex = 0;
 }
 }
 else
 {
 // Group doesn't exist
 fn_deQueueGrpScnAct(&sGroupSceneActionQueue);
 grpScnAct = 0;
 criticalOpnLock = false;
 }
 break;
 case 2:
 if (m_grpIndex & 0x02)
 {
 memset(&m_sDaliSceneAct, '\0', sizeof(m_sDaliSceneAct));
 m_sDaliSceneAct.d_ballastshId = (uint16_t) (m_grpID | 0x400);
 m_sDaliSceneAct.d_scn = fn_extractDaliScene(scnID);
 m_sDaliSceneAct.m_dataByte = sGroupSceneActionQueue.gpSnActFront->sGpSnAct.m_dataByte;
 m_sDaliSceneAct.d_scnFrom = sGroupSceneActionQueue.gpSnActFront->sGpSnAct.scnFrom;
 if (!fn_enQueueScnAct(&sDaliSceneActQueue, &m_sDaliSceneAct))
 {
 #ifdef DEVICE_DEBUG
 printf("Memory allocation fault Func: %s Line: %d", __func__, __LINE__);
 #endif
 }
 }
 if (m_grpIndex & 0x04)
 {
 memset(&m_sDaliSceneAct, '\0', sizeof(m_sDaliSceneAct));
 m_sDaliSceneAct.d_ballastshId = (uint16_t) (m_grpID | 0x600);
 m_sDaliSceneAct.d_scn = fn_extractDaliScene(scnID);
 m_sDaliSceneAct.d_scnFrom = sGroupSceneActionQueue.gpSnActFront->sGpSnAct.scnFrom;
 m_sDaliSceneAct.m_dataByte = sGroupSceneActionQueue.gpSnActFront->sGpSnAct.m_dataByte;
 if (!fn_enQueueScnAct(&sDaliSceneActQueue, &m_sDaliSceneAct))
 {
 #ifdef DEVICE_DEBUG
 printf("Memory allocation fault Func: %s Line: %d", __func__, __LINE__);
 #endif
 }
 }
 grpScnAct = 3;
 break;
 case 3:
 m_entityIndex = fn_extractSceneIndex(scnID);
 if (m_entityIndex != -1)
 {
 grpScnAct = 4;
 }
 else
 {
 #ifdef DEVICE_DEBUG
 printf("Scene doesn't exist\r\n");
 #endif
 fn_deQueueGrpScnAct(&sGroupSceneActionQueue);
 grpScnAct = 0;
 criticalOpnLock = false;
 }
 break;
 case 4:
 if (fn_readSector(sSNLookUp[m_entityIndex].sectorNumber_SN, (uint8_t*) &g_sSNEntity,
 sizeof(g_sSNEntity), sSNLookUp[m_entityIndex].sectorOffset_SN))
 {
 grpScnAct = 5;
 m_entityIndex = 0;
 }
 break;
 case 5:
 if (m_entityIndex < g_sGpEntity.totElts)
 {
 if (fn_extractDeviceTypeGP(g_sGpEntity.sGPEntityLoc[m_entityIndex].gpEntityLabel)
 == ZERO_TEN)
 {
 //				m_loopID = fn_extractLoop(g_sGpEntity.sGPEntityLoc[m_entityIndex].gpEntityLabel);
 //				fn_setzTLevel(sGroupSceneActionQueue.gpSnActFront->sGpSnAct.GpSnEntity.scnLevel, m_loopID);
 for (uint8_t m_id = 0; m_id < g_sSNEntity.totScns; m_id++)
 {
 if (!strcmp(g_sGpEntity.sGPEntityLoc[m_entityIndex].gpEntityLabel,
 g_sSNEntity.sSNEntityLoc[m_id].sSNEntity.gpEntityLabel))
 {
 fn_zTSet_ind(
 fn_extractLoop(
 g_sGpEntity.sGPEntityLoc[m_entityIndex].gpEntityLabel),
 (uint8_t) (g_sSNEntity.sSNEntityLoc[m_id].scnLevel * 2.55), 0,
 RECEIVED_ON_CLIENT,
 sGroupSceneActionQueue.gpSnActFront->sGpSnAct.scnFrom);
 }

 }

 }
 m_entityIndex++;
 }
 else
 {
 fn_deQueueGrpScnAct(&sGroupSceneActionQueue);
 grpScnAct = 0;
 criticalOpnLock = false;
 }
 break;
 }
 }
 */
#endif
#ifdef DALI_LIGHT
uint16_t fn_isDaliShidAvailable(uint8_t m_loop, uint32_t m_randId) {
	for (uint8_t m_id = 0; m_id < g_ballastCount[m_loop]; m_id++) {
		if (sDaliFlashLookUp[m_loop][m_id].daliRandomId == m_randId) {
			return sDaliFlashLookUp[m_loop][m_id].daliShID;
		}
	}
	return g_ballastCount[m_loop] + 1;
}

bool fn_isDaliRndIDAvailable(uint8_t m_loop, uint32_t m_randId) {
	for (uint8_t m_id = 0; m_id < g_ballastCount[m_loop]; m_id++) {
		if (sDaliFlashLookUp[m_loop][m_id].daliRandomId == m_randId) {
			return true;
		}
	}
	return false;
}

bool fn_isSHIDAvailable(uint8_t m_shid, daliLoop_typedef m_loop) {
	for (uint8_t m_i = 0;
			((m_i < 64) /*&& (sDaliLookUp[m_loop][m_i].dnodePtr != NULL)*/);
			m_i++) {
		if (sDaliFlashLookUp[m_loop][m_i].daliShID == m_shid) {
			return true;
		}
	}
	return false;
}

int8_t fn_findFree_id(daliLoop_typedef m_loop) {
	uint8_t m_i = 0;
	// to address with the deleted short id, please make sure the packet has NOT already been
	// enqueued
	for (m_i = 0;
			((m_i < 64)/* && (sDaliLookUp[m_loop][m_i].dnodePtr != NULL)*/);
			m_i++) {
		if ((sDaliFlashLookUp[m_loop][m_i].daliShID == 0)
				|| (sDaliFlashLookUp[m_loop][m_i].daliShID == 0xFF)) {
			break;
		}
	}
	return m_i + 1;
}
#endif

void fn_lightStatusQuery() {
	/*	static uint8_t m_state = 0;
	 static uint8_t m_ballastCount = 0;
	 static uint8_t m_loop = 0;
	 static uint8_t m_dresponse = 0;
	 static uint16_t m_timeout = 0;
	 switch(m_state){
	 case 0:
	 if(g_daliCommissionCplt == DALI_COMMISSION_CPLT){
	 if((g_ballastCount[m_loop]>0)){
	 if(criticalOpnLock == false){
	 memset(criticalOpnMonitor, '\0', sizeof(criticalOpnMonitor));
	 sprintf(criticalOpnMonitor, "Func: %s Line: %d", __func__, __LINE__);
	 criticalOpnLock = true;
	 m_ballastCount = (uint8_t)(m_ballastCount + 1);
	 if((m_ballastCount>g_ballastCount[DALI_LOOP1]) && (m_loop == DALI_LOOP1)){
	 //					(m_loop == DALI_LOOP1) ? (m_loop = DALI_LOOP2) : (m_loop = DALI_LOOP1);
	 m_loop = DALI_LOOP2;
	 if((g_ballastCount[m_loop]>0)){
	 if(sDALIStatus.sDaliBallastStatus[m_ballastCount].balShID == 0){
	 sDALIStatus.sDaliBallastStatus[m_ballastCount].balShID
	 = (uint8_t)((m_loop == DALI_LOOP2)
	 ? (((0x01)<<7)|(m_ballastCount-g_ballastCount[m_loop])) : m_ballastCount);
	 }
	 m_state = 1;
	 }
	 else{
	 m_state = 0;
	 m_ballastCount = 0;
	 m_loop = DALI_LOOP1;
	 criticalOpnLock = false;
	 }
	 }
	 else if(m_ballastCount>(g_ballastCount[DALI_LOOP1]+g_ballastCount[DALI_LOOP2])){
	 m_state = 0;
	 m_ballastCount = 0;
	 m_loop = DALI_LOOP1;
	 criticalOpnLock = false;
	 }
	 else{
	 if(sDALIStatus.sDaliBallastStatus[m_ballastCount].balShID == 0){
	 sDALIStatus.sDaliBallastStatus[m_ballastCount].balShID
	 = (uint8_t)((m_loop == DALI_LOOP2)
	 ? (((0x01)<<7)|(m_ballastCount-g_ballastCount[m_loop])) : m_ballastCount);
	 }
	 m_state = 1;
	 }
	 }
	 }
	 else{
	 (m_loop == DALI_LOOP1) ? (m_loop = DALI_LOOP2) : (m_loop = DALI_LOOP1);
	 }
	 }

	 break;
	 case 1:
	 if(ballastQuery( ((sDALIStatus.sDaliBallastStatus[m_ballastCount].balShID & 0x80) == 0x80) ? DALI_LOOP2 : DALI_LOOP1,
	 ((uint8_t)(sDALIStatus.sDaliBallastStatus[m_ballastCount].balShID<<1)|0x01), LUM_QSTATUS, &m_dresponse)){
	 if(!(m_dresponse & 0x10)){
	 m_state = 2;
	 sDALIStatus.sDaliBallastStatus[m_ballastCount].balStatus = BALLAST_IDLE;
	 }
	 else if(m_dresponse & 0x02){				// LAMP FAILURE
	 m_state = 0;
	 criticalOpnLock = false;
	 sDALIStatus.sDaliBallastStatus[m_ballastCount].balStatus = BALLAST_IDLE;
	 }
	 else{
	 sDALIStatus.sDaliBallastStatus[m_ballastCount].balStatus = BALLAST_BUSY;
	 m_state = 0;
	 criticalOpnLock = false;
	 }
	 }
	 break;
	 case 2:
	 if(ballastQuery(((sDALIStatus.sDaliBallastStatus[m_ballastCount].balShID & 0x80) == 0x80) ? DALI_LOOP2 : DALI_LOOP1,
	 ((uint8_t)(sDALIStatus.sDaliBallastStatus[m_ballastCount].balShID<<1)|0x01), LUM_QCUR_LEVEL, &m_dresponse)){
	 if(m_dresponse!=0xFF){
	 sDALIStatus.sDaliBallastStatus[m_ballastCount].balLevel = (uint8_t)LEVEL_PERCENT(m_dresponse);
	 }

	 m_timeout = fn_GetmSecTimerStart();
	 m_state = 3;
	 criticalOpnLock = false;
	 }
	 break;
	 case 3:
	 if(fn_IsmSecTimerElapsed(m_timeout, 200)){
	 m_state = 0;
	 }
	 break;
	 }*/
}

void fn_initDaliCommands() {
	daliResponse = 0;
	g_shortAddress = 1;
	memset(&g_sGpEntity, 0, sizeof(g_sGpEntity));
	memset(&g_sSNEntity, 0, sizeof(g_sSNEntity));
	memset(&g_sGpSnAction_obj, 0, sizeof(g_sGpSnAction_obj));
#ifdef DALI_LIGHT
	memset(sGpLookUp, 0, sizeof(sGpLookUp));
	memset(sSNLookUp, 0, sizeof(sSNLookUp));
	memset(sDaliFlashLookUp, 0, sizeof(sDaliFlashLookUp));
#endif
	memset(&m_sDaliSceneAct, 0, sizeof(m_sDaliSceneAct));
	memset(&edaliConfigStatus, 0, sizeof(edaliConfigStatus));
	memset(&edaliProcessStatus, 0, sizeof(edaliProcessStatus));
	memset(g_ballastCount, 0, sizeof(g_ballastCount));
//	memset(&sDALIStatus, 0, sizeof(sDALIStatus));

}

void fn_flushDaliCfg() {

	while (sDaliCfgQueue.dfront != NULL) {
		fn_deQueueDC(&sDaliCfgQueue);
	}

/*
	for (uint16_t i = 0; i < 65; i++) {
		if (sDaliLookUp[DALI_LOOP1][i].dnodePtr != NULL) {
			sDaliLookUp[DALI_LOOP1][i].dnodePtr = NULL;
		}
	}
	for (uint16_t i = 0; i < 65; i++) {
		if (sDaliLookUp[DALI_LOOP2][i].dnodePtr != NULL) {
			sDaliLookUp[DALI_LOOP2][i].dnodePtr = NULL;
		}
	}
*/
}

void fn_daliMode1_Level(uint8_t shID, uint8_t m_intensity, uint8_t m_statFrom)
{

	struct_DaliAction sDaliAction = { 0 };

	if ((shID != 0xFF) && (shID & 0x40)) {
		sDaliAction.d_add = (uint16_t) (((uint8_t) shID & 0x0F) | 0x400);
	} else {
//		sDALIStatus.sDaliHCLTempL1[shID - 1].isOn = (
//				(m_intensity == 0) ? false : true);
		sDaliAction.d_add = (uint16_t) ((uint8_t) shID);
		sDaliAction.d_report = 0x01;
	}
	g_daliLightStat = m_intensity;

	m_intensity =
			(m_intensity != 0) ?
					(uint8_t) ((253 * (1 + log10(m_intensity)) / 3) + 1) : 0;
	sDaliAction.d_statFrom = m_statFrom;
	sDaliAction.d_Intensity = (uint8_t) m_intensity;
	sDaliAction.d_dataByte = 0;

/*

	if (!fn_enQueueAC(&sDaliActQueue, &sDaliAction)) {
#ifdef DEVICE_DEBUG
		printf("Memory allocation fault Line Func: %s Line: %d", __func__, __LINE__);
#endif
	}
*/
	fn_enQDAct(&sDaliAction);

	if(g_sDaliRetention.g_RetentionFlag == 0)
	{
		g_sDaliRetention.g_RetentionFlag = 1;
	}
	g_sDaliRetention.DaliQueryWaitTimer = fn_GetSecTimerStart();

}

void fn_enqueueIdentify() {
	struct_DaliAction sDaliAction = { 0 };
	sDaliAction.d_add = (uint16_t) (0xFF | 0x100);
/*
	if (!fn_enQueueAC(&sDaliActQueue, &sDaliAction)) {
		// Unable to allocate memory for identify
	}
*/
	fn_enQDAct(&sDaliAction);
}

void gpioDALISetup(void) {
#ifdef DALI_FEATURE
	/* Configure Button PB0 as input and enable interrupt */
	GPIO_PinModeSet(DALI_RX_PORT, DALI_RX_PIN, gpioModeInput, 0);
//  GPIO_PinModeSet(BUTTON_0_PORT, BUTTON_0_PIN, gpioModeInputPull, 1);
	GPIO_ExtIntConfig(DALI_RX_PORT,
	DALI_RX_PIN,
	DALI_RX_PIN,
	true,
	true,
	true);

	/* Enable ODD interrupt to catch button press that changes slew rate */
	NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
	NVIC_EnableIRQ(GPIO_EVEN_IRQn);
	GPIOINT_CallbackRegister(DALI_RX_PIN, fn_switch_intrpt_callBack);

	/* Configure pB00 as a push pull output for LED drive */
	GPIO_PinModeSet(DALI_TX_PORT, DALI_TX_PIN, gpioModePushPull, 1);
#endif
}

void fn_ProcessDaliRetention(void) {
	static uint8_t Sys_Fail_flag = 0;
	static enum_DaliRetention eDaliRetentionState = DALI_RETENTION_INIT;
	switch (eDaliRetentionState) {
	case DALI_RETENTION_INIT: {
		if (g_sDaliRetention.g_RetentionFlag == 1) {
			{
				if (fn_IsSecTimerElapsed(g_sDaliRetention.DaliQueryWaitTimer,g_sDaliRetention.Dali_RetentionStroageTimeout)) {
					if ((flag & 0x100)
							|| (((uint8_t) flag != g_sDaliRetention.m_intensity)
									&& (abs(
											snsrAppData.global_daliStatus
													- g_sDaliRetention.m_intensity)
											>= 30))) {
						eDaliRetentionState = CHECK;

						printf("ballast_storePowOnLevel executed @ case 1\r\n");
						snsrAppData.global_daliStatus = flag =
								g_sDaliRetention.m_intensity;

					} else {
						if ((g_sDaliRetention.m_intensity == 0)
								&& ((uint8_t) flag
										!= g_sDaliRetention.m_intensity)) {

							eDaliRetentionState = CHECK;

							printf(
									"ballast_storePowOnLevel executed @ case 2\r\n");
							snsrAppData.global_daliStatus = flag =
									g_sDaliRetention.m_intensity;
						}
					}
				}
			}
		}
	}
		break;
	case CHECK:
		if (criticalOpnLock == false)
		{
			criticalOpnLock = true;
			eDaliRetentionState = POWER_ON_LEVEL_SET;
		}
		break;

	case POWER_ON_LEVEL_SET:
		if (ballast_storePowOnLevel(DALI_LOOP1, (uint8_t) ((0xFF << 1) | 0x01),
				true)) {
			printf("Power On level set\r\n");
//			eDaliRetentionState = POWER_FAIL_LEVEL_SET;
			eDaliRetentionState = DALI_RETENTION_INIT;
			g_sDaliRetention.g_RetentionFlag = 0;
			criticalOpnLock = false;
		}
		break;

	case POWER_FAIL_LEVEL_SET:

		break;
	}

	if(Sys_Fail_flag <= 5)
	{
		if (ballast_storeFailLevel(DALI_LOOP1, (uint8_t) ((0xFF << 1) | 0x01), /*g_sDaliRetention.m_intensity*/ 0xFE, true))
		{
			printf("Power Fail Intensity set to %d\r\n",0xFE);
			printf("Power Fail Level set to %d\r\n",(uint8_t)LEVEL_PERCENT(0xFE));
//			eDaliRetentionState = DALI_RETENTION_INIT;
//			g_sDaliRetention.g_RetentionFlag = 0;
			Sys_Fail_flag++;
		}
	}
}

void fn_dali_dimming()										// Srikanth 01/6/22
{
	static uint8_t fail_cnt = 0;
	static uint8_t m_set = STATE_ONE;

	switch (m_set)
	{
		case STATE_ONE :
		{
			if(s_DimEntity.dim_set == true)
			{
				s_DimEntity.dim_set = false;
				m_set = STATE_TWO;
			}
		}
		break;

		case STATE_TWO :
		{
			if(fn_daliDimSet(s_DimEntity.Dim_type, s_DimEntity.Intensity, DALI_LOOP1, s_DimEntity.Dim_shID, 15) == true)
			{
				printf("Dali DIM completed for shid = %d\r\n",s_DimEntity.Dim_shID);
				m_set = STATE_ONE;
				fail_cnt = 0;
			}
			else
			{
//				fail_cnt++;
//				if(fail_cnt >= 50)
//				{
//					fail_cnt = 0;
//					printf("Dali DIM failed for shid = %d\r\n",s_DimEntity.Dim_shID);
////					m_set = STATE_ONE;
//				}
			}
		}
		break;

		default:
			m_set = STATE_ONE;
		break;
	}




}

bool fn_daliDimSet(Dimming_typedef m_dimType, uint8_t m_intensity, uint8_t m_loop, uint8_t m_shid,		// Srikanth 01/6/22
		statusEn_typedef m_statFrom)
{
	static uint8_t relativeState = STEP_UP_ON;
	static uint8_t m_frIndex = 0;
	static uint8_t m_noOfCmds = 0;
	static uint16_t m_timeout = 0;
	uint8_t level = 0;
	uint8_t m_retval = false;
	switch (relativeState)
	{
	case STEP_UP_ON:
		if (m_intensity != 0)
		{
			if (m_dimType == RELATIVE_DIMMING_INC)
			{
				if (ballastStepUOn(m_loop, (uint8_t) ((m_shid << 1) | 0x01), false))
				{
					relativeState = STEP_DOWN_OFF;
				}
			}
			else
			{
				relativeState = STEP_DOWN_OFF;
			}
		}
		else
		{
			m_retval = true;
		}

		break;
	case STEP_DOWN_OFF:
		if (m_dimType == RELATIVE_DIMMING_DEC)
		{
			if (ballastStepDOff(m_loop, (uint8_t) ((m_shid << 1) | 0x01), false))
			{
				relativeState = DIM_QUERY;
			}
		}
		else
		{
			relativeState = DIM_QUERY;
		}
		break;

	case DIM_QUERY:
//		m_noOfCmds = fn_calcFadeRate(m_intensity, &m_frIndex);
		// TODO : it should be configurable, but switch and remote are hard coded for 5%. so the cmds are 1 and fadeindex is 5.
		m_noOfCmds = 1;
		m_frIndex  = 5;
		if (m_noOfCmds >= 10)
		{
			relativeState = STEP_UP_ON;
			m_retval = true;
		}
		else
		{
			relativeState = DIM_SET_FADERATE;
		}
		break;

	case DIM_SET_FADERATE:
		if (ballast_storeFadeRate(m_loop, (uint8_t) ((m_shid << 1) | 0x01), FADE_89/*m_frIndex*/ + 1, true)) //
		{
			relativeState = DIM_SET;
		}
		break;

	case DIM_SET:
		switch (m_dimType)
		{
		case RELATIVE_DIMMING_INC:
			if (ballastFadeUp(m_loop, (uint8_t) ((m_shid << 1) | 0x01), false))
			{
				m_noOfCmds--;
				printf("SHID = %d is Incremented\r\n", m_shid);
				m_timeout = fn_GetmSecTimerStart();
				relativeState = DIM_WAIT;
			}
			break;

		case RELATIVE_DIMMING_DEC:
			if (ballastFadeDown(m_loop, (uint8_t) ((m_shid << 1) | 0x01), false))
			{
				m_noOfCmds--;
				printf("SHID = %d is Decremented\r\n", m_shid);
				m_timeout = fn_GetmSecTimerStart();
				relativeState = DIM_WAIT;
			}
			break;

		default:
			m_retval = true;
		break;
		}
		break;

	case DIM_WAIT:
		if (fn_IsmSecTimerElapsed(m_timeout, 200))
		{
			if (m_noOfCmds <= 0)
			{
//				relativeState = DIM_CHECK;
//				TODO: Send the dim status to the gateway
				relativeState = STEP_UP_ON;
				m_retval = true;
			}
			else
			{
				relativeState = DIM_SET;
			}

		}
		break;
	case DIM_CHECK:
		if (ballastQuery(m_loop, (uint8_t) ((m_shid << 1) | 0x01), LUM_QCUR_LEVEL, &level))
		{
			printf("level= %d\r\n", level);
			relativeState = STEP_UP_ON;
			m_retval = true;
		}
		break;
	default:

		break;
	}
	return m_retval;
}


