/*
 * GSLink_uart.c
 *
 *  Created on: 04-Oct-2020
 *      Author: BINDUSHREE M V
 */
/*******************************************************************************************************************************************************/
#include "application/inc/gsLink/GSLink_uart.h"
#include "em_usart.h"
#include "udelay.h"
/*******************************************************************************************************************************************************/
// Size of the buffer for received data
#define BUFLEN  80

// Receive data buffer
uint8_t uart_txBuf[BUFLEN];

// Current position ins buffer
uint16_t uart_txBufSize = 0;
uint16_t uart_txBufPos = 0;

uint8_t gsLink_rxBuf = 0;
/*******************************************************************************************************************************************************/
/**************************************************************************//**
 * @brief Intializes UART
 *****************************************************************************/
void fn_gslinkUART_Init(void)
{
	/* Enable peripheral clocks */
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(GSLINK_UART_CLK, true);

	GPIO_PinModeSet(GSLink_RTS_DIR_PORT,GSLink_RTS_DIR_PIN,gpioModePushPull,1);
	GPIO_PinModeSet(GSLink_CTRL_PORT,GSLink_CTRL_PIN,gpioModePushPull,1);
	GPIO_PinModeSet(GSLink_RTS_PORT,GSLink_RTS_PIN,gpioModeInput,1);


	/*	initializing the RTS and CTL direction of GSLink to rcv(default) mode*/
	PULL_RTS_HIGH();
	GS_LINK_CTL_RECEIVE();
	fn_changeRTSdirection(RTS_INPUT);

	/* To avoid false start, configure output as high */
 	GPIO_PinModeSet(GSLink_UART_TX_PORT, GSLink_UART_TX_PIN, gpioModePushPull, 0);
	GPIO_PinModeSet(GSLink_UART_RX_PORT, GSLink_UART_RX_PIN, gpioModeInput, 0);

	USART_InitAsync_TypeDef init   = USART_INITASYNC_DEFAULT;
	/* Configure USART for basic async operation */
	init.enable = usartDisable;
	USART_InitAsync(GSLINK_UART_PERPHERAL, &init);

	GPIO->USARTROUTE[GSLINK_UART_INDEX].TXROUTE =
			(GSLink_UART_TX_PORT << _GPIO_USART_TXROUTE_PORT_SHIFT)
			| (GSLink_UART_TX_PIN << _GPIO_USART_TXROUTE_PIN_SHIFT);
	GPIO->USARTROUTE[GSLINK_UART_INDEX].RXROUTE =
			(GSLink_UART_RX_PORT << _GPIO_USART_RXROUTE_PORT_SHIFT)
			| (GSLink_UART_RX_PIN << _GPIO_USART_RXROUTE_PIN_SHIFT);

	// Enable RX and TX signals now that they have been routed
	  GPIO->USARTROUTE[GSLINK_UART_INDEX].ROUTEEN =
			  GPIO_USART_ROUTEEN_RXPEN | GPIO_USART_ROUTEEN_TXPEN;

	/* Finally enable it */
	USART_Enable(GSLINK_UART_PERPHERAL, usartEnable);

	// Enable NVIC USART sources
	NVIC_ClearPendingIRQ(GSLINK_RX_IRQn);
	NVIC_EnableIRQ(GSLINK_RX_IRQn);
	NVIC_ClearPendingIRQ(GSLINK_TX_IRQn);
	NVIC_EnableIRQ(GSLINK_TX_IRQn);

    // Enable receive data valid interrupt
	fn_enable_gsLink_RX_intrpt();
	UDELAY_Calibrate();
	return ;
}
/*****************************************************************************************************************************************/
void fn_enable_gsLink_TX_intrpt(void)
{
    // Enable transmit buffer level interrupt
    USART_IntEnable(GSLINK_UART_PERPHERAL, USART_IEN_TXBL);
	return ;
}
void fn_enable_gsLink_RX_intrpt(void)
{
    // Enable transmit buffer level interrupt
    USART_IntEnable(GSLINK_UART_PERPHERAL, USART_IEN_RXDATAV);
	return ;
}
void fn_disable_gsLink_RX_intrpt(void)
{
    // Enable transmit buffer level interrupt
	USART_IntDisable(GSLINK_UART_PERPHERAL, USART_IEN_RXDATAV);
	return ;
}
void fn_disable_gsLink_TX_intrpt(void)
{
    // Enable transmit buffer level interrupt
	USART_IntDisable(GSLINK_UART_PERPHERAL, USART_IEN_TXBL);
	return ;
}
/*****************************************************************************************************************************************/
void fn_uartTx_IT(USART_TypeDef *uart_n,uint8_t *pData, uint16_t Size)
{
	memcpy(uart_txBuf,pData,Size);
	uart_txBufSize = Size;
	uart_txBufPos = 0;
    // Enable transmit buffer level interrupt
    fn_enable_gsLink_TX_intrpt();
	return ;
}
/*****************************************************************************************************************************************/
#ifdef SNSR_F1024KB
void USART1_RX_IRQHandler(void)
{
	gsLink_rxBuf = USART1->RXDATA;
	fn_queue_gsLink_RxPkt(gsLink_rxBuf);

// Clear the requesting interrupt before exiting the handler
	USART_IntClear(USART1, USART_IF_RXDATAV);
	return;
}
/*****************************************************************************************************************************************/
void USART1_TX_IRQHandler(void)
{
	if(uart_txBufPos < uart_txBufSize)
	{
		USART1->TXDATA = uart_txBuf[uart_txBufPos++];
	}
	else
	{
		fn_disable_gsLink_TX_intrpt();
		s_gsLinkGlobalVar.txCmplt = TRANSMIT_COMPLETE;
	}
  // Clear the requesting interrupt before exiting the handler
	USART_IntClear(USART1, USART_IF_TXBL);
	return ;
}
#elif defined SNSR_F1024KB_v_5_2
/*****************************************************************************************************************************************/
void USART0_RX_IRQHandler(void)
{
	gsLink_rxBuf = USART0->RXDATA;
	fn_queue_gsLink_RxPkt(gsLink_rxBuf);

// Clear the requesting interrupt before exiting the handler
	USART_IntClear(USART0, USART_IF_RXDATAV);
	return;
}
/*****************************************************************************************************************************************/
void USART0_TX_IRQHandler(void)
{
	if(uart_txBufPos < uart_txBufSize)
	{
		USART0->TXDATA = uart_txBuf[uart_txBufPos++];
	}
	else
	{
		fn_disable_gsLink_TX_intrpt();
		s_gsLinkGlobalVar.txCmplt = TRANSMIT_COMPLETE;
	}
  // Clear the requesting interrupt before exiting the handler
	USART_IntClear(USART0, USART_IF_TXBL);
	return ;
}
#elif defined SNSR_F512KB_v_5_2
/*****************************************************************************************************************************************/
void USART0_RX_IRQHandler(void)
{
	gsLink_rxBuf = USART0->RXDATA;
	fn_queue_gsLink_RxPkt(gsLink_rxBuf);

// Clear the requesting interrupt before exiting the handler
	USART_IntClear(USART0, USART_IF_RXDATAV);
	return;
}
/*****************************************************************************************************************************************/
void USART0_TX_IRQHandler(void)
{
	if(uart_txBufPos < uart_txBufSize)
	{
		USART0->TXDATA = uart_txBuf[uart_txBufPos++];
	}
	else
	{
		fn_disable_gsLink_TX_intrpt();
		s_gsLinkGlobalVar.txCmplt = TRANSMIT_COMPLETE;
	}
  // Clear the requesting interrupt before exiting the handler
	USART_IntClear(USART0, USART_IF_TXBL);
	return ;
}
#endif
/*****************************************************************************************************************************************/
/*****************************************************************************************************************************************/
