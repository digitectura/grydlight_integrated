/*
 * intervalTimer.h
 *
 *  Created on: 21-May-2020
 *      Author: BINDUSHREE M V
 */
/*************************************************************************************************************************************************************/
#ifndef INTERVALTIMER_H_
#define INTERVALTIMER_H_
/****************************************************************************************************************/
#include "main.h"
/*************************************************************************************************************************************************************/

/*************************************************************************************************************************************************************/
#define MSEC_TIME_COUNTER				39999		//prescalar 1	- 1msec
#define USEC_TIMER_ARR					3999		//50usec
#define DALI_TIMER_ARR					26999		//
#define DALI_TIMER_ARR_MEANWELL					33999		//425usec
#define TOTAL_MILLISEC_TIME_VALUE		65536

extern uint16_t countmsec;
extern uint16_t countsec;
/****************************************************************************************************************/
void fn_msecTimerinit(void);
void fn_usecTimerinit(void);
void fn_DALItimerInit(void);

unsigned char fn_IsSecTimerElapsed ( uint16_t start_time, uint16_t delay );
unsigned char fn_IsuSecTimerElapsed ( uint16_t start_time, uint16_t delay );
unsigned char fn_IsmSecTimerElapsed ( uint16_t start_time, uint16_t delay );
uint16_t fn_GetmSecTimerStart(void);
uint16_t fn_GetSecTimerStart(void);
uint16_t fn_GetuSecTimerStart(void);
/****************************************************************************************************************/
#endif /* INTERVALTIMER_H_ */
