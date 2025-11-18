/*
 * @brief Main application file
 *
 * This file is the main application file that initializes and starts the various
 * modules and then runs the super loop.
 *
 * Based on template by Eugene R Schroeder
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Day 1 Essential Modules Only
#include "cmd.h"         // Console command infrastructure
#include "console.h"     // User interaction
#include "i2c.h"         // I2C bus driver (Day 1 - adding error detection)
#include "tmphm.h"       // Temperature/Humidity module
#include "tmr.h"         // Timer for periodic sampling
#include "ttys.h"        // Serial UART
#include "module.h"      // Module framework



////////////////////////////////////////////////////////////////////////////////
// Common macros
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Private (static) function declarations
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Private (static) variables
////////////////////////////////////////////////////////////////////////////////

static struct cmd_client_info cmd_info = {
    .name = "main",
    .num_cmds = 0,
    .cmds = NULL,
    .log_level_ptr = NULL,
    .num_u16_pms = 0,
    .u16_pms = NULL,
    .u16_pm_names = NULL,
};

////////////////////////////////////////////////////////////////////////////////
// Public (global) variables and externs
////////////////////////////////////////////////////////////////////////////////


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
    printc("  DAY 1: I2C Error Detection\n");
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
    
    // Timer (for periodic operations)
    tmr_init(NULL);
    
    // I2C Driver (Day 1 - adding error detection and return codes)
    i2c_get_def_cfg(I2C_INSTANCE_3, &i2c_cfg);
    i2c_init(I2C_INSTANCE_3, &i2c_cfg);
    
    // TMPHM Module
    tmphm_get_def_cfg(TMPHM_INSTANCE_1, &tmphm_cfg);
    tmphm_cfg.i2c_instance_id = I2C_INSTANCE_3;  // Tell TMPHM to use I2C bus 3
    tmphm_init(TMPHM_INSTANCE_1, &tmphm_cfg);

    // ===== START PHASE: Enable modules for operation =====
    printc("\n[START] Starting modules...\n");

    // Serial UART
    ttys_start(TTYS_INSTANCE_UART2);
    
    // Timer
    tmr_start();
    
    // I2C Driver (enables interrupts, gets guard timer)
    i2c_start(I2C_INSTANCE_3);
    
    // TMPHM Module
    tmphm_start(TMPHM_INSTANCE_1);
    
    // Console commands
    cmd_register(&cmd_info);

    // ===== SUPER LOOP: Run modules continuously =====
    printc("\n[READY] Entering super loop...\n");
    printc("Use console commands to test I2C error detection...\n\n");

    while (1)
    {
        // Console - Handle user commands
        console_run();

        // Timer - Fire callbacks (triggers TMPHM measurements!)
        tmr_run();

        // TMPHM - Process state machine (YOUR CODE!)
        tmphm_run(TMPHM_INSTANCE_1);

        // I2C - Process state machine (handles auto test polling)
        i2c_run(I2C_INSTANCE_3);
    }
}
