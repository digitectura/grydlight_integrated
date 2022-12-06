/*
 * mcp45hvx1.h
 *
 *  Created on: 20-Jan-2021
 *      Author: Rakshith
 */

#ifndef APPLICATION_ANALOG_MCP45HVX1_H_
#define APPLICATION_ANALOG_MCP45HVX1_H_

#include "application/inc/i2c.h"
#include "main.h"

#define			MAX_WIPER_VALUE					127
#define			MIN_WIPER_VALUE					0
#define 		BOARD_VOLTAGE					15
#define 		WIPER_VOLTAGE					10

#define	FADE_TIME_CALC(curLvl,trgtLvl,fadeTime)  	(uint8_t)( ( abs(curLvl-trgtLvl) < 5 ) ? 0 : ( ( fadeTime ) / abs(curLvl-trgtLvl) ) )
#define MAX_WIPER_CORRECTION						(WIPER_VOLTAGE * MAX_WIPER_VALUE) / BOARD_VOLTAGE
#define WIPER_VALUE(intensity)						(uint8_t)( ( intensity / 100.0 ) * MAX_WIPER_CORRECTION )

/* 7 Bit I2C Operation Components ......................................... */
#define POT_CH1_WRITE_ADDR		0x78
#define POT_CH1_READ_ADDR		0x79

/* TCON configuration.................. */
#define TCON_R0HW (0x08)  // Shutdown Resistor Force
#define TCON_R0A  (0x04)  // Terminal A Connection
#define TCON_R0W  (0x02)  // Wiper Connection
#define TCON_R0B  (0x01)  // Terminal B Connection

#define GCALL_TCON          (0x60)
#define GCALL_WIPER         (0x40)
#define GCALL_WIPERUP       (0x42)
#define GCALL_WIPERDWN      (0x44)
#define GCALL_COM_WRITE     (0x02)
#define GCALL_COM_RWRITE    (0x03)
#define GCALL_COM_WIPERINC  (0x42)
#define GCALL_COM_WIPERDEC  (0x44)

#define MEM_WIPER           (0x00)
#define MEM_TCON            (0x40)

#define COM_WRITE           (0x00)
#define COM_READ            (0x0C)
#define COM_WIPERINC        (0x04)
#define COM_WIPERDEC        (0x08)

#define WIPER_STEPS			5
#define MAX_WIPER_VALUE		127
#define MIN_WIPER_VALUE 	0

bool fn_setAnalogIntensity(uint8_t m_val, uint8_t);
bool fn_decrementWiper(uint8_t decriments);
bool fn_incrementWiper(uint8_t incriments);
bool fn_readWiper(uint8_t *rxBuf, uint8_t m_targetAdd, uint8_t m_len);;
void fn_IdenitfyLight();
void fn_sceneCfg(uint16_t m_id, uint8_t m_shid, uint8_t m_statFrom, uint8_t m_intensity);
void fn_sceneDel(uint16_t m_id);
void fn_goto_scene(uint16_t m_id, uint8_t m_statFrom);
void fn_AnalogFadeCntrl(void);

extern bool g_isIdentifyEnabled;

extern uint8_t g_analogLightStat;
extern uint16_t g_analogFadeTime;

#endif /* APPLICATION_ANALOG_MCP45HVX1_H_ */
