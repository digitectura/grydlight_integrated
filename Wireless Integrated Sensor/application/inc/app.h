/*******************************************************************************************************************************/
#ifndef APPLICATION_INC_APP_H_
#define APPLICATION_INC_APP_H_
/*******************************************************************************************************************************/
#include "main.h"
/*******************************************************************************************************************************/

#define ALS_TEST

//#define SELF_PROVISIONING
//#define AREA

#define DEFAULT_DEVICE_TYPE		263
#define DEFAULT_DEST_ADDR		0xFFFF
#define DEFAULT_SHID			0x00
#define MAX_SCENES		12

#define ADD_CNTRL_TYPE(cntrlType)			( (cntrlType & ANALOG_CH) ? 70 : ( (sCtrlCfg.cntrl_type & RELAY_CH) ? 80 : 0 ) )

#define MAX_SENSOR_IN_GRP		16

extern uint8_t debugPrints;


typedef enum {
	FADE_TIME,
	FADE_RATE,
	PRESET_MAX,
	PRESET_MIN,
	SYS_FAIL_LEVEL,
	POW_FAIL_LEVEL,
	DALI_MLEVEL_THRESHOLD
}enum_DaliLightConfig;

/*******************************************************************************************************************************/
enum
{
	APP_STATE_UNPROVISIONED,
	APP_STATE_PROVISIONED
};
enum
{
	HAND_SHAKE_WITH_CTRL	,
	REPORT_TO_AGG
};
/*******************************************************************************************************************************/
typedef struct
{
	uint16_t unoccupancyTimer_s;
	uint16_t wait_watch_Time_ms;
	uint16_t retransmission_timeout;
	uint16_t cfg_id;
}__attribute__((__packed__)) PIR_config_t;
/*******************************************************************************************************************************/
typedef struct
{
	uint16_t freq_THmeasure;
	uint16_t tempThreshold;
	uint16_t humidityThreshold;
	uint16_t retransmission_timeout;
	uint16_t cfg_id;
}__attribute__((__packed__)) th_cfg_t;
/*******************************************************************************************************************************/
typedef struct
{
	uint16_t freq_LUXmeasure_s;
	uint16_t luxThreshold;
	uint16_t calibration_factor;
	uint16_t retransmission_timeout;
	uint16_t cfg_id;
}__attribute__((__packed__)) als_cfg_t;
/*******************************************************************************************************************************/
typedef struct
{
	PIR_config_t pir_cfg;
	als_cfg_t als_cfg;
	th_cfg_t th_cfg;
	uint16_t WL_LIGHT_RcvdExternalCmd_timer;
	uint16_t WL_LIGHT_CONTROL_2_timer;
	uint8_t emergency_light;
}__attribute__((__packed__))snsrCfg_t;
/*******************************************************************************************************************************/
typedef struct
{
/*	uint8_t no_avlbl_cfg;
	uint8_t available_sch;
	uint8_t active_cfg;
	uint8_t current_cfg;
	uint8_t scheduled_cfg;
	uint32_t UTC;
	uint32_t next_sched_time;*/
	uint16_t pirRetransmitTimerStart;
	uint8_t switchState;
	uint8_t switchPIR_interrupt;
	uint8_t switchPIR_state;
	uint8_t identify;
	uint8_t broadCastComplete;
	uint8_t PIR_stabilized;
	uint8_t global_daliStatus;							//added for retention of state to store prev state value
	uint8_t WL_LIGHT_RcvdExternalCmd_flag;				//on receiving Light control commands from UI or switch set this flag
	uint8_t WL_LIGHT_CONTROL_2_flag;
	uint16_t thRetransmitTimerStart;
	uint16_t alsRetransmitTimerStart;
	bool ForcedReq;
}__attribute__((__packed__))snsrAppData_t;
/*******************************************************************************************************************************/
typedef struct
{
	uint8_t loopMap;
	uint16_t reqLux;
	uint8_t gain;
}__attribute__((__packed__))lghtMpng_t;
/*******************************************************************************************************************************/
typedef struct
{
	uint16_t snsrID;
	uint16_t device_type;
	uint8_t configversion;
	uint8_t firmwareversion;
	uint16_t dest_addr;
	bool aggBinded;
	bool cfgAvailable;
	lghtMpng_t lghtMpng;
} __attribute__((__packed__)) snsrMinCfg_t;
/*******************************************************************************************************************************/
typedef struct
{
	uint16_t UUID;
	uint16_t cntrl_type;
	uint8_t  ctrlFWver;
} __attribute__((__packed__)) snsrmap_CntrlCredntials_t;
/*******************************************************************************************************************************/
typedef struct struct_IAM_pkt_TAG
{
	uint16_t cntrl_deviceType;
	uint16_t snsr_deviceType;
	uint8_t cntrl_fwVer;
	uint8_t snsr_fwVer;
	uint16_t parent_ID;
}__attribute__((__packed__))struct_IAM_pkt_t;
/*******************************************************************************************************************************/
typedef struct struct_triacCfg_TAG
{
	uint8_t triacStatus;
}__attribute__((__packed__))triacCfg_t;
/*******************************************************************************************************************************/
typedef struct
{
	uint8_t pir_State;
	uint16_t als_LUXvalue;
	uint16_t temp;
	uint16_t humidity;
}__attribute__((__packed__))snsrStatus_t;
/*******************************************************************************************************************************/
typedef struct
{
	uint8_t snsrCfgID;
	uint8_t snsrType;
	uint8_t cfgParam;
	uint8_t cfgValue_LSB;
	uint8_t cfgValue_MSB;
}cfgRcvPkt_t;
/*******************************************************************************************************************************/

typedef struct sceneMapping_TAG
{
	uint8_t LightID;
	uint8_t intensity;
}__attribute__((__packed__))struct_sceneMappping;

/*******************************************************************************************************************************/

typedef struct sceneConfig_TAG
{
	uint16_t sceneID;							//2
	struct_sceneMappping sScene_mapping;     //2
}__attribute__((__packed__))struct_sceneConfig;

typedef struct
{
	uint16_t m_gain;
	uint16_t req_lux;
	float m;
	float c;
}__attribute__((__packed__))struct_alsLinearCalib;


#ifdef SELF_PROVISIONING
typedef struct
{
	uint32_t index_no;
	uint8_t netkeys[16];
	uint8_t appkeys[16];
}__attribute__((__packed__))struct_App_Net_Key_t;

typedef struct
{
	uint32_t self_provisioned_flag;
	struct_App_Net_Key_t s_App_Net_Key[16];
}__attribute__((__packed__))struct_Keys_Msc_t;


extern struct_Keys_Msc_t s_keys_msc;
#endif

typedef struct
{
	uint16_t SHID;
	uint8_t Occupancy;
}__attribute__((__packed__))struct_Configure_Area_t;

typedef struct
{
	uint8_t TotalArea_Sensors_count;				//Total number of sensors in area
	uint16_t Area_Group_ID;
	uint8_t SensorIsPartOfArea;
}__attribute__((__packed__))struct_Config_Area_Parameters_t;

/*******************************************************************************************************************************/
extern struct_Configure_Area_t sConfigArea[MAX_SENSOR_IN_GRP];
extern struct_Config_Area_Parameters_t sConfigAreaParameters;
extern snsrmap_CntrlCredntials_t sCtrlCfg;
extern snsrAppData_t snsrAppData;
extern snsrMinCfg_t	 snsrMinCfg;
extern snsrCfg_t	 snsrCfg;
extern snsrStatus_t	snsrCurrStatus;
extern snsrStatus_t snsrPrevStatus;
extern struct_sceneConfig	sSceneCfg[MAX_SCENES];
extern struct_alsLinearCalib sAlsCalibValue;
extern triacCfg_t triacCfg;
/*******************************************************************************************************************************/
void fn_handleVendorModel_Rcv			(uint32_t evt_id,struct gecko_cmd_packet *evt);
void handle_gecko_event					(uint32_t evt_id, struct gecko_cmd_packet *evt);
void fn_process_HardwareSoftTimer_Evnt	(struct gecko_cmd_packet *evt);
void fn_process_extrnlSgnl_Evnt			(struct gecko_cmd_packet *evt);
void fn_initiate_factory_reset			(uint8_t resetLevel);
void fn_update_snsrCfg					(void);
void fn_deleteSubAddr					(void);

void fn_sensorIdentify					(void);
void fn_SnsrPrcss						(void);
void fn_ctrlHandshake					(void);
void fn_subscribedAddr					(void);
void fn_initiate_Iam_Pkt				(void);
void fn_switchOnTriac					(void);
void fn_switchOffTriac					(void);
void fn_toggleTriac						(void);
void fn_pirBasedTriacCntrl				(void);
void fn_sendTriacStatus					(uint8_t attribute_Reason);
void fn_assign_snsrConfig				(cfgRcvPkt_t *cfg);
void fn_queueLightStat					(uint8_t m_shid, uint8_t m_level, uint8_t m_statusEn);

void fn_loopPIR_cfg(void);
void fn_loopALS_cfg(void);

void fn_send_Cmd_onGSLink				(struct gecko_msg_mesh_vendor_model_receive_evt_t *v_data);
void fn_handle_Vendor_FT_wl_LightCmds	(struct gecko_msg_mesh_vendor_model_receive_evt_t *v_data);
void fn_snsRestore						(void);
void fn_mux_init(void);
void fn_AreaControl(void);
/*******************************************************************************************************************************/
#endif /* APPLICATION_INC_APP_H_ */
