################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../platform/emdrv/gpiointerrupt/src/gpiointerrupt.c 

OBJS += \
./platform/emdrv/gpiointerrupt/src/gpiointerrupt.o 

C_DEPS += \
./platform/emdrv/gpiointerrupt/src/gpiointerrupt.d 


# Each subdirectory must supply rules for building sources it contributes
platform/emdrv/gpiointerrupt/src/gpiointerrupt.o: ../platform/emdrv/gpiointerrupt/src/gpiointerrupt.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m33 -mthumb -std=c99 '-DENABLE_LOGGING=1' '-D__STACK_SIZE=0xD00' '-DNVM3_DEFAULT_NVM_SIZE=24576' '-D__HEAP_SIZE=0x1500' '-DMESH_LIB_NATIVE=1' '-DNVM3_DEFAULT_MAX_OBJECT_SIZE=512' '-DHAL_CONFIG=1' '-DEFR32MG21A010F1024IM32=1' -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\se_manager\inc" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\bootloader\api" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\emdrv\sleep\src" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\Device\SiliconLabs\EFR32MG21\Include" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\emlib\inc" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\CMSIS\Include" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\hardware\kit\common\drivers" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\hardware\kit\common\bsp" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\emdrv\uartdrv\inc" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\emlib\src" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\hardware\kit\common\halconfig" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\protocol\bluetooth\bt_mesh\src" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\protocol\bluetooth\ble_stack\inc\soc" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\common\inc" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\service\sleeptimer\src" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\service\sleeptimer\inc" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\emdrv\nvm3\src" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\protocol\bluetooth\bt_mesh\inc\common" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\protocol\bluetooth\bt_mesh\inc" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\emdrv\nvm3\inc" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\halconfig\inc\hal-config" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\Device\SiliconLabs\EFR32MG21\Source\GCC" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\radio\rail_lib\plugin\coexistence\hal\efr32" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\radio\rail_lib\common" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\service\sleeptimer\config" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\protocol\bluetooth\ble_stack\src\soc" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\protocol\bluetooth\bt_mesh\inc\soc" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\emdrv\gpiointerrupt\inc" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\radio\rail_lib\chip\efr32\efr32xg2x" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\emdrv\sleep\inc" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\emdrv\common\inc" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\radio\rail_lib\protocol\ieee802154" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\radio\rail_lib\plugin\coexistence\common" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\Device\SiliconLabs\EFR32MG21\Source" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\radio\rail_lib\protocol\ble" -I"D:\GitHub\grydlight_integrated\Wireless Integrated Sensor\platform\radio\rail_lib\plugin" -O0 -fno-builtin -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv5-sp-d16 -mfloat-abi=hard -MMD -MP -MF"platform/emdrv/gpiointerrupt/src/gpiointerrupt.d" -MT"platform/emdrv/gpiointerrupt/src/gpiointerrupt.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


