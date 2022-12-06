/*
 * BLE_Queue.c
 *
 *  Created on: Aug 27, 2020
 *      Author: User
 */
/*****************************************************************************************************************************************/
#include "application/inc/BLE_Queue.h"
#include "stddef.h"
#include "stdlib.h"
/*****************************************************************************************************************************************/
bleQ_t sBLEQ =
				{
					.sFront = NULL,
					.sRear = NULL
				};
/*****************************************************************************************************************************************/
bool fn_isQEmpty(void)
{
	//if queue is empty
	if(sBLEQ.sFront == NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}
/*****************************************************************************************************************************************/
void fn_enQ_blePkt(FrameType_EN frameType,uint8_t len,uint8_t *payload, uint16_t m_addr)
{
	BLEnode_t *new = (BLEnode_t*)malloc(sizeof(BLEnode_t));
	if(new == NULL)
	{
		return ;
	}
	new->frameType 		= frameType;
	new->sBLE_pktSize 	= len;
	new->m_destAddr 	=  m_addr;
	memcpy(new->sBLEData,payload,len);
	//if queue is empty then new node is both front and rear;
	if( (sBLEQ.sFront == NULL) && (sBLEQ.sRear == NULL) )
	{
		sBLEQ.sFront = new;
		sBLEQ.sRear = new;
		sBLEQ.sFront -> sNext = NULL;
		sBLEQ.sRear	 -> sNext = NULL;
		return ;
	}
	sBLEQ.sRear->sNext = new;
	sBLEQ.sRear 	   = new;
	sBLEQ.sRear->sNext = NULL;
	return ;
}
/*****************************************************************************************************************************************/
BLEnode_t *fn_peek_blePkt(void)
{
	//if queue is empty
	if(fn_isQEmpty())
	{
		return NULL;
	}
	return sBLEQ.sFront;
}
/*****************************************************************************************************************************************/
void fn_deQ_blePkt(void)
{
	BLEnode_t *front;
	//if queue is empty
	if(fn_isQEmpty())
	{
		return ;
	}
	front = sBLEQ.sFront;
	sBLEQ.sFront = sBLEQ.sFront->sNext;
	if(sBLEQ.sFront == NULL)
	{
		sBLEQ.sRear = NULL;
	}
	free(front);
}
/*****************************************************************************************************************************************/
void fn_processBleQ(void)
{
	BLEnode_t* peek = fn_peek_blePkt();
	if(!peek)
	{
		return ;
	}
	DBG_PRINT("---------------sndg [%d]_pkt to %d------------------\r\n",peek->frameType,peek->m_destAddr);
	uint16_t sentRspns = gecko_cmd_mesh_vendor_model_send(
											PRIMARY_ELEMENT,			\
											MY_VENDOR_ID,				\
											MY_MODEL_ID,				\
											peek->m_destAddr,		\
											0,							\
											0,							\
											0,							\
											peek->frameType,			\
											0xff,						\
											peek->sBLE_pktSize,			\
											peek->sBLEData)->result;
	DBG_PRINT("sent rspns = %d\r\n",sentRspns);
	if(sentRspns == 0)
	{
//		TURN_ON_LED(APP_LED2);
//		gecko_cmd_hardware_set_soft_timer(MILLISECONDS(200),LED2_TIMER_ID,ONESHOT_TIMER);		//	Start timer for LED OFF
		fn_deQ_blePkt();
	}
	return ;
}
/*****************************************************************************************************************************************/
/*****************************************************************************************************************************************/
