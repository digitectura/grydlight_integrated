/*
 * mcp45hvx1.c
 *
 *  Created on: 20-Jan-2021
 *      Author: Rakshith
 */

#include "application/inc/analog/mcp45hvx1.h"
#include "application/inc/intervalTimer.h"
#include "application/inc/app.h"
#include "application/inc/dali/dali.h"

#define LEVEL_PERCENT_AN(num) pow(10, (float)((((float)num-1)/(125.0/3.0))-1))
#define DEFAULT_TOGGLE_COUNT   20
#define DEFAULT_TOGGLE_TIME    500

enum{
	IDENTIFY_TIMER_START,
	IS_TOGGLE_TIMER_ELAPSED,

};

bool fn_writeWiper(uint8_t wiperValue);
bool fn_writeWiperLen(uint8_t *txBuf, uint8_t m_len);
int fn_getSceneIndex(uint16_t sceneID);
int fn_getFreeSceneIndex();

uint8_t g_txData[10] = {0};
uint8_t g_rxData[10] = {0};
bool g_isIdentifyEnabled = 0;
uint8_t g_analogLightStat = 0;
uint16_t g_analogFadeTime = 2800;

static uint8_t wiperValueToBeSet = 0;
static uint8_t wiperValueMaintained = 0;
static uint8_t g_analogFadingStart = 0;
static uint8_t analog_stepDelay = 0;			//delay between each step of dim with respct to the FADE_TIME ≈ FADE_RATE defined in DALI
static uint16_t analog_stepFadingTimerStart = 0;
static uint8_t setToBeAnalogstatus = 0;

/* Wiper Register..........................................................*/
bool fn_writeWiper(uint8_t wiperValue)
{
	uint8_t result;
	result = fn_I2C_MasterWriteonly(ANALOG_I2C_HANDLE, POT_CH1_WRITE_ADDR, &wiperValue, 1);
	return result;
}

bool fn_readWiper(uint8_t *rxBuf, uint8_t m_targetAdd, uint8_t m_len){

	return fn_I2C_MasterRead(ANALOG_I2C_HANDLE, POT_CH1_READ_ADDR, m_targetAdd, rxBuf, m_len);

}

bool fn_writeWiperLen(uint8_t *txBuf, uint8_t m_len){

	return fn_I2C_MasterWriteonly(ANALOG_I2C_HANDLE, POT_CH1_WRITE_ADDR, txBuf, m_len);

}

bool fn_incrementWiper(uint8_t incriments)
{
    for(uint8_t x = 0; x < incriments; x++)
    {
    	if(!fn_writeWiper(MEM_WIPER|COM_WIPERINC)){
    		return false;
    	}
    }
    return true;

}

bool fn_decrementWiper(uint8_t decriments)
{

    for(uint8_t x = 0; x < decriments; x++)
    {
    	if(!fn_writeWiper(MEM_WIPER|COM_WIPERDEC)){
    		return false;
    	}
    }
    return true;
}

/*void fn_changeBallastIntensityCh1(void)
{
	static uint8_t wiperUpdateValue = 0;
	static uint8_t wiperState = 0;
	switch(wiperState)
	{
		case 0:
			wiperUpdateValue += WIPER_STEPS;
			if(wiperUpdateValue > MAX_WIPER_VALUE)
			{
				wiperUpdateValue = MAX_WIPER_VALUE;
				wiperState = 1;
			}
			fn_writeWiper_ch1(wiperUpdateValue);
//			fn_writeWiper_ch2(wiperUpdateValue);
		break;
		case 1:
			if(wiperUpdateValue < WIPER_STEPS)
			{
				wiperUpdateValue = MIN_WIPER_VALUE;
				wiperState = 0;
			}
			else
			{
				wiperUpdateValue -= WIPER_STEPS;
			}
			fn_writeWiper_ch1(wiperUpdateValue);
//			fn_writeWiper_ch2(wiperUpdateValue);
		break;
		default:
			wiperState = 0;
		break;
	}
	return ;
}*/
void fn_fading (void)
{
	if(wiperValueMaintained < wiperValueToBeSet)
	{
		fn_incrementWiper(1);
	}
	else if(wiperValueMaintained > wiperValueToBeSet)
	{
		fn_decrementWiper(1);
	}
	fn_readWiper(g_rxData, MEM_WIPER|COM_READ, 2);
	wiperValueMaintained = g_rxData[1];
	return ;
}
/*-----------------------------------------------------------------------------------------------------------------------------*/
void fn_AnalogFadeCntrl(void)
{
	if(g_analogFadingStart)
	{
		if(!analog_stepDelay)
		{
			g_txData[0] = MEM_WIPER|COM_WRITE;
			g_txData[1] = wiperValueToBeSet;
			if(fn_writeWiperLen(g_txData, 2))
			{
				if(fn_readWiper(g_rxData, MEM_WIPER|COM_READ, 2))
				{
					wiperValueMaintained = (uint8_t)(g_rxData[1]);
				}
			}
		}
		else
		{
			if(fn_IsmSecTimerElapsed(analog_stepFadingTimerStart, analog_stepDelay))
			{
				fn_fading();
				analog_stepFadingTimerStart = fn_GetmSecTimerStart();
			}
		}
		if(wiperValueMaintained == wiperValueToBeSet)
		{
			g_analogLightStat = setToBeAnalogstatus;
			g_analogFadingStart = false;
		}
	}

}
bool fn_setAnalogIntensity(uint8_t m_val, uint8_t m_statFrom)
{
	setToBeAnalogstatus = m_val;
	g_analogFadingStart = true;
	wiperValueMaintained = WIPER_VALUE(g_analogLightStat);
	wiperValueToBeSet = WIPER_VALUE(setToBeAnalogstatus);
	analog_stepDelay = FADE_TIME_CALC(wiperValueMaintained,wiperValueToBeSet,g_analogFadeTime);
	analog_stepFadingTimerStart = fn_GetmSecTimerStart();

	if(m_statFrom!=0)
	{
		gecko_cmd_flash_ps_save(ANALOGSTAT_PS_KEY,sizeof(m_val),(uint8_t *)&m_val);
		fn_queueLightStat(0xFF,setToBeAnalogstatus, m_statFrom);
	}

	return true;
}

void fn_IdenitfyLight()
{
	if(g_isIdentifyEnabled)
	{
		static uint8_t identifyToggleCount = 0;
		static uint16_t m_toggleLoopTimeSatrt = 0;
		static uint8_t identifystate = IDENTIFY_TIMER_START;
		static uint8_t loopState = 0;
		static uint8_t g_wiperValueMaintained = 0;

		switch (identifystate)
		{
			case IDENTIFY_TIMER_START:
				memset(g_rxData, '\0', sizeof(g_rxData));
				if(fn_readWiper(g_rxData, MEM_WIPER|COM_READ, 2)){
					g_wiperValueMaintained = g_rxData[1];
				}
				identifyToggleCount = DEFAULT_TOGGLE_COUNT;
				m_toggleLoopTimeSatrt = 1;
				identifystate = IS_TOGGLE_TIMER_ELAPSED;
			break;
			case IS_TOGGLE_TIMER_ELAPSED:
				if(identifyToggleCount)
				{
					if(fn_IsmSecTimerElapsed(m_toggleLoopTimeSatrt, DEFAULT_TOGGLE_TIME))
					{
						--identifyToggleCount;
						memset(g_txData, '\0', sizeof(g_txData));

						g_txData[0] = MEM_WIPER|COM_WRITE;
						g_txData[1] = loopState;
						if(fn_writeWiperLen(g_txData, 2)){
							if(loopState == 113)                 //max Wiper That can be set during identify
							{
								loopState = 10;                 //min wiper value that can be set during identify
							}
							else
							{
								loopState = 113;
							}
							m_toggleLoopTimeSatrt = fn_GetmSecTimerStart();
						}
						else{
							if(loopState == 113)                 //max Wiper That can be set during identify
							{
								loopState = 10;                 //min wiper value that can be set during identify
							}
							else
							{
								loopState = 113;
							}
							m_toggleLoopTimeSatrt = fn_GetmSecTimerStart();
						}
					}
				}
				else
				{
					g_isIdentifyEnabled = false;
					memset(g_txData, '\0', sizeof(g_txData));

					g_txData[0] = MEM_WIPER|COM_WRITE;
					g_txData[1] = g_wiperValueMaintained;
					if(fn_writeWiperLen(g_txData, 2)){
						identifystate = IDENTIFY_TIMER_START;
					}
				}
			break;
			default:
			break;
		}
	}
}

void fn_sceneCfg(uint16_t m_id, uint8_t m_shid, uint8_t m_statFrom, uint8_t m_intensity){

	int m_index = fn_getSceneIndex(m_id);
	if((m_index==-1)){
		m_index = fn_getFreeSceneIndex();
		if(m_index!=-1){
			sSceneCfg[m_index].sceneID = m_id;
			sSceneCfg[m_index].sScene_mapping.LightID = m_shid;
			sSceneCfg[m_index].sScene_mapping.intensity = m_intensity;
			gecko_cmd_flash_ps_save(SCENE_PS_KEY,sizeof(sSceneCfg),(uint8_t *)&sSceneCfg);
		}

	}
	else{
		sSceneCfg[m_index].sceneID = m_id;
		sSceneCfg[m_index].sScene_mapping.LightID = m_shid;
		sSceneCfg[m_index].sScene_mapping.intensity = m_intensity;
		gecko_cmd_flash_ps_save(SCENE_PS_KEY,sizeof(sSceneCfg),(uint8_t *)&sSceneCfg);
	}

}

void fn_goto_scene(uint16_t m_id, uint8_t m_statFrom){

	int m_index = fn_getSceneIndex(m_id);
	if(m_index!=-1){
		if(sSceneCfg[m_index].sceneID!=0){
			fn_setAnalogIntensity(sSceneCfg[m_index].sScene_mapping.intensity,m_statFrom);
		}
	}
}


void fn_sceneDel(uint16_t m_id){

	int m_index = fn_getSceneIndex(m_id);
	if(m_index!=-1){
			sSceneCfg[m_index].sceneID = 0;
			sSceneCfg[m_index].sScene_mapping.LightID = 0;
			sSceneCfg[m_index].sScene_mapping.intensity = 0;
			gecko_cmd_flash_ps_save(SCENE_PS_KEY,sizeof(sSceneCfg),(uint8_t *)&sSceneCfg);
	}

}


/****************************************************************************************************************/
int fn_getSceneIndex(uint16_t sceneID)
{
	for(uint8_t scnIndx = 0;scnIndx<MAX_SCENES;scnIndx++)
	{
		if(sSceneCfg[scnIndx].sceneID == sceneID)
		{
			return scnIndx;
		}
	}
	return -1;
}

/****************************************************************************************************************/
int fn_getFreeSceneIndex()
{
	for(uint8_t scnIndx = 0;scnIndx<MAX_SCENES;scnIndx++)
	{
		if(sSceneCfg[scnIndx].sceneID == 0)
		{
			return scnIndx;
		}
	}
	return -1;
}


