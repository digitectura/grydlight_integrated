/****************************************************************************************************************/
#ifndef APPLICATION_MAIN_H_
#define APPLICATION_MAIN_H_
/****************************************************************************************************************/
/* Board headers */
#include "init_mcu.h"
#include "application/inc/device_Init.h"
#include "ble-configuration.h"
#include "board_features.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"
#include <gecko_configuration.h>
#include <mesh_sizes.h>

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"
#include "em_rtcc.h"
#include <em_gpio.h>

/* Coex header */
#include "coexistence-ble.h"

/* Device initialization header */
#include "hal-config.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

#include "platform\emdrv\gpiointerrupt\inc\gpiointerrupt.h"
/****************************************************************************************************************/
#include "stdbool.h"
#include "stdlib.h"
#include "stdio.h"
#include <stdio.h>
#include "math.h"
/****************************************************************************************************************/
extern bool criticalOpnLock;
extern uint8_t mux_control_select;
extern int16_t Curr_mLevel;
/****************************************************************************************************************/
#endif /* APPLICATION_MAIN_H_ */
