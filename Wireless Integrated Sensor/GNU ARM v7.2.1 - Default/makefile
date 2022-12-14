################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include se_manager/src/subdir.mk
-include protocol/bluetooth/lib/EFR32XG21/GCC/subdir.mk
-include protocol/bluetooth/bt_mesh/src/subdir.mk
-include protocol/bluetooth/ble_stack/src/soc/subdir.mk
-include platform/service/sleeptimer/src/subdir.mk
-include platform/radio/rail_lib/plugin/coexistence/hal/efr32/subdir.mk
-include platform/radio/rail_lib/plugin/coexistence/common/subdir.mk
-include platform/emlib/src/subdir.mk
-include platform/emdrv/sleep/src/subdir.mk
-include platform/emdrv/nvm3/src/subdir.mk
-include platform/emdrv/gpiointerrupt/src/subdir.mk
-include platform/Device/SiliconLabs/EFR32MG21/Source/GCC/subdir.mk
-include platform/Device/SiliconLabs/EFR32MG21/Source/subdir.mk
-include hardware/kit/common/drivers/subdir.mk
-include hardware/kit/common/bsp/subdir.mk
-include application/src/gsLink/subdir.mk
-include application/src/dali/subdir.mk
-include application/src/analog/subdir.mk
-include application/src/subdir.mk
-include .metadata/.plugins/org.eclipse.cdt.make.core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Gryd_WireLess_DALI_1024_PIR_ALS_TH.axf

# Tool invocations
Gryd_WireLess_DALI_1024_PIR_ALS_TH.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m33 -mthumb -T "D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\efr32mg21a020f1024im32.ld" -Wl,--undefined,sl_app_properties,--undefined,__Vectors,--undefined,__aeabi_uldivmod,--undefined,ceil,--undefined,__nvm3Base -Xlinker -no-enum-size-warning -Xlinker -no-wchar-size-warning -Xlinker --gc-sections -Xlinker -Map="Gryd_WireLess_DALI_1024_PIR_ALS_TH.map" -mfpu=fpv5-sp-d16 -mfloat-abi=hard --specs=nano.specs -o Gryd_WireLess_DALI_1024_PIR_ALS_TH.axf -Wl,--start-group "./.metadata/.plugins/org.eclipse.cdt.make.core/specs.o" "./application/src/BLE_Queue.o" "./application/src/S1_pir.o" "./application/src/S2_als.o" "./application/src/S3_th.o" "./application/src/Self-Provision_Functions.o" "./application/src/app.o" "./application/src/command_handler.o" "./application/src/device_Init.o" "./application/src/i2c.o" "./application/src/intervalTimer.o" "./application/src/switch.o" "./application/src/analog/mcp45hvx1.o" "./application/src/dali/dali.o" "./application/src/dali/daliApplication.o" "./application/src/dali/daliCommand.o" "./application/src/dali/daliStatus.o" "./application/src/gsLink/GSLink_uart.o" "./application/src/gsLink/GS_Link.o" "./application/src/gsLink/gsLinkPktQ.o" "./dcd.o" "./gatt_db.o" "./init_mcu.o" "./main.o" "./hardware/kit/common/bsp/bsp_stk.o" "./hardware/kit/common/bsp/bsp_trace.o" "./hardware/kit/common/drivers/i2cspm.o" "./hardware/kit/common/drivers/retargetio.o" "./hardware/kit/common/drivers/retargetswo.o" "./hardware/kit/common/drivers/udelay.o" "./platform/Device/SiliconLabs/EFR32MG21/Source/GCC/startup_efr32mg21.o" "./platform/Device/SiliconLabs/EFR32MG21/Source/system_efr32mg21.o" "./platform/emdrv/gpiointerrupt/src/gpiointerrupt.o" "./platform/emdrv/nvm3/src/nvm3_default.o" "./platform/emdrv/nvm3/src/nvm3_hal_flash.o" "./platform/emdrv/nvm3/src/nvm3_lock.o" "./platform/emdrv/sleep/src/sleep.o" "./platform/emlib/src/em_assert.o" "./platform/emlib/src/em_burtc.o" "./platform/emlib/src/em_cmu.o" "./platform/emlib/src/em_core.o" "./platform/emlib/src/em_cryotimer.o" "./platform/emlib/src/em_crypto.o" "./platform/emlib/src/em_emu.o" "./platform/emlib/src/em_eusart.o" "./platform/emlib/src/em_gpio.o" "./platform/emlib/src/em_i2c.o" "./platform/emlib/src/em_msc.o" "./platform/emlib/src/em_rmu.o" "./platform/emlib/src/em_rtcc.o" "./platform/emlib/src/em_se.o" "./platform/emlib/src/em_system.o" "./platform/emlib/src/em_timer.o" "./platform/emlib/src/em_usart.o" "./platform/radio/rail_lib/plugin/coexistence/common/coexistence.o" "./platform/radio/rail_lib/plugin/coexistence/hal/efr32/coexistence-hal.o" "./platform/service/sleeptimer/src/sl_sleeptimer.o" "./platform/service/sleeptimer/src/sl_sleeptimer_hal_prortc.o" "./platform/service/sleeptimer/src/sl_sleeptimer_hal_rtcc.o" "./protocol/bluetooth/ble_stack/src/soc/coexistence-ble.o" "./protocol/bluetooth/ble_stack/src/soc/coexistence_counters-ble.o" "./protocol/bluetooth/bt_mesh/src/bg_application_properties.o" "./protocol/bluetooth/bt_mesh/src/mesh_lib.o" "./protocol/bluetooth/bt_mesh/src/mesh_sensor.o" "./protocol/bluetooth/bt_mesh/src/mesh_serdeser.o" "./se_manager/src/sl_se_manager.o" "./se_manager/src/sl_se_manager_attestation.o" "./se_manager/src/sl_se_manager_cipher.o" "./se_manager/src/sl_se_manager_entropy.o" "./se_manager/src/sl_se_manager_hash.o" "./se_manager/src/sl_se_manager_key_derivation.o" "./se_manager/src/sl_se_manager_key_handling.o" "./se_manager/src/sl_se_manager_signature.o" "./se_manager/src/sl_se_manager_util.o" "D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\radio\rail_lib\autogen\librail_release\librail_efr32xg21_gcc_release.a" "D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\protocol\bluetooth\lib\EFR32XG21\GCC\libbluetooth_mesh.a" "D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\emdrv\nvm3\lib\libnvm3_CM33_gcc.a" "D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\protocol\bluetooth\lib\EFR32XG21\GCC\binapploader.o" -lm -Wl,--end-group -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: Gryd_WireLess_DALI_1024_PIR_ALS_TH.hex'
	arm-none-eabi-objcopy -O ihex "Gryd_WireLess_DALI_1024_PIR_ALS_TH.axf" "Gryd_WireLess_DALI_1024_PIR_ALS_TH.hex"
	@echo ' '

	@echo 'Building bin file: Gryd_WireLess_DALI_1024_PIR_ALS_TH.bin'
	arm-none-eabi-objcopy -O binary "Gryd_WireLess_DALI_1024_PIR_ALS_TH.axf" "Gryd_WireLess_DALI_1024_PIR_ALS_TH.bin"
	@echo ' '

	@echo 'Building s37 file: Gryd_WireLess_DALI_1024_PIR_ALS_TH.s37'
	arm-none-eabi-objcopy -O srec "Gryd_WireLess_DALI_1024_PIR_ALS_TH.axf" "Gryd_WireLess_DALI_1024_PIR_ALS_TH.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "Gryd_WireLess_DALI_1024_PIR_ALS_TH.axf" -A
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) Gryd_WireLess_DALI_1024_PIR_ALS_TH.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
