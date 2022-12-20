################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../.metadata/.plugins/org.eclipse.cdt.make.core/specs.c 

OBJS += \
./.metadata/.plugins/org.eclipse.cdt.make.core/specs.o 

C_DEPS += \
./.metadata/.plugins/org.eclipse.cdt.make.core/specs.d 


# Each subdirectory must supply rules for building sources it contributes
.metadata/.plugins/org.eclipse.cdt.make.core/specs.o: ../.metadata/.plugins/org.eclipse.cdt.make.core/specs.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m33 -mthumb -std=c99 '-DENABLE_LOGGING=1' '-D__STACK_SIZE=0xD00' '-DNVM3_DEFAULT_NVM_SIZE=24576' '-D__HEAP_SIZE=0x1500' '-DMESH_LIB_NATIVE=1' '-DNVM3_DEFAULT_MAX_OBJECT_SIZE=512' '-DHAL_CONFIG=1' '-DEFR32BG21A010F1024IM32=1' -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\se_manager\inc" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\bootloader\api" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\emdrv\sleep\src" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\Device\SiliconLabs\EFR32MG21\Include" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\emlib\inc" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\CMSIS\Include" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\hardware\kit\common\drivers" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\hardware\kit\common\bsp" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\emdrv\uartdrv\inc" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\emlib\src" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\hardware\kit\common\halconfig" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\protocol\bluetooth\bt_mesh\src" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\protocol\bluetooth\ble_stack\inc\soc" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\common\inc" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\service\sleeptimer\src" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\service\sleeptimer\inc" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\emdrv\nvm3\src" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\protocol\bluetooth\bt_mesh\inc\common" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\protocol\bluetooth\bt_mesh\inc" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\emdrv\nvm3\inc" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\halconfig\inc\hal-config" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\Device\SiliconLabs\EFR32MG21\Source\GCC" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\radio\rail_lib\plugin\coexistence\hal\efr32" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\radio\rail_lib\common" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\service\sleeptimer\config" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\protocol\bluetooth\ble_stack\src\soc" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\protocol\bluetooth\bt_mesh\inc\soc" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\emdrv\gpiointerrupt\inc" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\radio\rail_lib\chip\efr32\efr32xg2x" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\emdrv\sleep\inc" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\emdrv\common\inc" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\radio\rail_lib\protocol\ieee802154" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\radio\rail_lib\plugin\coexistence\common" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\Device\SiliconLabs\EFR32MG21\Source" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\radio\rail_lib\protocol\ble" -I"C:\Github\grydlight_integrated\grydlight_integrated\Wireless Integrated Sensor\platform\radio\rail_lib\plugin" -O0 -fno-builtin -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv5-sp-d16 -mfloat-abi=hard -MMD -MP -MF".metadata/.plugins/org.eclipse.cdt.make.core/specs.d" -MT".metadata/.plugins/org.eclipse.cdt.make.core/specs.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


