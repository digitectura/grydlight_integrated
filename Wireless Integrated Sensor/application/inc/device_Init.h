/****************************************************************************************************************/
#ifndef APPLICATION_DEVICE_INIT_H_
#define APPLICATION_DEVICE_INIT_H_
/****************************************************************************************************************/
#include "main.h"
/****************************************************************************************************************/
// Select antenna path on EFR32xG2x devices:
//   - RF2G2_IO1: 0
//   - RF2G2_IO2: 1
#define APP_RF_ANTENNA   						1
#define APP_RF_CONFIG_ANTENNA   				0
#define GECKO_RF_ANTENNA        				APP_RF_ANTENNA
#define CFG_OUTPUT_POWER						20

/*................................gecko stack init Macros..................................*/
#define MY_VENDOR_ID							0x02ff
#define PRIMARY_ELEMENT							0
#define MY_MODEL_ID 							0xABCD
#define PUBLISH									1

/*.....................................Time Counters......................................*/
#define TIMER_CLK_FREQ							((uint32_t)32768)
#define MILLISECONDS(ms)						(ms*TIMER_CLK_FREQ)/1000
#define SECONDS(s)								s*TIMER_CLK_FREQ

/*.......................................Timer Modes.......................................*/
#define REPEATING_TIMER							0
#define ONESHOT_TIMER							1

/*.....................................Soft Timer Events...................................*/
#define FIRST_AGG_HANDSHAKE_PKT					1
#define BROADCAST_I_AM_PKT						2
#define LED1_TIMER_ID							3
#define LED2_TIMER_ID							4
#define TIMER_ID_RESTART						5
#define BLEQ_HANDLER							6
#define RESET_INDICATOR_START					7
#define RESET_INDICATION_STOP					8
#define PIR_INTRPT_ENABLE						9
#define FCTORY_RESET_ID							10
#define TIMER_ID_LIGHT_MAP_PKT					11
#define SWITCH_DEBOUNCE_TIMER					12
#define CONFIG_CHANGE_INDICATOR_START			13
#define CONFIG_CHANGE_INDICATOR_STOP			14
#define SWITCH_PRESS_INDICATION					15
#define LOOP_MAP_INDICATION						16
#define DALI_SAVE_TIMER							17
#define WL_LIGHT_EXTERNAL_CMD_TIMER				18
#define WL_LIGHT_CONTROL_2_TIMER_ID				19

#define EMERGENCY_LIGHT_TIMER_ID				20
#define EMERGENCY_LIGHT_TIMER_ID_2				21

#define TIMER_ID_CONFIGURE_AREA					22

/*.....................................PIR app Macros.....................................*/
#define DEFAULT_SNSR_MASKING_TIMEOUT			1800		//SECS ==> 30 mins
#define PIR_STABILIZE_TIME						15
#define WL_LIGHT_CONTROL_2_TIMEOUT				3600

#define DEFAULT_PIR_UNOCCUPANCY_TIME_1			120	//SEC==>2mins
#define DEFAULT_PIR_UNOCCUPANCY_TIME_2			300	//SEC==>5mins
#define DEFAULT_PIR_UNOCCUPANCY_TIME_3			480	//SEC==>8mins
#define DEFAULT_PIR_UNOCCUPANCY_TIME_4			600	//SEC==>10mins
#define DEFAULT_PIR_UNOCCUPANCY_TIME_5			900	//SEC==>15mins

#define DEFAULT_PIR_RETRANSMIT_TIME_FIVEMIN             300
#define DEFAULT_PIR_RETRANSMIT_TIME				1800	//SEC==>30mins
#define DEFAULT_PIR_WAIT_AND_WATCH_TIME			80	//mSEC
#define PIR_POWER_ON_STATUS_INTERVAL			60 //120 sec -- 2mins

/*.....................................ALS app Macros.....................................*/
#define DEFAULT_ALS_FREQ_OF_MEASURMT_1			15	//SEC==>15 secs
#define DEFAULT_ALS_FREQ_OF_MEASURMT_2			30	//SEC==>30 secs
#define DEFAULT_ALS_FREQ_OF_MEASURMT_3			60	//SEC==>60 secs
#define DEFAULT_ALS_FREQ_OF_MEASURMT_4			120	//SEC==>2mins
#define DEFAULT_ALS_FREQ_OF_MEASURMT_5			300	//SEC==>5mins

#define DEFAULT_ALS_LUX_THRESHOLD				50  //LUX
#define DEFAULT_ALS_CALBFACTOR					100	//multiplication factor

/*.....................................TH app Macros.....................................*/
#define DEFAULT_TH_RETRANSMIT_TIME				1800//sec ==> 30mins
#define DEFAULT_TH_FREQ_OF_MEASURMT				900	//SEC ==> 15mins
#define DEFAULT_TH_THRESHOLD					0  //th THRSHLD		0.5 degCelcius
#define DEFAULT_HUM_THRESHOLD					500	//HUM THRSHLD		5% humidity

/*..................................LED gpio fn_Calls.....................................*/
#define TURN_ON_LED								GPIO_PinOutSet
#define TURN_OFF_LED							GPIO_PinOutClear
#define LED_TOGGLING							GPIO_PinOutToggle

#define NETWORK_LEVEL							1
#define FACTORY_LEVEL							0
/*..................................deviceType bits......................................*/
#define PIR_SENSOR								0x01
#define ALS_SENSOR								0x02
#define TH_SENSOR								0x04
#define MORPHOUS_SESNSOR_DEVICE_ID				0x0100
#define TRIAC_SENSOR_DEVICE_ID					0X0400

/*................................PS_Keys for UserData...................................*/
#define PS_SNSR_MINCFG							0x4000
#define PS_SNSR_CFG_BASE						0x4010
#define PS_SNSR_CFG_BASE_1						0x4010
#define PS_SNSR_CFG_BASE_2						0x4011
#define PS_SNSR_CFG_BASE_3						0x4012
#define PS_SNSR_CFG_BASE_4						0x4013
#define PS_SNSR_CFG_BASE_5						0x4014


#define GRP_ADDR_SUBSCRIBED_PS_KEY				0x4016
#define SCENE_PS_KEY							0x4017
#define ANALOGSTAT_PS_KEY						0x4018
#define ANALOG_FADE_TIME_PS						0x4019
#define TRIAC_STATUS_KEY						0x4020
#define ALS_CALIB_KEY							0x4021
#define DALI_RETENTION_STORAGE_KEY				0x4022
#define MUX										0x4023

#define PS_SNSR_AREA_CONFIG						0x4024
#define PS_SNSR_AREA_CONFIG_PARAMETERS			0x4025

#define MAX_SUB_ADDR							16

/*................................Application Macros....................................*/
#define CONFIG_VER								1
#define FW_VER									12

#define SNSR_CFG_SAVE_INTERVAL					30		//sec
#define IDENTIFY_INTERVAL						1		//sec
#define	IDENTIFY_COUNT							10
#define FACTORY_RESET_INTERVAL					60
#define NETWORK_RESET_INTERVAL					20		//sec

#define MAGNET_SWITCH_NETWORK_RESET_TIMEOUT		40
#define MAGNET_SWITCH_FCATORY_RESET_TIMEOUT		35
#define OTA_TIMEOUT								30
#define MAGNET_SWITCH_SYSTEM_RESET_TIMEOUT		25
#define ALS_CONFIG_CHANGE						20
#define PIR_CONFIG_CHANGE						15
#define LOOP_MAP_CHANGE_INTERVAL				7
#define LOOP_MAP_ITRT							4
#define BUFFER_TIME								3

#define DEFAULT_GAIN_FACTOR						70
#define DEFAULT_REQ_LUX							350

#define BOTH_LOOP								0xFF
#define LOOP1									0x01
#define LOOP2									0x02
#define NO_LOOP									0x00

#define OTA_DFU_RESET							2
#define UART_DFU_RESET							1
#define NORMAL_RESET							0

#define ONE_LOOP_MAPPED							1
#define TWO_LOOP_MAPPED							2
#define PIR_MAPNG								0x00
#define ALS_MAPNG								0x80
#define ANALOG_CH								0x01
#define RELAY_CH								0x04

#define CFG_1									ZERO
#define CFG_2									ONE
#define CFG_3									TWO
#define CFG_4									THREE
#define CFG_5									FOUR
/*.......................................................................................*/
#define DEBUG_MODE
#ifdef DEBUG_MODE
#define DBG_PRINT(...) printf(__VA_ARGS__)
#else
#define DBG_PRINT(...)
#endif
/****************************************************************************************************************/
enum
{
	PIN_RESET,
	PIN_SET
};
enum
{
	PIR_PARAM = 0x01,
	ALS_PARAM = 0x02,
	TH_PARAM  = 0x04
};
enum
{
	ASSIGN_CONFIG = 0x80,
	FETCH_CONFIG  = 0x00,
};
enum
{
	PARAM1 = 0x01,
	PARAM2 = 0x02,
	PARAM3 = 0x04,
	PARAM4 = 0x08,
	PARAM5 = 0x10,
	PARAM6 = 0x20,
	PARAM7 = 0x40
};

enum
{
	STATE_ONE,
	STATE_TWO,
	STATE_THREE,
	STATE_FOUR,
	STATE_FIVE,
	STATE_SIX,
	STATE_SEVEN,
	STATE_EIGHT,
	STATE_NINE,
	STATE_TEN,
	STATE_ELEVEN,
	STATE_TWELVE,
	STATE_THIRTEEN,
	STATE_FOURTEEN,
};

typedef enum
{
	PIR_STAT = 1,
	ALS_STAT = 2,
	SCHEDULE_STAT = 4,
	SCENE_STAT = 8,
	USER_INT = 16,
	SWITCH_INT = 32,
	TOGGLE_SW = 64,
	TH_STAT = 128,
} statusEn_typedef;

typedef enum
{
	DO_NOT_REPORT, RECEIVED_ON_CLIENT, RECEIVED_ON_SERVER,
} recvEnd_typedef;

/****************************************************************************************************************/



/****************************************************************************************************************/
void fn_deviceInit(void);
void initBoard(void);
void fn_switchInit(void);
/****************************************************************************************************************/
#endif /* APPLICATION_DEVICE_INIT_H_ */
/****************************************************************************************************************/
