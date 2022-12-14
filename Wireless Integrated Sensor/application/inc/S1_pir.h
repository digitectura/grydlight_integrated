/*******************************************************************************************************************************************************************/
#ifndef APPLICATION_PIR_PIR_H_
#define APPLICATION_PIR_PIR_H_
/*******************************************************************************************************************************************************************/
/* Libraries containing default Gecko configuration values */
#include "main.h"
/*******************************************************************************************************************************************************************/
#define PIR_BACKUP_TIME					5000			//msec

#define PIR_DIVIDE_COUNTER				4
/*******************************************************************************************************************************************************************/
typedef enum
{
	NO_MOTION_DETECTED,
	MOTION_DETECTED
}enum_PIR_trigger_state;
/*******************************************************************************************************************************************************************/
typedef enum
{
	SET_PIR_CONFIG ,
	CHECK_PIR_STATUS ,
	UPDATE_PIR_STATUS_CHANGE,
	DECIDE_PIR_STATE,
	WAIT_AND_WATCH_PERIOD,
	SLEEP_PERIOD,
	BACKUP_TIME,
}enum_PIR_ProcessState;
/*******************************************************************************************************************************************************************/
typedef enum
{
	OCCUPIED = 0xFF,
	UNOCCUPIED = 0x00
}enum_PIR_State;
/*******************************************************************************************************************************************************************/
enum
{
	PIR_ENABLE = 0,
	PIR_DISABLE
};
/*******************************************************************************************************************************************************************/
void fn_PIR_Process(void);
void fn_PIR_int_CallBack(uint8_t);
//void pir_set_state(enum_PIR_trigger_state s);
void fn_pir_init(void);
void pir_disable(void);
void pir_enable(void);
void pir_enable_disable(uint8_t pir_en_dis);

extern uint16_t pir_unoccupancyTime;
extern enum_PIR_trigger_state ePIR_trggr_state;
extern enum_PIR_ProcessState ePIRprocess_state;
/*******************************************************************************************************************************************************************/
#endif /* APPLICATION_PIR_PIR_H_ */
