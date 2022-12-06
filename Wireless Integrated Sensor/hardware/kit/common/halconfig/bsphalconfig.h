#ifndef BSPHALCONFIG_H
#define BSPHALCONFIG_H

#include "hal-config.h"

#if !defined(RAIL_PA_2P4_CONFIG)
// HAL Config 2.4 GHz PA configuration enabled
#define RAIL_PA_2P4_CONFIG                                               \
  {                                                                      \
    RAIL_TX_POWER_MODE_2P4_HP,    /* Power Amplifier mode */             \
    BSP_PA_VOLTAGE,               /* Power Amplifier vPA Voltage mode */ \
    HAL_PA_RAMP,                  /* Desired ramp time in us */          \
  }
#endif

#if !defined(RAIL_PA_SUBGIG_CONFIG)
// HAL Config sub-GHz PA configuration enabled
#define RAIL_PA_SUBGIG_CONFIG                                            \
  {                                                                      \
    RAIL_TX_POWER_MODE_SUBGIG,    /* Power Amplifier mode */             \
    BSP_PA_VOLTAGE,               /* Power Amplifier vPA Voltage mode */ \
    HAL_PA_RAMP,                  /* Desired ramp time in us */          \
  }
#endif

#if defined(HAL_PA_POWER) && !defined(RAIL_PA_DEFAULT_POWER)
#define RAIL_PA_DEFAULT_POWER         HAL_PA_POWER
#endif

#if defined(HAL_PA_CURVE_HEADER) && !defined(RAIL_PA_CURVES)
#define RAIL_PA_CURVES                HAL_PA_CURVE_HEADER
#endif

#define BSP_BCP_VERSION 2
#include "bsp_bcp.h"

#endif // BSPHALCONFIG_H
