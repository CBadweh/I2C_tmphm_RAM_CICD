/*
 * @brief Main application file - Day 3: TMPHM Integration
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

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "console.h"
#include "dio.h"
#include "i2c.h"
#include "lwl.h"
#include "module.h"
#include "tmphm.h"
#include "ttys.h"
#include "tmr.h"

////////////////////////////////////////////////////////////////////////////////
// OPERATING MODE SELECTION
////////////////////////////////////////////////////////////////////////////////
//
// This application supports TWO mutually exclusive modes:
//
// MODE A: TMPHM Automatic Sensor Sampling (Day 3 - Production Mode)
//   - Temperature/humidity sensor runs in background
//   - Samples every 1 second automatically via timer
//   - Use console commands to query: "tmphm test lastmeas 0"
//   - CURRENTLY ENABLED (default for Day 3)
//
// MODE B: Manual I2C Button Test (Day 2 - Debug Mode)
//   - Press USER button to trigger I2C test sequence
//   - Manually tests I2C driver functionality
//   - CURRENTLY DISABLED (for Day 3)
//
// WHY MUTUALLY EXCLUSIVE? Both use the same I2C bus (I2C_INSTANCE_3).
// Running both simultaneously would require sophisticated arbitration.
// For learning, we run one at a time.
//
// TO SWITCH MODES:
//   1. Find the "MODE A" and "MODE B" sections below
//   2. Comment out one mode, uncomment the other
//   3. Rebuild with: ci-cd-tools/build.bat
//
////////////////////////////////////////////////////////////////////////////////

// Set this to 1 for MODE A (TMPHM), 0 for MODE B (Button Test)
#define ENABLE_MODE_A_TMPHM 1

////////////////////////////////////////////////////////////////////////////////
// Common macros
////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////
// Public (global) variables and externs
////////////////////////////////////////////////////////////////////////////////

// Button debouncing for I2C auto test trigger (MODE B only)
#if !ENABLE_MODE_A_TMPHM
static bool button_was_pressed = false;
static bool test_completed = false;
#endif

////////////////////////////////////////////////////////////////////////////////
// Public (global) functions
////////////////////////////////////////////////////////////////////////////////

void app_main(void)
{
    struct console_cfg console_cfg;
    struct i2c_cfg i2c_cfg;
    struct tmr_cfg tmr_cfg;
    struct ttys_cfg ttys_cfg;
    
#if ENABLE_MODE_A_TMPHM
    struct tmphm_cfg tmphm_cfg;
#endif

    //
    // Invoke the init API on modules the use it.
    //

    setvbuf(stdout, NULL, _IONBF, 0);
    printc("\n========================================\n");
    printc("  DAY 3: TMPHM Module Integration\n");
    printc("========================================\n");
    
#if ENABLE_MODE_A_TMPHM
    printc("\nMODE: TMPHM Automatic Sensor Sampling\n");
    printc("      (Background operation, 1 sec cycle)\n");
    printc("      Query with: tmphm test lastmeas 0\n");
#else
    printc("\nMODE: Manual I2C Button Test\n");
    printc("      (Press USER button to test)\n");
#endif
    
    // ===== INIT PHASE: Configure and initialize modules =====
    printc("\n[INIT] Initializing modules...\n");
    
    // Lightweight logging (for production diagnostics)
    // Note: LWL has no init function, just start
    
    // Timer module (required for guard timers and TMPHM timing)
    memset(&tmr_cfg, 0, sizeof(tmr_cfg));
    tmr_init(&tmr_cfg);

    // Serial UART (for console communication)
    ttys_get_def_cfg(TTYS_INSTANCE_UART2, &ttys_cfg);
    ttys_init(TTYS_INSTANCE_UART2, &ttys_cfg);
    
    // Console (user interface)
    console_get_def_cfg(&console_cfg);
    console_init(&console_cfg);
    
    // Digital I/O (for button test in MODE B)
    dio_init(&dio_cfg);
    
    // I2C Driver (Day 2 - foundation for sensor communication)
    i2c_get_def_cfg(I2C_INSTANCE_3, &i2c_cfg);
    i2c_init(I2C_INSTANCE_3, &i2c_cfg);
    
    // ========== MODE A: TMPHM Module ==========
#if ENABLE_MODE_A_TMPHM
    tmphm_get_def_cfg(TMPHM_INSTANCE_1, &tmphm_cfg);
    tmphm_init(TMPHM_INSTANCE_1, &tmphm_cfg);
    printc("  - TMPHM (Temp/Humidity) initialized\n");
#endif

    // ===== START PHASE: Enable modules for operation =====
    printc("\n[START] Starting modules...\n");
    
    // Lightweight logging (Day 3 - production diagnostics with LWL)
    lwl_start();
    lwl_enable(true);  // Enable LWL recording
    printc("  - LWL (Lightweight Logging) started\n");
    
    // Timer module
    tmr_start();

    // Serial UART
    ttys_start(TTYS_INSTANCE_UART2);
    
    // Digital I/O
    dio_start();
    
    // I2C Driver (enables interrupts, gets guard timer)
    i2c_start(I2C_INSTANCE_3);
    
    // ========== MODE A: TMPHM Module ==========
#if ENABLE_MODE_A_TMPHM
    tmphm_start(TMPHM_INSTANCE_1);
    printc("  - TMPHM started (1-sec sampling)\n");
#endif

    // ===== SUPER LOOP: Run modules continuously =====
    printc("\n[READY] Entering super loop...\n");
    
#if ENABLE_MODE_A_TMPHM
    printc("TMPHM running in background.\n");
    printc("Console commands available:\n");
    printc("  - tmphm status\n");
    printc("  - tmphm test lastmeas 0\n");
    printc("  - i2c status\n\n");
#else
    printc("Press USER button to run I2C auto test.\n\n");
#endif

    while (1)
    {
        // Console - Handle user commands (always active)
        console_run();
        tmr_run();

        // ========== MODE A: TMPHM Automatic Sampling ==========
#if ENABLE_MODE_A_TMPHM
        tmphm_run(TMPHM_INSTANCE_1);
        
        // ========== MODE B: Manual I2C Button Test ==========
#else
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
#endif
    }
}
