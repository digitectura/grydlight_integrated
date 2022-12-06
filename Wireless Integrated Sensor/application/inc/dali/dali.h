/*
 * dali.h
 *
 *  Created on: Jul 31, 2019
 *      Author: Rakshit
 */

#ifndef APP_DALI_DALI_H_
#define APP_DALI_DALI_H_

#include "main.h"
#include "application/inc/dali/commands.h"

#define WARM_A							8.5*pow(10,-6)
#define WARM_B							-0.12175
#define WARM_C							428

#define COOL_A							-9*pow(10,-6)
#define COOL_B							0.128
#define COOL_C							-350


#define WARM_INT(KV)					(WARM_A*KV*KV)+(WARM_B*KV)+ WARM_C
#define COOL_INT(KV)					(COOL_A*KV*KV)+(COOL_B*KV)+ COOL_C

#define MILLISECONDS_100				(1000/50)*100
#define TOTAL_DALI_BALLASTS				12
#define TOTAL_DALI_GROUPS				12
#define TOTAL_BALLASTS_INGP				12
#define TOTAL_ENTITIES_GP				12
#define TOTAL_SCENES_CTRL				12
#define TOTAL_BALLST_COUNT_PER_CTRL		12

#define TOTAL_DACTION_MAX				15

//#define BROADCAST_DP					0xFE
//#define BROADCAST_C						0XFF

typedef enum{
	DALI_LOOP1,
	NUM_OF_DALI_LOOPS,
	DALI_LOOP2,

}daliLoop_typedef;

typedef enum{
	NO_FADE,
	FADE_0p7SEC,
	FADE_1p0SEC,
	FADE_1p4SEC,
	FADE_2p0SEC,
	FADE_2p8SEC,
	FADE_4p0SEC,
	FADE_5p6SEC,
	FADE_8p0SEC,
	FADE_11p3SEC,
	FADE_16p0SEC,
	FADE_22p6SEC,
	FADE_32p0SEC,
	FADE_45p2SEC,
	FADE_64p0SEC,
	FADE_90p5SEC,
}daliFadeTime_typedef;

typedef enum{
	FADE_357 = 1,
	FADE_253,
	FADE_179,
	FADE_126,
	FADE_89,
	FADE_63,
	FADE_44,
	FADE_31,
	FADE_22,
	FADE_15,
	FADE_11,
	FADE_7,
	FADE_5,
	FADE_4,
	FADE_3
}daliFadeRate_typedef;



typedef struct{

	uint16_t 	rxStartTime;
	uint8_t 	rxDataBegin_flag;				// indicates the arrival of the negative edge of the start bit if set
	int8_t 		rxDataIndex;
	uint8_t 	rxDataRecord_flag;
	uint8_t 	rxData[160];
	uint16_t 	timeCount[160];
}__attribute__((aligned (2))) struct_daliRxParams;

typedef struct{
	char gpEntityLabel[8];
}__attribute__((__packed__)) struct_gpEntityLoc;

extern uint16_t transmitDataPacket[NUM_OF_DALI_LOOPS];
extern uint8_t 	transmitDataComplete[NUM_OF_DALI_LOOPS];
extern uint8_t  responsePacket[NUM_OF_DALI_LOOPS];
extern struct_daliRxParams sdaliRxParams[NUM_OF_DALI_LOOPS];
extern uint16_t a_fadeTime[FADE_90p5SEC];

/*		========= fn_daliTransmit ==========
  *
  * @brief 	This function is called to transmit DALI command on the DALI bus.
  *
  * @params (m_address) The short address assigned after ballast commissioning
  *
  * @params (m_command) The DALI Command to be executed on the ballast
  *
  * @retval true or false
  *
  */
bool fn_daliTransmit(uint8_t, uint8_t, daliLoop_typedef);


/*		========= processResponseFrame ==========
  *
  * @brief 	This function is called when all the bits from the manchester encoded response from the
  * 		DALI Bus is received
  *
  * @params (m_responseByte) Response after Manchester decoding
  *
  * @retval true or false
  *
  */
bool processResponseFrame(uint8_t *, daliLoop_typedef);

/*		========= fn_discardPacket ==========
  *
  * @brief 	This function is called to discard the invalid packet response
  *        	received on the DALI bus.
  * @params None
  *
  * @retval None
  */
void fn_discardPacket(daliLoop_typedef);

bool fn_commandTransmit(uint8_t, uint8_t, bool, daliLoop_typedef);

bool fn_commandTransmit_receive(uint8_t , uint8_t , uint8_t *, daliLoop_typedef);

void fn_initDaliDriver(void);
#endif /* APP_DALI_DALI_H_ */
