################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../modules/i2c/i2c.c 

OBJS += \
./modules/i2c/i2c.o 

C_DEPS += \
./modules/i2c/i2c.d 


# Each subdirectory must supply rules for building sources it contributes
modules/i2c/%.o modules/i2c/%.su modules/i2c/%.cyclo: ../modules/i2c/%.c modules/i2c/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32F401xE -DUSE_FULL_LL_DRIVER -DHSE_VALUE=8000000 -DHSE_STARTUP_TIMEOUT=100 -DLSE_STARTUP_TIMEOUT=5000 -DLSE_VALUE=32768 -DEXTERNAL_CLOCK_VALUE=12288000 -DHSI_VALUE=16000000 -DLSI_VALUE=32000 -DVDD_VALUE=3300 -DPREFETCH_ENABLE=1 -DINSTRUCTION_CACHE_ENABLE=1 -DDATA_CACHE_ENABLE=1 -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/Sheen/Desktop/Embedded_System/gene_Baremetal_I2CTmphm_RAM_CICD/tmphm/modules/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-modules-2f-i2c

clean-modules-2f-i2c:
	-$(RM) ./modules/i2c/i2c.cyclo ./modules/i2c/i2c.d ./modules/i2c/i2c.o ./modules/i2c/i2c.su

.PHONY: clean-modules-2f-i2c

