/****************************************************************************************************************/
#ifndef APPLICATION_INC_SWITCH_H_
#define APPLICATION_INC_SWITCH_H_
/****************************************************************************************************************/
#include "main.h"
/****************************************************************************************************************/
enum
{
	SWITCH_PRESSED = 1,
	SWITCH_RELEASED = 0
};
/****************************************************************************************************************/
void fn_indicate_LoopMapping_LEDstate(uint8_t loopMapState);
void fn_switch_intrpt_callBack(uint8_t intNum);
void fn_switchInit(void);
void fn_enableSwitch_int(void);
void fn_disableSWitch_int(void);
void fn_switchReleased(void);
void fn_handleSwitchIntrpt(void);
void fn_loopLightMapping(void);
void fn_sendLightMap_Pkt(void);
uint8_t fn_getCurntMappedIndx(void);

/****************************************************************************************************************/
#endif /* APPLICATION_INC_SWITCH_H_ */
