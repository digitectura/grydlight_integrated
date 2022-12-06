#ifndef __COMMAND_HANDLER_H
#define __COMMAND_HANDLER_H

#include "S1_pir.h"
#include "S2_als.h"
#include "S3_th.h"
#include "bg_types.h"
#define MAX_PAYLOAD		40
#define MAX_PAY			40
#define I_AM_PKT_LEN	8

#define MAX_NUM_PKTS	50

#define LINEAR_M		21.65
#define LINEAR_C		-105



// packet formation and sending
//enum init i1 1

enum
{
	MODE1 = 1,
	MODE2,
	MODE3
};

typedef enum
{
	CMD_WHOIS,
//	PAIRING_RESPONSE_LEN = 0x25,
	PAIRING_RESPONSE_LEN = 24,
	CMD_IAM  = 0x24,
	CMD_DATAREQ_AP  = 0x01,
	CMD_DATAREQ_BS = 0x01,
	CMD_DATAREQ_PIR = 0x01,
	CMD_DATAREQ_US = 0x01,
	CMD_DATAREQ_ALS = 0x01,
	CMD_DATAREQ_BL = 0x01,
	CMD_DATAREQ_TH = 0x01,
	CMD_DATARES_AP = 0x06,
	CMD_DATARES_BS  = 0x02,
	CMD_DATARES_PIR = 0x02,
	CMD_DATARES_US = 0x02,
	CMD_DATARES_ALS = 0x02,
	CMD_DATARES_BL = 0x02,
	CMD_DATARES_TH = 0x05,
	CMD_STATCH_BS  = 0x02,
	CMD_STATCH_PIR  = 0x02,
	CMD_STATCH_US  = 0x02,
	CMD_STATCH_ALS = 0x03,
	CMD_STATCH_BL  = 0x02,
	CMD_STATCH_TH  = 0x05,
	CMD_cmdSHID  = 0x14,
	CMD_COMMISSION  = 0x00,
	CMD_FACTORY_RESET = 0x01,
	CMD_SYSTEM_RESET = 0x01,
	CMD_SNESOR_CONFIG = 0x24,
	CMD_PGA_CONFIG,
	CMD_SYS_IDENTIFY = 0x01 ,
	CMD_PIR_RETRANS_EN_DIS = 0x01,
	CMD_SENSOR_MODE = 0x03,
	CMND_SHID_IR_BROADCAST	= 0x0D
}DataCmd_length;
enum
{
	UNREPORTED_SNSR_REQ,
	FORCED_REQ
};
typedef enum{
	FT_WHOIS=0,
    FT_IAM = 1,
    FT_DATAREQ = 2,
    FT_DATARES = 3,
    FT_STATUSCHANGE =4,
    FT_COMMISSIONING =5,
    FT_COMMAND =6,
    FT_ACK=7,
    FT_NACK=8,
	FT_WL_LIGHT_COMMANDS=9,
    FT_DATA_STREAM=10,
    FT_EVENT=11,
    FT_PING=12,
    FT_DALI_CUSTOM=13,
    FT_IAM_SWITCH=14,
	FT_ALERT	= 0x11,
    FT_CUSTOM_CMD = 63
}FrameType_EN;

typedef enum{
	ALL_PROPERTIES = 0x00,
	BATTERY_STATUS = 0x01,
	PIR = 0x02,
	ULTRASOUND = 0x03,
	ALS = 0x04,
	BAY_LIGHTING = 0x05,
	TEMP_HUMIDITY= 0x06,
	CO2= 0x07,
    tVOC = 0x08,
    DUST_SENSOR = 0x09 ,
    OZONE = 0x0a,
    PRESSURE = 0x0b,
    MIC = 0x0c,
	SENSOR_PKT_STATS = 0x0d,

	DALI_PERSISTENT_STORAGE_TIMEOUT		= 0x10,
	DALI_CONFIG							= 0x11,

	CURR_DALI_PERCENT					= 0x12,					//to retrieve current and previous Dali percent
	PREV_DALI_PERCENT					= 0x13,
	WL_LIGHT_EXT_CMD_TIMER				= 0x14,

	MUX_CONTROL							= 0x15,

	AREA_DETAILS						= 0x20,
	CONFIGURE_AREA						= 0x21,
	DELETE_SENSOR_IN_AREA				= 0x22,

	PAIRING_REQ 						= 0x30,
	PAIRING_ACCPT 						= 0x31,
	PAIRING_CMPLT 						= 0x32,
	OTA_REQ								= 0x39,
	START_APP_OTA 						= 0x3A,
	STOP_APP_OTA						= 0x3B,

	GOTO_OTA							= 0x3E,
	TIME_SYNC 							= 0x40,

	EVENTS_PKT							= 0x41,

	RESET_CAUSE							= 0x50,		//	Events
	SHID_UNASSIGNED						= 0x55,
	ENV_POLL 							= 0x71,

	SWITCH_CONFIG						= 0x80,
	BUTTON_CONFIG						= 0x81,
	SWITCH_SCENE_CONFIG					= 0x82,

	SHID_ASSIGNED						= 0xAA,
	BIND_TO_AGGREGATOR					= 0xAB,

	LIGHT_STATUS						= 0xB0,
	LIGHT_ON_OFF_COLOR					= 0xB1,
	LIGHT_IDENTIFY						= 0xB2,
	LIGHT_TOOGLE						= 0xB3,
	RELATIVE_LIGHT_CONTROL				= 0xB4,
	SET_SCENES							= 0xB5,
	SCENE_CONFIG						= 0xB6,
	SCENE_DELETE						= 0xB7,
	LIGHT_SENSOR_MAP					= 0xB8,
	LIGHT_SCHEDULE_CONFIG				= 0xB9,
	SCENE_SCHEDULE_CONFIG				= 0xBA,
	SCHEDULE_DELETE						= 0xBB,
	DIM_FADE_TIME						= 0xBC,
	LIGHT_CONTROL_2						= 0xBD,

	COMMISSIONED						= 0xC0,
	LED_CONTROL							= 0xC1,
	DEF_CONFIG 							= 0xD0,
	CUSTM_SENSOR_CONFIG 				= 0xD1,
	STOP_TXN 							= 0xD2,
	SYSTEM_RESET_CMD 					= 0xD3,
	FACTORY_RESET						= 0xD4,
	PGA_CONFIG 							= 0xD5,
	SENSOR_INDENTIFY 					= 0xD6,
	PIR_RETRANSMISSION_EN 				= 0xD7,
	PIR_RETRANSMISSION_DIS 				= 0xD8,
	SENSOR_MODE 						= 0xD9,
	CONFIG_SCHEDULE						= 0xDA,
	CONFIG_ENABLE						= 0xDB,
	SHID_REQUEST						= 0xDC,
	CONFIG_DELETE						= 0xDD,
	SHID_ASSIGNEMT 						= 0xE0,

	ASSIGN_GROUP						= 0xE1,
	DE_ASSIGN_GROUP						= 0xE2,
	SENSOR_LED_OPN						= 0xE3,

	MAG_SENSOR_UPT						= 0xE5,

	ALS_CALIB_CONF1						= 0xE6,
	ALS_CALIB_CONF2						= 0xE7,
	ALS_CALIB_GAIN						= 0xE8,
	ALS_CALIB_REQLUX					= 0xE9,

	UUID_REQ							= 0xEA,
#ifdef ALS_TEST
	DALI_M_LEVEL						= 0xEB,
	MAX_LUX_ON_TABLE					= 0xEC,

	DISABLE_DEBUG_PRINTS				= 0xEE,
#endif
	OTA_DATA							= 0xEF,
	ULTRA_SOUND_CONFIGPARAMS			= 0xF0,
	ULTRA_SOUND_TEST_EN 				= 0xF1,
	ULTRA_SOUND_TEST_DIS	 			= 0xF2,
	US_REG_WRITE		 				= 0xF3,
	PGA_REG_READ						= 0xF4,
	US_DIS_MEAS_CONSTRAINT				= 0xF5,
	CMD_ACK								= 0xF6,
	NETWORK_RESET						= 0xF7 ,
	TEST_CMD							= 0xFF
}Data_Cmd_t;

uint8_t g_m_level;

typedef struct
{
	uint16_t data_pkt_num;
	uint8_t *payload;
} __attribute__((__packed__)) ota_data_frame_t;

typedef struct
{
	uint8_t		cmd_id;
	uint16_t	target_device_type;
	uint16_t	expected_pkts;
	uint8_t		expected_ttl; // in seconds
}__attribute__((__packed__)) cmd_start_ota;

typedef struct sensorpayload_t
{
	uint8_t datacmd;
	uint8_t  data[MAX_PAYLOAD];
} __attribute__((__packed__)) sensorpayload_t;

/*typedef struct{
	uint8_t currDaliPercent;
	uint8_t PrevDaliPercent;
}struct_DaliPercent;

extern struct_DaliPercent sDaliPercent;*/

extern uint16_t MaxLuxOnTable;

void send_packet(Data_Cmd_t data_cmd_type, uint16_t destAddr , uint8_t isReq);

#endif
