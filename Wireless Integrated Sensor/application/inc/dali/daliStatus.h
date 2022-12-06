/*
 * daliStatus.h
 *
 *  Created on: Nov 17, 2020
 *      Author: Rakshith
 */

#ifndef APP_DALI_DALISTATUS_H_
#define APP_DALI_DALISTATUS_H_

#include "application/inc/dali/dali.h"

typedef struct{
	uint8_t m_statFrom;
	struct_gpEntityLoc gpID;
}__attribute__((__packed__))struct_gpBallastStatus;

struct gpBallastStatNode {
	struct gpBallastStatNode *GPSNext;
	struct_gpBallastStatus GPSInfo;
};

struct gpBallastStatQueue{
	struct gpBallastStatNode *front, *rear;
};

extern struct gpBallastStatQueue sgpBalStatQueue;
struct gpBallastStatNode *fn_newNodeGpStat(struct_gpBallastStatus *m_info);
struct gpBallastStatQueue *fn_createQGpStat(struct gpBallastStatQueue *m_q);
bool fn_enQueueGpStat(struct gpBallastStatQueue* q, struct_gpBallastStatus *m_info);
void fn_deQueueGpStat(struct gpBallastStatQueue *q);

void fn_queryDaliStatus();



#endif /* APP_DALI_DALISTATUS_H_ */
