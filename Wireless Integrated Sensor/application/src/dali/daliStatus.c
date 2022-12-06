/*
 * daliStatus.c
 *
 *  Created on: Nov 17, 2020
 *      Author: Rakshith
 */

#include "application/inc/dali/daliStatus.h"
#include "application/inc/dali/daliCommand.h"
#include "application/inc/dali/daliApplication.h"

struct gpBallastStatQueue sgpBalStatQueue;
struct_DaliAction g_sDaliAction = {0};

struct gpBallastStatNode *fn_newNodeGpStat(struct_gpBallastStatus *m_info){
    struct gpBallastStatNode *temp = (struct gpBallastStatNode*)malloc(sizeof(struct gpBallastStatNode));
    if(!temp){
    	return NULL;
    }
//    strcpy(temp->packetData, m_packet);
    memcpy(&temp->GPSInfo, m_info, sizeof(struct_gpBallastStatus));
    temp->GPSNext = NULL;
    return temp;
}
struct gpBallastStatQueue *fn_createQGpStat(struct gpBallastStatQueue *m_q){
    m_q->front = m_q->rear = NULL;
    return m_q;
}
bool fn_enQueueGpStat(struct gpBallastStatQueue* q, struct_gpBallastStatus *m_info){
    // Create a new LL node
    struct gpBallastStatNode *temp = fn_newNodeGpStat(m_info);

    if(!temp){
    	return false;
    }
    // If queue is empty, then new node is dfront and drear both
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return true;
    }

    // Add the new node at the end of queue and change drear
    q->rear->GPSNext = temp;
    q->rear = temp;
    return true;
}
void fn_deQueueGpStat(struct gpBallastStatQueue *q){
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return;

    // Store previous dfront and move dfront one node ahead
    struct gpBallastStatNode *temp = q->front;

    q->front = q->front->GPSNext;

    // If dfront becomes NULL, then change drear also as NULL
    if (q->front == NULL)
        q->rear = NULL;

    free(temp);
}
/*

void fn_queryDaliStatus(){
	static uint8_t m_daliQStat = STATE_ONE;
	static int8_t m_gpEntIndex = -1;
	static uint8_t m_loopID = 0;
	static uint32_t m_shortID = 0;

	switch(m_daliQStat){
	case STATE_ONE:
		if(sgpBalStatQueue.front!= NULL){
			m_daliQStat = STATE_TWO;
		}
		break;
	case STATE_TWO:
		m_gpEntIndex = fn_extractEntityIndex(sgpBalStatQueue.front->GPSInfo.gpID.gpEntityLabel);
		if(m_gpEntIndex!=-1){
			m_daliQStat = STATE_THREE;
		}
		else{
			m_daliQStat = STATE_ONE;
			fn_deQueueGpStat(&sgpBalStatQueue);
		}
		break;
	case STATE_THREE:
		if(criticalOpnLock != true){
			criticalOpnLock = true;

			memset(criticalOpnMonitor, '\0', sizeof(criticalOpnMonitor));
			sprintf(criticalOpnMonitor, "Func: %s Line: %d", __func__, __LINE__);

			memset(&g_sGpEntity, '\0', sizeof(g_sGpEntity));
			if(fn_readSector(sGpLookUp[m_gpEntIndex].sectorNumber_GP,
					(uint8_t *)&g_sGpEntity, sizeof(g_sGpEntity), sGpLookUp[m_gpEntIndex].sectorOffset_GP)){
				for(uint8_t m_idg = 0; m_idg<g_sGpEntity.totElts; m_idg++){
					if(fn_extractDeviceType(g_sGpEntity.sGPEntityLoc[m_idg].gpEntityLabel)==DALI){

						m_loopID = fn_extractDaliLoopID(strtok(g_sGpEntity.sGPEntityLoc[m_idg].gpEntityLabel, "-"));
						m_shortID = (uint32_t)(atoi(strtok(NULL, "#")));

						memset(&g_sDaliAction, '\0', sizeof(g_sDaliAction));
						g_sDaliAction.d_add = (uint16_t)((uint8_t)m_shortID | ((m_loopID == DALI_LOOP2) ? 0x200 : 0));
						g_sDaliAction.d_report = 0x01;
						g_sDaliAction.d_statFrom = sgpBalStatQueue.front->GPSInfo.m_statFrom;

						if(fn_enQueueAC(&sDaliActWaitQueue, &g_sDaliAction)!=true){
#ifdef DEVICE_DEBUG
							printf("Memory allocation fault Line Func: %s Line: %d", __func__, __LINE__);
#endif
						}
					}
				}
				m_daliQStat = STATE_ONE;
				fn_deQueueGpStat(&sgpBalStatQueue);
			}
			else{
				m_daliQStat = STATE_ONE;
				fn_deQueueGpStat(&sgpBalStatQueue);
			}
			criticalOpnLock = false;
		}
		break;
	}

}

*/
