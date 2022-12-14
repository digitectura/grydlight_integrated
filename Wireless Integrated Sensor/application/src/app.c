#include "application/inc/morphous_Snsr_Board.h"
#include <application/inc/app.h>
#include "application/inc/intervalTimer.h"
#include "application/inc/gsLink/GS_Link.h"
#include "application/inc/S1_pir.h"
#include "application/inc/command_handler.h"
#include "application/inc/BLE_Queue.h"
#include "application/inc/switch.h"
#include "application/inc/dali/daliCommand.h"
#include "application/inc/analog/mcp45hvx1.h"
#include "application/inc/dali/daliApplication.h"


//Self Provisioning
#include "sl_se_manager_util.h"
#include "application/inc/Self-Provision_Functions.h"
#include "bg_types.h"
#include "em_system.h"
/****************************************************************************************************************/
// Flag for indicating DFU Reset must be performed
uint8_t boot_to_dfu = 0;

extern uint8_t g_loopMapIndex ;
extern uint8_t g_loopInidcationCnt;
extern uint16_t g_switchPressed_StartTime;
extern uint16_t g_loopMapStratTime;

//FRameTypes
const uint8_t gc_opcodes[] = {
								FT_WHOIS,
								FT_IAM,
								FT_DATAREQ,
								FT_DATARES,
								FT_STATUSCHANGE,
								FT_COMMISSIONING,
								FT_COMMAND,
								FT_ACK,
								FT_NACK,
								FT_WL_LIGHT_COMMANDS,
								FT_DATA_STREAM,
								FT_EVENT,
								FT_PING,
								FT_DALI_CUSTOM,
								FT_IAM_SWITCH,
								FT_CUSTOM_CMD
							};
/****************************************************************************************************************/
snsrAppData_t snsrAppData 							= 	{
															.identify 						= 0 ,
															.PIR_stabilized					= 0 ,
															.switchPIR_interrupt 			= false ,
															.switchPIR_state				= UNOCCUPIED ,
															.broadCastComplete				= false ,
															.pirRetransmitTimerStart 		= 1 ,
															.thRetransmitTimerStart			= 1 ,
															.alsRetransmitTimerStart		= 1,
															.switchState					= SWITCH_RELEASED,
															.global_daliStatus				= 0,
															.WL_LIGHT_RcvdExternalCmd_flag 	= 0,
															.WL_LIGHT_CONTROL_2_flag		= 0,
															.ForcedReq						= false
														};

snsrMinCfg_t	 snsrMinCfg 	 					= 	{
															.configversion 		= CONFIG_VER,
															.firmwareversion 	= FW_VER,
															.device_type 		= 0x00,
															.dest_addr			= DEFAULT_DEST_ADDR,
															.snsrID				= DEFAULT_SHID,
															.cfgAvailable		= false,
															.aggBinded			= false,
															.lghtMpng			= {
															.loopMap			= BOTH_LOOP,
															.reqLux				= DEFAULT_REQ_LUX,
															.gain				= DEFAULT_GAIN_FACTOR
														  }
														};
snsrCfg_t		snsrCfg								=	{
															.pir_cfg			= 	{
																						.unoccupancyTimer_s 	= DEFAULT_PIR_UNOCCUPANCY_TIME_5 ,
																						.wait_watch_Time_ms 	= DEFAULT_PIR_WAIT_AND_WATCH_TIME ,
																						.retransmission_timeout	=  DEFAULT_PIR_RETRANSMIT_TIME ,
																						.cfg_id					= CFG_5
																					},
															.als_cfg			= 	{
																						.freq_LUXmeasure_s		= DEFAULT_ALS_FREQ_OF_MEASURMT_1 ,
																						.luxThreshold			= DEFAULT_ALS_LUX_THRESHOLD ,
																						.calibration_factor		= DEFAULT_ALS_CALBFACTOR ,
																						.retransmission_timeout	= DEFAULT_ALS_RETRANSMIT,
																						.cfg_id					= CFG_1
																					},
															.th_cfg				=	{
																						.freq_THmeasure			= DEFAULT_TH_FREQ_OF_MEASURMT ,
																						.tempThreshold			= DEFAULT_TH_THRESHOLD ,
																						.humidityThreshold		= DEFAULT_HUM_THRESHOLD ,
																						.retransmission_timeout = DEFAULT_TH_RETRANSMIT_TIME ,
																						.cfg_id					= CFG_1
																					},
															.WL_LIGHT_RcvdExternalCmd_timer = DEFAULT_SNSR_MASKING_TIMEOUT,
															.WL_LIGHT_CONTROL_2_timer		= 60,    		//minutes
															.emergency_light				= false
														};
snsrmap_CntrlCredntials_t sCtrlCfg					=	{
															.UUID				= DEFAULT_SHID,
															.cntrl_type			= 0,
															.ctrlFWver			= 0
														};
snsrStatus_t	snsrCurrStatus 						= 	{
															.pir_State			= UNOCCUPIED,
															.als_LUXvalue		= 0,
															.temp				= 0,
															.humidity			= 0
														};
triacCfg_t		triacCfg							= 	{
															.triacStatus		= false
														};
snsrStatus_t	snsrPrevStatus						=   {
															.pir_State			= UNOCCUPIED,
															.als_LUXvalue		= 0,
															.temp				= 0,
															.humidity			= 0
														};														
struct_sceneConfig sSceneCfg[MAX_SCENES] 			= { 0 };

#ifdef AREA
struct_Configure_Area_t sConfigArea[MAX_SENSOR_IN_GRP]				= { 0 };
struct_Config_Area_Parameters_t sConfigAreaParameters				= {
																		.TotalArea_Sensors_count = 0,
																		.Area_Group_ID = 0,
																		.SensorIsPartOfArea = 0
																	};
uint8_t AreaSensorCount = 0;
#endif

struct_alsLinearCalib sAlsCalibValue				= {
															.c = 0.0,
															.m = 1.0,
															.m_gain = DEFAULT_GAIN_FACTOR,
															.req_lux = DEFAULT_REQ_LUX,
													  };

uint8_t debugPrints = 1;

#ifdef SELF_PROVISIONING
struct_Keys_Msc_t s_keys_msc = {
									.self_provisioned_flag = 0,
									.s_App_Net_Key[0 ... 15].index_no	= 0,
									.s_App_Net_Key[0 ... 15].netkeys	= {[0 ... 15] = 0},
									.s_App_Net_Key[0 ... 15].appkeys    = {4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7}
							   };

#define USERDATA ((uint32_t*)USERDATA_BASE)
uint8_t keys_counter = 0;

sl_se_command_context_t cmd_ctx;
#endif
/****************************************************************************************************************/
void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt)
{
	uint16 result;
	if (NULL == evt)
	{
		return;
	}
	switch (evt_id)
	{
		case gecko_evt_system_boot_id:
		{
			DBG_PRINT("system boot_id evnt\r\n");
			// Initialize Mesh stack in Node operation mode, it will generate initialized event
			result = gecko_cmd_mesh_node_init()->result;
			if (result)
			{
				DBG_PRINT("mesh node init failed 0x%x\r\n",result);
				gecko_cmd_system_reset(NORMAL_RESET);
			}
			/*gecko_cmd_system_halt(1);
			struct gecko_msg_system_set_tx_power_rsp_t *txpower = gecko_cmd_system_set_tx_power(150);
			DBG_PRINT("tx power :%d dB\r\n",txpower->set_power/10);
			gecko_cmd_le_gap_set_advertise_tx_power(0,100);
			gecko_cmd_system_halt(0);*/
		}
		break;
		/*............................................................................................*/
		case gecko_cmd_mesh_friend_init_id:
			DBG_PRINT("frnd mesh node init id event\r\n");
		break;
		/*............................................................................................*/
		case gecko_evt_hardware_soft_timer_id:
			fn_process_HardwareSoftTimer_Evnt(evt);
		break;
		/*............................................................................................*/
		case gecko_evt_mesh_node_initialized_id:
		      for(uint8_t i = 0; i<=15; i++){
		       DBG_PRINT("%02x ", gecko_cmd_mesh_node_get_uuid()->uuid.data[i]);
		      }
			DBG_PRINT("Sensor node initialized\n");
			DBG_PRINT("FWver = %d/10\r\n",snsrMinCfg.firmwareversion);
			// Initialize generic client models
			result = gecko_cmd_mesh_generic_client_init()->result;
			if (result)
			{
				DBG_PRINT("mesh_generic_client_init failed, code 0x%x\r\n", result);
			}
			struct gecko_msg_mesh_node_initialized_evt_t *pData = (struct gecko_msg_mesh_node_initialized_evt_t*)&(evt->data);
			struct gecko_msg_mesh_vendor_model_init_rsp_t *vendorresponse;
			vendorresponse = gecko_cmd_mesh_vendor_model_init(PRIMARY_ELEMENT,
															  MY_VENDOR_ID,
															  MY_MODEL_ID,
															  PUBLISH,
															  sizeof(gc_opcodes),
															  gc_opcodes);

			DBG_PRINT("vendorModel init response:%d\r\n",vendorresponse->result);

			gecko_cmd_system_halt(1);
			struct gecko_msg_system_set_tx_power_rsp_t *txpower = gecko_cmd_system_set_tx_power(CFG_OUTPUT_POWER*10);
			printf("Transmit Power : %ddB\r\n", txpower->set_power/10);
			gecko_cmd_system_halt(0);

			if (pData->provisioned)
			{
				DBG_PRINT("node is provisioned. addr : %u[0x%04x], ivi:%lu\r\n", pData->address,pData->address, pData->ivi);
				snsrMinCfg.snsrID = pData->address;

				result = gecko_cmd_mesh_friend_init()->result;
				if (result) {
					DBG_PRINT("[E] Friend init failed 0x%x\r\n", result);
				}

			}
			else
			{
#ifdef SELF_PROVISIONING
				s_keys_msc.self_provisioned_flag = (*(uint32_t *)(USERDATA_BASE));
				if(s_keys_msc.self_provisioned_flag == 0xFFFFFFFF)
				{
					s_keys_msc.self_provisioned_flag = 0x12091998;

					// Clear the UserData page of any previous data stored
					sl_se_erase_user_data(&cmd_ctx);

					// Write the value into the 4th word of the Userdata portion of the flash
					sl_se_write_user_data(&cmd_ctx, 0, &s_keys_msc.self_provisioned_flag, 4);

					DBG_PRINT("Self Provisioning\r\n");
					Unicast = (uint16_t)(SYSTEM_GetUnique() & 0x7FFF);
					Self_Provision_Device();
				}
				else
				{
#endif
				DBG_PRINT("node is unprovisioned, starting beaconing...\r\n");
				gecko_cmd_mesh_node_start_unprov_beaconing(0x3);
#ifdef SELF_PROVISIONING
				}
#endif
			}
			gecko_cmd_hardware_set_soft_timer(MILLISECONDS(300),RESET_INDICATOR_START,REPEATING_TIMER);
			gecko_cmd_hardware_set_soft_timer(SECONDS(5),RESET_INDICATION_STOP,ONESHOT_TIMER);
	//		gecko_cmd_hardware_set_soft_timer(SECONDS(PIR_STABILIZE_TIME),PIR_INTRPT_ENABLE,ONESHOT_TIMER);		//	Muruga
		break;
		/*............................................................................................*/
		case gecko_evt_system_external_signal_id:
			fn_process_extrnlSgnl_Evnt(evt);
		break;
		/*............................................................................................*/
		case gecko_evt_mesh_node_provisioning_started_id:
			DBG_PRINT("Started provisioning\r\n");
			gecko_cmd_hardware_set_soft_timer(MILLISECONDS(200),RESET_INDICATOR_START,REPEATING_TIMER);
		break;
		/*............................................................................................*/
		case gecko_evt_mesh_node_provisioned_id:
		  DBG_PRINT("node provisioned, got address=%x\r\n", evt->data.evt_mesh_node_provisioned.address);
		  gecko_cmd_hardware_set_soft_timer(MILLISECONDS(10),RESET_INDICATION_STOP,ONESHOT_TIMER);
	    break;
		/*............................................................................................*/
		case gecko_evt_mesh_node_provisioning_failed_id:
		  DBG_PRINT("provisioning failed, code %x\r\n", evt->data.evt_mesh_node_provisioning_failed.result);
		  /* start a one-shot timer that will trigger soft reset after small delay */
		  gecko_cmd_hardware_set_soft_timer(SECONDS(3),FCTORY_RESET_ID , 1);
		break;
		/*............................................................................................*/
		case gecko_evt_mesh_node_key_added_id:
		  DBG_PRINT("got new %s key with index %x\r\n",
				 evt->data.evt_mesh_node_key_added.type == 0 ? "network" : "application",
				 evt->data.evt_mesh_node_key_added.index);
		break;
		/*............................................................................................*/
		case gecko_evt_mesh_node_model_config_changed_id:
		  DBG_PRINT("model config changed\r\n");
		break;
		/*............................................................................................*/
		case gecko_evt_mesh_node_config_set_id:
		  DBG_PRINT("model config set\r\n");
		  gecko_cmd_system_reset(NORMAL_RESET);
	    break;
		/*............................................................................................*/
		case gecko_evt_mesh_node_reset_id:
		  DBG_PRINT("evt gecko_evt_mesh_node_reset_id\r\n");
		  fn_initiate_factory_reset(0);
		break;
		/*............................................................................................*/
		case gecko_evt_le_connection_parameters_id:
			DBG_PRINT("connection params: interval %d, timeout %d\r\n",
				 evt->data.evt_le_connection_parameters.interval,
				 evt->data.evt_le_connection_parameters.timeout);
	    break;
	    /*............................................................................................*/
		case gecko_evt_mesh_vendor_model_receive_id:
		{
			fn_handleVendorModel_Rcv(evt_id,evt);
		}
		break;
		/*............................................................................................*/
		case gecko_evt_mesh_friend_friendship_established_id:
			DBG_PRINT("[I] evt fship_estd, lpn_address=%x\r\n", evt->data.evt_mesh_friend_friendship_established.lpn_address);
		break;
		/*............................................................................................*/
	    case gecko_evt_mesh_friend_friendship_terminated_id:
			DBG_PRINT("[I] evt fship_term, reason=%x\r\n", evt->data.evt_mesh_friend_friendship_terminated.reason);
		break;
		/*............................................................................................*/
	    case gecko_evt_le_gap_adv_timeout_id:
			// Silently ignore because too many instances figure out how to disable
			//DBG_PRINT("[!] Adv timeout");
			break;

	default:
		break;
	}
	return;
}
/****************************************************************************************************************/
void fn_initiate_Iam_Pkt(void)
{
	srand(RTCC_CounterGet());
	uint32_t randomBackOff = ((uint32_t)rand() & 0x1FFFFF) + (uint16_t)rand() + (5*32768);
	DBG_PRINT("random backoff = %ld secs\r\n",(randomBackOff/TIMER_CLK_FREQ));
	gecko_cmd_hardware_set_soft_timer((randomBackOff), BROADCAST_I_AM_PKT, ONESHOT_TIMER);		//broadcast the IAM pkt after random backoff
	gecko_cmd_hardware_set_soft_timer((SECONDS(2)), FIRST_AGG_HANDSHAKE_PKT, ONESHOT_TIMER);	//first packet before that random backoff
	return ;
}
/****************************************************************************************************************/
void fn_fetch_snsrConfig(cfgRcvPkt_t *cfg,uint16_t destAddr)
{
	uint8_t m_packet_size = 0;
	sensorpayload_t  m_data;
	switch (cfg->snsrType)
	{
		case PIR_PARAM:
		{
			cfg->snsrCfgID = (0x10 | (snsrCfg.pir_cfg.cfg_id+1));
			switch(cfg->cfgParam & 0x7F)
			{
				case PARAM1:		//unoccupancy Timer
					cfg->cfgValue_LSB = (uint8_t)snsrCfg.pir_cfg.unoccupancyTimer_s;
					cfg->cfgValue_MSB = (uint8_t)(snsrCfg.pir_cfg.unoccupancyTimer_s >> 8);
				break;
				case PARAM2:			//retransmission Timer
					cfg->cfgValue_LSB = (uint8_t)snsrCfg.pir_cfg.retransmission_timeout;
					cfg->cfgValue_MSB = (uint8_t)(snsrCfg.pir_cfg.retransmission_timeout >> 8);

				break;
				default:
					return ;
				break;
			}
		}
		break;
		case ALS_PARAM:
		{
			if(snsrMinCfg.device_type & ALS_SENSOR)
			{
				cfg->snsrCfgID = (0x10 | (snsrCfg.als_cfg.cfg_id+1));
				switch(cfg->cfgParam & 0x7F)
				{
					case PARAM1:		//lux threshold
						cfg->cfgValue_LSB = (uint8_t)snsrCfg.als_cfg.luxThreshold;
						cfg->cfgValue_MSB = (uint8_t)(snsrCfg.als_cfg.luxThreshold >> 8);
					break;
					case PARAM2:		//freq of measurements
						cfg->cfgValue_LSB = (uint8_t)snsrCfg.als_cfg.freq_LUXmeasure_s;
						cfg->cfgValue_MSB = (uint8_t)(snsrCfg.als_cfg.freq_LUXmeasure_s >> 8);
					break;
					case PARAM3:		//als calibration factor
						cfg->cfgValue_LSB = (uint8_t)snsrCfg.als_cfg.calibration_factor;
						cfg->cfgValue_MSB = (uint8_t)(snsrCfg.als_cfg.calibration_factor >> 8);
					break;
					default:
						return ;
					break;
				}
			}
			else
			{
				return ;
			}
		}
		break;
		case TH_PARAM:
		{
			cfg->snsrCfgID = (0x10 | (snsrCfg.th_cfg.cfg_id+1));
			switch(cfg->cfgParam & 0x7F)
			{
				default:
					return ;
				break;
			}
		}
		break;
		default:
			return ;
		break;
	}
	m_data.datacmd = CUSTM_SENSOR_CONFIG;
	memcpy(m_data.data,cfg,sizeof(cfgRcvPkt_t));
	m_packet_size = 6;
	fn_enQ_blePkt(FT_COMMAND,(m_packet_size),(uint8_t*)&m_data,destAddr);
	return ;
}
/****************************************************************************************************************/
void fn_assign_snsrConfig(cfgRcvPkt_t *cfg)
{
	uint16 cfgVal = ( (cfg->cfgValue_MSB<<8) | (cfg->cfgValue_LSB) );
	uint8_t paramN = cfg->cfgParam & 0x7F;
	DBG_PRINT("param_%d\r\n",paramN);
	switch(cfg->snsrType)
	{
/*..............................................................................................................*/
		case PIR_PARAM:
		{
			switch(paramN)
			{
				case PARAM1:			//unOccupancy Timer
					snsrCfg.pir_cfg.unoccupancyTimer_s = cfgVal;
					snsrCfg.pir_cfg.cfg_id = cfg->snsrCfgID;
					DBG_PRINT("PIR_unoccupancyTime 0x%04x(%d)\r\n",snsrCfg.pir_cfg.unoccupancyTimer_s,snsrCfg.pir_cfg.unoccupancyTimer_s);
				break;
				case PARAM2:			//retransmission Timer
					snsrCfg.pir_cfg.retransmission_timeout = cfgVal;
					snsrCfg.pir_cfg.cfg_id = cfg->snsrCfgID;
					DBG_PRINT("PIR_retransmit 0x%04x(%d)\r\n",snsrCfg.pir_cfg.retransmission_timeout,snsrCfg.pir_cfg.retransmission_timeout);
				break;
				case PARAM3:			//wait and watch Timer
					snsrCfg.pir_cfg.wait_watch_Time_ms = cfgVal;
					snsrCfg.pir_cfg.cfg_id = cfg->snsrCfgID;
					DBG_PRINT("PIR_waitWatch 0x%04x(%d)\r\n",snsrCfg.pir_cfg.wait_watch_Time_ms,snsrCfg.pir_cfg.wait_watch_Time_ms);
				break;
				default:
				break;
			}
		}
		break;
/*..............................................................................................................*/
		case ALS_PARAM:
		{
			switch(paramN)
			{
				case PARAM1:			//lux threshold
					snsrCfg.als_cfg.luxThreshold = cfgVal;
					snsrCfg.als_cfg.cfg_id = cfg->snsrCfgID;
					DBG_PRINT("ALS thrshld 0x%04x(%d)\r\n",snsrCfg.als_cfg.luxThreshold,snsrCfg.als_cfg.luxThreshold);
				break;
				case PARAM2:			//frq of measurements
					snsrCfg.als_cfg.freq_LUXmeasure_s = cfgVal;
					snsrCfg.als_cfg.cfg_id = cfg->snsrCfgID;
					DBG_PRINT("ALS snsngTime 0x%04x(%d)\r\n",snsrCfg.als_cfg.freq_LUXmeasure_s,snsrCfg.als_cfg.freq_LUXmeasure_s);
				break;
				case PARAM3:			//als_calibration fcator
					snsrCfg.als_cfg.calibration_factor = cfgVal;
					snsrCfg.als_cfg.cfg_id = cfg->snsrCfgID;
					DBG_PRINT("ALS calbFctr 0x%04x(%d)\r\n",snsrCfg.als_cfg.calibration_factor,snsrCfg.als_cfg.calibration_factor);
				break;
				default:
				break;
			}
		}
		break;
/*..............................................................................................................*/
		case TH_PARAM:
		{
			switch(paramN)
			{
				case PARAM1:			//TMP_THRESHOLD
					snsrCfg.th_cfg.tempThreshold = cfgVal;
					snsrCfg.th_cfg.cfg_id = cfg->snsrCfgID;
					DBG_PRINT("temp thrshld 0x%04x(%d)\r\n",snsrCfg.th_cfg.tempThreshold,snsrCfg.th_cfg.tempThreshold);
				break;
				case PARAM2:			//HUMIDITY_THRESHOLD
					snsrCfg.th_cfg.humidityThreshold = cfgVal;
					snsrCfg.th_cfg.cfg_id = cfg->snsrCfgID;
					DBG_PRINT("hum thrshld 0x%04x(%d)\r\n",snsrCfg.th_cfg.humidityThreshold,snsrCfg.th_cfg.humidityThreshold);
				break;
				case PARAM3:			//FREQ_MEASUREMENT
					snsrCfg.th_cfg.freq_THmeasure = cfgVal;
					snsrCfg.th_cfg.cfg_id = cfg->snsrCfgID;
					DBG_PRINT("th snsng time 0x%04x(%d)\r\n",snsrCfg.th_cfg.freq_THmeasure,snsrCfg.th_cfg.freq_THmeasure);
				break;
				case PARAM4:			//PERIODIC_TRANSMISSION
					snsrCfg.th_cfg.retransmission_timeout = cfgVal;
					snsrCfg.th_cfg.cfg_id = cfg->snsrCfgID;
					DBG_PRINT("th periodicTransmit time 0x%04x(%d)\r\n",snsrCfg.th_cfg.retransmission_timeout,snsrCfg.th_cfg.retransmission_timeout);
				break;
				default:
				break;
			}
		}
		break;
/*..............................................................................................................*/
		default:
		break;
	}
	snsrMinCfg.cfgAvailable = true;
	gecko_cmd_hardware_set_soft_timer(SECONDS(SNSR_CFG_SAVE_INTERVAL), TIMER_ID_RESTART, ONESHOT_TIMER);
}
/****************************************************************************************************************/
void fn_handle_Vendor_FT_wl_LightCmds(struct gecko_msg_mesh_vendor_model_receive_evt_t *v_data)
{
	switch (v_data->payload.data[0])
	{
		case LIGHT_TOOGLE:
		{
			if(mux_control_select == 1)									//Analog
			{
				fn_setAnalogIntensity((g_analogLightStat)?0:100, 0);
			}
			else if(mux_control_select == 0)							//DALI
			{
				fn_daliMode1_Level(0xFF,(g_daliLightStat)?0:100, 0);
			}

			#ifdef DALI_SPACE											//TRIAC
				fn_toggleTriac();
			#endif
		}
		break;
		/*............................................................................................*/
		case LIGHT_ON_OFF_COLOR:
		{
			if(mux_control_select == 1)														//Analog
			{
				fn_setAnalogIntensity(v_data->payload.data[3], v_data->payload.data[2]);
			}
			else if(mux_control_select == 0)												//DALI
			{
				fn_daliMode1_Level(v_data->payload.data[1],v_data->payload.data[3], v_data->payload.data[2]);
			}

			#ifdef DALI_SPACE																//TRIAC
				if(v_data->payload.data[3])													//intensityLevel
				{
//					fn_switchOnTriac();
					DBG_PRINT("RTS high\r\n");
					GPIO_PinOutSet(TRIAC_PORT,TRIAC_PIN);
					triacCfg.triacStatus = true;
					fn_saveTriacState();
				}
				else
				{
					fn_switchOffTriac();
				}
				fn_sendTriacStatus(v_data->payload.data[2]);								//attribute&reason
			#endif
		}
		break;
		/*............................................................................................*/
		case LIGHT_IDENTIFY:
			if(mux_control_select == 1)				//Analog
			{
				if(!g_isIdentifyEnabled)
				{
				g_isIdentifyEnabled = true;
				}
			}
			else if(mux_control_select == 0)		//DALI
			{
				fn_enqueueIdentify();
			}
		break;
		/*............................................................................................*/
		case SET_SCENES:
			if(mux_control_select == 1)				//Analog
			{
				fn_goto_scene(*(uint16_t *)(v_data->payload.data+1), 0);
			}
			else if(mux_control_select == 0)		//DALI
			{
				if(fn_gotoScene(*(uint16_t *)(v_data->payload.data+1), 0)==-1)
				{
					printf("Unable to set scene %d\r\n", *(uint16_t *)(v_data->payload.data+1));
				}
			}
		break;
		/*............................................................................................*/
		case RELATIVE_LIGHT_CONTROL:
		{
			uint8_t currIntensity = 0;
			uint8_t rltvFctr = ((0x7F&v_data->payload.data[3])/100.0)*100;
			if(mux_control_select == 1)						//Analog
			{
				currIntensity = g_analogLightStat;
			}

			else if(mux_control_select == 0)				//DALI
			{
				memset(&s_DimEntity, 0 ,sizeof(s_DimEntity));

				s_DimEntity.Dim_shID = 	0xFF;

				if(v_data->payload.data[3] >> 7)
				{
					s_DimEntity.Dim_type = RELATIVE_DIMMING_INC;
				}
				else
				{
					s_DimEntity.Dim_type = RELATIVE_DIMMING_DEC;
				}

				// TODO : it should be configurable, but switch and remote are hard coded for 5%
				s_DimEntity.Intensity = 5;
				s_DimEntity.dim_set = true;

				currIntensity = g_daliLightStat;
			}

			if(v_data->payload.data[3] >> 7)
			{
				//inc
				currIntensity  += rltvFctr;
				currIntensity  = (currIntensity >100)? 100:currIntensity ;
			}
			else
			{
				//dec
				(rltvFctr>currIntensity)?(currIntensity  = 0):(currIntensity  -= rltvFctr);
			}

			if(mux_control_select == 1)
			{
				fn_setAnalogIntensity(currIntensity, 0);
			}
		}
		break;

		/*............................................................................................*/


		case LIGHT_CONTROL_2:
		{
			snsrCfg.WL_LIGHT_CONTROL_2_timer = (uint16_t)((v_data->payload.data[5] << 8) | v_data->payload.data[4]);

			if(mux_control_select == 1)					//Analog
			{
				fn_setAnalogIntensity(v_data->payload.data[3], v_data->payload.data[2]);
			}
			else if(mux_control_select == 0)			//DALI
			{
				fn_daliMode1_Level(v_data->payload.data[1],v_data->payload.data[3], v_data->payload.data[2]);
			}

			#ifdef DALI_SPACE							//TRIAC
				if(v_data->payload.data[3])								//intensityLevel
				{
//					fn_switchOnTriac();
					DBG_PRINT("RTS high\r\n");
					GPIO_PinOutSet(TRIAC_PORT,TRIAC_PIN);
					triacCfg.triacStatus = true;
					fn_saveTriacState();
				}
				else
				{
					fn_switchOffTriac();
				}
				fn_sendTriacStatus(v_data->payload.data[2]);			//attribute&reason
			#endif
		}
		break;
		default:
		break;
	}
	switch(v_data->payload.data[0])
	{
		case LIGHT_TOOGLE:
		case LIGHT_ON_OFF_COLOR:
		case SET_SCENES:
			if(mux_control_select <= 1)
			{
				snsrAppData.WL_LIGHT_RcvdExternalCmd_flag = 1;
				gecko_cmd_hardware_set_soft_timer(SECONDS(snsrCfg.WL_LIGHT_RcvdExternalCmd_timer), WL_LIGHT_EXTERNAL_CMD_TIMER, ONESHOT_TIMER);
			}
		break;
		default:
		break;
	}

	if(v_data->payload.data[0] == LIGHT_CONTROL_2)
	{
		if(snsrCfg.WL_LIGHT_CONTROL_2_timer)
		{
			printf("Enabled Sensor Masking for %d mins\r\n", snsrCfg.WL_LIGHT_CONTROL_2_timer);
			snsrAppData.WL_LIGHT_CONTROL_2_flag = 1;
			gecko_cmd_hardware_set_soft_timer(SECONDS(snsrCfg.WL_LIGHT_CONTROL_2_timer*60), WL_LIGHT_CONTROL_2_TIMER_ID, ONESHOT_TIMER);
		}
		else
		{
			printf("Sensor Not Masked\r\n");
			snsrAppData.WL_LIGHT_CONTROL_2_flag = 0;
		}
	}

	return ;
}
/****************************************************************************************************************/
void fn_handle_Vendor_FT_Commands(struct gecko_msg_mesh_vendor_model_receive_evt_t *v_data)
{
	switch (v_data->payload.data[0])
	{
		case SENSOR_INDENTIFY:
		if(!snsrAppData.identify)
		{
			snsrAppData.identify = true;
		}
		break;
	/*............................................................................................*/
		case DIM_FADE_TIME:
		{
			if(mux_control_select == 1)
			{
				if(v_data->payload.data[2] < FADE_90p5SEC)
				{
					g_analogFadeTime = a_fadeTime[v_data->payload.data[2]];
					gecko_cmd_flash_ps_save(ANALOG_FADE_TIME_PS,sizeof(g_analogFadeTime),(uint8_t *)&g_analogFadeTime);
				}
			}
			else if(mux_control_select == 0)
			{
				static struct_DaliBallastCfg sDaliLightBallastCfg = { 0 };
				sDaliLightBallastCfg.ballastShortID = v_data->payload.data[1];
				sDaliLightBallastCfg.sLightConfig.fadeTime = v_data->payload.data[2];
				uint16_t m_statusChangeFlag = 0x01;
				fn_enQueueDC(&sDaliCfgQueue, &sDaliLightBallastCfg, m_statusChangeFlag);
			}
		}
		break;
	/*............................................................................................*/
		case GOTO_OTA:
			TURN_ON_LED(APP_LED2);
			gecko_cmd_hardware_set_soft_timer(MILLISECONDS(200),LED2_TIMER_ID,ONESHOT_TIMER);		//	Start timer for LED OFF(green led)
			gecko_cmd_system_reset(OTA_DFU_RESET);
		break;
	/*............................................................................................*/
		case ASSIGN_GROUP:
		{
			struct gecko_msg_mesh_test_add_local_model_sub_rsp_t* rsp;
			uint16_t subAddr = ((v_data->payload.data[1] << 8) | v_data->payload.data[2]);
			rsp = gecko_cmd_mesh_test_add_local_model_sub(PRIMARY_ELEMENT,MY_VENDOR_ID, MY_MODEL_ID,subAddr);
			DBG_PRINT("self addr config resp %04x  with grp_id %04x \r\n",rsp->result,subAddr);
		}
		break;
	/*............................................................................................*/
		case DE_ASSIGN_GROUP:
		{
			struct gecko_msg_mesh_test_del_local_model_sub_rsp_t* rsp;
			rsp = gecko_cmd_mesh_test_del_local_model_sub(PRIMARY_ELEMENT,
			MY_VENDOR_ID, MY_MODEL_ID,((v_data->payload.data[1] << 8) | v_data->payload.data[2]));
			DBG_PRINT("de-assigning from grp %04x : resp %04x \r\n",((v_data->payload.data[1] << 8) | v_data->payload.data[2]),	rsp->result);
		}
		break;
	/*............................................................................................*/
		case SYSTEM_RESET_CMD:
		{
			if (!v_data->payload.data[1])
			{
				gecko_cmd_hardware_set_soft_timer(SECONDS(1), FCTORY_RESET_ID,ONESHOT_TIMER);
			}
		}
		break;
	/*............................................................................................*/
		case FACTORY_RESET:
		{
			if (!v_data->payload.data[1])
			{
				fn_initiate_factory_reset(FACTORY_LEVEL);
			}
		}
		break;
	/*............................................................................................*/
		case NETWORK_RESET:
		{
			fn_initiate_factory_reset(NETWORK_LEVEL);
		}
		break;
	/*............................................................................................*/
		case BIND_TO_AGGREGATOR:
		{
			DBG_PRINT("agg binding to %d\r\n", v_data->source_address);
			snsrMinCfg.dest_addr = v_data->source_address;
			snsrMinCfg.aggBinded = true;
			DBG_PRINT("agg bind PS resp %d\r\n",
			gecko_cmd_flash_ps_save(PS_SNSR_MINCFG,sizeof(snsrMinCfg),(uint8_t *)&snsrMinCfg)->result);
			TURN_ON_LED(APP_LED2);
			gecko_cmd_hardware_set_soft_timer(MILLISECONDS(200),LED2_TIMER_ID,ONESHOT_TIMER);		//	Start timer for LED OFF(green led)
		}
		break;
	/*............................................................................................*/
		case LIGHT_SENSOR_MAP:

		break;
	/*............................................................................................*/
		case SCENE_CONFIG:
			if(mux_control_select == 1)							//Analog
			{
				fn_sceneCfg(*(uint16_t *)(v_data->payload.data+1), v_data->payload.data[3], v_data->payload.data[4], v_data->payload.data[5]);
			}
			else if(mux_control_select == 0)					//Dali
			{
				if(fn_sceneCfgD(true, *(uint16_t *)(v_data->payload.data+1),
						v_data->payload.data[3], v_data->payload.data[5])==-1){
					printf("Unable to configure scene %d\r\n", *(uint16_t *)(v_data->payload.data+1));
				}
			}
		break;
	/*............................................................................................*/
		case SCENE_DELETE:
			if(mux_control_select == 1)
			{
				fn_sceneDel(*(uint16_t *)(v_data->payload.data+1));
			}
			else if(mux_control_select == 0)
			{
				if(fn_sceneCfgD(false, *(uint16_t *)(v_data->payload.data+1),
						v_data->payload.data[3], 0)==-1)
				{
					printf("Unable to delete scene %d\r\n", *(uint16_t *)(v_data->payload.data+1));
				}
			}
		break;
	/*............................................................................................*/
		case CUSTM_SENSOR_CONFIG:
		{
			if (v_data->payload.data[3] & ASSIGN_CONFIG)		//if the MSB is set assign the config
			{
				fn_assign_snsrConfig((cfgRcvPkt_t *) &v_data->payload.data[1]);
			}
			if (!(v_data->payload.data[3] >> 7))				//if the MSB is not set fetch the config
			{
				fn_fetch_snsrConfig((cfgRcvPkt_t *)&v_data->payload.data[1],v_data->source_address);
			}
		}
		break;
	/*............................................................................................*/
		case SENSOR_LED_OPN:
		{
			if(v_data->payload.data[1] == 0x01){
				(v_data->payload.data[2] == 0)
						? TURN_OFF_LED(APP_LED1) : (v_data->payload.data[2] == 1)
								? TURN_ON_LED(APP_LED1):LED_TOGGLING(APP_LED1);
			}
			else{
				(v_data->payload.data[2] == 0)
						? TURN_OFF_LED(APP_LED2) : (v_data->payload.data[2] == 1)
								? TURN_ON_LED(APP_LED2):LED_TOGGLING(APP_LED2);
			}
		}
		break;
	/*............................................................................................*/
		case ALS_CALIB_CONF1:
		{
			sAlsCalibValue.m = *(float *)(v_data->payload.data+1);
			gecko_cmd_flash_ps_save(ALS_CALIB_KEY,sizeof(sAlsCalibValue),(uint8_t *)&sAlsCalibValue);
			DBG_PRINT("conf1 = %f\r\n",sAlsCalibValue.m);
			TURN_ON_LED(APP_LED2);
			gecko_cmd_hardware_set_soft_timer(MILLISECONDS(200),LED2_TIMER_ID,ONESHOT_TIMER);		//	Start timer for LED OFF(green led)
		}
		break;
	/*............................................................................................*/
		case ALS_CALIB_CONF2:
		{
			sAlsCalibValue.c = *(float *)(v_data->payload.data+1);
			gecko_cmd_flash_ps_save(ALS_CALIB_KEY,sizeof(sAlsCalibValue),(uint8_t *)&sAlsCalibValue);
			DBG_PRINT("conf2 = %f\r\n",sAlsCalibValue.c);
			TURN_ON_LED(APP_LED2);
			gecko_cmd_hardware_set_soft_timer(MILLISECONDS(200),LED2_TIMER_ID,ONESHOT_TIMER);		//	Start timer for LED OFF(green led)
		}
		break;
	/*............................................................................................*/
		case ALS_CALIB_GAIN:
		{
			sAlsCalibValue.m_gain = *(uint16_t *)(v_data->payload.data+1);
			gecko_cmd_flash_ps_save(ALS_CALIB_KEY,sizeof(sAlsCalibValue),(uint8_t *)&sAlsCalibValue);
			DBG_PRINT("gain:%d\r\n", sAlsCalibValue.m_gain);
			TURN_ON_LED(APP_LED2);
			gecko_cmd_hardware_set_soft_timer(MILLISECONDS(200),LED2_TIMER_ID,ONESHOT_TIMER);		//	Start timer for LED OFF(green led)
		}
		break;
	/*............................................................................................*/
		case ALS_CALIB_REQLUX:
		{
			sAlsCalibValue.req_lux = *(uint16_t *)(v_data->payload.data+1);
			gecko_cmd_flash_ps_save(ALS_CALIB_KEY,sizeof(sAlsCalibValue),(uint8_t *)&sAlsCalibValue);
			printf("rlux:%d\r\n", sAlsCalibValue.req_lux);
			TURN_ON_LED(APP_LED2);
			gecko_cmd_hardware_set_soft_timer(MILLISECONDS(200),LED2_TIMER_ID,ONESHOT_TIMER);		//	Start timer for LED OFF(green led)
		}
		break;
	/*............................................................................................*/
		case DALI_PERSISTENT_STORAGE_TIMEOUT:
		{
			g_sDaliRetention.Dali_RetentionStroageTimeout = *(uint8_t *)(v_data->payload.data+1);
			DBG_PRINT("dali level save rspns %d\r\n",(gecko_cmd_flash_ps_save(DALI_RETENTION_STORAGE_KEY, 1, (uint8_t *)&g_sDaliRetention.Dali_RetentionStroageTimeout))->result);
		}
		break;

	/*............................................................................................*/
		case DALI_CONFIG:		//0x11
		{
			uint16_t m_statusChangeFlag = 0;
			static struct_DaliBallastCfg sDaliLightBallastCfg = { 0 };

			switch(v_data->payload.data[1])
			{
				case FADE_TIME:							//03 56 06 00 00 2C 00 AB F6 04 11 00 FF 05 75 0F
					sDaliLightBallastCfg.ballastShortID = v_data->payload.data[2];
					sDaliLightBallastCfg.sLightConfig.fadeTime = v_data->payload.data[3];
					m_statusChangeFlag = 0x01;
					fn_enQueueDC(&sDaliCfgQueue, &sDaliLightBallastCfg, m_statusChangeFlag);
				break;

				case FADE_RATE:
					sDaliLightBallastCfg.ballastShortID = v_data->payload.data[2];
					sDaliLightBallastCfg.sLightConfig.fadeRate = v_data->payload.data[3];
					m_statusChangeFlag = 0x02;
					fn_enQueueDC(&sDaliCfgQueue, &sDaliLightBallastCfg, m_statusChangeFlag);
                 break;

				case DALI_MLEVEL_THRESHOLD:				//03 56 06 00 00 FF FF B6 86 03 11 06 xx CRCH CRCL
					g_sDaliRetention.m_LevelThreshold = *(uint8_t *)(v_data->payload.data + 2);
				break;

				case PRESET_MAX:
					sDaliLightBallastCfg.ballastShortID = v_data->payload.data[2];

					v_data->payload.data[3] = (v_data->payload.data[3] != 0) ?	(uint8_t) ((253 * (1 + log10(v_data->payload.data[3])) / 3) + 1) : 255;

					sDaliLightBallastCfg.sLightConfig.presetMax = v_data->payload.data[3];
					m_statusChangeFlag = 0x04;
					fn_enQueueDC(&sDaliCfgQueue, &sDaliLightBallastCfg, m_statusChangeFlag);
                 break;

				case PRESET_MIN:
					sDaliLightBallastCfg.ballastShortID = v_data->payload.data[2];

					v_data->payload.data[3] = (v_data->payload.data[3] != 0) ?	(uint8_t) ((253 * (1 + log10(v_data->payload.data[3])) / 3) + 1) : 0;

					sDaliLightBallastCfg.sLightConfig.presetMin = v_data->payload.data[3];
#ifdef DALI_SPACE
					if(sDaliLightBallastCfg.sLightConfig.presetMin > 0)
					{
						snsrCfg.emergency_light = true;
					}
					else
					{
						snsrCfg.emergency_light = false;
					}
					snsrMinCfg.cfgAvailable = true;
					DBG_PRINT("CFG SAVE RESPONSE %d\r\n",gecko_cmd_flash_ps_save(PS_SNSR_CFG_BASE,sizeof(snsrCfg),(uint8_t *)&snsrCfg)->result);
					DBG_PRINT("MINCFG SAVE RESPONSE %d\r\n",gecko_cmd_flash_ps_save(PS_SNSR_MINCFG,sizeof(snsrCfg),(uint8_t *)&snsrMinCfg)->result);
#endif
					m_statusChangeFlag = 0x08;
					fn_enQueueDC(&sDaliCfgQueue, &sDaliLightBallastCfg, m_statusChangeFlag);
                 break;

				case SYS_FAIL_LEVEL:
					sDaliLightBallastCfg.ballastShortID = v_data->payload.data[2];

					v_data->payload.data[3] = (v_data->payload.data[3] != 0) ?	(uint8_t) ((253 * (1 + log10(v_data->payload.data[3])) / 3) + 1) : 255;

					sDaliLightBallastCfg.sLightConfig.sysFailLevel = v_data->payload.data[3];
					m_statusChangeFlag = 0x10;
					fn_enQueueDC(&sDaliCfgQueue, &sDaliLightBallastCfg, m_statusChangeFlag);
                 break;


//				case FADE_RATE:
//				case PRESET_MAX:
//				case PRESET_MIN:
//				case SYS_FAIL_LEVEL:
//				case POW_FAIL_LEVEL:
//					break;
			}
		}
		break;
	/*............................................................................................*/
		case WL_LIGHT_EXT_CMD_TIMER:
		{
			snsrCfg.WL_LIGHT_RcvdExternalCmd_timer = (uint16_t)((v_data->payload.data[2] << 8) | v_data->payload.data[1]);
			snsrMinCfg.cfgAvailable = true;
			DBG_PRINT("CFG SAVE RESPONSE %d\r\n",gecko_cmd_flash_ps_save(PS_SNSR_CFG_BASE,sizeof(snsrCfg),(uint8_t *)&snsrCfg)->result);
			DBG_PRINT("MINCFG SAVE RESPONSE %d\r\n",gecko_cmd_flash_ps_save(PS_SNSR_MINCFG,sizeof(snsrCfg),(uint8_t *)&snsrMinCfg)->result);
		}
		break;
	/*............................................................................................*/
		case MUX_CONTROL:			//Analog/DALI driver to be used
			if(mux_control_select == 0)
			{
				mux_control_select = 1;
				printf("Analog Driver in use\r\n");
			}
			else
			{
				mux_control_select = 0;
				printf("DALI Driver in use\r\n");
			}
			printf("Mux_control_select = %d\r\n", mux_control_select);
			struct gecko_msg_flash_ps_save_rsp_t* rsp;
			rsp = gecko_cmd_flash_ps_save(MUX, 1 , &mux_control_select);
			printf("Mux Settings save result : %04x\r\n", rsp->result);
			gecko_cmd_hardware_set_soft_timer(SECONDS(1), FCTORY_RESET_ID,ONESHOT_TIMER);
		break;
	/*............................................................................................*/
		case DISABLE_DEBUG_PRINTS:
			if(v_data->payload.data[1] == 1)
			{
				debugPrints = 1;
			}
			else if(v_data->payload.data[1] == 0)
			{
				debugPrints = 0;
			}
		break;

		case MAX_LUX_ON_TABLE:
			MaxLuxOnTable = (uint16_t)((v_data->payload.data[2] << 8) | v_data->payload.data[1]);
			printf("Mux_on_table = %d\r\n", MaxLuxOnTable);
		break;

		//.......................................................................................................//
#ifdef AREA
		case AREA_DETAILS:
		{
			sConfigAreaParameters.TotalArea_Sensors_count = v_data->payload.data[1];
			printf("Total number of sensors in Area = %d\r\n", sConfigAreaParameters.TotalArea_Sensors_count);
			sConfigAreaParameters.Area_Group_ID = (uint16_t)((v_data->payload.data[3] << 8) | v_data->payload.data[2]);
			printf("Group ID for AREA = %d\r\n", sConfigAreaParameters.Area_Group_ID);

			printf("AREA config Parameters saved in PS result : %d\r\n",gecko_cmd_flash_ps_save(PS_SNSR_AREA_CONFIG_PARAMETERS,sizeof(sConfigAreaParameters), &sConfigAreaParameters)->result);
		}
		break;

		case CONFIGURE_AREA:
		{
			uint16_t SHID1 = (uint16_t)((v_data->payload.data[2] << 8) | v_data->payload.data[1]);
			uint16_t SHID2 = (uint16_t)((v_data->payload.data[4] << 8) | v_data->payload.data[3]);
			uint16_t SHID3 = (uint16_t)((v_data->payload.data[6] << 8) | v_data->payload.data[5]);
			sConfigArea[AreaSensorCount++].SHID = SHID1;
			if(AreaSensorCount  < sConfigAreaParameters.TotalArea_Sensors_count)
			{
				sConfigArea[AreaSensorCount++].SHID = SHID2;
			}
			if(AreaSensorCount  < sConfigAreaParameters.TotalArea_Sensors_count)
			{
				sConfigArea[AreaSensorCount++].SHID = SHID3;
			}

			struct gecko_msg_flash_ps_save_rsp_t *result = gecko_cmd_flash_ps_save(PS_SNSR_AREA_CONFIG,sizeof(sConfigArea), &sConfigArea);
			printf("AREA config saved in PS result : %d\r\n",result->result);


			if((snsrMinCfg.snsrID == SHID1) || (snsrMinCfg.snsrID == SHID2) || (snsrMinCfg.snsrID == SHID3))
			{
				sConfigAreaParameters.SensorIsPartOfArea = 1;
				for(int i = 0; i < AreaSensorCount;)
				{
					if(sConfigArea[i].SHID == snsrMinCfg.snsrID)
					{
						sConfigArea[i].Occupancy = snsrCurrStatus.pir_State;
						break;
					}
					i++;
				}
			}

			printf("AREA config Parameters saved in PS result : %d\r\n",
					gecko_cmd_flash_ps_save(PS_SNSR_AREA_CONFIG_PARAMETERS,sizeof(sConfigAreaParameters), &sConfigAreaParameters)->result);

			//Soft Timer to check if all CONFIGURE_AREA packets area received.
			//If not send an FT_ACK packet
			gecko_cmd_hardware_set_soft_timer(SECONDS(10), TIMER_ID_CONFIGURE_AREA, 1);
		}
		break;

		case DELETE_SENSOR_IN_AREA:
		{
			uint16_t SHID = (uint16_t)((v_data->payload.data[2] << 8) | v_data->payload.data[1]);
			for(int i = 0; i < MAX_SENSOR_IN_GRP; i++)
			{
				if(sConfigArea[i].SHID == SHID)
				{
					if(SHID == snsrMinCfg.snsrID)
					{
						sConfigAreaParameters.SensorIsPartOfArea = 0;
					}
					if(i == MAX_SENSOR_IN_GRP - 1)
					{
						memset(&sConfigArea[i].SHID, 0, (sizeof(struct_Configure_Area_t)));
					}
					else
					{
						for(uint8_t j = i; j < sConfigAreaParameters.TotalArea_Sensors_count; j++ )
						{
							memcpy(&sConfigArea[j], &sConfigArea[j+1], sizeof(struct_Configure_Area_t));
							memset(&sConfigArea[j+1], 0, sizeof(struct_Configure_Area_t));
						}
					}
					sConfigAreaParameters.TotalArea_Sensors_count--;
				}
			}
			struct gecko_msg_flash_ps_save_rsp_t *result = gecko_cmd_flash_ps_save(PS_SNSR_AREA_CONFIG,sizeof(sConfigArea), &sConfigArea);
			printf("AREA config saved after Deletion in PS result : %d\r\n",result->result);

			printf("AREA config Parameters saved after deletion in PS result : %d\r\n",gecko_cmd_flash_ps_save(PS_SNSR_AREA_CONFIG_PARAMETERS,sizeof(sConfigAreaParameters), &sConfigAreaParameters)->result);
		}
		break;
#endif

		default:
		break;
	}
	return;
}
/****************************************************************************************************************/
void fn_handleVendorModel_Rcv(uint32_t evt_id,struct gecko_cmd_packet *evt)
{
	struct gecko_msg_mesh_vendor_model_receive_evt_t *v_data;
	v_data = (struct gecko_msg_mesh_vendor_model_receive_evt_t*)(&(evt->data.evt_mesh_vendor_model_receive));

//#ifdef DEBUG_MODE			//	Commented out by Muruga
//	DBG_PRINT("Data rcvd \r\n SrcAdrr --> %d || opcode --> %d || cmd --> %d || len --> %d \r\n",
//											v_data->source_address,v_data->opcode,v_data->payload.data[0],v_data->payload.len);
//
//	for(uint8_t i = 0;i<v_data->payload.len;i++)
//	{
//		printf("0x%02x\t",v_data->payload.data[i]);
//	}
//	printf("\r\n");
//#endif

	switch(v_data->opcode)
	{
/*..............................................................................................................*/
		case FT_WHOIS:
			TURN_ON_LED(APP_LED2);
			gecko_cmd_hardware_set_soft_timer(MILLISECONDS(200),LED2_TIMER_ID,ONESHOT_TIMER);		//	Start timer for LED OFF(green led)
			switch (v_data->payload.data[0])
			{
				case UNREPORTED_SNSR_REQ:
				{
					if(!snsrMinCfg.aggBinded)
					{
						DBG_PRINT("snsr is not binded to agg\r\n");
						fn_initiate_Iam_Pkt();
						break;
					}
				}
				break;
				case FORCED_REQ:
				{
					snsrAppData.ForcedReq = true;
					fn_initiate_Iam_Pkt();
				}
				break;
				default:
				break;
			}
		break;
/*..............................................................................................................*/
		case FT_WL_LIGHT_COMMANDS:
		{
			fn_handle_Vendor_FT_wl_LightCmds(v_data);
		}
		break;
/*..............................................................................................................*/
		case FT_COMMAND:		//any system commands from user through  UI , including configurations
		{
			fn_handle_Vendor_FT_Commands(v_data);
		}
		break;
/*..............................................................................................................*/
		case FT_DATAREQ:
		{
			send_packet(v_data->payload.data[0],v_data->source_address,true);
		}
		break;
#ifdef AREA
		case FT_STATUSCHANGE:
		{
			if(v_data->payload.data[0] == PIR)
			{
				for(int i = 0; i < sConfigAreaParameters.TotalArea_Sensors_count;)
				{
					if(sConfigArea[i].SHID == v_data->source_address)
					{
						sConfigArea[i].Occupancy = v_data->payload.data[1];
						if(sConfigArea[i].Occupancy)
						{
#ifdef DALI_FEATURE
							fn_daliMode1_Level(0xFF, 100, 0x16);
#endif
#ifdef ANALOG_FEATURE
							fn_setAnalogIntensity(100, 0x26);
#endif
#ifdef TRIAC_FEATURE
							GPIO_PinOutSet(TRIAC_PORT,TRIAC_PIN);
							triacCfg.triacStatus = true;
							fn_saveTriacState();
							fn_sendTriacStatus(0x16);
#endif
						}
					}
					i++;
				}
			}
		}
		break;
#endif
/*..............................................................................................................*/
		default:
		break;
/*..............................................................................................................*/
	}
	return;
}
/****************************************************************************************************************/
void fn_send_Cmd_onGSLink(struct gecko_msg_mesh_vendor_model_receive_evt_t *v_data)
{
	if( (v_data->destination_address != snsrMinCfg.snsrID)
	&& (snsrMinCfg.snsrID != sCtrlCfg.UUID) )
		return;
	fn_gsLink_command(&v_data->payload.data[0],v_data->payload.len);
	DBG_PRINT("sent the light control command to controller\r\n");
	return ;
}
/****************************************************************************************************************/
void fn_process_extrnlSgnl_Evnt(struct gecko_cmd_packet *evt)
{
	if(evt->data.evt_system_external_signal.extsignals & EXT_SIGNAL_PIR_INTERRUPT)									//	PIR Interrupt
	{
		pir_unoccupancyTime = fn_GetSecTimerStart();
		DBG_PRINT(".........motion detected...... %d \r\n",pir_unoccupancyTime);
		if(!snsrCurrStatus.pir_State){
			ePIRprocess_state = UPDATE_PIR_STATUS_CHANGE;
		}
		snsrCurrStatus.pir_State = OCCUPIED;

		(snsrCurrStatus.pir_State && Curr_mLevel) ? fn_switchOnTriac() : fn_switchOffTriac();
		fn_sendTriacStatus(0x12);
	}
	if(evt->data.evt_system_external_signal.extsignals & EXT_SIGNAL_SWITCH_INTERRUPT)								//	Switch Intterupt
	{
		DBG_PRINT("s1\r\n");
		NVIC_ClearPendingIRQ(SWITCH_INTRPT_IRQn);
		gecko_cmd_hardware_set_soft_timer(MILLISECONDS(20),SWITCH_DEBOUNCE_TIMER,ONESHOT_TIMER);
	}
	if((evt->data.evt_system_external_signal.extsignals & EXT_SIGNAL_DALI_INTERRUPT) && (mux_control_select == 0))		//	DALI Interrupt
	{
		switch (GPIO_PinModeGet(DALI_RX_PORT, DALI_RX_PIN))
		{

		case PIN_RESET:
			if (sdaliRxParams[DALI_LOOP1].rxDataBegin_flag == 0)
			{
				// Invalid packet : To be Discarded
				fn_discardPacket(DALI_LOOP1);
			}
			else
			{
				sdaliRxParams[DALI_LOOP1].timeCount[sdaliRxParams[DALI_LOOP1].rxDataIndex] =
						fn_GetuSecTimerStart();
				sdaliRxParams[DALI_LOOP1].rxData[sdaliRxParams[DALI_LOOP1].rxDataIndex++] = 0; //0
				__NOP();
			}
			break;
		case PIN_SET:
			if (sdaliRxParams[DALI_LOOP1].rxDataIndex == 0)
			{
				sdaliRxParams[DALI_LOOP1].rxDataBegin_flag = 1;
				sdaliRxParams[DALI_LOOP1].rxStartTime = fn_GetuSecTimerStart();
			}
			if (sdaliRxParams[DALI_LOOP1].rxDataBegin_flag == 1)
			{
				sdaliRxParams[DALI_LOOP1].timeCount[sdaliRxParams[DALI_LOOP1].rxDataIndex] =
						fn_GetuSecTimerStart();
				sdaliRxParams[DALI_LOOP1].rxData[sdaliRxParams[DALI_LOOP1].rxDataIndex++] = 1; //1
				__NOP();
			}
			break;
		default:

			break;
		}

	}
	return ;
}
/****************************************************************************************************************/
void fn_process_HardwareSoftTimer_Evnt(struct gecko_cmd_packet *evt)
{
	switch(evt->data.evt_hardware_soft_timer.handle)
	{
	/*..............................................................................................................*/
		case BROADCAST_I_AM_PKT:
		case FIRST_AGG_HANDSHAKE_PKT:		//
		{
			struct_IAM_pkt_t snsr_IAM_pkt = {0};

			snsr_IAM_pkt.cntrl_deviceType = snsrMinCfg.device_type;
			snsr_IAM_pkt.snsr_deviceType = 0;
			snsr_IAM_pkt.cntrl_fwVer = snsrMinCfg.firmwareversion;
			snsr_IAM_pkt.snsr_fwVer = 0;
			snsr_IAM_pkt.parent_ID = 0;

			TURN_ON_LED(APP_LED1);
			gecko_cmd_hardware_set_soft_timer(MILLISECONDS(200),LED1_TIMER_ID,ONESHOT_TIMER);		//	Start timer for LED OFF(red led)
			if(snsrAppData.ForcedReq)
			{
				fn_enQ_blePkt(FT_IAM, I_AM_PKT_LEN, (uint8_t*) &snsr_IAM_pkt, 0xFFFF);				// Broadcast
			}
			else
			{
				fn_enQ_blePkt(FT_IAM, I_AM_PKT_LEN, (uint8_t*) &snsr_IAM_pkt, snsrMinCfg.dest_addr);
			}
			if (evt->data.evt_hardware_soft_timer.handle == BROADCAST_I_AM_PKT)
			{
				snsrAppData.broadCastComplete = true;
				snsrAppData.ForcedReq = false;
			}
			return;
		}
		break;
	/*..............................................................................................................*/
		case BLEQ_HANDLER:
		{
			fn_processBleQ();
		}
		break;
	/*..............................................................................................................*/
		case LED1_TIMER_ID:
		{
			TURN_OFF_LED(APP_LED1);
		}
		break;
	/*..............................................................................................................*/
		case LED2_TIMER_ID:
		{
			TURN_OFF_LED(APP_LED2);
		}
		break;
	/*..............................................................................................................*/
		case SWITCH_PRESS_INDICATION:
		{
#ifdef DALI_SPACE
			if(GPIO_PinInGet(SWITCH_PORT, SWITCH_PIN) == SWITCH_RELEASED)
#else
			if(GPIO_PinInGet(SWITCH_PORT, SWITCH_PIN) == SWITCH_PRESSED)
#endif
			{
				LED_TOGGLING(APP_LED2);
			}
			else
			{
				//stop press indication
				TURN_OFF_LED(APP_LED2);
				gecko_cmd_hardware_set_soft_timer(MILLISECONDS(0),SWITCH_PRESS_INDICATION,REPEATING_TIMER);
			}
		}
		break;
	/*..............................................................................................................*/
		case LOOP_MAP_INDICATION:
		{
			if(g_loopInidcationCnt)
			{
				LED_TOGGLING(APP_LED1);
				--g_loopInidcationCnt;
				gecko_cmd_hardware_set_soft_timer(MILLISECONDS(500),LOOP_MAP_INDICATION,ONESHOT_TIMER);
			}
			else
			{
				TURN_OFF_LED(APP_LED1);
				g_loopInidcationCnt = 0;
			}
		}
		break;
	/*..............................................................................................................*/
		case SWITCH_DEBOUNCE_TIMER:
		{
#ifdef DALI_SPACE
			if (GPIO_PinInGet(SWITCH_PORT, SWITCH_PIN) == SWITCH_RELEASED)
#else
			if (GPIO_PinInGet(SWITCH_PORT, SWITCH_PIN) == SWITCH_PRESSED)
#endif
			{
				DBG_PRINT("SW1_pressed\r\n");
				snsrAppData.switchState = SWITCH_PRESSED;
				g_switchPressed_StartTime = fn_GetSecTimerStart();
				g_loopMapStratTime = fn_GetSecTimerStart();
				g_loopMapIndex = fn_getCurntMappedIndx();
				//start press indication
				gecko_cmd_hardware_set_soft_timer(MILLISECONDS(500),SWITCH_PRESS_INDICATION,REPEATING_TIMER);
			}
#ifdef DALI_SPACE
			else if (GPIO_PinInGet(SWITCH_PORT, SWITCH_PIN) == SWITCH_PRESSED)
#else
			else if (GPIO_PinInGet(SWITCH_PORT, SWITCH_PIN) == SWITCH_RELEASED)
#endif
				{
				if (snsrAppData.switchState == SWITCH_PRESSED)//check if previously the intrpt was enabled
				{
					fn_switchReleased();
				}
				DBG_PRINT("SW1_released\r\n");
				snsrAppData.switchState = SWITCH_RELEASED;
			}
		}
		break;
	/*..............................................................................................................*/
		case FCTORY_RESET_ID:
		{
			gecko_cmd_system_reset(NORMAL_RESET);
		}
		break;
	/*..............................................................................................................*/
		case TIMER_ID_RESTART:
		{
			DBG_PRINT("CFG SAVE RESPONSE %d\r\n",gecko_cmd_flash_ps_save(PS_SNSR_CFG_BASE,sizeof(snsrCfg),(uint8_t *)&snsrCfg)->result);
			DBG_PRINT("MINCFG SAVE RESPONSE %d\r\n",gecko_cmd_flash_ps_save(PS_SNSR_MINCFG,sizeof(snsrCfg),(uint8_t *)&snsrMinCfg)->result);
			gecko_cmd_system_reset(NORMAL_RESET);
		}
		break;
	/*..............................................................................................................*/
		case RESET_INDICATOR_START:
		{
			LED_TOGGLING(APP_LED1);
			LED_TOGGLING(APP_LED2);
		}
		break;
	/*..............................................................................................................*/
		case RESET_INDICATION_STOP:
		{
			gecko_cmd_hardware_set_soft_timer(0, RESET_INDICATOR_START,	REPEATING_TIMER);
			TURN_OFF_LED(APP_LED1);
			TURN_OFF_LED(APP_LED2);
		}
		break;
	/*..............................................................................................................*/
		case TIMER_ID_LIGHT_MAP_PKT:
		{
			fn_sendLightMap_Pkt();
			struct gecko_msg_flash_ps_save_rsp_t *rsp = gecko_cmd_flash_ps_save(PS_SNSR_MINCFG, sizeof(snsrMinCfg), (uint8_t *) &snsrMinCfg);
			DBG_PRINT("minSnsrCfg save rsp : %d\r\n", rsp->result);
		}
		break;
	/*..............................................................................................................*/
		case PIR_INTRPT_ENABLE:
		{
			snsrAppData.PIR_stabilized = true;
			//pir_enable();
		}
		break;	
	/*..............................................................................................................*/
		case DALI_SAVE_TIMER:
			while(!ballast_storePowOnLevel(DALI_LOOP1, (uint8_t) ((0xFF << 1) | 0x01), true));
		break;
	/*..............................................................................................................*/
		case WL_LIGHT_EXTERNAL_CMD_TIMER:
			snsrAppData.WL_LIGHT_RcvdExternalCmd_flag = 0;
		break;
	/*..............................................................................................................*/
		case WL_LIGHT_CONTROL_2_TIMER_ID:
			snsrAppData.WL_LIGHT_CONTROL_2_flag = 0;
		break;

		case EMERGENCY_LIGHT_TIMER_ID:
			if(!(snsrCurrStatus.pir_State))
			{
//				printf("TRIAC set to low \r\n");
//				GPIO_PinOutClear(gpioPortC, 5); 					//to drive TRIAC
				fn_sendTriacStatus(0x11);

				DBG_PRINT("RTS low\r\n");
				GPIO_PinOutClear(TRIAC_PORT,TRIAC_PIN);
				triacCfg.triacStatus = false;
				fn_saveTriacState();

			}
		break;

		case EMERGENCY_LIGHT_TIMER_ID_2:
			printf("Command send\r\n");
			fn_daliMode1_Level(0xFF, ((snsrCurrStatus.pir_State)?g_m_level:0), 0x11);
			printf("g_m_level = %d\r\n", g_m_level);
		break;

#ifdef AREA
		case TIMER_ID_CONFIGURE_AREA:
			send_packet(PIR, 0xFFFF, false);
#ifdef DALI_FEATURE
			fn_daliMode1_Level(0xFF, 50, 0x16);
#endif
#ifdef ANALOG_FEATURE
			fn_setAnalogIntensity(50, 0x26);
#endif
#ifdef TRIAC_FEATURE
			GPIO_PinOutSet(TRIAC_PORT,TRIAC_PIN);
			triacCfg.triacStatus = true;
			fn_saveTriacState();
			fn_sendTriacStatus(0x16);
#endif


			if(AreaSensorCount != sConfigAreaParameters.TotalArea_Sensors_count)
			{
				uint8_t data[2];
				data[0] = EVENTS_PKT;
				data[1] = CONFIGURE_AREA;		//Didnt receive SHID of all sensors in AREA(Packet loss)
				fn_enQ_blePkt(FT_ACK, 2, data, snsrMinCfg.dest_addr);
			}
		break;
#endif

		default:
		break;
	}
	return ;
}
/*******************************************************************************************************************************/
void fn_loopPIR_cfg(void)
{
	switch(snsrCfg.pir_cfg.cfg_id)
	{
		case CFG_1:
			snsrCfg.pir_cfg.cfg_id = CFG_2;
			snsrCfg.pir_cfg.unoccupancyTimer_s = DEFAULT_PIR_UNOCCUPANCY_TIME_2;
		break;
		case CFG_2:
			snsrCfg.pir_cfg.cfg_id = CFG_3;
			snsrCfg.pir_cfg.unoccupancyTimer_s = DEFAULT_PIR_UNOCCUPANCY_TIME_3;
		break;
		case CFG_3:
			snsrCfg.pir_cfg.cfg_id = CFG_4;
			snsrCfg.pir_cfg.unoccupancyTimer_s = DEFAULT_PIR_UNOCCUPANCY_TIME_4;
		break;
		case CFG_4:
			snsrCfg.pir_cfg.cfg_id = CFG_5;
			snsrCfg.pir_cfg.unoccupancyTimer_s = DEFAULT_PIR_UNOCCUPANCY_TIME_5;
		break;
		case CFG_5:
		default:
			snsrCfg.pir_cfg.cfg_id = CFG_1;
			snsrCfg.pir_cfg.unoccupancyTimer_s = DEFAULT_PIR_UNOCCUPANCY_TIME_1;
		break;
	}
	g_loopInidcationCnt = (2*(snsrCfg.pir_cfg.cfg_id+1));
	gecko_cmd_hardware_set_soft_timer(MILLISECONDS(500),LOOP_MAP_INDICATION,ONESHOT_TIMER);
	DBG_PRINT("pir_cfg id = %d\r\n",snsrCfg.pir_cfg.cfg_id+1);
	DBG_PRINT("pirUnOcc_Tm_ = %d secs\r\n",snsrCfg.pir_cfg.unoccupancyTimer_s);
	snsrMinCfg.cfgAvailable = true;
	DBG_PRINT("CFG SAVE RESPONSE %d\r\n",gecko_cmd_flash_ps_save(PS_SNSR_CFG_BASE,sizeof(snsrCfg),(uint8_t *)&snsrCfg)->result);
	DBG_PRINT("MINCFG SAVE RESPONSE %d\r\n",gecko_cmd_flash_ps_save(PS_SNSR_MINCFG,sizeof(snsrCfg),(uint8_t *)&snsrMinCfg)->result);
	return ;
}
/*******************************************************************************************************************************/
void fn_loopALS_cfg(void)
{
	switch(snsrCfg.als_cfg.cfg_id)
	{
		case CFG_1:
			snsrCfg.als_cfg.cfg_id = CFG_2;
			snsrCfg.als_cfg.freq_LUXmeasure_s = DEFAULT_ALS_FREQ_OF_MEASURMT_2;
		break;
		case CFG_2:
			snsrCfg.als_cfg.cfg_id = CFG_3;
			snsrCfg.als_cfg.freq_LUXmeasure_s = DEFAULT_ALS_FREQ_OF_MEASURMT_3;
		break;
		case CFG_3:
			snsrCfg.als_cfg.cfg_id = CFG_4;
			snsrCfg.als_cfg.freq_LUXmeasure_s = DEFAULT_ALS_FREQ_OF_MEASURMT_4;
		break;
		case CFG_4:
			snsrCfg.als_cfg.cfg_id = CFG_5;
			snsrCfg.als_cfg.freq_LUXmeasure_s = DEFAULT_ALS_FREQ_OF_MEASURMT_5;
		break;
		case CFG_5:
		default:
			snsrCfg.als_cfg.cfg_id = CFG_1;
			snsrCfg.als_cfg.freq_LUXmeasure_s = DEFAULT_ALS_FREQ_OF_MEASURMT_1;
		break;
	}
	g_loopInidcationCnt = (2*(snsrCfg.als_cfg.cfg_id+1));
	gecko_cmd_hardware_set_soft_timer(MILLISECONDS(500),LOOP_MAP_INDICATION,ONESHOT_TIMER);
	DBG_PRINT("als_cfg id = %d\r\n",snsrCfg.als_cfg.cfg_id+1);
	DBG_PRINT("als_frqMsrmnt = %d secs\r\n",snsrCfg.als_cfg.freq_LUXmeasure_s);
	snsrMinCfg.cfgAvailable = true;
	DBG_PRINT("CFG SAVE RESPONSE %d\r\n",gecko_cmd_flash_ps_save(PS_SNSR_CFG_BASE,sizeof(snsrCfg),(uint8_t *)&snsrCfg)->result);
	DBG_PRINT("MINCFG SAVE RESPONSE %d\r\n",gecko_cmd_flash_ps_save(PS_SNSR_MINCFG,sizeof(snsrCfg),(uint8_t *)&snsrMinCfg)->result);
	return ;
}
/*******************************************************************************************************************************/
void fn_deleteSubAddr(void)
{
	struct gecko_msg_mesh_test_get_local_model_sub_rsp_t* rsp;
	struct gecko_msg_mesh_test_del_local_model_sub_rsp_t* delRsp;
	rsp = gecko_cmd_mesh_test_get_local_model_sub(PRIMARY_ELEMENT,MY_VENDOR_ID,MY_MODEL_ID);
	if(!rsp->result)
	{
		for(uint8_t idx = 1;idx<rsp->addresses.len-2;idx+=2)
		{
			printf("delAddr = 0x%4x\r\n",((rsp->addresses.data[idx] <<8)|rsp->addresses.data[idx+1]));
			delRsp = gecko_cmd_mesh_test_del_local_model_sub(PRIMARY_ELEMENT,MY_VENDOR_ID,MY_MODEL_ID,((rsp->addresses.data[idx] <<8)|rsp->addresses.data[idx+1]));
			if(delRsp->result)
			{
				printf("delRsp unsuccess = %d\r\n",delRsp->result);
			}
			else
			{
				printf("delRsp success = %d\r\n",delRsp->result);
			}
		}
	}
	return ;
}
/****************************************************************************************************************/
void fn_initiate_factory_reset(uint8_t resetLevel)
{
	DBG_PRINT("factory reset initiated\r\n");
	if(resetLevel)
	{
		DBG_PRINT("erasing prov data\r\n");
		gecko_cmd_flash_ps_erase_all();
		gecko_cmd_hardware_set_soft_timer(MILLISECONDS(500),RESET_INDICATOR_START,REPEATING_TIMER);
		gecko_cmd_hardware_set_soft_timer(SECONDS(NETWORK_RESET_INTERVAL),FCTORY_RESET_ID,ONESHOT_TIMER);
	}
	else
	{
		DBG_PRINT("Erase minCFG rsp : %d\r\n",gecko_cmd_flash_ps_erase(PS_SNSR_MINCFG)->result);
		DBG_PRINT("Erase snsrCFG \r\n");
		for(uint8_t i = 0;i<5;i++)
		{
 			DBG_PRINT("snsrCfg_%d rsp : %d\r\n",i+1,gecko_cmd_flash_ps_erase(PS_SNSR_CFG_BASE+i)->result);
		}
		DBG_PRINT("Erase ALSCFG rsp : %d\r\n",gecko_cmd_flash_ps_erase(ALS_CALIB_KEY)->result);
		gecko_cmd_hardware_set_soft_timer(MILLISECONDS(500),RESET_INDICATOR_START,REPEATING_TIMER);
		gecko_cmd_hardware_set_soft_timer(SECONDS(FACTORY_RESET_INTERVAL),FCTORY_RESET_ID,ONESHOT_TIMER);
		DBG_PRINT("Erase PS_SNSR_AREA_CONFIG rsp : %d\r\n",gecko_cmd_flash_ps_erase(PS_SNSR_AREA_CONFIG)->result);
		DBG_PRINT("Erase PS_SNSR_AREA_CONFIG_PARAMETERS rsp : %d\r\n",gecko_cmd_flash_ps_erase(PS_SNSR_AREA_CONFIG_PARAMETERS)->result);
		DBG_PRINT("Erase MUX rsp : %d\r\n",gecko_cmd_flash_ps_erase(MUX)->result);
	}
	fn_deleteSubAddr();

	uint8_t freset_ack[2];
	freset_ack[0] = EVENTS_PKT;
	freset_ack[1] = (resetLevel) ? NETWORK_RESET : FACTORY_RESET;
	uint16_t sentRspns = gecko_cmd_mesh_vendor_model_send(
												PRIMARY_ELEMENT,			\
												MY_VENDOR_ID,				\
												MY_MODEL_ID,				\
												0xFFFF,		\
												0,							\
												0,							\
												0,							\
												FT_ACK,			\
												0xff,						\
												sizeof(freset_ack),			\
												freset_ack)->result;
		DBG_PRINT("sent rspns = %d\r\n",sentRspns);

	return ;
}
/****************************************************************************************************************/
void fn_update_snsrCfg(void)
{
	if(snsrMinCfg.cfgAvailable)
	{
		struct gecko_msg_flash_ps_load_rsp_t *rsp = gecko_cmd_flash_ps_load(PS_SNSR_CFG_BASE);
		if(!rsp->result)
		{
			memcpy(&snsrCfg,&rsp->value.data,sizeof(snsrCfg));
			if((!snsrCfg.WL_LIGHT_RcvdExternalCmd_timer) || (snsrCfg.WL_LIGHT_RcvdExternalCmd_timer == 0xFFFF))
			{
				snsrCfg.WL_LIGHT_RcvdExternalCmd_timer = DEFAULT_SNSR_MASKING_TIMEOUT;
				snsrMinCfg.cfgAvailable = true;
				DBG_PRINT("CFG SAVE RESPONSE %d\r\n",gecko_cmd_flash_ps_save(PS_SNSR_CFG_BASE,sizeof(snsrCfg),(uint8_t *)&snsrCfg)->result);
				DBG_PRINT("MINCFG SAVE RESPONSE %d\r\n",gecko_cmd_flash_ps_save(PS_SNSR_MINCFG,sizeof(snsrCfg),(uint8_t *)&snsrMinCfg)->result);
			}
			DBG_PRINT("SnsrCfg is loaded from PS\r\n");
		}
		else
		{
			DBG_PRINT("couldn't Load snsrCfg from PS : error %d\r\n",rsp->result);
		}
	}
	else
	{
		DBG_PRINT("no snsrcfg data in PS\r\n");
	}

#ifdef ALS_FEATURE
		struct gecko_msg_flash_ps_load_rsp_t *rsp = gecko_cmd_flash_ps_load(ALS_CALIB_KEY);
		if(!rsp->result)
		{
			memcpy(&sAlsCalibValue,&rsp->value.data,sizeof(sAlsCalibValue));
			DBG_PRINT("ALS is loaded from PS\r\n");
		}
		else
		{
			DBG_PRINT("couldn't Load ALSCfg from PS : error %d\r\n",rsp->result);
		}
#endif

	// device type and firmware version are not allowed to be set by configuration packet
#ifdef TRIAC_FEATURE
	snsrMinCfg.device_type = TRIAC_SENSOR_DEVICE_ID;
#else
//TODO: Uncomment the line below for SPACE with DALI
	snsrMinCfg.device_type = MORPHOUS_SESNSOR_DEVICE_ID;
//	snsrMinCfg.device_type = TRIAC_SENSOR_DEVICE_ID;			//comment out for SPACE with DALI
#endif

#ifdef PIR_FEATURE
	snsrMinCfg.device_type |= PIR_SENSOR;
#endif

#ifdef ALS_FEATURE
	i2c_txBuffer[0]=ALS_PART_ID;
	if( fn_I2C_MasterWriteonly(ALS_TH_I2C_HANDLE, ALS_WDEV_ADDR,i2c_txBuffer,1) )
	{
		snsrMinCfg.device_type |= ALS_SENSOR;
	}
#endif

#ifdef TH_FEATURE
	i2c_txBuffer[0] = 0xB0;
	i2c_txBuffer[1] = 0x98;
	if(fn_I2C_MasterWriteonly(ALS_TH_I2C_HANDLE, TH_WDEV_ADDR, i2c_txBuffer, 2))
	{
		snsrMinCfg.device_type |= TH_SENSOR;
	}
#endif

	if(mux_control_select == 0)					//DALI device type
	{
		snsrMinCfg.device_type |= 0x0320;
	}
	else if(mux_control_select == 1)			//Analog device type
	{
		snsrMinCfg.device_type |= 0x0310;
	}

	printf("Device Type : 0x%04x\r\n", snsrMinCfg.device_type);
	if(snsrMinCfg.device_type & PIR_SENSOR)
	{
		DBG_PRINT("PIR : unocc[%d] , retrans[%d] , ww[%d]\r\n",
				snsrCfg.pir_cfg.unoccupancyTimer_s,
				snsrCfg.pir_cfg.retransmission_timeout,
				snsrCfg.pir_cfg.wait_watch_Time_ms);
	}
	if(snsrMinCfg.device_type & ALS_SENSOR)
	{
		DBG_PRINT("ALS : freqM[%d] , thrshld[%d] , calbFactr[%d]\r\n",
				snsrCfg.als_cfg.freq_LUXmeasure_s, snsrCfg.als_cfg.luxThreshold,
				snsrCfg.als_cfg.calibration_factor);
	}
	if(snsrMinCfg.device_type & TH_SENSOR)
	{
		DBG_PRINT("TH : freqM[%d] , t_thrshld[%d] , humd_thrshld[%d] , th_reTransmit[%d]\r\n",
				snsrCfg.th_cfg.freq_THmeasure, snsrCfg.th_cfg.tempThreshold,
				snsrCfg.th_cfg.humidityThreshold,snsrCfg.th_cfg.retransmission_timeout);
	}
	struct gecko_msg_flash_ps_load_rsp_t *rsp1 = gecko_cmd_flash_ps_load(DALI_RETENTION_STORAGE_KEY);
	if(!rsp1->result)
	{
		g_sDaliRetention.Dali_RetentionStroageTimeout = rsp1->value.data[0];
		DBG_PRINT("daliRetention Value = %d\r\n",g_sDaliRetention.Dali_RetentionStroageTimeout);
	}
	else
	{
		DBG_PRINT("Couldn't restore the DALI retended values : read rspns %d\r\n",rsp1->result);
	}

	struct gecko_msg_flash_ps_load_rsp_t *sceneCfgrsp = gecko_cmd_flash_ps_load(SCENE_PS_KEY);
	if(!sceneCfgrsp->result)
	{
		if(mux_control_select == 1)
		{
			memcpy(&sSceneCfg,&sceneCfgrsp->value.data,sceneCfgrsp->value.len);
		}
		else if(mux_control_select == 0)
		{
			memcpy(&sScnUIDLP,&sceneCfgrsp->value.data,sceneCfgrsp->value.len);
			DBG_PRINT("sceneCfg is loaded from PS\r\n");
		}
	}
	else
	{
		DBG_PRINT("couldn't Load sceneCfg from PS : error %d\r\n",sceneCfgrsp->result);
	}

	if(mux_control_select == 1)
	{
		rsp1 = gecko_cmd_flash_ps_load (ANALOGSTAT_PS_KEY);
		if(!rsp1->result)
		{
			fn_setAnalogIntensity(rsp1->value.data[0],0);
		}
		rsp1 = gecko_cmd_flash_ps_load (ANALOG_FADE_TIME_PS);
		if(!rsp1->result)
		{
			memcpy(&g_analogFadeTime,rsp1->value.data,sizeof(g_analogFadeTime));
		}
	}
//	struct gecko_msg_flash_ps_save_rsp_t *rsp = gecko_cmd_flash_ps_save(PS_SNSR_MINCFG,sizeof(snsrMinCfg),(uint8_t *)&snsrMinCfg);
//	DBG_PRINT("minSnsrCfg save rsp : %d\r\n",rsp->result);
	return;
}
/****************************************************************************************************************/
void fn_sensorIdentify(void)
{
	static uint16_t idenitfyInterval = 1;
	static uint8_t identifyCnt = 0;
	if(fn_IsSecTimerElapsed(idenitfyInterval,IDENTIFY_INTERVAL))
	{
		LED_TOGGLING(APP_LED1);
		LED_TOGGLING(APP_LED2);
		idenitfyInterval = fn_GetSecTimerStart();
		++identifyCnt;
	}
	if(identifyCnt >= IDENTIFY_COUNT)
	{
		snsrAppData.identify = false;
		identifyCnt = 0;
		TURN_OFF_LED(APP_LED1);
		TURN_OFF_LED(APP_LED2);
	}
	return ;
}

void fn_ALS_Retransmit(void)
{
//	if(fn_IsSecTimerElapsed(snsrAppData.alsRetransmitTimerStart ,snsrCfg.als_cfg.retransmission_timeout))
//	{
//		printf("ALS data retransmitted\r\n");
//		send_packet(ALS,snsrMinCfg.dest_addr,false);
//	}
}
/****************************************************************************************************************/
void fn_thRetransmit(void)
{
	if(fn_IsSecTimerElapsed(snsrAppData.thRetransmitTimerStart,snsrCfg.th_cfg.retransmission_timeout))
	{
		send_packet(TEMP_HUMIDITY,snsrMinCfg.dest_addr,false);
	}
}
/****************************************************************************************************************/
void fn_pirRetransmit(void)
{
	static uint8_t reTrnsState = ZERO;
	switch(reTrnsState)
	{
		case ZERO:
			if(fn_IsSecTimerElapsed(snsrAppData.pirRetransmitTimerStart,PIR_POWER_ON_STATUS_INTERVAL))
			{
				if(!snsrCurrStatus.pir_State)		//snsrCurrStatus.pir_State == UNOCCUPIED
				{
					printf("first Time PIR status out \r\n");
					reTrnsState = TWO;
				}
			}
		break;
		case ONE:
			if(fn_IsSecTimerElapsed(snsrAppData.pirRetransmitTimerStart,snsrCfg.pir_cfg.retransmission_timeout))
			{
				reTrnsState = TWO;
			}
		break;
		case TWO:
		{
			send_packet(PIR,snsrMinCfg.dest_addr,false);
			DBG_PRINT("REtxmsn pkt\r\n");
			reTrnsState = ONE;
		}
		break;
		default:
		break;
	}
	return ;
}
/****************************************************************************************************************/
void fn_SnsrPrcss(void)
{
	if(snsrMinCfg.device_type & PIR_SENSOR)
	{
		fn_PIR_Process();
		fn_pirRetransmit();
	}
	if(snsrMinCfg.device_type & ALS_SENSOR)
	{
		fn_ALSprocess();
		fn_ALS_Retransmit();
	}
	if(snsrMinCfg.device_type & TH_SENSOR)
	{
		fn_THprocess();
		fn_thRetransmit();
	}
	if(snsrAppData.identify)
	{
		fn_sensorIdentify();
	}
	return ;
}
/****************************************************************************************************************/
void fn_ctrlHandshake(void)
{
	static uint8_t handler = HAND_SHAKE_WITH_CTRL;
	static uint16_t itrvlBw_HndShks_start = 0;
	switch(handler)
	{
		case HAND_SHAKE_WITH_CTRL:
			fn_initiate_Iam_Pkt();
			handler = 0xFF;
		break;

		case REPORT_TO_AGG:
			if(sCtrlCfg.cntrl_type || (fn_IsSecTimerElapsed(itrvlBw_HndShks_start,2)))
			{
				if(!sCtrlCfg.cntrl_type)
				{
					DBG_PRINT("retrying handshake with cntrl\r\n");
					handler = HAND_SHAKE_WITH_CTRL;
				}
				else
				{
					DBG_PRINT("handshake with cntrl is successful\r\n");
					DBG_PRINT("cntrl shid = %d",sCtrlCfg.UUID);
					if(!snsrMinCfg.aggBinded)
					{
						DBG_PRINT("no agg mapped\r\n");
						fn_initiate_Iam_Pkt();
					}
					else
					{
						DBG_PRINT("agg mapped = %d\r\n",snsrMinCfg.dest_addr);
					}
					handler = 0xFF;
				}
			}
		break;

		default:
		break;
	}
	return ;
}
/****************************************************************************************************************/
void fn_subscribedAddr(void)
{
	struct gecko_msg_mesh_test_get_local_model_sub_rsp_t* rsp;
	rsp	= gecko_cmd_mesh_test_get_local_model_sub(PRIMARY_ELEMENT,MY_VENDOR_ID,MY_MODEL_ID);
	if(!rsp->result)
	{
		for(uint8_t idx = 1;idx<rsp->addresses.len-2;idx+=2)
		{
			printf("SubAddr = 0x%4x\r\n",((rsp->addresses.data[idx] <<8)|rsp->addresses.data[idx+1]));
		}
	}
	else
	{
		DBG_PRINT("subscribed to NO addr\r\n");
	}
	return ;
}
/****************************************************************************************************************/
void fn_queueLightStat(uint8_t m_shid, uint8_t m_level, uint8_t m_statusEn) {
	uint8_t m_packet_size;
	sensorpayload_t m_data;
//	m_data.packet_num = ++app_data.packet_no;
	m_data.datacmd = LIGHT_STATUS;
	m_data.data[0] = m_shid;
	m_data.data[1] = m_statusEn;
	m_data.data[2] = m_level;

	m_packet_size = 4;		//plus one byte for packet number

	if (snsrAppData.broadCastComplete) {
		fn_enQ_blePkt(FT_STATUSCHANGE, m_packet_size, (uint8_t*) &m_data, snsrMinCfg.dest_addr);
	}
}
/****************************************************************************************************************/
void fn_saveTriacState(void)
{
	struct gecko_msg_flash_ps_save_rsp_t *rsp = gecko_cmd_flash_ps_save(TRIAC_STATUS_KEY,sizeof(triacCfg),(uint8_t *)&triacCfg);
	DBG_PRINT("Triac status save rsp = %d\r\n",rsp->result);
	return ;
}
/****************************************************************************************************************/
void fn_switchOnTriac(void)
{
#ifdef ALS_FEATURE
	if(snsrCurrStatus.als_LUXvalue < sAlsCalibValue.req_lux)
#endif
	{
		DBG_PRINT("RTS high\r\n");
		GPIO_PinOutSet(TRIAC_PORT,TRIAC_PIN);
		triacCfg.triacStatus = true;
		fn_saveTriacState();
	}
	return ;
}
/****************************************************************************************************************/
void fn_switchOffTriac(void)
{
	DBG_PRINT("RTS low\r\n");
	GPIO_PinOutClear(TRIAC_PORT,TRIAC_PIN);
	triacCfg.triacStatus = false;
	fn_saveTriacState();
	return ;
}
/****************************************************************************************************************/
void fn_toggleTriac(void)
{
	GPIO_PinOutToggle(TRIAC_PORT,TRIAC_PIN);
	triacCfg.triacStatus = ~triacCfg.triacStatus;
	fn_saveTriacState();
	return ;
}
/****************************************************************************************************************/
void fn_pirBasedTriacCntrl(void)
{
	if(snsrAppData.switchPIR_interrupt)
	{
		(snsrAppData.switchPIR_state) ?  fn_switchOnTriac() : fn_switchOffTriac();
		snsrAppData.switchPIR_interrupt = false;
	}
	return ;
}
/****************************************************************************************************************/
void fn_sendTriacStatus(uint8_t attribute_Reason)
{

	uint8_t m_packet_size;
	sensorpayload_t m_data;
//	m_data.packet_num = ++app_data.packet_no;
	m_data.datacmd = LIGHT_STATUS;
	m_data.data[0] = 0x5A;
	m_data.data[1] = attribute_Reason;
	m_data.data[2] = (triacCfg.triacStatus)?0x64:0x00;

	m_packet_size = 4;		//plus one byte for packet number

	if (snsrAppData.broadCastComplete)
	{
		fn_enQ_blePkt(FT_STATUSCHANGE, m_packet_size, (uint8_t*) &m_data, snsrMinCfg.dest_addr);
	}
	return ;
}
/****************************************************************************************************************/


void fn_mux_init(void)
{
	GPIO_PinModeSet(MUX_CNTRL_PORT, MUX_CNTRL_PIN, gpioModePushPull, 0);
	if(mux_control_select == 0)
	{
		//DALI operations
		GPIO_PinOutClear(MUX_CNTRL_PORT, MUX_CNTRL_PIN);
	}
	else
	{
		//ANALOG operations
		GPIO_PinOutSet(MUX_CNTRL_PORT, MUX_CNTRL_PIN);
	}
	return;
}

/****************************************************************************************************************/
#ifdef AREA
uint8_t counter = 0;
void fn_AreaControl(void)
{
	uint8_t flag = 0;
	for(int i = 0; i < sConfigAreaParameters.TotalArea_Sensors_count;i++)
	{
		if(sConfigArea[i].Occupancy)
		{
			counter = 0;
			flag = 1;
		}
	}
	if((flag == 0) && (counter ==0))
	{
		printf("Lights off from fn_AreaControl\r\n");
		counter = 1;
#ifdef DALI_FEATURE
		fn_daliMode1_Level(0xFF, 0, 0x16);
#endif
#ifdef ANALOG_FEATURE
		fn_setAnalogIntensity(0, 0x26);
#endif
#ifdef TRIAC_FEATURE
		fn_switchOffTriac();
		fn_sendTriacStatus(0x16);
#endif

	}
}
#endif

void fn_snsRestore(void){
	//restoring data from flash
	struct gecko_msg_flash_ps_load_rsp_t *rsp = gecko_cmd_flash_ps_load(PS_SNSR_MINCFG);
	if(!rsp->result)
	{
		DBG_PRINT("Restored MinCfg Data from PS\n");
		snsrMinCfg_t tData;
		memcpy(&tData,&rsp->value.data,sizeof(tData));
		if(tData.aggBinded)
		{
			snsrMinCfg.aggBinded = tData.aggBinded;
			snsrMinCfg.dest_addr = tData.dest_addr;
		}
		snsrMinCfg.cfgAvailable = tData.cfgAvailable;
		snsrMinCfg.lghtMpng = tData.lghtMpng;
	}

#ifdef AREA
	struct gecko_msg_flash_ps_load_rsp_t *rspns = gecko_cmd_flash_ps_load(PS_SNSR_AREA_CONFIG);
	if(!rspns->result)
	{
		DBG_PRINT("Restored AREA Config Data from PS\n");
		memcpy(&sConfigArea, &rspns->value.data, sizeof(sConfigArea));
	}

	rspns = gecko_cmd_flash_ps_load(PS_SNSR_AREA_CONFIG_PARAMETERS);
	if(!rspns->result)
	{
		DBG_PRINT("Restored AREA Config Parameters Data from PS\n");
		memcpy(&sConfigAreaParameters, &rspns->value.data, sizeof(sConfigAreaParameters));
	}
#endif

	fn_update_snsrCfg();
	gecko_cmd_hardware_set_soft_timer(MILLISECONDS(200), BLEQ_HANDLER, REPEATING_TIMER);


#ifdef DALI_SPACE													//TRIAC
	rsp = gecko_cmd_flash_ps_load(TRIAC_STATUS_KEY);
	if(!rsp->result)
	{
		DBG_PRINT("PIR_prev state\t");
		memcpy(&triacCfg,&rsp->value.data,sizeof(triacCfg));
		if(triacCfg.triacStatus)
		{
			DBG_PRINT("RTS PULLED HIGH\r\n");
			fn_switchOnTriac();
		}
		else
		{
			DBG_PRINT("RTS PULLED LOW\r\n");
			fn_switchOffTriac();
		}
	}
#endif
}
