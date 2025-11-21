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

#include "config.h"
#include CONFIG_STM32_LL_RCC_HDR // Needed for IDE bug (see below).

#include "cmd.h"
#include "console.h"
#include "fault.h"
#include "flash.h"
#include "i2c.h"
#include "lwl.h"
#include "module.h"
#include "os.h"
#include "tmphm.h"
#include "ttys.h"
#include "tmr.h"
#include "wdg.h"

////////////////////////////////////////////////////////////////////////////////
// Common macros
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Private (static) function declarations
////////////////////////////////////////////////////////////////////////////////

static int32_t cmd_main_status();

////////////////////////////////////////////////////////////////////////////////
// Private (static) variables
////////////////////////////////////////////////////////////////////////////////

static struct cmd_cmd_info cmds[] = {
    {
        .name = "status",
        .func = cmd_main_status,
        .help = "Get main status, usage: main status [clear]",
    },
};

static struct cmd_client_info cmd_info = {
    .name = "main",
    .num_cmds = ARRAY_SIZE(cmds),
    .cmds = cmds,
};

struct console_cfg console_cfg;
static struct i2c_cfg i2c_cfg;
static struct ttys_cfg ttys_cfg_2;
static struct tmphm_cfg tmphm_cfg;

////////////////////////////////////////////////////////////////////////////////
// Public (global) variables and externs
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Public (global) functions
////////////////////////////////////////////////////////////////////////////////

void app_main(void)
{
    struct tmr_cfg tmr_cfg;
    struct fault_cfg fault_cfg;
    struct wdg_cfg wdg_cfg;

    wdg_start_init_hdw_wdg();

    memset(&tmr_cfg, 0, sizeof(tmr_cfg));
    memset(&fault_cfg, 0, sizeof(fault_cfg));
    memset(&wdg_cfg, 0, sizeof(wdg_cfg));

    setvbuf(stdout, NULL, _IONBF, 0);
    printc("\nInit: Init modules\n");

    tmr_init(&tmr_cfg);

    ttys_get_def_cfg(TTYS_INSTANCE_2, &ttys_cfg_2);
    ttys_init(TTYS_INSTANCE_2, &ttys_cfg_2);

    fault_init(&fault_cfg);

    cmd_init(NULL);

    console_get_def_cfg(&console_cfg);
    console_init(&console_cfg);

    i2c_get_def_cfg(I2C_INSTANCE_3, &i2c_cfg);
    i2c_init(I2C_INSTANCE_3, &i2c_cfg);

    tmphm_get_def_cfg(TMPHM_INSTANCE_1, &tmphm_cfg);
    tmphm_cfg.i2c_instance_id = I2C_INSTANCE_3;
    tmphm_init(TMPHM_INSTANCE_1, &tmphm_cfg);

    printc("Init: Start modules\n");

    ttys_start(TTYS_INSTANCE_2);
    fault_start();
    flash_start();
    lwl_start();
    lwl_enable(true);
    wdg_init(&wdg_cfg);
    wdg_start();
    tmr_start();
    i2c_start(I2C_INSTANCE_3);
    tmphm_start(TMPHM_INSTANCE_1);

    cmd_register(&cmd_info);

    printc("Init: Enter super loop\n");
    wdg_init_successful();
    wdg_start_hdw_wdg(CONFIG_WDG_HARD_TIMEOUT_MS);

    printc("\n[READY] Entering super loop...\n");
    printc("TMPHM running in background.\n");
    printc("Console commands available:\n");
    printc("  - tmphm status\n");
    printc("  - tmphm test lastmeas 0\n");
    printc("  - i2c status\n\n");

    while (1)
    {
        console_run();
        tmr_run();
        tmphm_run(TMPHM_INSTANCE_1);
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



    if (clear) {
        printc("Clearing loop stat\n");

    }
    return 0;
}
