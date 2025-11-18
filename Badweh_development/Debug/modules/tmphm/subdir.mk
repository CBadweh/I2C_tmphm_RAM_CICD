################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../modules/tmphm/tmphm.c 

OBJS += \
./modules/tmphm/tmphm.o 

C_DEPS += \
./modules/tmphm/tmphm.d 


# Each subdirectory must supply rules for building sources it contributes
modules/tmphm/%.o modules/tmphm/%.su modules/tmphm/%.cyclo: ../modules/tmphm/%.c modules/tmphm/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_FULL_LL_DRIVER -DHSE_VALUE=8000000 -DHSE_STARTUP_TIMEOUT=100 -DLSE_STARTUP_TIMEOUT=5000 -DLSE_VALUE=32768 -DEXTERNAL_CLOCK_VALUE=12288000 -DHSI_VALUE=16000000 -DLSI_VALUE=32000 -DVDD_VALUE=3300 -DPREFETCH_ENABLE=1 -DINSTRUCTION_CACHE_ENABLE=1 -DDATA_CACHE_ENABLE=1 -DSTM32F401xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/Sheen/Desktop/Embedded_System/gene_Baremetal_I2CTmphm_RAM_CICD/Badweh_Development/modules/include" -I"C:/Users/Sheen/Desktop/Embedded_System/gene_Baremetal_I2CTmphm_RAM_CICD/Badweh_Development/app1" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-modules-2f-tmphm

clean-modules-2f-tmphm:
	-$(RM) ./modules/tmphm/tmphm.cyclo ./modules/tmphm/tmphm.d ./modules/tmphm/tmphm.o ./modules/tmphm/tmphm.su

.PHONY: clean-modules-2f-tmphm

