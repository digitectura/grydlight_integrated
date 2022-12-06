/*
 * BLE_Queue.h
 *
 *  Created on: Aug 29, 2020
 *      Author: User
 */
/*****************************************************************************************************************************************/
#ifndef BLE_QUEUE_H_
#define BLE_QUEUE_H_
/*****************************************************************************************************************************************/
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "app.h"
#include "application/inc/command_handler.h"
/*****************************************************************************************************************************************/
enum
{
	CHECK_Q_EMPTY,
	SEND_PCKT
};
/*****************************************************************************************************************************************/
typedef struct BLEnode_t
{
	struct BLEnode_t *sNext;
	uint16_t m_destAddr;
	uint8_t frameType;
	uint8_t sBLE_pktSize;
	uint8_t sBLEData[100];
}BLEnode_t;
/*****************************************************************************************************************************************/
typedef struct bleQ_t
{
	 BLEnode_t *sFront, *sRear;
}bleQ_t;
/*****************************************************************************************************************************************/
bleQ_t sBLEQ;

bool fn_isQEmpty(void);
void fn_enQ_blePkt(FrameType_EN FT,uint8_t len,uint8_t* data, uint16_t);
void fn_deQ_blePkt(void);
void fn_processBleQ(void);
/*****************************************************************************************************************************************/

#endif /* BLE_QUEUE_H_ */
