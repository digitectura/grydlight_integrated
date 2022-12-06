/*
 * intervalTimer.c
 *
 *  Created on: 21-May-2020
 *      Author: BINDUSHREE M V
 */
/****************************************************************************************************************/
#include "application/inc/intervalTimer.h"
#include "em_timer.h"
#include "application/inc/dali/dali.h"
/****************************************************************************************************************/
uint16_t countmsec = 0;
uint16_t countsec = 0;
uint16_t countusec = 0;
/****************************************************************************************************************/
void fn_msecTimerinit(void)
{
	// Initialize the timer
	TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
	timerInit.prescale = timerPrescale2;
	TIMER_Init(TIMER2, &timerInit);
	TIMER_TopSet(TIMER2, MSEC_TIME_COUNTER);							//counter value = 1msec
	TIMER_IntEnable(TIMER2, TIMER_IF_OF);
	NVIC_EnableIRQ(TIMER2_IRQn);
	return ;
}
void fn_usecTimerinit(void)
{
	// Initialize the timer
	TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
	timerInit.prescale = timerPrescale1;
	TIMER_Init(TIMER3, &timerInit);
	TIMER_TopSet(TIMER3, USEC_TIMER_ARR);							//counter value = 50usec
	TIMER_IntEnable(TIMER3, TIMER_IF_OF);
	NVIC_EnableIRQ(TIMER3_IRQn);
	TIMER_Enable(TIMER3, false);
	return ;
}
void fn_DALItimerInit(void)
{
	// Initialize the timer
	TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
	timerInit.prescale = timerPrescale1;
	TIMER_Init(TIMER1, &timerInit);
	TIMER_TopSet(TIMER1, DALI_TIMER_ARR_MEANWELL);							//counter value changed from 335usec to 425 usec
	TIMER_IntEnable(TIMER1, TIMER_IF_OF);
	NVIC_EnableIRQ(TIMER1_IRQn);
	return ;
}
/****************************************************************************************************************/
void TIMER2_IRQHandler(void)						//mSec_timer
{
	if(TIMER_IntGet(TIMER2) == TIMER_IF_OF)
	{
		countmsec++;								//each count = 1msec
		if((countmsec % 1000) == 0)
		{
			countsec++;								//each count = 1sec
		}
	}
	// Acknowledge the interrupt
	TIMER_IntClear(TIMER2, TIMER_IntGet(TIMER2));
}
/****************************************************************************************************************/
void TIMER3_IRQHandler(void)						//mSec_timer
{
	if(TIMER_IntGet(TIMER3) == TIMER_IF_OF)
	{
		countusec++;								//each count = 1msec
	}
	// Acknowledge the interrupt
	TIMER_IntClear(TIMER3, TIMER_IntGet(TIMER3));
}
/****************************************************************************************************************/
/*void fn_secTimerinit(void)
{
	TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
	timerInit.prescale = timerPrescale1024;
	TIMER_Init(TIMER3, &timerInit);
	TIMER_TopSet(TIMER3, SEC_TIME_COUNTER);								//counter value = 1sec
	TIMER_IntEnable(TIMER3, TIMER_IF_OF);
	NVIC_EnableIRQ(TIMER3_IRQn);
	return ;
}*/
/****************************************************************************************************************/
void TIMER1_IRQHandler(void)						//Sec_timer
{
	static int8_t dataIndex = 15;
	static uint8_t transmitState = 0;
	static uint8_t transmitState_firstbit = 0;

	if(TIMER_IntGet(TIMER1) == TIMER_IF_OF)
	{
		switch(transmitState){
		case 0:
			if(transmitState_firstbit==0){
				transmitDataComplete[DALI_LOOP1] = 0;
				GPIO_PinOutSet(DALI_TX_PORT, DALI_TX_PIN);
				transmitState_firstbit = 1;
			}
			else{
				GPIO_PinOutClear(DALI_TX_PORT, DALI_TX_PIN);
				transmitState_firstbit = 0;
				transmitState = 1;
			}
			break;
		case 1:
			if(transmitState_firstbit==0){
				if((transmitDataPacket[DALI_LOOP1]>>dataIndex)&0x01){
					GPIO_PinOutSet(DALI_TX_PORT, DALI_TX_PIN);
					transmitState_firstbit = 1;
				}
				else{
					GPIO_PinOutClear(DALI_TX_PORT, DALI_TX_PIN);
					transmitState_firstbit = 1;
				}
			}
			else{
				if((transmitDataPacket[DALI_LOOP1]>>dataIndex)&0x01){
					GPIO_PinOutClear(DALI_TX_PORT, DALI_TX_PIN);
					transmitState_firstbit = 0;
					dataIndex--;
					if(dataIndex<0){
						dataIndex = 15;
						transmitState = 2;
					}
				}
				else{
					GPIO_PinOutSet(DALI_TX_PORT, DALI_TX_PIN);
					transmitState_firstbit = 0;
					dataIndex--;
					if(dataIndex<0){
						dataIndex = 15;
						transmitState = 2;
					}
				}
			}
			if(dataIndex<0){
				dataIndex = 15;
				transmitState = 2;
			}
			break;
		case 2:

			GPIO_PinOutClear(DALI_TX_PORT, DALI_TX_PIN);
			transmitState = 0;
			transmitDataComplete[DALI_LOOP1] = 1;
			TIMER_Enable(TIMER1, false);
			break;
		}
	}
	// Acknowledge the interrupt
	TIMER_IntClear(TIMER1, TIMER_IntGet(TIMER1));
}
/****************************************************************************************************************/
unsigned char fn_IsSecTimerElapsed ( uint16_t start_time, uint16_t delay )
{
    if ( start_time == 0 )
    {
        return false;
    }
    uint16_t current_time = fn_GetSecTimerStart();

    /* This variable holds the time difference between current ember time and start time when
       ember timer count has rolled over (0xFFFF) and restarted the count */
    uint16_t time_diff;

    /* Check if current system time is greater than start time */
    if( current_time >= start_time ){
        /* Check if current system time has crossed the required delay */
        if ( (current_time - start_time) >= delay )
        {
            /* If the delay has been achieved the function returns true */
            return true;
        }
    }
    /* In case the current system time is less than the start time which is the case then the timer
       overflows, the offset is calculated using the TOTAL_MILLISEC_TIME_VALUE (0xFFFF) as shown */
    else if ( current_time < start_time ){
       /* Calculating the actual difference when the timer has overflowed */
       time_diff = ( TOTAL_MILLISEC_TIME_VALUE - start_time ) + current_time;

       /* Checking if the time difference has crossed the delay required */
       if ( time_diff >= delay )
       {
           /* If the delay has been achieved the function returns true */
           return true;
       }
    }
    /* If the delay has not been achieved the function returns false */
    return false;
}
/*************************************************************************************************************************************************************/
unsigned char fn_IsuSecTimerElapsed ( uint16_t start_time, uint16_t delay )
{
    if ( start_time == 0 )
    {
        return false;
    }
    uint16_t current_time = fn_GetuSecTimerStart();

    /* This variable holds the time difference between current ember time and start time when
       ember timer count has rolled over (0xFFFF) and restarted the count */
    uint16_t time_diff;

    /* Check if current system time is greater than start time */
    if( current_time >= start_time ){
        /* Check if current system time has crossed the required delay */
        if ( (current_time - start_time) >= delay )
        {
            /* If the delay has been achieved the function returns true */
            return true;
        }
    }
    /* In case the current system time is less than the start time which is the case then the timer
       overflows, the offset is calculated using the TOTAL_MILLISEC_TIME_VALUE (0xFFFF) as shown */
    else if ( current_time < start_time ){
       /* Calculating the actual difference when the timer has overflowed */
       time_diff = ( TOTAL_MILLISEC_TIME_VALUE - start_time ) + current_time;

       /* Checking if the time difference has crossed the delay required */
       if ( time_diff >= delay )
       {
           /* If the delay has been achieved the function returns true */
           return true;
       }
    }
    /* If the delay has not been achieved the function returns false */
    return false;
}
/*************************************************************************************************************************************************************/
unsigned char fn_IsmSecTimerElapsed ( uint16_t start_time, uint16_t delay )
{
    if ( start_time == 0 )
    {
        return false;
    }
    uint16_t current_time = fn_GetmSecTimerStart();

    /* This variable holds the time difference between current ember time and start time when
       ember timer count has rolled over (0xFFFF) and restarted the count */
    uint16_t time_diff;

    /* Check if current system time is greater than start time */
    if( current_time >= start_time ){
        /* Check if current system time has crossed the required delay */
        if ( (current_time - start_time) >= delay )
        {
            /* If the delay has been achieved the function returns true */
            return true;
        }
    }
    /* In case the current system time is less than the start time which is the case then the timer
       overflows, the offset is calculated using the TOTAL_MILLISEC_TIME_VALUE (0xFFFF) as shown */
    else if ( current_time < start_time ){
       /* Calculating the actual difference when the timer has overflowed */
       time_diff = ( TOTAL_MILLISEC_TIME_VALUE - start_time ) + current_time;

       /* Checking if the time difference has crossed the delay required */
       if ( time_diff >= delay )
       {
           /* If the delay has been achieved the function returns true */
           return true;
       }
    }
    /* If the delay has not been achieved the function returns false */
    return false;
}
/*************************************************************************************************************************************************************/
/*  This function returns the current value/count of the timer */
uint16_t fn_GetmSecTimerStart(void)
{
    if ( countmsec == 0 ){
    	countmsec++;
    }
    return countmsec;
}
/*************************************************************************************************************************************************************/
/*  This function returns the current value/count of the timer */
uint16_t fn_GetSecTimerStart(void)
{
    if ( countsec == 0 ){
    	countsec++;
    }
    return countsec;
}
/*************************************************************************************************************************************************************/
/*  This function returns the current value/count of the timer */
uint16_t fn_GetuSecTimerStart(void)
{
    if ( countusec == 0 ){
    	countusec++;
    }
    return countusec;
}
/*************************************************************************************************************************************************************/
