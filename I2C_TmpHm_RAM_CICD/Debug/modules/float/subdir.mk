################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../modules/float/float.c \
../modules/float/whetstone.c 

OBJS += \
./modules/float/float.o \
./modules/float/whetstone.o 

C_DEPS += \
./modules/float/float.d \
./modules/float/whetstone.d 


# Each subdirectory must supply rules for building sources it contributes
modules/float/%.o modules/float/%.su modules/float/%.cyclo: ../modules/float/%.c modules/float/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32F401xE -DUSE_FULL_LL_DRIVER -DHSE_VALUE=8000000 -DHSE_STARTUP_TIMEOUT=100 -DLSE_STARTUP_TIMEOUT=5000 -DLSE_VALUE=32768 -DEXTERNAL_CLOCK_VALUE=12288000 -DHSI_VALUE=16000000 -DLSI_VALUE=32000 -DVDD_VALUE=3300 -DPREFETCH_ENABLE=1 -DINSTRUCTION_CACHE_ENABLE=1 -DDATA_CACHE_ENABLE=1 -DCONFIG_FEAT_FAULT=1 -DCONFIG_FEAT_TMPHM=1 -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/Sheen/Desktop/Embedded_System/gene_Baremetal_I2CTmphm_RAM_CICD/I2C_TmpHm_RAM_CICD/modules/include" -O0 -ffunction-sections -fdata-sections -Wall -Werror -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-modules-2f-float

clean-modules-2f-float:
	-$(RM) ./modules/float/float.cyclo ./modules/float/float.d ./modules/float/float.o ./modules/float/float.su ./modules/float/whetstone.cyclo ./modules/float/whetstone.d ./modules/float/whetstone.o ./modules/float/whetstone.su

.PHONY: clean-modules-2f-float

