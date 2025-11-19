#ifndef _CONFIG_H_
#define _CONFIG_H_

////////////////////////////////////////////////////////////////////////////////
// Below are #defines based directly on "MCU type" #define from the IDE
// (makefile).
////////////////////////////////////////////////////////////////////////////////

#if defined STM32F401xE

    #define CONFIG_STM32_LL_BUS_HDR "stm32f4xx_ll_bus.h"
    #define CONFIG_STM32_LL_CORTEX_HDR "stm32f4xx_ll_cortex.h"
    #define CONFIG_STM32_LL_GPIO_HDR "stm32f4xx_ll_gpio.h"
    #define CONFIG_STM32_LL_I2C_HDR "stm32f4xx_ll_i2c.h"
    #define CONFIG_STM32_LL_RCC_HDR "stm32f4xx_ll_rcc.h"
    #define CONFIG_STM32_LL_USART_HDR "stm32f4xx_ll_usart.h"
    // #define CONFIG_STM32_LL_IWDG_HDR "stm32f4xx_ll_iwdg.h"  // Commented out - file not present in Day 3

    #define CONFIG_DIO_TYPE 1
    #define CONFIG_I2C_TYPE 1
    #define CONFIG_USART_TYPE 1
    #define CONFIG_MPU_TYPE 1

    #define CONFIG_FLASH_TYPE 2
    #define CONFIG_FLASH_BASE_ADDR 0x08000000
    #define CONFIG_FLASH_WRITE_BYTES 8

    #define CONFIG_FAULT_FLASH_PANIC_ADDR 0x08004000

#else
    #error Unknown processor
#endif

////////////////////////////////////////////////////////////////////////////////
// Common settings.
////////////////////////////////////////////////////////////////////////////////

// Module cmd.
#define CONFIG_CMD_MAX_TOKENS 10
#define CONFIG_CMD_MAX_CLIENTS 12

// Modules console and ttys.
#define CONFIG_CONSOLE_PRINT_BUF_SIZE 240
#define CONFIG_TTYS_2_PRESENT 1
#define CONFIG_CONSOLE_DFLT_TTYS_INSTANCE TTYS_INSTANCE_UART2

// Module i2c.
#define CONFIG_I2C_DFLT_TRANS_GUARD_TIME_MS 100
#define CONFIG_I2C_3_PRESENT 1

// For compatibility with modules that use old naming
#define CONFIG_I2C_HAVE_INSTANCE_1 0
#define CONFIG_I2C_HAVE_INSTANCE_2 0
#define CONFIG_I2C_HAVE_INSTANCE_3 1

// Module tmphm.
#define CONFIG_TMPHM_1_DFLT_I2C_INSTANCE I2C_INSTANCE_3
#define CONFIG_TMPHM_1_DFLT_I2C_ADDR 0x44
#define CONFIG_TMPHM_DFLT_SAMPLE_TIME_MS 1000
#define CONFIG_TMPHM_DFLT_MEAS_TIME_MS 17

// Module wdg.
#define CONFIG_WDG_RUN_CHECK_MS 10
#define CONFIG_WDG_HARD_TIMEOUT_MS 4000

////////////////////////////////////////////////////////////////////////////////
// Feature-dependent configuration.
////////////////////////////////////////////////////////////////////////////////

// FAULT feature (enabled for Day 4)
#define CONFIG_FEAT_FAULT 0

#if defined CONFIG_FEAT_FAULT
    #define CONFIG_FAULT_PRESENT 1
    #define CONFIG_LWL_PRESENT 1
    #define CONFIG_WDG_PRESENT 1
    #define CONFIG_FLASH_PRESENT 1

    #define CONFIG_WDG_MAX_INIT_FAILS 3
    #define CONFIG_WDG_INIT_TIMEOUT_MS 8000

    #define CONFIG_FAULT_PANIC_TO_CONSOLE 1
    #define CONFIG_FAULT_PANIC_TO_FLASH 0    // DISABLED: Flash hangs (hardware issue, needs investigation)

    #define CONFIG_TMPHM_WDG_ID 0
    #define CONFIG_WDG_NUM_WDGS 1
#endif

#endif // _CONFIG_H_
