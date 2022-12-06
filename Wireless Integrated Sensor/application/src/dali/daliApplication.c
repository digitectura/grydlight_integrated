/*
 * daliApplication.c
 *
 *  Created on: 23-Aug-2019
 *      Author: Rakshit
 */
#include <application/inc/dali/daliApplication.h>
#include "application/inc/intervalTimer.h"


struct DaliCfgQueue sDaliCfgQueue;
struct DaliActQueue sDaliActQueue;
struct DaliGrpQueue sDaliGrpQueue;
struct DaliActQueue sDaliActWaitQueue;
struct GPEntityQueue sGPEntityQueue;
struct SNEntityQueue sSNEntityQueue;
struct DaliSceneCfgQueue sDaliSceneCfgQueue;
struct DaliSceneActQueue sDaliSceneActQueue;
struct GP_SNActNodeQueue sGroupSceneActionQueue;
struct_daliLookUp sDaliLookUp[NUM_OF_DALI_LOOPS][TOTAL_DALI_BALLASTS];
struct_dt1Identify sDT1IdentifyStat[10] = { 0 };
dalisensorCommission_cplt_typedef g_daliCommissionCplt = DALI_COMMISSION_INCPLT;
struct_scnUIDLookup sScnUIDLP[TOTAL_SCENES_CTRL] = { 0 };

struct DaliActQueueStat sDaliActQueueStat = {
		-1,
		-1,
};

struct_DaliSceneCfg m_sDaliSceneCfg = { 0 };

// A utility function to create a new linked list node.
struct DaliCfgNode *fn_newNodeDC(struct_DaliBallastCfg *m_cfg,
		uint8_t *m_cfgChange) {
	struct DaliCfgNode *temp = (struct DaliCfgNode*) malloc(
			sizeof(struct DaliCfgNode));
	if (!temp) {
		return NULL;
	}
//    strcpy(temp->packetData, m_packet);
	memcpy(&temp->sDaliCfg, m_cfg, sizeof(struct_DaliBallastCfg));
	temp->cfgChangeFlag = *m_cfgChange;
	if (m_cfg->ballastShortID != 0xFF) {
//    	temp->cfgFlashUpdateFlag = 0;
	}
	temp->sDNext = NULL;
	return temp;
}

struct DaliCfgQueue *fn_createDCQ(struct DaliCfgQueue *m_q) {
//    m_q = (struct DaliCfgQueue*)malloc(sizeof(struct DaliCfgQueue));
	m_q->dfront = m_q->drear = NULL;
	return m_q;
}

// The function to add a key k to q
bool fn_enQueueDC(struct DaliCfgQueue* q, struct_DaliBallastCfg *m_cfg,
		uint8_t m_cfgChange) {
	// Create a new LL node
	struct DaliCfgNode *temp = fn_newNodeDC(m_cfg, &m_cfgChange);

	if (!temp) {
		return false;
	}
	// If queue is empty, then new node is dfront and drear both
	if (q->drear == NULL) {
		q->dfront = q->drear = temp;
		return true;
	}

	// Add the new node at the end of queue and change drear
	q->drear->sDNext = temp;
	q->drear = temp;
	return true;
}

// Function to remove a key from given queue q
void fn_deQueueDC(struct DaliCfgQueue *q) {
	// If queue is empty, return NULL.
	if (q->dfront == NULL)
		return;

	// Store previous dfront and move dfront one node ahead
	struct DaliCfgNode *temp = q->dfront;

	q->dfront = q->dfront->sDNext;

	// If dfront becomes NULL, then change drear also as NULL
	if (q->dfront == NULL)
		q->drear = NULL;

	free(temp);
}

// A utility function to create a new linked list node.
struct DaliActNode *fn_newNodeAC(struct_DaliAction *m_cfg) {
	struct DaliActNode *temp = (struct DaliActNode*) malloc(
			sizeof(struct DaliActNode));
	if (!temp) {
		return NULL;
	}
//    strcpy(temp->packetData, m_packet);
	memcpy(&temp->sDaliAct, m_cfg, sizeof(struct_DaliAction));
	temp->sDNext = NULL;
	return temp;
}

struct DaliActQueue *fn_createACQ(struct DaliActQueue *m_q) {
//    m_q = (struct DaliActQueue*)malloc(sizeof(struct DaliActQueue));
	m_q->dfront = m_q->drear = NULL;
	return m_q;
}


// The function to add a key k to q
bool fn_enQueueAC(struct DaliActQueue* q, struct_DaliAction *m_cfg) {
	// Create a new LL node
	struct DaliActNode *temp = fn_newNodeAC(m_cfg);

	if (!temp) {
		return false;
	}
	// If queue is empty, then new node is dfront and drear both
	if (q->drear == NULL) {
		q->dfront = q->drear = temp;
		return true;
	}

	// Add the new node at the end of queue and change drear
	q->drear->sDNext = temp;
	q->drear = temp;
	return true;
}

// Function to remove a key from given queue q
void fn_deQueueAC(struct DaliActQueue *q) {
	// If queue is empty, return NULL.
	if (q->dfront == NULL)
		return;

	// Store previous dfront and move dfront one node ahead
	struct DaliActNode *temp = q->dfront;

	q->dfront = q->dfront->sDNext;

	// If dfront becomes NULL, then change drear also as NULL
	if (q->dfront == NULL)
		q->drear = NULL;

	free(temp);
}

// A utility function to create a new linked list node.
struct DaliGrpNode *fn_newNodeGP(struct_DaliGroupCfg *m_cfg,
		daliGrpCfg_typedef *gpFlag) {
	struct DaliGrpNode *temp = (struct DaliGrpNode*) malloc(
			sizeof(struct DaliGrpNode));
	if (!temp) {
		return NULL;
	}
//    strcpy(temp->packetData, m_packet);
	memcpy(&temp->sDaliGrp, m_cfg, sizeof(struct_DaliGroupCfg));
	temp->d_Flg = *gpFlag;
	temp->sDGNext = NULL;
	return temp;
}

// A utility function to create a new linked list node.
struct GPEntityNode *fn_newNodeGPE(struct_GPentity *m_gpEntity) {
	struct GPEntityNode *temp = (struct GPEntityNode*) malloc(
			sizeof(struct GPEntityNode));
	if (!temp) {
		return NULL;
	}
	memcpy(&temp->sGPEntityND, m_gpEntity, sizeof(struct_GPentity));
	temp->sGPEntityNext = NULL;
	return temp;
}

struct DaliGrpQueue *fn_createGPQ(struct DaliGrpQueue *m_q) {
//    m_q = (struct DaliGrpQueue*)malloc(sizeof(struct DaliGrpQueue));
	m_q->dfront = m_q->drear = NULL;
	return m_q;
}

struct GPEntityQueue *fn_createGPEnQ(struct GPEntityQueue *m_q) {
//    m_q = (struct GPEntityQueue*)malloc(sizeof(struct GPEntityQueue));
	m_q->gpFront = m_q->gpRear = NULL;
	return m_q;
}

bool fn_enQueueGP(struct DaliGrpQueue* q, struct_DaliGroupCfg *m_cfg,
		daliGrpCfg_typedef gpGlag) {
	// Create a new LL node
	struct DaliGrpNode *temp = fn_newNodeGP(m_cfg, &gpGlag);

	if (!temp) {
		return false;
	}
	// If queue is empty, then new node is dfront and drear both
	if (q->drear == NULL) {
		q->dfront = q->drear = temp;
		return true;
	}

	// Add the new node at the end of queue and change drear
	q->drear->sDGNext = temp;
	q->drear = temp;
	return true;
}

bool fn_enQueueGPEn(struct GPEntityQueue* q, struct_GPentity *m_gpE) {
	// Create a new LL node
	struct GPEntityNode *temp = fn_newNodeGPE(m_gpE);

	if (!temp) {
		return false;
	}
	// If queue is empty, then new node is dfront and drear both
	if (q->gpRear == NULL) {
		q->gpFront = q->gpRear = temp;
		return true;
	}

	// Add the new node at the end of queue and change drear
	q->gpRear->sGPEntityNext = temp;
	q->gpRear = temp;
	return true;
}

// Function to remove a key from given queue q
void fn_deQueueGP(struct DaliGrpQueue *q) {
	// If queue is empty, return NULL.
	if (q->dfront == NULL)
		return;

	// Store previous dfront and move dfront one node ahead
	struct DaliGrpNode *temp = q->dfront;

	q->dfront = q->dfront->sDGNext;

	// If dfront becomes NULL, then change drear also as NULL
	if (q->dfront == NULL)
		q->drear = NULL;

	free(temp);
}

// Function to remove a key from given queue q
void fn_deQueueGPEn(struct GPEntityQueue *q) {
	// If queue is empty, return NULL.
	if (q->gpFront == NULL)
		return;

	// Store previous dfront and move dfront one node ahead
	struct GPEntityNode *temp = q->gpFront;

	q->gpFront = q->gpFront->sGPEntityNext;

	// If dfront becomes NULL, then change drear also as NULL
	if (q->gpFront == NULL)
		q->gpRear = NULL;

	free(temp);
}

struct DaliSceneActNode *fn_newNodeScnAct(struct_DaliSceneAction *m_scnAct) {
	struct DaliSceneActNode *temp = (struct DaliSceneActNode *) malloc(
			sizeof(struct DaliSceneActNode));
	if (!temp) {
		return NULL;
	}
	memcpy(&temp->sDaliScnAct, m_scnAct, sizeof(struct_DaliSceneAction));
	temp->sDSActNext = NULL;
	return temp;
}

struct DaliSceneActQueue *fn_createScnActQ(struct DaliSceneActQueue *m_q) {
//	m_q = (struct DaliSceneActQueue *)malloc(sizeof(struct DaliSceneActQueue));
	m_q->d_scnActFront = m_q->d_scnActRear = NULL;
	return m_q;
}

bool fn_enQueueScnAct(struct DaliSceneActQueue* m_q,
		struct_DaliSceneAction *m_scnAct) {
	struct DaliSceneActNode *temp = fn_newNodeScnAct(m_scnAct);

	if (!temp) {
		return false;
	}

	if (m_q->d_scnActRear == NULL) {
		m_q->d_scnActFront = m_q->d_scnActRear = temp;
		return true;
	}

	m_q->d_scnActRear->sDSActNext = temp;
	m_q->d_scnActRear = temp;

	return true;
}

void fn_deQueueScnAct(struct DaliSceneActQueue *m_q) {
	// If queue is empty, return NULL.
	if (m_q->d_scnActRear == NULL)
		return;

	// Store previous dfront and move dfront one node ahead
	struct DaliSceneActNode *temp = m_q->d_scnActFront;

	m_q->d_scnActFront = m_q->d_scnActFront->sDSActNext;

	// If dfront becomes NULL, then change drear also as NULL
	if (m_q->d_scnActFront == NULL)
		m_q->d_scnActRear = NULL;

	free(temp);
}

struct DaliSceneCfgNode *fn_newNodeScnCfg(struct_DaliSceneCfg *m_scnCfg,
		daliScnCfg_typedef *m_cfgFlag) {
	struct DaliSceneCfgNode *temp = (struct DaliSceneCfgNode *) malloc(
			sizeof(struct DaliSceneCfgNode));
	if (!temp) {
		return NULL;
	}
	memcpy(&temp->sDaliScnCfg, m_scnCfg, sizeof(struct_DaliSceneCfg));
	temp->d_ScFlag = *m_cfgFlag;
	temp->sDSCfgNext = NULL;
	return temp;
}

struct DaliSceneCfgQueue *fn_createScnCfgQ(struct DaliSceneCfgQueue *m_q) {
//	m_q = (struct DaliSceneCfgQueue *)malloc(sizeof(struct DaliSceneCfgQueue));
	m_q->d_scnCfgFront = m_q->d_scnCfgRear = NULL;
	return m_q;
}

bool fn_enQueueScnCfg(struct DaliSceneCfgQueue *m_q,
		struct_DaliSceneCfg *m_scnCfg, daliScnCfg_typedef m_cfgFlag) {
	struct DaliSceneCfgNode *temp = fn_newNodeScnCfg(m_scnCfg, &m_cfgFlag);

	if (!temp) {
		return false;
	}

	if (m_q->d_scnCfgRear == NULL) {
		m_q->d_scnCfgFront = m_q->d_scnCfgRear = temp;
		return true;
	}

	m_q->d_scnCfgRear->sDSCfgNext = temp;
	m_q->d_scnCfgRear = temp;

	return true;
}

void fn_deQueueScnCfg(struct DaliSceneCfgQueue *m_q) {
	// If queue is empty, return NULL.
	if (m_q->d_scnCfgRear == NULL)
		return;

	// Store previous dfront and move dfront one node ahead
	struct DaliSceneCfgNode *temp = m_q->d_scnCfgFront;

	m_q->d_scnCfgFront = m_q->d_scnCfgFront->sDSCfgNext;

	// If dfront becomes NULL, then change drear also as NULL
	if (m_q->d_scnCfgFront == NULL)
		m_q->d_scnCfgRear = NULL;

	free(temp);
}

struct SNEntityNode *fn_newNodeScnEntityCfg(struct_SNentity *m_scnEntity) {
	struct SNEntityNode *temp = (struct SNEntityNode *) malloc(
			sizeof(struct SNEntityNode));
	if (!temp) {
		return NULL;
	}
	memcpy(&temp->sSNConfig, m_scnEntity, sizeof(struct_SNentity));
	temp->sSNnext = NULL;
	return temp;
}

struct SNEntityQueue *fn_createScnEntityCfgQ(struct SNEntityQueue *m_q) {
//	m_q = (struct SNEntityQueue *)malloc(sizeof(struct SNEntityQueue));
	m_q->sFront = m_q->sRear = NULL;
	return m_q;
}

bool fn_enQueueScnEntityCfg(struct SNEntityQueue *m_q,
		struct_SNentity *m_scnEntity) {
	struct SNEntityNode *temp = fn_newNodeScnEntityCfg(m_scnEntity);

	if (!temp) {
		return false;
	}

	if (m_q->sFront == NULL) {
		m_q->sFront = m_q->sRear = temp;
		return true;
	}

	m_q->sRear->sSNnext = temp;
	m_q->sRear = temp;

	return true;
}

void fn_deQueueScnEntityCfg(struct SNEntityQueue *m_q) {
	// If queue is empty, return NULL.
	if (m_q->sRear == NULL)
		return;

	// Store previous dfront and move dfront one node ahead
	struct SNEntityNode *temp = m_q->sFront;

	m_q->sFront = m_q->sFront->sSNnext;

	// If dfront becomes NULL, then change drear also as NULL
	if (m_q->sFront == NULL)
		m_q->sRear = NULL;

	free(temp);
}

struct GP_SNActNode *fn_newNodeGrpScnAct(struct_gpSnAction *gpSnAct) {
	struct GP_SNActNode *temp = (struct GP_SNActNode *) malloc(
			sizeof(struct GP_SNActNode));
	if (!temp) {
		return NULL;
	}
	memcpy(&temp->sGpSnAct, gpSnAct, sizeof(struct_gpSnAction));
	temp->sGPSN_ActNext = NULL;
	return temp;
}

struct GP_SNActNodeQueue *fn_createGrpScnActQ(struct GP_SNActNodeQueue *m_q) {
//	m_q = (struct GP_SNActNodeQueue *)malloc(sizeof(struct GP_SNActNodeQueue));
	m_q->gpSnActFront = m_q->gpSnActRear = NULL;
	return m_q;
}

bool fn_enQueueGrpScnAct(struct GP_SNActNodeQueue *m_q,
		struct_gpSnAction *gpSnAct) {
	struct GP_SNActNode *temp = fn_newNodeGrpScnAct(gpSnAct);

	if (!temp) {
		return false;
	}

	if (m_q->gpSnActRear == NULL) {
		m_q->gpSnActFront = m_q->gpSnActRear = temp;
		return true;
	}

	m_q->gpSnActRear->sGPSN_ActNext = temp;
	m_q->gpSnActRear = temp;

	return true;
}

void fn_deQueueGrpScnAct(struct GP_SNActNodeQueue *m_q) {
	// If queue is empty, return NULL.
	if (m_q->gpSnActFront == NULL)
		return;

	// Store previous dfront and move dfront one node ahead
	struct GP_SNActNode *temp = m_q->gpSnActFront;

	m_q->gpSnActFront = m_q->gpSnActFront->sGPSN_ActNext;

	// If dfront becomes NULL, then change drear also as NULL
	if (m_q->gpSnActFront == NULL)
		m_q->gpSnActRear = NULL;

	free(temp);
}

void fn_identifyDT1() {
	static uint8_t m_statusDT1ID = STATE_ONE;
	static uint8_t m_id = 0;
	switch (m_statusDT1ID) {
	case STATE_ONE:
		if (sDT1IdentifyStat[m_id].d_ballastshId != 0) {
			m_statusDT1ID = STATE_TWO;
		} else {
			m_id++;
			if (m_id >= 10) {
				m_id = 0;
			}
		}
		break;
	case STATE_TWO:
		if (sDT1IdentifyStat[m_id].d_balLpCount != 0) {
			m_statusDT1ID = STATE_THREE;
		} else {
			m_statusDT1ID = STATE_FIVE;
		}
		break;
	case STATE_THREE:
		if (fn_IsmSecTimerElapsed(sDT1IdentifyStat[m_id].d_balTimeout, 500)) {
			m_statusDT1ID = STATE_FOUR;
		} else {
			m_id++;
			if (m_id >= 10) {
				m_id = 0;
			}
			m_statusDT1ID = STATE_ONE;
		}
		break;
	case STATE_FOUR:
		if (sDT1IdentifyStat[m_id].d_balLevel == 100) {
			if (ballastRecallMax(
					(((sDT1IdentifyStat[m_id].d_ballastshId >> 8) & 0x02)
							== 0x02) ? DALI_LOOP2 : DALI_LOOP1,
					((uint8_t) sDT1IdentifyStat[m_id].d_ballastshId) << 1 | 1,
					false)) {
				sDT1IdentifyStat[m_id].d_balLpCount -= 1;
				sDT1IdentifyStat[m_id].d_balLevel = 0;
				sDT1IdentifyStat[m_id].d_balTimeout = fn_GetmSecTimerStart();
				m_id++;
				if (m_id >= 10) {
					m_id = 0;
				}
				m_statusDT1ID = STATE_ONE;
			}
		} else {
			if (ballastRecallMin(
					(((sDT1IdentifyStat[m_id].d_ballastshId >> 8) & 0x02)
							== 0x02) ? DALI_LOOP2 : DALI_LOOP1,
					((uint8_t) sDT1IdentifyStat[m_id].d_ballastshId) << 1 | 1,
					false)) {
				sDT1IdentifyStat[m_id].d_balLpCount -= 1;
				sDT1IdentifyStat[m_id].d_balLevel = 100;
				sDT1IdentifyStat[m_id].d_balTimeout = fn_GetmSecTimerStart();
				m_id++;
				if (m_id >= 10) {
					m_id = 0;
				}
				m_statusDT1ID = STATE_ONE;
			}
		}
		break;
	case STATE_FIVE:
		if (ballastLastActLev(
				(((sDT1IdentifyStat[m_id].d_ballastshId >> 8) & 0x02) == 0x02) ?
						DALI_LOOP2 : DALI_LOOP1,
				((uint8_t) sDT1IdentifyStat[m_id].d_ballastshId) << 1 | 1,
				false)) {
			m_statusDT1ID = STATE_ONE;
			sDT1IdentifyStat[m_id].d_ballastshId = 0;
			m_id++;
			if (m_id >= 10) {
				m_id = 0;
			}
		}
		break;
	}

}

int8_t fn_isDT1Identify_free() {
	for (uint8_t m_in = 0; m_in < 10; m_in++) {
		if (sDT1IdentifyStat[m_in].d_ballastshId == 0) {
			return m_in;
		}
	}
	return -1;
}

int8_t fn_isFreeScnUIDavailable() {
	for (uint8_t m_in = 0; m_in < TOTAL_SCENES_CTRL; m_in++) {
		if (sScnUIDLP[m_in].m_uid == 0) {
			return m_in;
		}
	}
	return -1;
}

int8_t fn_getScnUIDIndex(uint16_t m_scnID) {
	for (uint8_t m_in = 0; m_in < TOTAL_SCENES_CTRL; m_in++) {
		if (sScnUIDLP[m_in].m_uid == m_scnID) {
			return m_in;
		}
	}
	return -1;
}

int8_t fn_sceneCfgD(bool m_add, uint16_t m_id, uint8_t m_shid,uint8_t m_intensity)
{

	int8_t m_index = fn_getScnUIDIndex(m_id);
	if (m_index != -1)
	{
		if (m_add)
		{
			sScnUIDLP[m_index].m_uid = m_id;
			sScnUIDLP[m_index].m_sceneID = m_index;
			sScnUIDLP[m_index].m_shid = (uint16_t) (m_shid);

			memset(&m_sDaliSceneCfg, '\0', sizeof(m_sDaliSceneCfg));
			m_sDaliSceneCfg.d_ballastshId =
					(uint16_t) (sScnUIDLP[m_index].m_shid);

			m_intensity =
					(m_intensity != 0) ?
							(uint8_t) ((253 * (1 + log10(m_intensity)) / 3) + 1) :
							0;

			m_sDaliSceneCfg.d_level = m_intensity;
			m_sDaliSceneCfg.d_scn = sScnUIDLP[m_index].m_sceneID;
			if (!fn_enQueueScnCfg(&sDaliSceneCfgQueue, &m_sDaliSceneCfg,
					(m_add == true) ? SCN_ADDITION : SCN_DELETION))
			{
				printf("Memory Allocation fault  fn_sceneCfg 588 \r\n");
			}
		}
		else
		{
//				sScnUIDLP[m_index].m_uid = m_id;
//				sScnUIDLP[m_index].m_sceneID = m_index;
//				sScnUIDLP[m_index].m_shid = (uint16_t) (m_shid);

			memset(&m_sDaliSceneCfg, '\0', sizeof(m_sDaliSceneCfg));
			m_sDaliSceneCfg.d_ballastshId =
					(uint16_t) (sScnUIDLP[m_index].m_shid);

			m_intensity =
					(m_intensity != 0) ?
							(uint8_t) ((253 * (1 + log10(m_intensity)) / 3) + 1) : 0;

			m_sDaliSceneCfg.d_level = m_intensity;
			m_sDaliSceneCfg.d_scn = sScnUIDLP[m_index].m_sceneID;
			if (!fn_enQueueScnCfg(&sDaliSceneCfgQueue, &m_sDaliSceneCfg,
					(m_add == true) ? SCN_ADDITION : SCN_DELETION))
			{
				printf("Memory Allocation fault  fn_sceneCfg 603 \r\n");
			}
			sScnUIDLP[m_index].m_uid = 0;
			sScnUIDLP[m_index].m_sceneID = 0;
			sScnUIDLP[m_index].m_shid = 0;
		}

		gecko_cmd_flash_ps_save(SCENE_PS_KEY, sizeof(sScnUIDLP),
				(uint8_t *) &sScnUIDLP);
	}
	else
	{
		if (m_add)
		{

			m_index = fn_isFreeScnUIDavailable();
			if (m_index != -1)
			{

				sScnUIDLP[m_index].m_uid = m_id;
				sScnUIDLP[m_index].m_sceneID = m_index;
				sScnUIDLP[m_index].m_shid = (uint16_t) (m_shid);

				memset(&m_sDaliSceneCfg, '\0', sizeof(m_sDaliSceneCfg));
				m_sDaliSceneCfg.d_ballastshId =
						(uint16_t) (sScnUIDLP[m_index].m_shid);

				m_intensity =
						(m_intensity != 0) ?
								(uint8_t) ((253 * (1 + log10(m_intensity)) / 3) + 1) : 0;

				m_sDaliSceneCfg.d_level = m_intensity;
				m_sDaliSceneCfg.d_scn = sScnUIDLP[m_index].m_sceneID;
				if (!fn_enQueueScnCfg(&sDaliSceneCfgQueue, &m_sDaliSceneCfg,
						SCN_ADDITION))
				{
					printf("Memory Allocation fault  fn_sceneCfg 615\r\n");
				}
				gecko_cmd_flash_ps_save(SCENE_PS_KEY, sizeof(sScnUIDLP),
						(uint8_t *) &sScnUIDLP);
			}

		}
	}
	return m_index;
}

int8_t fn_gotoScene(uint16_t m_id, uint8_t m_statFrom) {
	int8_t m_index = fn_getScnUIDIndex(m_id);
	if (m_index != -1) {
		memset(&m_sDaliSceneAct, '\0', sizeof(m_sDaliSceneAct));

		m_sDaliSceneAct.d_ballastshId = (uint16_t) (sScnUIDLP[m_index].m_shid);
		m_sDaliSceneAct.d_scn = sScnUIDLP[m_index].m_sceneID;
		m_sDaliSceneAct.d_scnFrom = m_statFrom;
		if (!fn_enQueueScnAct(&sDaliSceneActQueue, &m_sDaliSceneAct)) {
#ifdef DEVICE_DEBUG
			printf("Memory Allocation fault Func: %s Line: %d\r\n", __func__, __LINE__);
#endif
		}

	}
	return m_index;
}

/***************************************************************************************************/


void fn_enQDAct(struct_DaliAction *m_cfg){

	if(sDaliActQueueStat.mTail < TOTAL_DACTION_MAX-1){
		if(sDaliActQueueStat.mHead == -1){
			sDaliActQueueStat.mHead = sDaliActQueueStat.mTail = 0;
			memset(&sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead],
					0, sizeof(&sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead]));
			memcpy(&sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mHead], m_cfg, sizeof(struct_DaliAction));
			return;
		}
		sDaliActQueueStat.mTail++;
		memset(&sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mTail],
				0, sizeof(&sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mTail]));
		memcpy(&sDaliActQueueStat.sDaliActSQ[sDaliActQueueStat.mTail], m_cfg, sizeof(struct_DaliAction));
	}
	return;
}

void fn_deQDAct(){
	if(sDaliActQueueStat.mHead == sDaliActQueueStat.mTail){
		sDaliActQueueStat.mHead = sDaliActQueueStat.mTail = -1;
	}
	else{
		sDaliActQueueStat.mHead++;
	}
}

bool fn_isDActQEmpty(){
	if(sDaliActQueueStat.mHead == -1){
		return true;
	}
	return false;
}




/***************************************************************************************************/

