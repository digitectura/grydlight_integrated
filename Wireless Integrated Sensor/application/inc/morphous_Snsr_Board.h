/***************************************************************************//**
 * @file
 * @brief hal-config-board.h
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef HAL_CONFIG_BOARD_H
#define HAL_CONFIG_BOARD_H

#include "em_device.h"
#include "hal-config-types.h"
#include "gryd_features.h"
/******************************************************************************/
#define BSP_CLK_LFXO_PRESENT                          (0)
#define BSP_CLK_HFXO_PRESENT                          (1)
#define BSP_CLK_LFXO_INIT                              CMU_LFXOINIT_DEFAULT
#define BSP_CLK_LFXO_CTUNE                            (79U)
#define BSP_CLK_LFXO_FREQ                             (32768U)
#define BSP_CLK_HFXO_FREQ                             (38400000UL)
#define BSP_CLK_HFXO_CTUNE                            (129)
#define BSP_CLK_HFXO_INIT                              CMU_HFXOINIT_DEFAULT

// SWO debug print serial emulator
#define BSP_TRACE_SWO_PIN					(3U)
#define BSP_TRACE_SWO_PORT					(gpioPortA)
/* ****************************************************************************
 * 		Sensors
 * ***************************************************************************/
 /********************************************************************************************************************/
 #ifdef SNSR_F1024KB
#define PIR_OUT_PIN						(0U)
#define PIR_OUT_PORT					(gpioPortB)
#define PIR_INTRPT_IRQn					GPIO_EVEN_IRQn
#define PIR_INTRPT_NUM					0x2

#define PIR_PWR_PIN						(0U)
#define PIR_PWR_PORT					(gpioPortA)
/***************************************************************************/
#define ALS_TH_I2C_SDA_PORT				gpioPortC
#define ALS_TH_I2C_SDA_PIN				(5U)

#define ALS_TH_I2C_SCL_PORT				gpioPortC
#define ALS_TH_I2C_SCL_PIN				(4U)

#define ALS_TH_I2C_HANDLE				I2C1
#define ALS_TH_I2C_IDX					1
/***************************************************************************/
#define DALI_TX_PIN                     (6U)
#define DALI_TX_PORT                    (gpioPortA)

#define DALI_RX_PIN                     (5U)
#define DALI_RX_PORT                    (gpioPortA)
/***************************************************************************/
#define GSLink_CTRL_PORT				gpioPortC
#define GSLink_CTRL_PIN					(3U)

#define GSLink_RTS_PORT					gpioPortC
#define GSLink_RTS_PIN					(2U)

#define GSLink_RTS_DIR_PORT				gpioPortC
#define GSLink_RTS_DIR_PIN				(1U)

#define GSLink_UART_TX_PORT				gpioPortA
#define GSLink_UART_TX_PIN				(5U)

#define GSLink_UART_RX_PORT				gpioPortA
#define GSLink_UART_RX_PIN				(6U)
/***************************************************************************/
#define ANALOG_I2C_SDA_PORT             (gpioPortA)
#define ANALOG_I2C_SDA_PIN              (5U)

#define ANALOG_I2C_SCL_PORT             (gpioPortA)
#define ANALOG_I2C_SCL_PIN              (6U)

#define ANALOG_I2C_HANDLE				I2C0
#define ANALOG_I2C_IDX					0
/***************************************************************************/
#define SWITCH_PIN						(1U)
#define SWITCH_PORT						(gpioPortB)
#define SWITCH_INTRPT_IRQn				GPIO_ODD_IRQn
#define SWITCH_INTRPT_NUM				0x1
/***************************************************************************/
#define APP_LED1						gpioPortD,1
#define APP_LED2						gpioPortD,0
/***************************************************************************/
/********************************************************************************************************************/
#elif defined SNSR_F1024KB_v_5_2
#define PIR_OUT_PIN						(1U)
#define PIR_OUT_PORT					(gpioPortB)
#define PIR_INTRPT_IRQn					GPIO_ODD_IRQn
#define PIR_INTRPT_NUM					0x1

#define PIR_PWR_PIN						(1U)
#define PIR_PWR_PORT					(gpioPortD)
/***************************************************************************/
#define ALS_TH_I2C_SDA_PORT				gpioPortA
#define ALS_TH_I2C_SDA_PIN				(6U)

#define ALS_TH_I2C_SCL_PORT				gpioPortA
#define ALS_TH_I2C_SCL_PIN				(5U)

#define ALS_TH_I2C_HANDLE				I2C0
#define ALS_TH_I2C_IDX					0
/***************************************************************************/
#define DALI_TX_PIN                     (3U)
#define DALI_TX_PORT                    (gpioPortC)

#define DALI_RX_PIN                     (2U)
#define DALI_RX_PORT                    (gpioPortC)
/***************************************************************************/
#define GSLink_CTRL_PORT				gpioPortD
#define GSLink_CTRL_PIN					(0U)

#define GSLink_RTS_PORT					gpioPortC
#define GSLink_RTS_PIN					(5U)

#define GSLink_RTS_DIR_PORT				gpioPortC
#define GSLink_RTS_DIR_PIN				(1U)

#define GSLink_UART_TX_PORT				gpioPortC
#define GSLink_UART_TX_PIN				(2U)

#define GSLink_UART_RX_PORT				gpioPortC
#define GSLink_UART_RX_PIN				(3U)
/***************************************************************************/
#define ANALOG_I2C_SDA_PORT             (gpioPortC)
#define ANALOG_I2C_SDA_PIN              (2U)

#define ANALOG_I2C_SCL_PORT             (gpioPortC)
#define ANALOG_I2C_SCL_PIN              (3U)

#define ANALOG_I2C_HANDLE				I2C0
#define ANALOG_I2C_IDX					0
/***************************************************************************/
#define MUX_CNTRL_PIN						(4U)
#define MUX_CNTRL_PORT						(gpioPortC)
/***************************************************************************/
#define SWITCH_PIN						(0U)
#define SWITCH_PORT						(gpioPortB)
#define SWITCH_INTRPT_IRQn				GPIO_EVEN_IRQn
#define SWITCH_INTRPT_NUM				0x0
/***************************************************************************/
#define APP_LED1						gpioPortD,4									//RED
#define APP_LED2						gpioPortD,3									//GREEN
/***************************************************************************/
/********************************************************************************************************************/
#elif defined (SNSR_F512KB_v_5_2)

#define PIR_OUT_PIN						(0U)
#define PIR_OUT_PORT					(gpioPortA)
#define PIR_INTRPT_IRQn					GPIO_EVEN_IRQn
#define PIR_INTRPT_NUM					0x2

#define PIR_PWR_PIN						(1U)
#define PIR_PWR_PORT					(gpioPortD)
/***************************************************************************/
#define ALS_TH_I2C_SDA_PORT				gpioPortA
#define ALS_TH_I2C_SDA_PIN				(6U)

#define ALS_TH_I2C_SCL_PORT				gpioPortA
#define ALS_TH_I2C_SCL_PIN				(5U)

#define ALS_TH_I2C_HANDLE				I2C0
#define ALS_TH_I2C_IDX					0
/***************************************************************************/
#define DALI_TX_PIN                     (3U)
#define DALI_TX_PORT                    (gpioPortC)

#define DALI_RX_PIN                     (2U)
#define DALI_RX_PORT                    (gpioPortC)
/***************************************************************************/
#define GSLink_CTRL_PORT				gpioPortD
#define GSLink_CTRL_PIN					(0U)

#define GSLink_RTS_PORT					gpioPortC
#define GSLink_RTS_PIN					(5U)

#define GSLink_RTS_DIR_PORT				gpioPortC
#define GSLink_RTS_DIR_PIN				(1U)

#define GSLink_UART_TX_PORT				gpioPortC
#define GSLink_UART_TX_PIN				(2U)

#define GSLink_UART_RX_PORT				gpioPortC
#define GSLink_UART_RX_PIN				(3U)
/***************************************************************************/
#define ANALOG_I2C_SDA_PORT             (gpioPortC)
#define ANALOG_I2C_SDA_PIN              (4U)

#define ANALOG_I2C_SCL_PORT             (gpioPortC)
#define ANALOG_I2C_SCL_PIN              (5U)

#define ANALOG_I2C_HANDLE				I2C0
#define ANALOG_I2C_IDX					0
/***************************************************************************/
#define SWITCH_PIN						(0U)
#define SWITCH_PORT						(gpioPortB)
#define SWITCH_INTRPT_IRQn				GPIO_EVEN_IRQn
#define SWITCH_INTRPT_NUM				0x0
/***************************************************************************/
#define APP_LED1						gpioPortD,4
#define APP_LED2						gpioPortD,3
/***************************************************************************/
#endif
/********************************************************************************************************************/
/****************************************************************************/
#define TRIAC_PORT 						GSLink_RTS_PORT
#define TRIAC_PIN  						GSLink_RTS_PIN
/****************************************************************************/
#endif /* HAL_CONFIG_BOARD_H */
