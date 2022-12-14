/*
 * GS_Link.c
 *
 *  Created on: Aug 19, 2020
 *      Author: User
 */
/*****************************************************************************************************************************************/
#include "application/inc/gsLink/GS_LINK.h"
#include "application/inc/gsLink/gsLinkPktQ.h"
#include "application/inc/gsLink/GSLink_uart.h"
#include "application/inc/command_handler.h"
#include "application/inc/intervalTimer.h"
#include "application/inc/BLE_Queue.h"
#include "application/inc/app.h"
#include "em_rtcc.h"
#include "stdlib.h"
/*-----------------------------------------------------------------------------------------------------------------------------*/
struct_gsLinkGlobalVar			s_gsLinkGlobalVar =
													{
														.rx_frameState 			= S_PREAMBLEH ,
														.rx_process_State		= RXQ_EMPTY_CHECK ,
														.tx_process_State		= TXQ_EMPTY_CHECK ,
														.rx_frameTimeOut		= 0 ,
														.rx_processTimeOut		= 0 ,
														.is_crcFirstByte 		= true ,
														.is_addrFirstByte		= true ,
														.Indx					= 0 ,
														.txCmplt				= TRANSMIT_COMPLETE ,
														.txPkt					= { [0 ... MAX_PKT_BUFFER_LEN-1] = 0 },
														.gslnk_PktData			= { [0 ... MAX_CMDDATA_LEN-1]	 = 0 }
													};
/*.........................................................................................................*/
struct_gsLinkPktQ 				s_gsLink_TxQ	=
													{
														.head 	= -1,
														.tail 	= -1,
														.num_pkts = 0
													};
/*.........................................................................................................*/
struct_gsLinkPktQ 				s_gsLink_RxQ	=
													{
														.head 	= -1,
														.tail	= -1,
														.num_pkts = 0
													};
/*-----------------------------------------------------------------------------------------------------------------------------*/
/*
struct_gsLinkCmdParser gsLink_cmdFrame[MAX_NUM_PKTS] =
														{
															[FT_WHOIS]				= {NULL},
															[FT_IAM]				= {fn_gsLink_Iam} ,
															[FT_DATAREQ]			= {NULL} ,
															[FT_DATARES]			= {NULL} ,
															[FT_STATUSCHANGE]		= {fn_gsLink_statusChange_from_Ctrl} ,
															[FT_COMMISSIONING]		= {NULL} ,
															[FT_COMMAND]			= {fn_gsLink_command} ,
															[FT_ACK]				= {NULL} ,
															[FT_NACK]				= {NULL} ,
															[FT_WL_LIGHT_COMMANDS]	= {NULL} ,
															[FT_DATA_STREAM]		= {NULL}
														};
*/

struct_gsLinkCmdParser gsLink_cmdFrame[MAX_NUM_PKTS] =
														{
															[FT_WHOIS]				= {NULL},
															[FT_IAM]				= {(void *)fn_gsLink_Iam} ,
															[FT_DATAREQ]			= {NULL} ,
															[FT_DATARES]			= {NULL} ,
															[FT_STATUSCHANGE]		= {(void *)fn_gsLink_statusChange_from_Ctrl} ,
															[FT_COMMISSIONING]		= {NULL} ,
															[FT_COMMAND]			= {(void *)fn_gsLink_command} ,
															[FT_ACK]				= {NULL} ,
															[FT_NACK]				= {NULL} ,
															[FT_WL_LIGHT_COMMANDS]	= {NULL} ,
															[FT_DATA_STREAM]		= {NULL}
														};


struct_gsLinkPkt  				s_tmp_gsLink_rxPkt = {0};
struct_gsLinkPkt  				s_tmp_gsLink_Pkt = {0};
/*****************************************************************************************************************************************/
uint8_t fn_createGSLinkPkt(uint8_t ft, uint16_t dstAddr, uint8_t *pktData, uint8_t dataLen)
{

	uint16_t crc = 0;

	s_tmp_gsLink_Pkt.preambleH = PREAMBLEH;
	s_tmp_gsLink_Pkt.preambleL = PREAMBLEL;
	s_tmp_gsLink_Pkt.frameType = ft;
	s_tmp_gsLink_Pkt.sourceAddress = snsrMinCfg.snsrID;
	s_tmp_gsLink_Pkt.destinationAddress = dstAddr;

	crc = fn_gen_crc16((const uint8_t *)&s_tmp_gsLink_Pkt, GSLINK_PKT_FRAME_HEADER_LEN);
	s_tmp_gsLink_Pkt.HeaderCRCL = (uint8_t)crc;
	s_tmp_gsLink_Pkt.HeaderCRCH = (uint8_t)(crc>>8);
	s_tmp_gsLink_Pkt.DataLength = dataLen;
	memcpy(s_tmp_gsLink_Pkt.PacketData,pktData,s_tmp_gsLink_Pkt.DataLength);
	uint8_t buf[s_tmp_gsLink_Pkt.DataLength+1];

	buf[0] = s_tmp_gsLink_Pkt.DataLength;
	memcpy(&buf[1],s_tmp_gsLink_Pkt.PacketData,s_tmp_gsLink_Pkt.DataLength);

	crc = fn_gen_crc16(buf,s_tmp_gsLink_Pkt.DataLength+1);
	s_tmp_gsLink_Pkt.DataCRCL = (uint8_t)crc;
	s_tmp_gsLink_Pkt.DataCRCH = (uint8_t)(crc>>8);

	return 1;

}
/*****************************************************************************************************************************************/
uint16_t fn_gen_crc16(const uint8_t *data, uint8_t size)
{
    uint16_t out = 0;
    int bits_read = 0, bit_flag;

    /* Sanity check: */
    if(data == NULL)
        return 0;

    while(size > 0)
    {
        bit_flag = out >> 15;

        /* Get next bit: */
        out <<= 1;
        out |= (*data >> bits_read) & 1; // item a) work from the least significant bits

        /* Increment bit counter: */
        bits_read++;
        if(bits_read > 7)
        {
            bits_read = 0;
            data++;
            size--;
        }

        /* Cycle check: */
        if(bit_flag)
            out ^= 0x8005;

    }

    // item b) "push out" the last 16 bits
    int i;
    for (i = 0; i < 16; ++i) {
        bit_flag = out >> 15;
        out <<= 1;
        if(bit_flag)
            out ^= 0x8005;
    }

    // item c) reverse the bits
    uint16_t crc = 0;
    i = 0x8000;
    int j = 0x0001;
    for (; i != 0; i >>=1, j <<= 1) {
        if (i & out) crc |= j;
    }

    return crc;
}
/*****************************************************************************************************************************************/
void fn_GSLinkTimeoutHandler(void)
{
	if( (s_gsLinkGlobalVar.rx_frameState != S_PREAMBLEH) &&
			fn_IsmSecTimerElapsed(s_gsLinkGlobalVar.rx_frameTimeOut, RX_FRAME_TIMEOUT))
	{
		fn_resetGSlinkRxState();
	}
	return ;
}
/*****************************************************************************************************************************************/
void fn_resetGSlinkRxState(void)
{
	s_gsLinkGlobalVar.rx_frameState = S_PREAMBLEH;
	s_gsLinkGlobalVar.Indx = 0;
	s_gsLinkGlobalVar.is_crcFirstByte = true;
	s_gsLinkGlobalVar.is_addrFirstByte = true;
	memset(&s_tmp_gsLink_rxPkt,'\0',sizeof(s_tmp_gsLink_rxPkt));
	return ;
}
/*****************************************************************************************************************************************/
void fn_queue_gsLink_RxPkt(uint8_t frameByte)
{
	if(s_gsLinkGlobalVar.rx_frameState != S_PREAMBLEH && s_gsLinkGlobalVar.tx_process_State <= TRANSMIT_CHECK)
	{
		//	Stop tranmission and reset the state to empty check
		fn_changeRTSdirection(RTS_INPUT);
		GS_LINK_CTL_RECEIVE();
		s_gsLinkGlobalVar.tx_process_State = TXQ_EMPTY_CHECK;
	}
	switch(s_gsLinkGlobalVar.rx_frameState)
	{
		case S_PREAMBLEH:
			if(frameByte == PREAMBLEH)
			{
				s_gsLinkGlobalVar.rx_frameTimeOut = fn_GetmSecTimerStart();
				s_tmp_gsLink_rxPkt.preambleH = frameByte;
				s_gsLinkGlobalVar.rx_frameState = S_PREAMBLEL;
				return ;
			}
		break;
		case S_PREAMBLEL:
			if(frameByte == PREAMBLEL)
			{
				s_tmp_gsLink_rxPkt.preambleL = frameByte;
				s_gsLinkGlobalVar.rx_frameState = S_FRAMETYPE;
				return ;
			}
		break;
		case S_FRAMETYPE:
			if(frameByte<MAX_NUM_PKTS)
			{
				s_tmp_gsLink_rxPkt.frameType = frameByte;
				s_gsLinkGlobalVar.rx_frameState = S_SADD;
				return ;
			}
		break;
		case S_SADD:
			if(s_gsLinkGlobalVar.is_addrFirstByte)
			{
				s_tmp_gsLink_rxPkt.sourceAddress = frameByte;
				s_gsLinkGlobalVar.is_addrFirstByte = false;
			}
			else
			{
				s_tmp_gsLink_rxPkt.sourceAddress |= (frameByte << 8);
				s_gsLinkGlobalVar.is_addrFirstByte = true;
				s_gsLinkGlobalVar.rx_frameState = S_DADD;
			}
			return ;
		break;
		case S_DADD:
			if(s_gsLinkGlobalVar.is_addrFirstByte)
			{
				s_tmp_gsLink_rxPkt.destinationAddress = frameByte;
				s_gsLinkGlobalVar.is_addrFirstByte = false;
				return ;
			}
			else
			{
				s_tmp_gsLink_rxPkt.destinationAddress |= (frameByte << 8);
				if(s_tmp_gsLink_rxPkt.destinationAddress == snsrMinCfg.snsrID)
				{
					s_gsLinkGlobalVar.is_addrFirstByte = true;
					s_gsLinkGlobalVar.rx_frameState = S_HCRC;
					return ;
				}
			}
		break;
		case S_HCRC:
			if(s_gsLinkGlobalVar.is_crcFirstByte)
			{
				s_tmp_gsLink_rxPkt.HeaderCRCL = frameByte;
				s_gsLinkGlobalVar.is_crcFirstByte = false;
			}
			else
			{
				s_tmp_gsLink_rxPkt.HeaderCRCH = frameByte;
				s_gsLinkGlobalVar.is_crcFirstByte = true;
				s_gsLinkGlobalVar.rx_frameState = S_LEN;
			}
			return ;
		break;
		case S_LEN:
			s_tmp_gsLink_rxPkt.DataLength = frameByte;
			if(s_tmp_gsLink_rxPkt.DataLength == 0)
			{
				fn_enQ(&s_gsLink_RxQ, &s_tmp_gsLink_rxPkt);
			}
			else
			{
				s_gsLinkGlobalVar.rx_frameState = S_DATA;
				return ;
			}
		break;
		case S_DATA:
			s_tmp_gsLink_rxPkt.PacketData[s_gsLinkGlobalVar.Indx++] = frameByte;
			if(s_gsLinkGlobalVar.Indx == (s_tmp_gsLink_rxPkt.DataLength))
			{
				s_gsLinkGlobalVar.rx_frameState = S_DCRC;
			}
			return ;
		break;
		case S_DCRC:
			if(s_gsLinkGlobalVar.is_crcFirstByte)
			{
				s_tmp_gsLink_rxPkt.DataCRCL = frameByte;
				s_gsLinkGlobalVar.is_crcFirstByte = false;
				return ;
			}
			else
			{
				s_tmp_gsLink_rxPkt.DataCRCH = frameByte;
				s_gsLinkGlobalVar.is_crcFirstByte = true;
				fn_enQ(&s_gsLink_RxQ, &s_tmp_gsLink_rxPkt);
			}
		break;
		default:
		break;
	}
	fn_resetGSlinkRxState();
	return;
}
/*****************************************************************************************************************************************/
void fn_queue_gsLink_TxPkt(struct_gsLinkPkt *gsLink_pkt)
{
	fn_enQ(&s_gsLink_TxQ, gsLink_pkt);
	return ;
}
/*****************************************************************************************************************************************/
uint8_t fn_CRCcheck(struct_gsLinkPkt *gsLink_pkt)
{
	uint16_t crc = 0;
	uint8_t dataBuffer[MAX_CMDDATA_LEN];

	crc = fn_gen_crc16((const uint8_t*)gsLink_pkt,GSLINK_PKT_FRAME_HEADER_LEN);

	if( ((uint8_t)(crc) == gsLink_pkt->HeaderCRCL)  &&
			((uint8_t)(crc >> 8) == gsLink_pkt->HeaderCRCH) )
	{
		dataBuffer[0] = gsLink_pkt->DataLength;
		memcpy( &dataBuffer[1] , gsLink_pkt->PacketData , gsLink_pkt->DataLength );

		crc = fn_gen_crc16( (const uint8_t*)dataBuffer , (gsLink_pkt->DataLength+1) );

		if( ((uint8_t)(crc) == gsLink_pkt->DataCRCL)  &&
			((uint8_t)(crc >> 8) == gsLink_pkt->DataCRCH) )
		{
			return true;								//if CRC is equal,return as the pkt can be processed
		}
	}
	return PKT_REJECTED;
}
/*****************************************************************************************************************************************/
void fn_process_RxPkts(void)
{
	static struct_gsLinkPkt m_rxPkt;
	switch(s_gsLinkGlobalVar.rx_process_State)
	{
		case RXQ_EMPTY_CHECK:
		{
			if(!fn_isQempty(&s_gsLink_RxQ))
			{
				fn_peekQ(&s_gsLink_RxQ, &m_rxPkt);
				s_gsLinkGlobalVar.rx_process_State = CHECK_FOR_CRC;
			}
		}
		break;
		case CHECK_FOR_CRC:
		{
			if(fn_CRCcheck(&m_rxPkt) == PKT_REJECTED)
			{
				s_gsLinkGlobalVar.rx_process_State = DEQ_RX_PKT;
			}
			else
			{
				s_gsLinkGlobalVar.rx_process_State = CHECK_FOR_READY_STATE;
			}
		}
		break;
		case CHECK_FOR_READY_STATE:
		{
			if(1)
			{
				s_gsLinkGlobalVar.rx_processTimeOut = fn_GetSecTimerStart();
				s_gsLinkGlobalVar.rx_process_State = PROCESS_PKT_EVNT;
			}
		}
		break;
		case PROCESS_PKT_EVNT:
		{
			if(gsLink_cmdFrame[m_rxPkt.frameType].gsLink_cmdParser(&m_rxPkt)
					|| fn_IsSecTimerElapsed(s_gsLinkGlobalVar.rx_process_State, RX_PROCESS_TIMEOUT))
			{
				s_gsLinkGlobalVar.rx_process_State = DEQ_RX_PKT;
			}
		}
		break;
		case DEQ_RX_PKT:
		{
			fn_deQ(&s_gsLink_RxQ);
			s_gsLinkGlobalVar.rx_process_State = RXQ_EMPTY_CHECK;
		}
		break;
		default:
		break;
	}
	return ;
}
/*****************************************************************************************************************************************/
/* 1	---->	Check if Q is empty
 * 2	---->	Check if RTS is high ? (yes) Pull it low and got to step 3 : (no) RandomBackOff and go back to step 2
 * 3	---->	pull down RTS line , Random BackOff and go to step 4		//	for arbitration
 * 4	---->	check if there is some data rcvng from other devices(when RTS is low) ? (yes) release RTS line and go back to step 2 : (no) go to step 5
 * 5	---->	dequeue the pkt from Tx and transmit out and check for transmit complete
 * 6	---->	if Tx complete ? (yes) go back to step 1 : (no) after random backoff go back to step 1
 */
void fn_process_TxPkts(void)
{


	switch(s_gsLinkGlobalVar.tx_process_State)
	{
		case TXQ_EMPTY_CHECK :																	//step 1
		{
			if(!fn_isQempty(&s_gsLink_TxQ))
			{
				srand(RTCC_CounterGet());
				s_gsLinkGlobalVar.randmBckOff_StartTime = fn_GetmSecTimerStart();
				s_gsLinkGlobalVar.randomNumber = (uint16_t)((rand()%50)+15);
				s_gsLinkGlobalVar.tx_process_State = TX_RANDOM_BCKOFF;
			}
		}
		break;
		case TX_RANDOM_BCKOFF :
		{
			if( fn_IsmSecTimerElapsed(s_gsLinkGlobalVar.randmBckOff_StartTime, s_gsLinkGlobalVar.randomNumber) )
			{
				s_gsLinkGlobalVar.tx_process_State = TX_RTS_CHECK;
			}
		}
		break;
		case TX_RTS_CHECK :																		//step 2
		{
			if( IS_RTS_HIGH() )
			{
				// Bus is idle, RTS will be pulled low to get ready for transmission			//arbitration
				fn_changeRTSdirection(RTS_OUTPUT);
				PULL_RTS_LOW();
				s_gsLinkGlobalVar.tx_process_State = TRANSMIT_CHECK;
			}
			else
			{
				s_gsLinkGlobalVar.tx_process_State = TX_RANDOM_BCKOFF;
			}
			s_gsLinkGlobalVar.randmBckOff_StartTime = fn_GetmSecTimerStart();
			s_gsLinkGlobalVar.randomNumber = (uint16_t)((rand()%50)+15);
		}
		break;
		case TRANSMIT_CHECK:
		{
			if( fn_IsmSecTimerElapsed(s_gsLinkGlobalVar.randmBckOff_StartTime, s_gsLinkGlobalVar.randomNumber))
			{
				if(s_gsLinkGlobalVar.rx_frameState != S_PREAMBLEH)
				{
					fn_changeRTSdirection(RTS_INPUT);
					GS_LINK_CTL_RECEIVE();

					// Give a random timeout in case for arbitration clash between another device
					s_gsLinkGlobalVar.randmBckOff_StartTime = fn_GetmSecTimerStart();
					s_gsLinkGlobalVar.randomNumber = (uint16_t)((rand()%50)+15);
					s_gsLinkGlobalVar.tx_process_State = TX_RANDOM_BCKOFF;
				}
//				if( fn_IsmSecTimerElapsed(s_gsLinkGlobalVar.randmBckOff_StartTime, s_gsLinkGlobalVar.randomNumber))
				else
				{
					s_gsLinkGlobalVar.tx_process_State = TRANSMIT_PKT;
				}
			}
		}
		break;
		case TRANSMIT_PKT:
		{
			GS_LINK_CTL_TRANSMIT();
			s_gsLinkGlobalVar.txTimeout = fn_GetmSecTimerStart();
			fn_transmitPacket();
			s_gsLinkGlobalVar.tx_process_State = TRANSMIT_CMPLT_CHECK;
		}
		break;
		case TRANSMIT_CMPLT_CHECK:
		{
			//NOTE :::: IF THE RTS DIRECTION IS CAHNGED IMIDIATELY AFTER TXCMPLT FLAG RAISED,
			//::::::::: THE LAST FEW BYTES RCVD BY DEST DEVICE WILL GET CORUUPTED,,,
			//::::::::: SO WAIT FOR 10MSEC ATLEAST AND THEN CHANGE THE DIRECTION
			if(s_gsLinkGlobalVar.txCmplt &&
					fn_IsmSecTimerElapsed(s_gsLinkGlobalVar.txTimeout,TX_CMPLT_WAIT_DELAY))
			{
				fn_changeRTSdirection(RTS_INPUT);
				GS_LINK_CTL_RECEIVE();
				s_gsLinkGlobalVar.tx_process_State = DEQ_TX_PKT;
			}
		}
		break;
		case DEQ_TX_PKT:
		{
			fn_deQ(&s_gsLink_TxQ);
			s_gsLinkGlobalVar.tx_process_State = TXQ_EMPTY_CHECK;
		}
		break;
		default:
		break;
	}
	return ;
}
/*****************************************************************************************************************************************/
void fn_changeRTSdirection(uint8_t m_dir)
{
	switch(m_dir)
	{
		case RTS_INPUT:
			CHANGE_RTS_DIR_IP();
			GPIO_PinModeSet(GSLink_RTS_PORT,GSLink_RTS_PIN, gpioModeInput, GPIO_PULL_UP);
		break;
		case RTS_OUTPUT:
			CHANGE_RTS_DIR_OP();
			GPIO_PinModeSet(GSLink_RTS_PORT,GSLink_RTS_PIN, gpioModePushPull, 0);
		break;
		default:
		break;
	}
	return ;
}
/*****************************************************************************************************************************************/
void fn_transmitPacket(void)
{
	s_gsLinkGlobalVar.txCmplt = TRANSMIT_INPROCESS;
	struct_gsLinkPkt s_tmp_gsLink_txPkt;
	fn_peekQ(&s_gsLink_TxQ, &s_tmp_gsLink_txPkt);
	memcpy(s_gsLinkGlobalVar.txPkt,&s_tmp_gsLink_txPkt,FRAME_HEADER_LEN);
	memcpy(&s_gsLinkGlobalVar.txPkt[FRAME_HEADER_LEN],s_tmp_gsLink_txPkt.PacketData,s_tmp_gsLink_txPkt.DataLength);
	memcpy(&s_gsLinkGlobalVar.txPkt[FRAME_HEADER_LEN+s_tmp_gsLink_txPkt.DataLength],&s_tmp_gsLink_txPkt.DataCRCL,DATA_CRC_LEN);

//	uint8_t len = FRAME_HEADER_LEN +s_tmp_gsLink_txPkt.DataLength+DATA_CRC_LEN;
//	for(uint8_t i = 0;i<len;i++)
//	{
//		DBG_PRINT("0x%x ",s_gsLinkGlobalVar.txPkt[i]);
//	}
//	DBG_PRINT("\r\n");
	fn_uartTx_IT(GSLINK_UART_PERPHERAL, s_gsLinkGlobalVar.txPkt, FRAME_HEADER_LEN +s_tmp_gsLink_txPkt.DataLength+DATA_CRC_LEN);
	return ;
}
/*****************************************************************************************************************************************/
uint8_t fn_gsLink_Iam(struct_gsLinkPkt *mpkt)
{
	sCtrlCfg.UUID = mpkt->sourceAddress;
	sCtrlCfg.cntrl_type = mpkt->PacketData[0] | (((uint16_t)mpkt->PacketData[1]<<8));
	sCtrlCfg.ctrlFWver = mpkt->PacketData[2];
	return 1;
}
/*****************************************************************************************************************************************/
void fn_gsLink_Iam_toCntrl(void)
{
	memset(&s_tmp_gsLink_Pkt,'\0',sizeof(struct_gsLinkPkt));
	s_gsLinkGlobalVar.gslnk_PktData[0]=(uint8_t)snsrMinCfg.device_type;
	s_gsLinkGlobalVar.gslnk_PktData[1]=(uint8_t)(snsrMinCfg.device_type>>8);
	fn_createGSLinkPkt(FT_IAM,sCtrlCfg.UUID ,s_gsLinkGlobalVar.gslnk_PktData, 2);
	fn_enQ(&s_gsLink_TxQ,&s_tmp_gsLink_Pkt);
	return ;
}
/*****************************************************************************************************************************************/
uint8_t fn_gsLink_statusChange_from_Ctrl(struct_gsLinkPkt *mpkt)
{
	switch(mpkt->PacketData[0])
	{
		case LIGHT_STATUS:
			{
				DBG_PRINT("rcvd CntrlLightSattus\r\n");
				uint8_t m_packet_size ;
				sensorpayload_t  m_data;
				m_data.datacmd = LIGHT_STATUS;
				memcpy(m_data.data,&mpkt->PacketData[1],mpkt->DataLength-1);
				m_packet_size = mpkt->DataLength;		//plus one byte for packet number
				if(snsrAppData.broadCastComplete)
				{
					fn_enQ_blePkt(FT_STATUSCHANGE,m_packet_size,(uint8_t*)&m_data, snsrMinCfg.dest_addr);
				}
				return 1;
			}
		break;
		default:
		break;
	}
	return 0;
}
/*****************************************************************************************************************************************/
void fn_gsLink_statusChange_to_Ctrl(uint8_t cmd)
{
	static DataCmd_length m_dataLength = 0;
	{
		switch(cmd)
		{
			case PIR:
				memset(s_gsLinkGlobalVar.gslnk_PktData, '\0', MAX_CMDDATA_LEN);
				*s_gsLinkGlobalVar.gslnk_PktData = cmd;
				m_dataLength = CMD_STATCH_PIR;
				fn_fetchPIRstatus(s_gsLinkGlobalVar.gslnk_PktData+1);
			break;
			case ALS:
				memset(s_gsLinkGlobalVar.gslnk_PktData, '\0', sizeof(s_gsLinkGlobalVar.gslnk_PktData));
				*s_gsLinkGlobalVar.gslnk_PktData = cmd;
				m_dataLength = CMD_STATCH_ALS;
				fn_fetchALSstatus(s_gsLinkGlobalVar.gslnk_PktData+1);
			break;
			default:
			break;
		}
		fn_createGSLinkPkt(FT_STATUSCHANGE, sCtrlCfg.UUID ,s_gsLinkGlobalVar.gslnk_PktData, m_dataLength);
		fn_enQ(&s_gsLink_TxQ,&s_tmp_gsLink_Pkt);
		return ;
	}
}
/*****************************************************************************************************************************************/
void fn_fetchPIRstatus(uint8_t *m_databuf)
{
	if(snsrAppData.switchPIR_interrupt)
	{
		m_databuf[0] = snsrAppData.switchPIR_state;
		snsrAppData.switchPIR_interrupt = false;
	}
	else
	{
		m_databuf[0] = snsrCurrStatus.pir_State;
	}
	return;
}
/*****************************************************************************************************************************************/
void fn_fetchALSstatus(uint8_t *m_databuf)
{
	m_databuf[0] = (uint8_t)(snsrCurrStatus.als_LUXvalue);
	m_databuf[1] = (uint8_t)(snsrCurrStatus.als_LUXvalue>>8);
	return ;
}
/*****************************************************************************************************************************************/
uint8_t fn_gsLink_command(uint8_t *data,uint8_t dataLen)
{
	memset(s_gsLinkGlobalVar.gslnk_PktData, '\0', MAX_CMDDATA_LEN);
	memcpy(s_gsLinkGlobalVar.gslnk_PktData,data,dataLen);
	fn_createGSLinkPkt(FT_COMMAND, sCtrlCfg.UUID ,s_gsLinkGlobalVar.gslnk_PktData, dataLen);
	fn_enQ(&s_gsLink_TxQ,&s_tmp_gsLink_Pkt);
	return 0;
}
/*****************************************************************************************************************************************/
