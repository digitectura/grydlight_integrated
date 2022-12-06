/****************************************************************************************************************/
#include "main.h"
#include "application/inc/app.h"
#include "application/inc/gsLink/GS_Link.h"
#include "application/inc/BLE_Queue.h"
#include "application/inc/dali/daliCommand.h"
#include "application/inc/analog/mcp45hvx1.h"
#include "application/inc/intervalTimer.h"
/****************************************************************************************************************/
bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);

uint8_t mux_control_select = 0;		//	O = DALI & 1 = ANALOG

int main()
{
	// Initialize device
	fn_deviceInit();
#ifdef DALI_SPACE							//on bootup delay of 5 sec. As the ESP is taking time to derive the i2c_SDA pin high
	uint16_t bootup_delay = fn_GetSecTimerStart();
#endif

	struct gecko_msg_flash_ps_load_rsp_t* rsp = gecko_cmd_flash_ps_load(MUX);
	if(!rsp->result)
	{
		memcpy(&mux_control_select,&rsp->value.data,1);
		printf("Mux_Control_select = %d\r\n", mux_control_select);
		fn_mux_init();
	}
	else
	{
		printf("Mux PS Load Response = %04x\r\n", rsp->result);
	}

	if(mux_control_select == 1)					//Analog
	{
		printf("Analog I2C init done \r\n");
		initI2C1_Analog();
	}
	fn_initI2C();		//	ALS and Temperature Humidity Sensor I2C...

	printf("Restoring sensor configuration and details from PS \r\n ");
	fn_snsRestore();
	printf("System bootup Delay ... 15 secs\r\n ");
	while(!(fn_IsSecTimerElapsed(bootup_delay, 15)));		//	Muruga	-	Changed bootup delay from 5 sec to 15 seconds
	//pir_enable();
	while (1)
	{

		struct gecko_cmd_packet *evt = gecko_peek_event();
		bool pass = mesh_bgapi_listener(evt);
		if (pass)
		{
			handle_gecko_event(BGLIB_MSG_ID(evt->header), evt);
		}
		fn_ctrlHandshake();
		fn_SnsrPrcss();

		fn_AreaControl();

		if(mux_control_select == 0)					//DALI
		{
			fn_daliProcess();
			fn_updateDaliConfig();
			fn_ProcessDaliRetention();
			fn_dali_dimming();
		}
		else if(mux_control_select == 1)			//Analog
		{
			fn_IdenitfyLight();
			fn_AnalogFadeCntrl();
		}
	}
	return 0;
}
/****************************************************************************************************************/
