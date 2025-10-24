################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include modules/wdg/subdir.mk
-include modules/ttys/subdir.mk
-include modules/tmr/subdir.mk
-include modules/tmphm/subdir.mk
-include modules/step/subdir.mk
-include modules/stat/subdir.mk
-include modules/os/subdir.mk
-include modules/mem/subdir.mk
-include modules/lwl/subdir.mk
-include modules/log/subdir.mk
-include modules/i2c/subdir.mk
-include modules/gps_gtu7/subdir.mk
-include modules/float/subdir.mk
-include modules/flash/subdir.mk
-include modules/fault/subdir.mk
-include modules/draw/subdir.mk
-include modules/dio/subdir.mk
-include modules/console/subdir.mk
-include modules/cmd/subdir.mk
-include modules/can/subdir.mk
-include modules/blinky/subdir.mk
-include app1/subdir.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := ram-class-nucleo-f401re
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
ram-class-nucleo-f401re.elf \

MAP_FILES += \
ram-class-nucleo-f401re.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
ram-class-nucleo-f401re.list \


# All Target
all: main-build

# Main-build Target
main-build: ram-class-nucleo-f401re.elf secondary-outputs

# Tool invocations
ram-class-nucleo-f401re.elf ram-class-nucleo-f401re.map: $(OBJS) $(USER_OBJS) C:\Users\Sheen\Desktop\Embedded_System\gene_Baremetal_I2CTmphm_RAM_CICD\ram-class-nucleo-f401re\STM32F401RETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "ram-class-nucleo-f401re.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Users\Sheen\Desktop\Embedded_System\gene_Baremetal_I2CTmphm_RAM_CICD\ram-class-nucleo-f401re\STM32F401RETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="ram-class-nucleo-f401re.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

ram-class-nucleo-f401re.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "ram-class-nucleo-f401re.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) default.size.stdout ram-class-nucleo-f401re.elf ram-class-nucleo-f401re.list ram-class-nucleo-f401re.map
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
