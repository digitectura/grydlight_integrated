/*
 * GS_LINK.h
 *
 *  Created on: Aug 19, 2020
 *      Author: User
 */
/*******************************************************************************************************************************/
#ifndef APPLICATION_GS_LINK_H_
#define APPLICATION_GS_LINK_H_
/*******************************************************************************************************************************/
#include "main.h"
#include "application/inc/command_handler.h"
#include "GSLink_uart.h"
/*******************************************************************************************************************************/
/*	Definitions	*/
#define		MAX_GSLINK_PKTS										25

#define 	PREAMBLEH  											0x03
#define		PREAMBLEL											0x56

#define		PREAMBLE_LEN										2
#define		FRAME_TYPE_LEN										1
#define		UUID_LEN											2
#define		HEADER_CRC_LEN										2
#define 	DATALEN_BYTE										1
#define		MAX_CMDDATA_LEN										40
#define		DATA_CRC_LEN										2

#define 	GSLINK_PKT_FRAME_HEADER_LEN 						(PREAMBLE_LEN+FRAME_TYPE_LEN+(2*UUID_LEN))		//len(PREAMBLE+FT+SRC_ADDR+DEST_ADDR)
#define		FRAME_HEADER_LEN	 								GSLINK_PKT_FRAME_HEADER_LEN + HEADER_CRC_LEN + DATALEN_BYTE

#define 	MAX_PKT_BUFFER_LEN									100

#define		RX_PROCESS_TIMEOUT		 							5		//secs
#define		RX_FRAME_TIMEOUT 									100		//msec

#define		TX_CMPLT_WAIT_DELAY 								10

#define		DEVICE_TYPE_INDEX_I_AM								0
#define		FW_VER_INDEX_I_AM									1

#define 	DEFAULT_CTRL_UUID									0
//#define TX_RX_CHECK		1

#define IS_RTS_HIGH() 							GPIO_PinOutGet(GSLink_RTS_PORT,GSLink_RTS_PIN)

#define PULL_RTS_LOW()							GPIO_PinOutClear(GSLink_RTS_PORT,GSLink_RTS_PIN)
#define PULL_RTS_HIGH()							GPIO_PinOutSet(GSLink_RTS_PORT,GSLink_RTS_PIN)

#define CHANGE_RTS_DIR_IP()						GPIO_PinOutClear(GSLink_RTS_DIR_PORT, GSLink_RTS_DIR_PIN)
#define CHANGE_RTS_DIR_OP()						GPIO_PinOutSet(GSLink_RTS_DIR_PORT, GSLink_RTS_DIR_PIN)


#define GS_LINK_CTL_TRANSMIT()					GPIO_PinOutSet(GSLink_CTRL_PORT, GSLink_CTRL_PIN )
#define GS_LINK_CTL_RECEIVE()					GPIO_PinOutClear(GSLink_CTRL_PORT, GSLink_CTRL_PIN)

/*******************************************************************************************************************************/
enum
{
	RTS_INPUT,
	RTS_OUTPUT
};
/*............................................................*/
enum
{
	TRANSMIT_INPROCESS,
	TRANSMIT_COMPLETE
};
/*............................................................*/
enum
{
	S_PREAMBLEH,
	S_PREAMBLEL,
	S_FRAMETYPE,
	S_SADD,
	S_DADD,
	S_HCRC,
	S_LEN,
	S_DATA,
	S_DCRC
};
/*............................................................*/
enum
{
	TXQ_EMPTY_CHECK ,
	TX_RTS_CHECK,
	TX_RANDOM_BCKOFF ,
	TRANSMIT_CHECK ,
	TRANSMIT_PKT ,
	TRANSMIT_CMPLT_CHECK,
	DEQ_TX_PKT,
};
/*............................................................*/
enum
{
	RXQ_EMPTY_CHECK,
	CHECK_FOR_CRC,
	CHECK_FOR_READY_STATE ,
	DEQ_RX_PKT,
	PROCESS_PKT_EVNT,
};
/*............................................................*/
enum
{
	PKT_REJECTED,
	PKT_PROCESSED
};
/*............................................................*/
enum
{
	ZERO,
	ONE,
	TWO,
	THREE,
	FOUR,
	FIVE,
	SIX,
	SEVEN,
	EIGHT,
	NINE,
	TEN
};
/*............................................................*/
enum
{
	GPIO_PULL_DOWN,
	GPIO_PULL_UP
};
/*............................................................*/
/*******************************************************************************************************************************/
typedef struct
{
	uint8_t preambleH;
	uint8_t preambleL;
	uint8_t frameType;
	uint16_t sourceAddress;
	uint16_t destinationAddress;
	uint8_t HeaderCRCL;
	uint8_t HeaderCRCH;
	uint8_t DataLength;
	uint8_t PacketData[MAX_CMDDATA_LEN];
	uint8_t DataCRCL;
	uint8_t DataCRCH;
}__attribute__((__packed__))struct_gsLinkPkt;
/*............................................................*/
typedef struct
{
	uint8_t (*gsLink_cmdParser)(void*);
}__attribute__((__packed__))struct_gsLinkCmdParser;
/*............................................................*/
typedef struct
{
	int8_t head;
	int8_t tail;
	uint8_t num_pkts;
	struct_gsLinkPkt gsLinkPkt[MAX_GSLINK_PKTS];
}__attribute__((__packed__))struct_gsLinkPktQ;
/*............................................................*/
typedef struct
{
	/*queue rx pkt fn*/
	uint8_t rx_frameState;
	uint16_t rx_frameTimeOut;
	uint8_t Indx;
	uint8_t is_addrFirstByte;
	uint8_t is_crcFirstByte;

	/*process rxQ fn*/
	uint8_t rx_process_State;
	uint16_t rx_processTimeOut;

	/*process txQ fn*/
	uint8_t tx_process_State;
	uint16_t randmBckOff_StartTime;
	uint16_t randomNumber;
	uint8_t txCmplt;
	uint16_t txTimeout;
	uint8_t txPkt[MAX_PKT_BUFFER_LEN];

	/*whois fn*/
	uint8_t whois_cmdPrcs_State;
	uint8_t statusChngPrcs_State;
	uint8_t gsLinkCmd_Prcs_State;

	/*buffer for queuing the pkt into txQ*/
	uint8_t gslnk_PktData[MAX_CMDDATA_LEN];

}__attribute__((__packed__))struct_gsLinkGlobalVar;
/*............................................................*/
/*******************************************************************************************************************************/
extern struct_gsLinkPkt  				s_tmp_gsLink_rxPkt;
extern struct_gsLinkPkt  				s_tmp_gsLink_Pkt;
extern struct_gsLinkGlobalVar			s_gsLinkGlobalVar;
/*............................................................*/
extern struct_gsLinkPktQ 				s_gsLink_TxQ;
extern struct_gsLinkPktQ 				s_gsLink_RxQ;
/*******************************************************************************************************************************/
void 	fn_process_TxPkts			(void);
void 	fn_process_RxPkts			(void);
void 	fn_changeRTSdirection		(uint8_t m_dir);
void	fn_gsLinkRxTimeout_task		(void);
void 	fn_queue_gsLink_RxPkt		(uint8_t frameByte);
void 	fn_queue_gsLink_TxPkt		(struct_gsLinkPkt *gsLink_pkt);
void 	fn_transmitPacket			(void);
void 	fn_resetGSlinkRxState		(void);
void 	fn_GSLinkTimeoutHandler		(void);

uint8_t fn_CRCcheck(struct_gsLinkPkt *gsLink_pkt);
uint16_t fn_gen_crc16(const uint8_t *data, uint8_t size);
uint8_t fn_createGSLinkPkt(uint8_t ft, uint16_t dstAddr, uint8_t *pktData,uint8_t dataLen);

uint8_t fn_gsLink_whois(struct_gsLinkPkt *mpkt);
uint8_t fn_gsLink_Iam(struct_gsLinkPkt *mpkt);
void fn_gsLink_Iam_toCntrl(void);

uint8_t fn_gsLink_command(uint8_t *data,uint8_t dataLen);
void fn_fetchPIRstatus(uint8_t *m_databuf);
void fn_fetchALSstatus(uint8_t *m_databuf);
void fn_broadCast_SnsrIam_Pckt(void);

void fn_gsLink_statusChange_to_Ctrl(uint8_t cmd);
uint8_t fn_gsLink_statusChange_from_Ctrl(struct_gsLinkPkt* mpkt);
/*******************************************************************************************************************************/
#endif /* APPLICATION_GS_LINK_H_ */
