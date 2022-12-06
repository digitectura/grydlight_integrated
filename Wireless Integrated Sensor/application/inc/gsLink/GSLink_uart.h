/*
 * GSLink_uart.h
 *
 *  Created on: 04-Oct-2020
 *      Author: BINDUSHREE M V
 */

#ifndef APPLICATION_GSLINK_GSLINK_UART_H_
#define APPLICATION_GSLINK_GSLINK_UART_H_

#include "main.h"
#include "GS_LINK.h"
#include "em_usart.h"

#ifdef SNSR_F1024KB
#define GSLINK_UART_CLK				cmuClock_USART1
#define GSLINK_UART_PERPHERAL		USART1
#define GSLINK_UART_INDEX 			1

#define GSLINK_RX_IRQ_HANDLER		USART1_RX_IRQHandler
#define GSLINK_RX_IRQn				USART1_RX_IRQn

#define GSLINK_TX_IRQ_HANDLER		USART1_TX_IRQHandler
#define GSLINK_TX_IRQn				USART1_TX_IRQn
#elif defined SNSR_F512KB_v_5_2
#define GSLINK_UART_CLK				cmuClock_USART0
#define GSLINK_UART_PERPHERAL		USART0
#define GSLINK_UART_INDEX 			0

#define GSLINK_RX_IRQ_HANDLER		USART0_RX_IRQHandler
#define GSLINK_RX_IRQn				USART0_RX_IRQn

#define GSLINK_TX_IRQ_HANDLER		USART0_TX_IRQHandler
#define GSLINK_TX_IRQn				USART0_TX_IRQn
#elif defined SNSR_F1024KB_v_5_2
#define GSLINK_UART_CLK				cmuClock_USART0
#define GSLINK_UART_PERPHERAL		USART0
#define GSLINK_UART_INDEX 			0

#define GSLINK_RX_IRQ_HANDLER		USART0_RX_IRQHandler
#define GSLINK_RX_IRQn				USART0_RX_IRQn

#define GSLINK_TX_IRQ_HANDLER		USART0_TX_IRQHandler
#define GSLINK_TX_IRQn				USART0_TX_IRQn
#endif


#define MAX_QUEUE_LEN				50

#define DATALENGTH					64

/*****************************************************************************************************************************************/
void fn_gslinkUART_Init(void);
void fn_enable_gsLink_TX_intrpt(void);
void fn_enable_gsLink_RX_intrpt(void);
void fn_disable_gsLink_RX_intrpt(void);
void fn_disable_gsLink_TX_intrpt(void);
void fn_uartTx_IT(USART_TypeDef *uart_n,uint8_t *pData, uint16_t Size);
/*****************************************************************************************************************************************/
#endif /* APPLICATION_GSLINK_GSLINK_UART_H_ */

