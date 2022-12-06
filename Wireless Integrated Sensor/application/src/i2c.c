/* * app_i2c.c
 *
 *  Created on: 28-May-2020
 *      Author: BINDUSHREE M V
 */
//*******************************************************************************************************************//
#include "application/inc/i2c.h"
//*******************************************************************************************************************//
uint8_t i2c_txBuffer[I2C_TXBUFFER_SIZE] = {[0 ... I2C_TXBUFFER_SIZE-1] = 0};
uint8_t i2c_rxBuffer[I2C_RXBUFFER_SIZE] = {[0 ... I2C_RXBUFFER_SIZE-1] = 0};

uint8_t eI2C_rx_state = RX_INCMPLT;
uint8_t eI2C_tx_state = TX_INCMPLT;
//*******************************************************************************************************************//
void fn_initI2C(void)
{
	// Using default settings
	I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;

	// Using PC5 (SDA) and PC4 (SCL)
	GPIO_PinModeSet(ALS_TH_I2C_SDA_PORT, ALS_TH_I2C_SDA_PIN, gpioModeWiredAndPullUpFilter, 1);
	GPIO_PinModeSet(ALS_TH_I2C_SCL_PORT, ALS_TH_I2C_SCL_PIN, gpioModeWiredAndPullUpFilter, 1);

	// Route GPIO pins to I2C module
	GPIO->I2CROUTE[ALS_TH_I2C_IDX].SDAROUTE = (GPIO->I2CROUTE[ALS_TH_I2C_IDX].SDAROUTE & ~_GPIO_I2C_SDAROUTE_MASK)
						| (ALS_TH_I2C_SDA_PORT << _GPIO_I2C_SDAROUTE_PORT_SHIFT
						| (ALS_TH_I2C_SDA_PIN << _GPIO_I2C_SDAROUTE_PIN_SHIFT));
	GPIO->I2CROUTE[ALS_TH_I2C_IDX].SCLROUTE = (GPIO->I2CROUTE[ALS_TH_I2C_IDX].SCLROUTE & ~_GPIO_I2C_SCLROUTE_MASK)
						| (ALS_TH_I2C_SCL_PORT << _GPIO_I2C_SCLROUTE_PORT_SHIFT
						| (ALS_TH_I2C_SCL_PIN << _GPIO_I2C_SCLROUTE_PIN_SHIFT));
	GPIO->I2CROUTE[ALS_TH_I2C_IDX].ROUTEEN = GPIO_I2C_ROUTEEN_SDAPEN | GPIO_I2C_ROUTEEN_SCLPEN;

	// Initializing the I2C
	I2C_Init(ALS_TH_I2C_HANDLE, &i2cInit);

	ALS_TH_I2C_HANDLE->CTRL = I2C_CTRL_AUTOSN;
	I2C_Enable(ALS_TH_I2C_HANDLE, true);
}
//*******************************************************************************************************************//
void initI2C1_Analog(void){
	  // Using default settings
	  I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;

	  // Using PA5 (SDA) and PA6 (SCL)
	  GPIO_PinModeSet(ANALOG_I2C_SDA_PORT, ANALOG_I2C_SDA_PIN, gpioModeWiredAndPullUpFilter, 1);
	  GPIO_PinModeSet(ANALOG_I2C_SCL_PORT, ANALOG_I2C_SCL_PIN, gpioModeWiredAndPullUpFilter, 1);

	  // Route GPIO pins to I2C module

	  GPIO->I2CROUTE[ANALOG_I2C_IDX].SDAROUTE = (GPIO->I2CROUTE[ANALOG_I2C_IDX].SDAROUTE & ~_GPIO_I2C_SDAROUTE_MASK)
	                        | (ANALOG_I2C_SDA_PORT << _GPIO_I2C_SDAROUTE_PORT_SHIFT
	                        | (ANALOG_I2C_SDA_PIN << _GPIO_I2C_SDAROUTE_PIN_SHIFT));
	  GPIO->I2CROUTE[ANALOG_I2C_IDX].SCLROUTE = (GPIO->I2CROUTE[ANALOG_I2C_IDX].SCLROUTE & ~_GPIO_I2C_SCLROUTE_MASK)
	                        | (ANALOG_I2C_SCL_PORT << _GPIO_I2C_SCLROUTE_PORT_SHIFT
	                        | (ANALOG_I2C_SCL_PIN << _GPIO_I2C_SCLROUTE_PIN_SHIFT));
	  GPIO->I2CROUTE[ANALOG_I2C_IDX].ROUTEEN = GPIO_I2C_ROUTEEN_SDAPEN | GPIO_I2C_ROUTEEN_SCLPEN;

	  // Initializing the I2C
	  I2C_Init(ANALOG_I2C_HANDLE, &i2cInit);

	  ANALOG_I2C_HANDLE->CTRL = I2C_CTRL_AUTOSN;
	  I2C_Enable(ANALOG_I2C_HANDLE, true);
	  DBG_PRINT("Analog I2C (Re)Initialised \r\n");
}

//*******************************************************************************************************************//
uint8_t fn_I2C_MasterReadonly(I2C_TypeDef *m_i2cH, uint16_t slaveAddress, uint8_t *rxBuff, uint8_t numBytes)
{
	// Transfer structure
	I2C_TransferSeq_TypeDef i2cTransfer;
	I2C_TransferReturn_TypeDef result;

	// Initializing I2C transfer
	i2cTransfer.addr          = slaveAddress;
	i2cTransfer.flags         = I2C_FLAG_READ; // must write target address before reading
	i2cTransfer.buf[0].data   = rxBuff;
	i2cTransfer.buf[0].len    = numBytes;
	i2cTransfer.buf[1].data   = NULL;
	i2cTransfer.buf[1].len    = 0;

	result = I2C_TransferInit(m_i2cH, &i2cTransfer);

	// Sending data
	volatile uint32_t timeout = 80000;
	while (result == i2cTransferInProgress && --timeout > 0)
	{
		result = I2C_Transfer(m_i2cH);
	}

	if( (timeout == 0) || (result != i2cTransferDone) )
	{
		memset(i2c_rxBuffer,'\0',sizeof(i2c_rxBuffer));
		eI2C_rx_state = RX_INCMPLT;
		return false;
	}
	eI2C_rx_state = RX_CMPLT;
	return true;
}
//*******************************************************************************************************************//
uint8_t fn_I2C_MasterWriteonly(I2C_TypeDef *m_i2cH, uint16_t slaveAddress, uint8_t *txBuff, uint8_t numBytes)
{
	// Transfer structure
	I2C_TransferSeq_TypeDef i2cTransfer;
	I2C_TransferReturn_TypeDef result;

	// Initializing I2C transfer
	i2cTransfer.addr          = slaveAddress;
	i2cTransfer.flags         = I2C_FLAG_WRITE;
	i2cTransfer.buf[0].data   = txBuff;
	i2cTransfer.buf[0].len    = numBytes;
	i2cTransfer.buf[1].data   = NULL;
	i2cTransfer.buf[1].len    = 0;

	result = I2C_TransferInit(m_i2cH, &i2cTransfer);

	// Sending data
	volatile uint32_t timeout = 80000;
	while (result == i2cTransferInProgress  && --timeout > 0)
	{
		result = I2C_Transfer(m_i2cH);
	}
	if(timeout == 0 || (result != i2cTransferDone) )
	{
		memset(i2c_txBuffer,'\0',sizeof(i2c_txBuffer));
		eI2C_tx_state = TX_INCMPLT;
		return false;
	}

	eI2C_tx_state = TX_CMPLT;
	return true;
}
//*******************************************************************************************************************//
int8_t fn_I2C_MasterWrite(I2C_TypeDef *m_i2cH, uint8_t i2c_slaveAddress, uint8_t reg_addr, uint8_t *buf, uint8_t len)
{
	I2C_TransferSeq_TypeDef i2cTransfer;
	I2C_TransferReturn_TypeDef result;
	uint8_t txBuffer[I2C_TXBUFFER_SIZE + 1];

	  txBuffer[0] = reg_addr;
	  for(int i = 0; i < len; i++)
	  {
	      txBuffer[i + 1] = buf[i];
	  }

	  // Initializing I2C transfer
	  i2cTransfer.addr          = i2c_slaveAddress << 1;
	  i2cTransfer.flags         = I2C_FLAG_WRITE;
	  i2cTransfer.buf[0].data   = txBuffer;
	  i2cTransfer.buf[0].len    = len + 1;
	  i2cTransfer.buf[1].data   = NULL;
	  i2cTransfer.buf[1].len    = 0;

	/*	i2cTransfer.addr          = i2c_slaveAddress;					// Initializing I2C transfer
		i2cTransfer.flags         = I2C_FLAG_WRITE;
		i2cTransfer.buf[0].data   = &reg_addr;
		i2cTransfer.buf[0].len    = 1;
		i2cTransfer.buf[1].data   = buf;
		i2cTransfer.buf[1].len    = len;*/

	  result = I2C_TransferInit(m_i2cH, &i2cTransfer);
	  volatile uint32_t timeout = 80000;
	  while (result == i2cTransferInProgress && --timeout > 0)
	  {
	    result = I2C_Transfer(m_i2cH);
	  }
	  if((timeout == 0) || (result != i2cTransferDone))
	  {
		  return false;
	  }
	return true;
}
int8_t fn_I2C_MasterRead(I2C_TypeDef *m_i2cH, uint8_t i2c_slaveAddress, uint8_t reg_addr, uint8_t *buf, uint8_t len)
{
  I2C_TransferSeq_TypeDef i2cTransfer;
  I2C_TransferReturn_TypeDef result;

  //uint8_t address[2] ={0x7C,0xA2};

  // Initializing I2C transfer
  i2cTransfer.addr          = i2c_slaveAddress;
  i2cTransfer.flags         = I2C_FLAG_WRITE_READ; // must write target address before reading
  i2cTransfer.buf[0].data   = &reg_addr;
  i2cTransfer.buf[0].len    = 1;
  i2cTransfer.buf[1].data   = buf;
  i2cTransfer.buf[1].len    = len;

  result = I2C_TransferInit(m_i2cH, &i2cTransfer);

  // Sending data
  volatile uint32_t timeout = 80000;
  while (result == i2cTransferInProgress && --timeout > 0)
  {
    result = I2C_Transfer(m_i2cH);
  }

  if((result != i2cTransferDone) || (timeout == 0))
  {
	  DBG_PRINT("I2C Read unsuccessful\n");
	  return false;
  }
  return true;
}
