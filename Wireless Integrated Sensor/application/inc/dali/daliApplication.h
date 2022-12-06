/*
 * daliApplication.h
 *
 *  Created on: 23-Aug-2019
 *      Author: Rakshit
 */

#ifndef APP_DALI_DALIAPPLICATION_H_
#define APP_DALI_DALIAPPLICATION_H_

#include <application/inc/dali/daliCommand.h>


typedef enum{
	DALI_COMMISSION_INCPLT,
	DALI_COMMISSION_CPLT
}dalisensorCommission_cplt_typedef;


struct DaliCfgNode {
	struct DaliCfgNode *sDNext;
	uint8_t cfgChangeFlag;
	bool cfgFlashUpdateFlag;
	struct_DaliBallastCfg sDaliCfg;
};

typedef struct{
	uint8_t  d_balLevel;
	uint8_t d_balLpCount;
	uint16_t d_ballastshId;			// 0b XXXX X | 1/0: Group | 1/0: Loop | X: (not used) | XXXX XXXX : ShID
	uint16_t d_balTimeout;
}struct_dt1Identify;


struct DaliActNode {
	struct DaliActNode *sDNext;
	struct_DaliAction sDaliAct;
};

struct DaliGrpNode{
	struct DaliGrpNode *sDGNext;
	daliGrpCfg_typedef d_Flg;
	struct_DaliGroupCfg sDaliGrp;
};

struct GPEntityNode{
	struct GPEntityNode *sGPEntityNext;
	struct_GPentity sGPEntityND;
};

struct SNEntityNode{
	struct SNEntityNode *sSNnext;
	struct_SNentity sSNConfig;
};

struct DaliSceneCfgNode{
	struct DaliSceneCfgNode *sDSCfgNext;
	struct_DaliSceneCfg sDaliScnCfg;
	daliScnCfg_typedef d_ScFlag;
};

struct DaliSceneActNode{
	struct DaliSceneActNode *sDSActNext;
	struct_DaliSceneAction sDaliScnAct;
};

struct GP_SNActNode{
	struct GP_SNActNode *sGPSN_ActNext;
	struct_gpSnAction sGpSnAct;
};

struct DaliCfgQueue{
	struct DaliCfgNode *dfront, *drear;
};

struct DaliActQueue{
	struct DaliActNode *dfront, *drear;
};

struct GPEntityQueue{
	struct GPEntityNode *gpFront, *gpRear;
};

struct DaliGrpQueue{
	struct DaliGrpNode *dfront, *drear;
};

struct SNEntityQueue{
	struct SNEntityNode *sFront, *sRear;
};

struct DaliSceneCfgQueue{
	struct DaliSceneCfgNode *d_scnCfgFront, *d_scnCfgRear;
};

struct DaliSceneActQueue{
	struct DaliSceneActNode *d_scnActFront, *d_scnActRear;
};

struct GP_SNActNodeQueue{
	struct GP_SNActNode *gpSnActFront, *gpSnActRear;
};

typedef struct{
	uint8_t bShID;
	struct DaliCfgNode *dnodePtr;
}struct_daliLookUp;

typedef struct{
	uint8_t m_sceneID;
	uint8_t m_shid;
	uint16_t m_uid;
}struct_scnUIDLookup;

struct DaliActQueueStat{
	int8_t mHead;
	int8_t mTail;
	struct_DaliAction sDaliActSQ[TOTAL_DACTION_MAX];
};

extern struct_daliLookUp sDaliLookUp[NUM_OF_DALI_LOOPS][TOTAL_DALI_BALLASTS];
extern struct_dt1Identify sDT1IdentifyStat[10];
extern struct_scnUIDLookup sScnUIDLP[TOTAL_SCENES_CTRL];

extern struct DaliCfgQueue sDaliCfgQueue;
extern struct DaliActQueue sDaliActQueue;
extern struct DaliGrpQueue sDaliGrpQueue;
extern struct DaliActQueue sDaliActWaitQueue;
extern struct GPEntityQueue sGPEntityQueue;
extern struct SNEntityQueue sSNEntityQueue;
extern struct DaliSceneCfgQueue sDaliSceneCfgQueue;
extern struct DaliSceneActQueue sDaliSceneActQueue;
extern struct GP_SNActNodeQueue sGroupSceneActionQueue;

extern struct DaliActQueueStat sDaliActQueueStat;

extern dalisensorCommission_cplt_typedef		g_daliCommissionCplt;

struct DaliCfgNode *fn_newNodeDC(struct_DaliBallastCfg *, uint8_t*);
struct DaliCfgQueue *fn_createDCQ(struct DaliCfgQueue *);
bool fn_enQueueDC(struct DaliCfgQueue* , struct_DaliBallastCfg *, uint8_t);
void fn_deQueueDC(struct DaliCfgQueue *);

struct DaliActNode *fn_newNodeAC(struct_DaliAction *);
struct DaliActQueue *fn_createACQ(struct DaliActQueue *);
bool fn_enQueueAC(struct DaliActQueue* , struct_DaliAction *);
void fn_deQueueAC(struct DaliActQueue *);

struct DaliGrpNode *fn_newNodeGP(struct_DaliGroupCfg *, daliGrpCfg_typedef *);
struct DaliGrpQueue *fn_createGPQ(struct DaliGrpQueue *);
bool fn_enQueueGP(struct DaliGrpQueue* , struct_DaliGroupCfg *, daliGrpCfg_typedef );
void fn_deQueueGP(struct DaliGrpQueue *);

struct GPEntityNode *fn_newNodeGPEn(struct_GPentity *);
struct GPEntityQueue *fn_createGPEnQ(struct GPEntityQueue *);
bool fn_enQueueGPEn(struct GPEntityQueue* , struct_GPentity *);
void fn_deQueueGPEn(struct GPEntityQueue *);

struct DaliSceneCfgNode *fn_newNodeScnCfg(struct_DaliSceneCfg *, daliScnCfg_typedef *);
struct DaliSceneCfgQueue *fn_createScnCfgQ(struct DaliSceneCfgQueue *);
bool fn_enQueueScnCfg(struct DaliSceneCfgQueue* , struct_DaliSceneCfg *, daliScnCfg_typedef);
void fn_deQueueScnCfg(struct DaliSceneCfgQueue *);

struct DaliSceneActNode *fn_newNodeScnAct(struct_DaliSceneAction *);
struct DaliSceneActQueue *fn_createScnActQ(struct DaliSceneActQueue *);
bool fn_enQueueScnAct(struct DaliSceneActQueue* , struct_DaliSceneAction *);
void fn_deQueueScnAct(struct DaliSceneActQueue *);

struct SNEntityNode *fn_newNodeScnEntityCfg(struct_SNentity *);
struct SNEntityQueue *fn_createScnEntityCfgQ(struct SNEntityQueue *);
bool fn_enQueueScnEntityCfg(struct SNEntityQueue* , struct_SNentity *);
void fn_deQueueScnEntityCfg(struct SNEntityQueue *);

struct GP_SNActNode *fn_newNodeGrpScnAct(struct_gpSnAction *);
struct GP_SNActNodeQueue *fn_createGrpScnActQ(struct GP_SNActNodeQueue *);
bool fn_enQueueGrpScnAct(struct GP_SNActNodeQueue* , struct_gpSnAction *);
void fn_deQueueGrpScnAct(struct GP_SNActNodeQueue *);

void fn_enQDAct(struct_DaliAction *m_cfg);
void fn_deQDAct();
bool fn_isDActQEmpty();

void fn_identifyDT1();
int8_t fn_isDT1Identify_free();
int8_t fn_getScnUIDIndex(uint16_t m_scnID);
int8_t fn_isFreeScnUIDavailable();
int8_t fn_sceneCfgD(bool m_add, uint16_t m_id, uint8_t m_shid, uint8_t m_intensity);
int8_t fn_gotoScene(uint16_t m_id, uint8_t m_statFrom);

#endif /* APP_DALI_DALIAPPLICATION_H_ */
