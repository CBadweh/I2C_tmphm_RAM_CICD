/*
 * @brief Main application file
 *
 * This file is the main application file that initializes and starts the various
 * modules and then runs the super loop.
 *
 * MIT License
 * 
 * Copyright (c) 2021 Eugene R Schroeder
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Day 3 Essential Modules Only
#include "cmd.h"         // Console command infrastructure
#include "console.h"     // User interaction
#include "dio.h"         // Button input
#include "i2c.h"         // I2C bus driver (Day 2)
#include "lwl.h"         // Lightweight logging (Day 3 afternoon)
#include "tmphm.h"       // Temperature/Humidity module (Day 3 - YOU'RE BUILDING THIS!)
#include "tmr.h"         // Timer for periodic sampling
#include "ttys.h"        // Serial UART
#include "module.h"      // Module framework
#include "stat.h"        // Statistics
#include "log.h"         // Logging macros

// Day 4: Fault Handling and Watchdog
#include "fault.h"       // Fault detection and handling
#include "wdg.h"         // Watchdog protection

// Removed (not needed for Day 3):
// #include "blinky.h"   - Visual indicator, not critical
// #include "gps_gtu7.h" - GPS module, unrelated to sensor
// #include "mem.h"      - Memory management, not needed yet

////////////////////////////////////////////////////////////////////////////////
// Common macros
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////////////

enum main_u16_pms {
    CNT_INIT_ERR,
    CNT_START_ERR,
    CNT_RUN_ERR,

    NUM_U16_PMS
};

////////////////////////////////////////////////////////////////////////////////
// Private (static) function declarations
////////////////////////////////////////////////////////////////////////////////

static int32_t cmd_main_status();

////////////////////////////////////////////////////////////////////////////////
// Private (static) variables
////////////////////////////////////////////////////////////////////////////////

static int32_t log_level = LOG_DEFAULT;

static struct cmd_cmd_info cmds[] = {
    {
        .name = "status",
        .func = cmd_main_status,
        .help = "Get main status, usage: main status [clear]",
    },
};

static uint16_t cnts_u16[NUM_U16_PMS];

static const char* cnts_u16_names[NUM_U16_PMS] = {
    "init err",
    "start err",
    "run err",
};

static struct cmd_client_info cmd_info = {
    .name = "main",
    .num_cmds = ARRAY_SIZE(cmds),
    .cmds = cmds,
    .log_level_ptr = &log_level,
    .num_u16_pms = NUM_U16_PMS,
    .u16_pms = cnts_u16,
    .u16_pm_names = cnts_u16_names,
};


// Config info for dio module. These variables must be static since the dio
// module holds a pointer to them.

enum dout_index {
    DIN_BUTTON_1,
    DIN_GPS_PPS,

    DIN_NUM
};

static struct dio_in_info d_inputs[DIN_NUM] = {
    {
        // Button 1
        .name = "Button_1",
        .port = DIO_PORT_C,
        .pin = DIO_PIN_13,
        .pull = DIO_PULL_NO,
        .invert = 1,
    },
    {
        // GPS PPS, connected to PB2 (CN10, pin 22).
        .name = "PPS",
        .port = DIO_PORT_B,
        .pin = DIO_PIN_3,
        .pull = DIO_PULL_NO,
    }
};

enum din_index {
    DOUT_LED_2,

    DOUT_NUM
};

static struct dio_out_info d_outputs[DOUT_NUM] = {
    {
        // LED 2
        .name = "LED_2",
        .port = DIO_PORT_A,
        .pin = DIO_PIN_5,
        .pull = DIO_PULL_NO,
        .init_value = 0,
        .speed = DIO_SPEED_FREQ_LOW,
        .output_type = DIO_OUTPUT_PUSHPULL,
    }
};

static struct dio_cfg dio_cfg = {
    .num_inputs = ARRAY_SIZE(d_inputs),
    .inputs = d_inputs,
    .num_outputs = ARRAY_SIZE(d_outputs),
    .outputs = d_outputs,
};

static struct stat_dur stat_loop_dur;

////////////////////////////////////////////////////////////////////////////////
// Public (global) variables and externs
////////////////////////////////////////////////////////////////////////////////

// Button debouncing for I2C auto test trigger
static bool button_was_pressed = false;
static bool test_completed = false;

////////////////////////////////////////////////////////////////////////////////
// Public (global) functions
////////////////////////////////////////////////////////////////////////////////

void app_main(void)
{
    struct console_cfg console_cfg;
    struct i2c_cfg i2c_cfg;
    struct ttys_cfg ttys_cfg;
    struct tmphm_cfg tmphm_cfg;

    //
    // Invoke the init API on modules the use it.
    //

    setvbuf(stdout, NULL, _IONBF, 0);
    printc("\n========================================\n");
    printc("  DAY 3: TMPHM Module Build Challenge\n");
    printc("========================================\n");
    
    // ===== INIT PHASE: Configure and initialize modules =====
    printc("\n[INIT] Initializing modules...\n");
    
    // Serial UART (for console communication)
    ttys_get_def_cfg(TTYS_INSTANCE_UART2, &ttys_cfg);
    ttys_init(TTYS_INSTANCE_UART2, &ttys_cfg);
    
    // Command processor (handles console commands)
    cmd_init(NULL);
    
    // Console (user interface)
    console_get_def_cfg(&console_cfg);
    console_init(&console_cfg);
    
    // Timer (for TMPHM periodic sampling - CRITICAL!)
    tmr_init(NULL);
    
    // Digital I/O (for button test)
    dio_init(&dio_cfg);
    
    // I2C Driver (Day 2 - foundation for sensor communication)
    i2c_get_def_cfg(I2C_INSTANCE_3, &i2c_cfg);
    i2c_init(I2C_INSTANCE_3, &i2c_cfg);
    
    // TMPHM Module (Day 3 - YOU'RE BUILDING THIS!)
    tmphm_get_def_cfg(TMPHM_INSTANCE_1, &tmphm_cfg);
    tmphm_cfg.i2c_instance_id = I2C_INSTANCE_3;  // Tell TMPHM to use I2C bus 3
    tmphm_init(TMPHM_INSTANCE_1, &tmphm_cfg);

    fault_init(NULL);     // Line ~219
    wdg_init(NULL);       // Line ~220

    // ===== START PHASE: Enable modules for operation =====
    printc("\n[START] Starting modules...\n");
    
    // Serial UART
    ttys_start(TTYS_INSTANCE_UART2);
    
    // Timer (CRITICAL - without this, TMPHM won't get periodic trigger!)
    tmr_start();
    
    // Digital I/O (for button)
    dio_start();
    
    // I2C Driver (enables interrupts, gets guard timer)
    i2c_start(I2C_INSTANCE_3);
    
    // TMPHM Module (registers 1-second timer - YOUR CODE DOES THIS!)
    tmphm_start(TMPHM_INSTANCE_1);
    
    // LWL Logging (Day 3 afternoon - flight recorder)
    lwl_start();
    lwl_enable(true);  // Start recording activity
    LWL("sys_init", 0);                      // Simple log
    LWL("i2c_reserve", 1, LWL_1(3));        // Log with 1-byte arg (I2C instance 3)
    
    // Console commands
    cmd_register(&cmd_info);

    stat_dur_init(&stat_loop_dur);

    // ===== SUPER LOOP: Run modules continuously =====
    printc("\n[READY] Entering super loop...\n");
    printc("Waiting for sensor readings (every 1 second)...\n\n");

    while (1)
    {
        stat_dur_restart(&stat_loop_dur);

        // Console - Handle user commands
        console_run();

        // Timer - Fire callbacks (triggers TMPHM measurements!)
        tmr_run();

        // TMPHM - Process state machine (YOUR CODE!)
        tmphm_run(TMPHM_INSTANCE_1);

        // Button polling for I2C auto test trigger
        int32_t button_state = dio_get(DIN_BUTTON_1);
        if (button_state > 0) {  // Button pressed (active low on this board)
            if (!button_was_pressed) {
                button_was_pressed = true;
                test_completed = false;
                // Button just pressed - start I2C auto test
                printc("\n>> Button pressed - Starting I2C auto test...\n");
            }
            // Poll the auto test state machine only if test hasn't completed yet
            if (!test_completed) {
                if (i2c_run_auto_test() > 0) {
                    printc(">> I2C auto test completed\n\n");
                    test_completed = true;  // Mark test as done for this press
                }
            }
        } else {
            // Button released - reset for next press
            button_was_pressed = false;
            test_completed = false;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Private (static) functions
////////////////////////////////////////////////////////////////////////////////

/*
 * @brief Console command function for "main status".
 *
 * @param[in] argc Number of arguments, including "main"
 * @param[in] argv Argument values, including "main"
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 *
 * Command usage: main status [clear]
 */
static int32_t cmd_main_status(int32_t argc, const char** argv)
{
    bool clear = false;
    bool bad_arg = false;

    if (argc == 3) {
        if (strcasecmp(argv[2], "clear") == 0)
            clear = true;
        else
            bad_arg = true;
    } else if (argc > 3) {
        bad_arg = true;
    }

    if (bad_arg) {
        printc("Invalid arguments\n");
        return MOD_ERR_ARG;
    }

    printc("Super loop samples=%lu min=%lu ms, max=%lu ms, avg=%lu us\n",
           stat_loop_dur.samples, stat_loop_dur.min, stat_loop_dur.max,
           stat_dur_avg_us(&stat_loop_dur));

    if (clear) {
        printc("Clearing loop stat\n");
        stat_dur_init(&stat_loop_dur);
    }
    return 0;
}
