/*
 * i2c.h
 *
 *  Created on: 28-May-2020
 *      Author: BINDUSHREE M V
 */
//*******************************************************************************************************************//
#ifndef APPLICATION_INC_APP_I2C_H_
#define APPLICATION_INC_APP_I2C_H_
//*******************************************************************************************************************//
#include "main.h"
#include "em_i2c.h"

#define I2C_TXBUFFER_SIZE                 30
#define I2C_RXBUFFER_SIZE                 20

extern uint8_t i2c_txBuffer[I2C_TXBUFFER_SIZE] ;
extern uint8_t i2c_rxBuffer[I2C_RXBUFFER_SIZE] ;

enum
{
	TX_INCMPLT = false,
	TX_CMPLT = true
};
enum
{
	RX_INCMPLT = 0,
	RX_CMPLT
};
extern uint8_t eI2C_rx_state;
extern uint8_t eI2C_tx_state;
//*******************************************************************************************************************//
void fn_initI2C(void);
void initI2C1_Analog(void);
uint8_t fn_I2C_MasterReadonly(I2C_TypeDef *m_i2cH, uint16_t slaveAddress, uint8_t *rxBuff, uint8_t numBytes);
int8_t fn_I2C_MasterRead(I2C_TypeDef *m_i2cH,uint8_t i2c_slaveAddress, uint8_t reg_addr, uint8_t *buf, uint8_t len);
uint8_t fn_I2C_MasterWriteonly(I2C_TypeDef *m_i2cH, uint16_t slaveAddress, uint8_t *txBuff, uint8_t numBytes);
int8_t fn_I2C_MasterWrite(I2C_TypeDef *m_i2cH, uint8_t i2c_slaveAddress, uint8_t reg_addr, uint8_t *buf, uint8_t len);
//*******************************************************************************************************************//
#endif /* APPLICATION_INC_APP_I2C_H_ */
//*******************************************************************************************************************//
