/*
 * daliCommand.h
 *
 *  Created on: Aug 7, 2019
 *      Author: Rakshit
 */

#ifndef APP_DALI_DALICOMMAND_H_
#define APP_DALI_DALICOMMAND_H_

#include <application/inc/dali/dali.h>

extern uint8_t g_IsDALIAddressingCplt[NUM_OF_DALI_LOOPS];
extern uint8_t g_Daliselection[NUM_OF_DALI_LOOPS];
extern uint16_t g_ballastCount[NUM_OF_DALI_LOOPS];

extern uint8_t g_daliLightStat;

#define FR1 	357.796
#define FR2 	253
#define FR3		178.898
#define FR4 	126.5
#define FR5		89.449
#define FR6		63.25
#define FR7		44.725
#define FR8		31.625
#define FR9		22.362
#define FR10	15.813
#define FR11	11.181
#define FR12	7.906
#define FR13	5.591
#define FR14	3.953
#define FR15	2.795

#define DALI_M_LEVEL_THRESHOLD	10
typedef enum {
	DALI_FREE, DALI_CONFIG_PENDING
} enum_daliConfigStatus;

typedef enum {
	DALI_PROCESS_FREE, DALI_PROCESSING
} enum_daliProcessStatus;

typedef enum {
	DALI_RETENTION_INIT,
	CHECK,
	POWER_ON_LEVEL_SET,
	POWER_FAIL_LEVEL_SET,
}enum_DaliRetention;

typedef enum {
	ABSOLUTE_DIMMING = 1,
	RELATIVE_DIMMING_INC,
	RELATIVE_DIMMING_DEC,
	RELATIVE_TRIMMING,
	TC_DIMMING,
	RELATIVE_TC_DIMMING_INC,
	RELATIVE_TC_DIMMING_DEC,
	TOTAL_DIMMING_TYPES,
} Dimming_typedef;

enum
{
	STEP_UP_ON, STEP_DOWN_OFF, DIM_QUERY, DIM_SET_FADERATE, DIM_SET, DIM_WAIT, DIM_CHECK,    // Srikanth 01/06/22
};

typedef struct {
//	uint16_t lumId;	// 0b XXXX X | 1/0: Group | 1/0: Loop | X: (not used) | XXXX XXXX : ShID
	bool isOn;
	uint8_t sHCLLevel;
	uint16_t sHCLTempVal;
}__attribute__((__packed__)) struct_sHCLTempearture;

extern enum_daliConfigStatus edaliConfigStatus;
extern enum_daliProcessStatus edaliProcessStatus;
extern uint8_t g_randomizeDali;

void fn_dali_ind(uint8_t, uint8_t, uint8_t, recvEnd_typedef);

/*		========= ballastOn ==========
 *
 * @brief 	This function is called to switch on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballastOn(daliLoop_typedef, uint8_t, bool);

/*		========= ballastOff ==========
 *
 * @brief 	This function is called to switch off the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballastOff(daliLoop_typedef, uint8_t, bool);

bool ballastFadeUp(daliLoop_typedef, uint8_t, bool);

bool ballastFadeDown(daliLoop_typedef, uint8_t, bool);

/*		========= ballastBlink ==========
 *
 * @brief 	This function is called to toggle the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
uint8_t ballastBlink(daliLoop_typedef, uint8_t, bool, uint16_t);

/*		========= ballastStepUp ==========
 *
 * @brief 	This function is called to step up the intensity of the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballastStepUp(daliLoop_typedef, uint8_t, bool);

/*		========= ballastStepDown ==========
 *
 * @brief 	This function is called to step down the intensity of the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballastStepDown(daliLoop_typedef, uint8_t, bool);

/*		========= ballastRecallMax ==========
 *
 * @brief 	This function is called to step down the intensity of the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballastRecallMax(daliLoop_typedef, uint8_t, bool);

/*		========= ballastRecallMin ==========
 *
 * @brief 	This function is called to step down the intensity of the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballastRecallMin(daliLoop_typedef, uint8_t, bool);

/*		========= ballastStepDOff ==========
 *
 * @brief 	This function is called to step down the intensity and switch off
 * 		the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballastStepDOff(daliLoop_typedef, uint8_t, bool);

/*		========= ballastStepUOn ==========
 *
 * @brief 	This function is called to step down the intensity and switch off
 * 		the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballastStepUOn(daliLoop_typedef, uint8_t, bool);

bool ballastLastActLev(daliLoop_typedef, uint8_t, bool);
/*		========= ballast_gotoScene ==========
 *
 * @brief 	This function is called to go to then scene set on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @params (m_sceneNumber) The scene number 0-15
 *
 * @retval true or false
 *
 */
bool ballast_gotoScene(daliLoop_typedef, uint8_t, bool, uint8_t);

/*		========= ballast_reset ==========
 *
 * @brief 	This function is called to reset the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballast_reset(daliLoop_typedef, uint8_t);

bool ballast_identify(daliLoop_typedef, uint8_t);

/*		========= ballast_storeActLevel ==========
 *
 * @brief 	This function is called to reset the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballast_storeActLevel(daliLoop_typedef, uint8_t, bool);

/*		========= ballast_storeMaxLevel ==========
 *
 * @brief 	This function is called to set max level on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballast_storeMaxLevel(daliLoop_typedef, uint8_t, uint8_t, bool);

/*		========= ballast_storeMinLevel ==========
 *
 * @brief 	This function is called to set min level on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballast_storeMinLevel(daliLoop_typedef, uint8_t, uint8_t, bool);

/*		========= ballast_storeFailLevel ==========
 *
 * @brief 	This function is called to set system fail level on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballast_storeFailLevel(daliLoop_typedef, uint8_t, uint8_t, bool);

/*		========= ballast_storePowOnLevel ==========
 *
 * @brief 	This function is called to set power on level on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballast_storePowOnLevel(daliLoop_typedef, uint8_t, bool);

/*		========= ballast_storeFadeTime ==========
 *
 * @brief 	This function is called to set Fade time on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballast_storeFadeTime(daliLoop_typedef, uint8_t, daliFadeTime_typedef,
		bool);

/*		========= ballast_storeFadeRate ==========
 *
 * @brief 	This function is called to set Fade rate on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @retval true or false
 *
 */
bool ballast_storeFadeRate(daliLoop_typedef, uint8_t, daliFadeRate_typedef,
		bool);

/*		========= ballast_storeScene ==========
 *
 * @brief 	This function is called to store the scene set on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @params (m_sceneNumber) The scene number 0-15
 *
 * @retval true or false
 *
 */
bool ballast_storeScene(daliLoop_typedef, uint8_t, bool, uint8_t, uint8_t);

/*		========= ballast_removeFromScene ==========
 *
 * @brief 	This function is called to remove the luminaries on the DALI bus from the scene.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @params (m_sceneNumber) The scene number 0-15
 *
 * @retval true or false
 *
 */
bool ballast_removeFromScene(daliLoop_typedef, uint8_t, bool, uint8_t);

/*		========= ballast_addToGroup ==========
 *
 * @brief 	This function is called to add the luminaries on the DALI bus to the group.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @params (m_groupNumber) The group number 0-15
 *
 * @retval true or false
 *
 */
bool ballast_addToGroup(daliLoop_typedef, uint8_t, bool, uint8_t);

/*		========= ballast_removeFromGroup ==========
 *
 * @brief 	This function is called to remove the luminaries on the DALI bus from the Group.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @params (m_GroupNumber) The Group number 0-15
 *
 * @retval true or false
 *
 */
bool ballast_removeFromGroup(daliLoop_typedef, uint8_t, bool, uint8_t);

/*		========= ballast_storeShAdd ==========
 *
 * @brief 	This function is called to store the ShAdd set on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 *
 * @retval true or false
 *
 */
bool ballast_storeShAdd(daliLoop_typedef, uint8_t, bool);

/*		========= ballast_removeFromGroup ==========
 *
 * @brief 	This function is called to remove the luminaries on the DALI bus from the Group.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @params (m_GroupNumber) The Group number 0-15
 *
 * @retval true or false
 *
 */
bool ballast_removeFromGroup(daliLoop_typedef, uint8_t, bool, uint8_t);

/*		========= ballast_storeShAdd ==========
 *
 * @brief 	This function is called to store the ShAdd set on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 *
 * @retval true or false
 *
 */
bool ballast_storeShAdd(daliLoop_typedef, uint8_t, bool);

/*		========= ballast_selectDimmingCurve ==========
 *
 *	DT6 -DT8 Only
 * @brief 	This function is called to store the dimming curve  on the luminaries on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @params (m_isLinear) Linear dimming curve selection
 * 1 - Linear
 * 0 - Logarithmic (default)
 *
 * @retval true or false
 *
 */
bool ballast_selectDimmingCurve(daliLoop_typedef, uint8_t, bool, uint8_t);

/*		========= ballastQuery ==========
 *
 * @brief 	This function is called to execute the query commands 144-255 on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) true or false (if the commands must be sent repeatedly)
 *
 * @params (m_response) The response received
 *
 * @retval true or false
 *
 */

bool ballastQuery(daliLoop_typedef, uint8_t, uint8_t, uint8_t*);

/*		========= ballast_addToGroup ==========
 *
 * @brief 	This function is called to add the luminaries on the DALI bus to the group.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_address) The short address assigned after ballast commissioning
 *
 * @params (m_repeat) The number of times the command must be executed
 *
 * @params (m_groupNumber) The group number 0-15
 *
 * @retval true or false
 *
 */

/*		========= ballastSplCommand ==========
 *
 * @brief 	This function is called to execute the special commands on the DALI bus.
 * @note   Must be called in a loop until returned true
 *
 * @params (m_command1) Command 1
 *
 * @params (m_command2) Command 2
 *
 * @params (m_repeat) true or false (if the commands must be sent repeatedly)
 *
 * @params (m_response) The response received
 *
 * @retval true or false
 *
 */
bool ballastSplCommand(daliLoop_typedef, uint8_t, uint8_t, bool, bool, uint8_t*);

bool ballastStoreGearFeatures(daliLoop_typedef m_daliLoop, uint8_t m_address,
		uint8_t m_command);

bool ballastAddressSearch(daliLoop_typedef, uint8_t*, uint8_t*);

bool ballastInputAddress(daliLoop_typedef, uint8_t, uint8_t, uint8_t, uint8_t);

bool ballastQueryRandomAddress(daliLoop_typedef, uint8_t, uint32_t*);

void fn_daliProcess(void);

void fn_updateDaliConfig(void);

bool fn_queryDT();

uint8_t fn_writeDaliCfg_flashWhole(uint8_t isAddressingCplt);

void fn_lightStatusQuery(void);

void fn_GroupSceneAction();

bool fn_activateColourType(daliLoop_typedef m_daliLoop, uint8_t m_address);

bool fn_Set16bitValue(daliLoop_typedef m_loop, uint8_t m_val, uint16_t *m_retvl);

bool fn_SetSpecific16bitValue(daliLoop_typedef m_loop, uint16_t m_val);

bool fn_Get16bitValue(daliLoop_typedef m_loop, uint8_t m_address,
		uint16_t *m_retvl);

bool fn_setTempColourTemp_Tc(uint16_t m_temp, uint8_t m_address,
		daliLoop_typedef m_daliLoop);

bool fn_findValidTcValue(daliLoop_typedef m_daliLoop, uint8_t m_address);
//typedef struct{
//
//}struct_DaliBallastID;

typedef enum {
	DALI_CASE_1 = 1, DALI_CASE_2, DALI_CASE_3, DALI_CASE_4
} dali_typedef;

typedef enum {
	DALI_FADE_TIME,
	DALI_FADE_RATE,
	DALI_PRESET_MAX,
	DALI_PRESET_MIN,
	DALI_SYS_FAIL_LEV,
	DALI_POW_ON_LEV,
	DALI_TOTAL_CONFIG
} dali_configPar_typedef;

typedef enum {
	GRP_NONE, GRP_ADDITION, GRP_DELETION
} daliGrpCfg_typedef;

typedef enum {
	SCN_NONE, SCN_ADDITION, SCN_DELETION,
} daliScnCfg_typedef;

typedef enum {
	SCENE_0,
	SCENE_1,
	SCENE_2,
	SCENE_3,
	SCENE_4,
	SCENE_5,
	SCENE_6,
	SCENE_7,
	SCENE_8,
	SCENE_9,
	SCENE_10,
	SCENE_11,
	SCENE_12,
	SCENE_13,
	SCENE_14,
	SCENE_15,
	TOTAL_SCENES,
} daliScenes_typedef;

typedef enum {
	GRP_0,
	GRP_1,
	GRP_2,
	GRP_3,
	GRP_4,
	GRP_5,
	GRP_6,
	GRP_7,
	GRP_8,
	GRP_9,
	GRP_10,
	GRP_11,
	GRP_12,
	GRP_13,
	GRP_14,
	GRP_15,
	TOTAL_GROUPS,
} daliGroups_typedef;

typedef enum {
	BALLAST_IDLE, BALLAST_BUSY
} enum_daliBallastState;

typedef struct {
	uint32_t ballastRandomID;
	uint8_t ballastShortID;
	uint8_t ballastIntensity;
	bool ballastStatusChange;
	bool ballastIdentify;
	recvEnd_typedef loopStatus_reportEnd;
} struct_DaliBallastControl;

typedef struct {
	bool grpStatusChange[TOTAL_DALI_GROUPS];
	bool grpIdentify[TOTAL_DALI_GROUPS];
	uint8_t grpIntensity[TOTAL_DALI_GROUPS];
} struct_DaliGroupStatusChange;

typedef struct {
	daliScenes_typedef daliSceneNumber[TOTAL_SCENES];
	uint8_t daliSceneLevel[TOTAL_SCENES];
} struct_sceneLevel;

typedef struct {
	uint16_t totalNumberofGroups;
	daliGroups_typedef DaliGroups[TOTAL_GROUPS];
} struct_groupDetails;

typedef struct {
	uint8_t d_groupAdditionFlag;			// 0: none, 1: addition, 2: deletion
	bool d_groupConfig_StatChange[TOTAL_DALI_BALLASTS];
	daliGroups_typedef d_groupInf[TOTAL_GROUPS];
	int16_t d_ModificationIndex[TOTAL_DALI_BALLASTS];
} struct_daligroupStatus;

typedef struct {
	uint8_t fadeTime;
	uint8_t fadeRate;
	uint8_t presetMax;
	uint8_t presetMin;
	uint8_t sysFailLevel;
	uint8_t powFailLevel;
//	struct_sceneLevel sSceneLevel;
//	struct_groupDetails sGroupDet;
} struct_daliLightConfig;

//typedef struct{
//	uint16_t ballastCount;
////	struct_DaliBallastID sDaliBallastID[64];
//	struct_DaliBallastControl sDaliBallastCtrl[TOTAL_DALI_BALLASTS];
//	struct_daliLightConfig sLightConfig[TOTAL_DALI_BALLASTS];
//}struct_DaliBallastConfig;

typedef struct {
	uint16_t ballastShortID;
	uint32_t ballastRandomID;
	daliLoop_typedef ballastloop;
	uint32_t areaID;
	uint8_t isDT8;
	uint16_t defaultKVTc;
	struct_daliLightConfig sLightConfig;
}__attribute__((__packed__)) struct_DaliBallastCfg;

typedef struct {
	uint32_t balRandomID;
	uint8_t shortID;
} struct_DaliCommissionInfo;

typedef struct {
	uint8_t d_ballastshID;			// 0b|1/0: Loop| XXXXXXX : ShID |
	daliGroups_typedef d_grp;
} struct_DaliGroupCfg;

typedef struct {
	uint16_t d_ballastshId;	// 0b XXXX X | 1/0: Group | 1/0: Loop | X: (not used) | XXXX XXXX : ShID
	uint8_t d_level;
	daliScenes_typedef d_scn;
} struct_DaliSceneCfg;

typedef struct {
	uint16_t d_ballastshId;	// 0b XXXX X | 1/0: Group | 1/0: Loop | X: (not used) | XXXX XXXX : ShID
	daliScenes_typedef d_scn;
	statusEn_typedef d_scnFrom;
	uint32_t m_dataByte;
} struct_DaliSceneAction;

typedef struct {
	uint16_t d_add;
	uint8_t d_Intensity;
	uint8_t d_report;
	uint8_t d_colourType;
	uint16_t d_Tc;
	uint8_t d_statFrom;
	uint32_t d_dataByte;
} struct_DaliAction;

typedef struct {
	uint16_t daliShID;
	uint8_t isDT8;	//Device type			// 1: DT8, 2: DT6, 3: DT1
	uint8_t colourType;					// 1: Tc
	uint16_t defaultKVTc;
	uint16_t minWarm;
	uint16_t maxCool;
	uint32_t daliRandomId;
	uint32_t areaID;
	uint32_t sectorNumber;
	uint32_t sectorOffset;
	uint8_t m_reserved[4];
}__attribute__((__packed__)) struct_DaliFlashLookUp;

typedef struct {
	struct_gpEntityLoc sSNEntity;
	uint16_t scnLevel;
} struct_snEntityLoc;

typedef struct {
	char grpID[8];
	struct_snEntityLoc GpSnEntity;
	statusEn_typedef scnFrom;
	uint32_t scnUqID;
	uint32_t m_dataByte; //procID
} struct_gpSnAction;

typedef struct {
	char grpId[8];
	uint16_t totElts;
//    uint8_t areaID;
	struct_gpEntityLoc sGPEntityLoc[TOTAL_BALLASTS_INGP];
}__attribute__((__packed__)) struct_GPentity;

typedef struct {
	bool dim_set;
	uint16_t Dim_shID;
	uint8_t Intensity;
	Dimming_typedef Dim_type;
}__attribute__((__packed__)) struct_Dim;					// srikanth 01-06-22

typedef struct {
	char grpIdLU[8];
	uint8_t isDT8;			// 1: DT8, 2: DT6, 3: DT1
	uint8_t colourType;
	uint32_t areaID;
	uint16_t defaultKV;
	uint16_t minWarm;
	uint16_t maxCool;
	uint32_t sectorNumber_GP;
	uint32_t sectorOffset_GP;
	uint8_t m_reserved[12];
}__attribute__((__packed__)) struct_gpLookup;

typedef struct {
	char scnID[8];
//	uint8_t areaID;
	uint16_t totScns;
	struct_snEntityLoc sSNEntityLoc[TOTAL_BALLASTS_INGP];
}__attribute__((__packed__)) struct_SNentity;

typedef struct {
	char scnIDLU[8];
//	uint8_t areaID;
	uint32_t sectorNumber_SN;
	uint32_t sectorOffset_SN;
}__attribute__((__packed__)) struct_SNLookup;

typedef struct {

	uint8_t balShID;
	uint8_t balPirStatus;
	uint8_t balLevel;		// 0-100
	uint8_t balPMax;
	uint8_t balPMin;
	enum_daliBallastState balStatus;
	uint16_t balAlsStatus;
}__attribute__((__packed__)) struct_DALIBallastStatus;

typedef struct {
	uint8_t totalBallastsL1;
	uint8_t totalBallastsL2;
	uint8_t totalLevelAvg;
	struct_DALIBallastStatus sDaliBallastStatusL1[TOTAL_DALI_BALLASTS];
	struct_sHCLTempearture sDaliHCLTempL1[TOTAL_DALI_BALLASTS];
	struct_DALIBallastStatus sDaliBallastStatusL2[TOTAL_DALI_BALLASTS];
	struct_sHCLTempearture sDaliHCLTempL2[TOTAL_DALI_BALLASTS];
}__attribute__((__packed__)) struct_DALIStatus;

typedef struct {
	bool gpPIRStatus;
	uint16_t gpAlsStatus;
	uint16_t gpId;
	uint16_t gpLevel;
	struct_sHCLTempearture TcStatus;
}__attribute__((__packed__)) struct_GPStatus;

typedef struct {
	uint8_t g_RetentionFlag;
	uint8_t m_intensity;
	uint16_t DaliQueryWaitTimer;
	uint8_t Dali_RetentionStroageTimeout;
	uint8_t m_LevelThreshold;										//Act on light only if m_level >= m_LevelThreshold
}struct_DaliRetention;

extern struct_DaliRetention g_sDaliRetention;

extern struct_GPentity g_sGpEntity;
extern struct_SNentity g_sSNEntity;
extern struct_gpSnAction g_sGpSnAction_obj;
//extern struct_gpLookup sGpLookUp[TOTAL_ENTITIES_GP];
//extern struct_SNLookup sSNLookUp[TOTAL_SCENES_CTRL];
//extern struct_DaliFlashLookUp sDaliFlashLookUp[NUM_OF_DALI_LOOPS][TOTAL_DALI_BALLASTS];
//extern struct_DALIStatus sDALIStatus;
extern struct_DaliSceneAction m_sDaliSceneAct;
//extern struct_GPStatus m_sGpStatus[TOTAL_ENTITIES_GP];
extern struct_DaliAction g_sDaliAction;
extern struct_Dim s_DimEntity;                                  // Srikanth 01-6-22
//extern struct_DaliCommissionInfo sDaliComInfo[2][65];
//dali_typedef daliCase;

//int8_t fn_isBallastAvailable_asScene(char *, struct_SNentity *);
daliGroups_typedef fn_extractGroup(char *m_grp);
daliScenes_typedef fn_extractDaliScene(char *m_scn);

bool fn_DALIDimming(Dimming_typedef m_dimType, uint32_t m_intensity,
		uint8_t m_loopID, uint32_t shID, uint8_t gpIndex,
		statusEn_typedef m_statFrom, uint8_t isDT8, uint32_t m_procID,
		uint8_t m_dapc);
bool fn_queryBallastInfo();
uint16_t fn_kelvinToMirek(uint16_t m_kelvin);
uint16_t fn_mirekToKelvin(uint16_t m_mirek);
void fn_initDaliCommands();
void fn_flushDaliCfg();
void fn_daliMode1_Level(uint8_t shID, uint8_t m_intensity,
		statusEn_typedef m_statFrom);
void fn_enqueueIdentify();
void gpioDALISetup(void);

void fn_ProcessDaliRetention(void);

void fn_dali_dimming(void); 				// srikanth 01-6-22
bool fn_daliDimSet(Dimming_typedef, uint8_t, uint8_t, uint8_t, statusEn_typedef); 	// srikanth 01-6-22

#endif /* APP_DALI_DALICOMMAND_H_ */
