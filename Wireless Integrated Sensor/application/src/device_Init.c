/****************************************************************************************************************/
#include "application/inc/device_Init.h"
#include "application/inc/intervalTimer.h"
#include "hardware/kit/common/drivers/retargetswo.h"
#include "application/inc/gsLink/GSLink_uart.h"
#include "application/inc/gsLink/gsLinkPktQ.h"
#include "application/inc/gsLink/GS_Link.h"
#include <application/inc/S1_pir.h>
#include "application/inc/switch.h"
#include "application/inc/dali/daliCommand.h"
#include "application/inc/dali/daliApplication.h"
/****************************************************************************************************************/
// bluetooth stack heap
#define MAX_CONNECTIONS 2

uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS) + BTMESH_HEAP_SIZE + 1760];

// Bluetooth advertisement set configuration
//
// At minimum the following is required:
// * One advertisement set for Bluetooth LE stack (handle number 0)
// * One advertisement set for Mesh data (handle number 1)
// * One advertisement set for Mesh unprovisioned beacons (handle number 2)
// * One advertisement set for Mesh unprovisioned URI (handle number 3)
// * N advertisement sets for Mesh GATT service advertisements
// (one for each network key, handle numbers 4 .. N+3)
//
#define MAX_ADVERTISERS (4 + MESH_CFG_MAX_NETKEYS)

static gecko_bluetooth_ll_priorities linklayer_priorities = GECKO_BLUETOOTH_PRIORITIES_DEFAULT;

// bluetooth stack configuration
extern const struct bg_gattdb_def bg_gattdb_data;


const gecko_configuration_t config =
{
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.max_advertisers = MAX_ADVERTISERS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap) - BTMESH_HEAP_SIZE,
#if defined(FEATURE_LFXO)
  .bluetooth.sleep_clock_accuracy = 100, // ppm
#elif defined(PLFRCO_PRESENT)
  .bluetooth.sleep_clock_accuracy = 500, // ppm
#endif
  .bluetooth.linklayer_priorities = &linklayer_priorities,
  .gattdb = &bg_gattdb_data,
  .btmesh_heap_size = BTMESH_HEAP_SIZE,
  .pa.config_enable = 1, // Set this to be a valid PA config
#if defined(FEATURE_PA_INPUT_FROM_VBAT)
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
#else
  .pa.input = GECKO_RADIO_PA_INPUT_DCDC,
#endif // defined(FEATURE_PA_INPUT_FROM_VBAT)
  .max_timers = 16,
  .rf.flags = APP_RF_CONFIG_ANTENNA,   // Enable antenna configuration.
  .rf.antenna = APP_RF_ANTENNA,        // Select antenna path!
  .ota.flags = 0,                                      /* Check flag options from UG136 */
  .ota.device_name_len = 3,                            /* Length of the device name in OTA DFU mode */
  .ota.device_name_ptr = "OTA",                        /* Device name in OTA DFU mode */
  .ota.antenna_defined = APP_RF_CONFIG_ANTENNA,
  .ota.antenna = APP_RF_ANTENNA
};
/****************************************************************************************************************/
void fn_led_init(void)
{
  // configure LED0 and LED1 as outputs
  GPIO_PinModeSet(APP_LED1, gpioModePushPull, 0);
  GPIO_PinModeSet(APP_LED2, gpioModePushPull, 0);
  GPIO_PinOutClear(APP_LED1);
  GPIO_PinOutClear(APP_LED2);
  return ;
}
/****************************************************************************************************************/
void initBoard(void)
{
	CMU_ClockEnable(cmuClock_GPIO, true);									// Enable GPIO clock source
	RETARGET_SwoInit();														//initializing for dbg_prints ; just comment out this line for disabling the dbg_prints


#ifndef DALI_SPACE
	fn_initI2C();
#endif
	if(mux_control_select == 1)					//Analog
	{
		MAN_PRINT("Analog I2C init done \r\n");
		initI2C1_Analog();
	}
	else if(mux_control_select == 0)			//DALI
	{
		gpioDALISetup();
		fn_initDaliDriver();
		fn_usecTimerinit();
		fn_DALItimerInit();

		fn_createDCQ(&sDaliCfgQueue);
	//	fn_createACQ(&sDaliActQueue);
		fn_createACQ(&sDaliActWaitQueue);
		fn_createScnCfgQ(&sDaliSceneCfgQueue);
		fn_createScnActQ(&sDaliSceneActQueue);
	}
	fn_led_init();
	fn_switchInit();
	fn_msecTimerinit();
#ifdef TRIAC_FEATURE
	GPIO_PinModeSet(GSLink_RTS_DIR_PORT,GSLink_RTS_DIR_PIN,gpioModePushPull,1);
	GPIO_PinModeSet(GSLink_RTS_PORT,GSLink_RTS_PIN,gpioModePushPull,0);
	fn_changeRTSdirection(RTS_OUTPUT);
#endif

#ifdef DALI_SPACE
	GPIO_PinModeSet(gpioPortC, 5, gpioModePushPull, 0);
	GPIO_PortOutSet(gpioPortC, 5);
#endif

	fn_mux_init();

	return;
}
/*******************************************************************************************************************************/
void fn_deviceInit(void)
{
	initMcu();
	// Initialize board
	initBoard();

	uint16_t led_delay = fn_GetSecTimerStart();
	LED_TOGGLING(APP_LED1);
	LED_TOGGLING(APP_LED2);
	while(!(fn_IsSecTimerElapsed(led_delay, 2)));
	TURN_OFF_LED(APP_LED1);
	TURN_OFF_LED(APP_LED2);

	// Minimize advertisement latency by allowing the advertiser to always
	// interrupt the scanner.
	linklayer_priorities.scan_max = linklayer_priorities.adv_min + 1;

	// Bt stack initialization
	gecko_stack_init(&config);
	gecko_bgapi_class_dfu_init();
	gecko_bgapi_class_system_init();
	gecko_bgapi_class_le_gap_init();
	gecko_bgapi_class_le_connection_init();
	gecko_bgapi_class_gatt_init();
	gecko_bgapi_class_gatt_server_init();
	gecko_bgapi_class_hardware_init();
	gecko_bgapi_class_flash_init();
	gecko_bgapi_class_test_init();
	gecko_bgapi_class_sm_init();

	// Bt Mesh stack initialization
	gecko_bgapi_class_mesh_node_init();
	gecko_bgapi_class_mesh_prov_init();
	gecko_bgapi_class_mesh_proxy_init();
	gecko_bgapi_class_mesh_proxy_server_init();
	gecko_bgapi_class_mesh_proxy_client_init();
	gecko_bgapi_class_mesh_generic_client_init();
	gecko_bgapi_class_mesh_generic_server_init();
	gecko_bgapi_class_mesh_vendor_model_init();
	gecko_bgapi_class_mesh_health_client_init();
	gecko_bgapi_class_mesh_health_server_init();
	gecko_bgapi_class_mesh_test_init();
	gecko_bgapi_class_mesh_lpn_init();
	gecko_bgapi_class_mesh_friend_init();
//	gecko_bgapi_class_mesh_sensor_client_init();
//	gecko_bgapi_class_mesh_sensor_server_init();
//	gecko_bgapi_class_mesh_sensor_setup_server_init();
//	gecko_bgapi_class_mesh_lc_client_init();
//	gecko_bgapi_class_mesh_lc_server_init();
//	gecko_bgapi_class_mesh_lc_setup_server_init();
//	gecko_bgapi_class_mesh_scene_client_init();
//	gecko_bgapi_class_mesh_scene_server_init();
//	gecko_bgapi_class_mesh_scene_setup_server_init();

	// Initialize the random number generator which is needed for proper radio work.
//	gecko_cmd_system_get_random_data(16);

	// Coex initialization
	gecko_initCoexHAL();

	return ;
}
/****************************************************************************************************************/
