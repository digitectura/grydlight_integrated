/*
 * dali.c
 *
 *  Created on: Jul 31, 2019
 *      Author: Rakshith
 */

#include <application/inc/dali/dali.h>
#include "em_timer.h"
#include <main.h>
#include "application/inc/intervalTimer.h"

#define BAUD	0.7

#ifdef SEND_DEBUG

uint8_t testOne = 1;
uint8_t testZero = 0;

#endif

/***********************************************************************************************/
/*                                  Global Variables                                           */
/***********************************************************************************************/
uint16_t	transmitDataPacket[NUM_OF_DALI_LOOPS];
uint8_t		transmitDataComplete[NUM_OF_DALI_LOOPS];
uint8_t  	responsePacket[NUM_OF_DALI_LOOPS];
uint16_t 	repeatDelay = 0;
struct_daliRxParams sdaliRxParams[NUM_OF_DALI_LOOPS];
uint16_t a_fadeTime[FADE_90p5SEC] = 	{
											0x00,
											700,
											1000,
											1400,
											2000,
											2800,
											4000,
											5600,
											8000,
											11300,
											16000,
											22600,
											32000,
											45200,
											64000
										};
/**********************************************************************************************/
/*                                  Local Function Definitions                                */
/**********************************************************************************************/



/**********************************************************************************************/
/*                                  Function Declarations                                     */
/**********************************************************************************************/

/*		========= fn_discardPacket ==========
  *
  * @brief 	This function is called to discard the invalid packet response
  *        	received on the DALI bus.
  * @params None
  *
  * @retval None
  */
void fn_discardPacket(daliLoop_typedef m_daliLoop){
	sdaliRxParams[m_daliLoop].rxDataIndex = 0;
	sdaliRxParams[m_daliLoop].rxDataBegin_flag = 0;
	sdaliRxParams[m_daliLoop].rxStartTime =0;
	memset(sdaliRxParams[m_daliLoop].rxData, '\0', sizeof(sdaliRxParams[m_daliLoop].rxData));
	memset(sdaliRxParams[m_daliLoop].timeCount, '\0', sizeof(sdaliRxParams[m_daliLoop].timeCount));
}

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
bool fn_daliTransmit(uint8_t m_address, uint8_t m_command, daliLoop_typedef m_daliLoop){
	uint16_t tempCommand = 0;
	if(transmitDataComplete[m_daliLoop] != 0){
		tempCommand = (uint16_t)(((tempCommand|m_address)<<8)|m_command);
		//assign tempCommand as global so that the timer capture can access it
		transmitDataPacket[m_daliLoop] = tempCommand;

//		(m_daliLoop == DALI_LOOP1) ? TIMER_Enable(TIMER1, true) : __NOP();
		if(m_daliLoop == DALI_LOOP1){
			TIMER_Enable(TIMER1, true);
			TIMER_Enable(TIMER3, true);
//			printf("Timer3 enabled\r\n");
		}
		// stop the timer in the call back function after sending
		return true;
	}
	else{
		return false;
	}

}

/*		========= HAL_GPIO_EXTI_Callback ==========
  *
  * @brief 	This function is called when the DALI_x_RX pin has a positive or negative edge interrupt
  * 		in order to receive the manchester encoded response
  *
  * @params (GPIO_Pin) GPIO Pin
  *
  * @retval none
  *
  */

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
bool processResponseFrame(uint8_t *m_responseByte, daliLoop_typedef m_daliLoop){


	int8_t m_index = 0;
	int8_t m_index_2 = 0;
	int8_t m_index_3 = 0;
	uint16_t rxResponse_Sent = 0;
	uint8_t  rxResponse_Received = 0;
//	float m_baud = 0.7;

//	if(((fn_IsuSecTimerElapsed (rxStartTime, 400) == true) && (rxDataBegin_flag == 1))){
//		rxDataIndex = 0;
//		rxDataBegin_flag = 0;
//		response_main = 0;
//		memset(rxData, '\0', sizeof(rxData));
//		return false;
//	}
	if( (sdaliRxParams[m_daliLoop].rxDataIndex<100)
			&& ((fn_IsuSecTimerElapsed (sdaliRxParams[m_daliLoop].rxStartTime, 1000) == true) &&(sdaliRxParams[m_daliLoop].rxDataBegin_flag == 1))){
		for(m_index = 0; m_index<sdaliRxParams[m_daliLoop].rxDataIndex-1; m_index++){
			if((sdaliRxParams[m_daliLoop].timeCount[m_index+1]-sdaliRxParams[m_daliLoop].timeCount[m_index])*50> BAUD*1000 /*1.1*1000 */){
				for(m_index_2 = sdaliRxParams[m_daliLoop].rxDataIndex; m_index_2>m_index; m_index_2--){
					sdaliRxParams[m_daliLoop].timeCount[m_index_2] = sdaliRxParams[m_daliLoop].timeCount[m_index_2-1];
					sdaliRxParams[m_daliLoop].rxData[m_index_2] = sdaliRxParams[m_daliLoop].rxData[m_index_2-1];
				}
				sdaliRxParams[m_daliLoop].rxDataIndex++;
				sdaliRxParams[m_daliLoop].timeCount[m_index+1]=(uint16_t)((sdaliRxParams[m_daliLoop].timeCount[m_index]
																	+sdaliRxParams[m_daliLoop].timeCount[m_index+2])/2);
				sdaliRxParams[m_daliLoop].rxData[m_index+1] = sdaliRxParams[m_daliLoop].rxData[m_index];
			}
		}
		m_index_2 = 15;
		for(m_index = 2; m_index<34; m_index=(int8_t)(m_index + 2)){  // 34
			if(sdaliRxParams[m_daliLoop].rxData[m_index]==1){
//				if((int)round(((timeCount[m_index]-timeCount[0])*50)/(0.8*460))&0x01){
//						rxResponse += (0x01<<m_index_2);
//				}
				if(sdaliRxParams[m_daliLoop].rxData[m_index+1]==0){
					__NOP();
				}
			}
			else{
				if(sdaliRxParams[m_daliLoop].rxData[m_index+1]==1){
					rxResponse_Sent = (uint16_t)(rxResponse_Sent + (0x01<<m_index_2));
				}
			}
			m_index_2--;
		}

		if(sdaliRxParams[m_daliLoop].rxDataIndex>40){
			m_index_3 = 35;
			while(sdaliRxParams[m_daliLoop].rxData[m_index_3]==0){
				m_index_3++;
				if(m_index_3>65){
					break;
				}
			}
			if(sdaliRxParams[m_daliLoop].rxData[m_index_3]==1){
				if(sdaliRxParams[m_daliLoop].rxData[m_index_3+1]==0){
					m_index_2 = 7;
					m_index_3= (int8_t)(m_index_3 + 2);
				}
			}

			for(m_index = m_index_3; m_index_2>=0 /*&& m_index<m_index_3+(8*2)*/; m_index=(int8_t)(m_index + 2)){
				if(sdaliRxParams[m_daliLoop].rxData[m_index]==1){
					if(sdaliRxParams[m_daliLoop].rxData[m_index+1]==0){
						__NOP();
						rxResponse_Received |= (uint8_t)(0x80>>(m_index-m_index_3)/2);
					}
				}
				else{
					if(sdaliRxParams[m_daliLoop].rxData[m_index+1]==1){
//						rxResponse_Received |= (0x01<<(m_index-m_index_3)/2);
						__NOP();
					}
				}
				m_index_2--;
			}
		}

//		if(sdaliRxParams[m_daliLoop].rxDataIndex>40){
//			m_index_2 = 7;
//			m_index_3 = m_index_2;
////			for(m_index = (sdaliRxParams[m_daliLoop].rxData[sdaliRxParams[m_daliLoop].rxDataIndex] == 0 &&
////					sdaliRxParams[m_daliLoop].rxData[sdaliRxParams[m_daliLoop].rxDataIndex-1] == 1 ) ? (sdaliRxParams[m_daliLoop].rxDataIndex)
////					: (sdaliRxParams[m_daliLoop].rxData[sdaliRxParams[m_daliLoop].rxDataIndex-2] == 1) ? (sdaliRxParams[m_daliLoop].rxDataIndex-1)
////							: (sdaliRxParams[m_daliLoop].rxDataIndex-2) ; m_index_2>=0; m_index-=2){
//			for(m_index = (sdaliRxParams[m_daliLoop].rxDataIndex%2 != 0) ? (sdaliRxParams[m_daliLoop].rxDataIndex-1)
//					:(sdaliRxParams[m_daliLoop].rxDataIndex-2) ; m_index_2>=0; m_index-=2){
//				if(sdaliRxParams[m_daliLoop].rxData[m_index]==1){
////				if((int)round(((timeCount[m_index]-timeCount[0])*50)/(0.8*460))&0x01){
////						rxResponse += (0x01<<m_index_2);
////				}
//					if(sdaliRxParams[m_daliLoop].rxData[m_index-1]==0){
//						__NOP();
//					}
//				}
//				else{
//					if(sdaliRxParams[m_daliLoop].rxData[m_index-1]==1){
//						rxResponse_Received |= (0x01<<(m_index_3 - m_index_2));
//					}
//				}
//				m_index_2--;
//			}
//		}
		sdaliRxParams[m_daliLoop].rxDataIndex = 0;
		sdaliRxParams[m_daliLoop].rxDataBegin_flag = 0;
		memset(sdaliRxParams[m_daliLoop].rxData, '\0', sizeof(sdaliRxParams[m_daliLoop].rxData));
		memset(sdaliRxParams[m_daliLoop].timeCount, '\0', sizeof(sdaliRxParams[m_daliLoop].timeCount));
//		rxResponse_Received = (uint8_t)rxResponse_Received;
		sdaliRxParams[m_daliLoop].rxStartTime = 0;

		// Packet to be discarded after processing
//		fn_discardPacket();

		*m_responseByte = rxResponse_Received;
//		if(rxResponse_Received == 0){
//			return true;
//		}
		return true;
	}


	return false;

}


bool fn_commandTransmit(uint8_t m_address, uint8_t m_command, bool m_repeat, daliLoop_typedef m_daliLoop){
	static uint8_t daliCT_state = COMMAND_INIT_STATE;
	bool daliCT_retval = false;
	static int8_t m_repeatLoc = 0;


	switch(daliCT_state){
	case COMMAND_INIT_STATE:
		if(m_repeat){
			m_repeatLoc = REPEAT_NUMBER;
		}
		else{
			m_repeatLoc = 0;
		}
		daliCT_state = COMMAND_EXECUTION_STATE;
		fn_discardPacket(m_daliLoop);
		break;
	case COMMAND_EXECUTION_STATE:
		if(fn_daliTransmit(m_address, m_command, m_daliLoop)){
			daliCT_state = COMPLETION_STATE;
		}
		break;
	case COMPLETION_STATE:
		if(transmitDataComplete[m_daliLoop] == true){
			if(m_repeat){

				m_repeatLoc--;
				daliCT_state = REPEAT_STATE;
				repeatDelay = fn_GetmSecTimerStart();

			}
			else{
				daliCT_state = REPEAT_STATE;
				repeatDelay = fn_GetmSecTimerStart();
			}
		}
		break;
	case REPEAT_STATE:
		if(fn_IsmSecTimerElapsed(repeatDelay, 35)){
			if(m_repeatLoc > 0){
				daliCT_state = COMMAND_EXECUTION_STATE;
//				HAL_Delay(50);
			}
			else{
//				HAL_Delay(50);

				daliCT_retval = true;
				daliCT_state = COMMAND_INIT_STATE;
			}
			TIMER_Enable(TIMER3, false);
//			printf("Timer3 disabled\r\n");

		}
		break;
	}
	return daliCT_retval;
}

bool fn_commandTransmit_receive(uint8_t m_address, uint8_t m_command, uint8_t *m_response, daliLoop_typedef m_daliLoop){
	static uint8_t daliCTR_state = COMMAND_EXECUTION_STATE;
	bool daliCTR_retval = false;
//	uint8_t m_responseByte = 0;

		switch(daliCTR_state){
		case COMMAND_EXECUTION_STATE:
			// Third argument is always false in this context
			if(fn_commandTransmit(m_address, m_command, false, m_daliLoop) == true){
				daliCTR_state = RESPONSE_STATE;
			}
			break;
		case RESPONSE_STATE:
			if(processResponseFrame(m_response, m_daliLoop)){

				daliCTR_state = COMMAND_EXECUTION_STATE;
				daliCTR_retval = true;
			}
			break;

		}
	return daliCTR_retval;
}

void fn_initDaliDriver(){
	memset(transmitDataPacket, 0, sizeof(transmitDataPacket));
	memset(transmitDataComplete, 1, sizeof(transmitDataComplete));
	memset(responsePacket, 0, sizeof(responsePacket));
	memset(sdaliRxParams, 0, sizeof(sdaliRxParams));
}

