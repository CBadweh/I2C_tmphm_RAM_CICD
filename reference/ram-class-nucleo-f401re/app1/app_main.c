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
// Type definitions
////////////////////////////////////////////////////////////////////////////////

#define MOD_NO_INSTANCE -1

typedef int32_t (*mod_get_def_cfg)(void* cfg);
typedef int32_t (*mod_init)(void* cfg);
typedef int32_t (*mod_start)(void);
typedef int32_t (*mod_run)(void);

typedef int32_t (*mod_instance_get_def_cfg)(int instanced, void* cfg);
typedef int32_t (*mod_instance_init)(int instance, void* cfg);
typedef int32_t (*mod_instance_start)(int instance);
typedef int32_t (*mod_instance_run)(int instance);

struct mod_info {
    const char* name;
    int instance;
    union {
        struct {
            mod_get_def_cfg mod_get_def_cfg;
            mod_init mod_init;
            mod_start mod_start;
            mod_run mod_run;
        } singleton;
        struct {
            mod_instance_get_def_cfg mod_get_def_cfg;
            mod_instance_init mod_init;
            mod_instance_start mod_start;
            mod_instance_run mod_run;
        } multi_instance;
    } ops;
    void* cfg_obj;
};


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


static struct mod_info mods[] = {

    {
        .name = "ttys",
        .instance = TTYS_INSTANCE_2,
        .ops.multi_instance.mod_get_def_cfg =
            (mod_instance_get_def_cfg)ttys_get_def_cfg,
        .ops.multi_instance.mod_init = (mod_instance_init)ttys_init,
        .ops.multi_instance.mod_start = (mod_instance_start)ttys_start,
        .cfg_obj = &ttys_cfg_2,
    },

    {
        .name = "fault",
        .instance = MOD_NO_INSTANCE,
        .ops.singleton.mod_init = (mod_init)fault_init,
        .ops.singleton.mod_start = (mod_start)fault_start,
    },
    {
        .name = "flash",
        .instance = MOD_NO_INSTANCE,
        .ops.singleton.mod_start = (mod_start)flash_start,
    },

    {
        .name = "lwl",
        .instance = MOD_NO_INSTANCE,
        .ops.singleton.mod_start = (mod_start)lwl_start,
    },


    {
        .name = "wdg",
        .instance = MOD_NO_INSTANCE,
        .ops.singleton.mod_init = (mod_init)wdg_init,
        .ops.singleton.mod_start = (mod_start)wdg_start,
    },
    {
        .name = "cmd",
        .instance = MOD_NO_INSTANCE,
        .ops.singleton.mod_init = (mod_init)cmd_init,
    },
    {
        .name = "console",
        .instance = MOD_NO_INSTANCE,
        .ops.singleton.mod_get_def_cfg = (mod_get_def_cfg)console_get_def_cfg,
        .ops.singleton.mod_init = (mod_init)console_init,
        .ops.singleton.mod_run = (mod_run)console_run,
        .cfg_obj = &console_cfg,
    },
    {
        .name = "tmr",
        .instance = MOD_NO_INSTANCE,
        .ops.singleton.mod_init = (mod_init)tmr_init,
        .ops.singleton.mod_start = (mod_start)tmr_start,
        .ops.singleton.mod_run = (mod_run)tmr_run,
    },

    {
        .name = "i2c",
        .instance = I2C_INSTANCE_3,
        .ops.singleton.mod_get_def_cfg = (mod_get_def_cfg)i2c_get_def_cfg,
        .ops.singleton.mod_init = (mod_init)i2c_init,
        .ops.singleton.mod_start = (mod_start)i2c_start,
        .cfg_obj = &i2c_cfg,
    },

    {
        .name = "tmphm",
        .instance = TMPHM_INSTANCE_1,
        .ops.singleton.mod_get_def_cfg = (mod_get_def_cfg)tmphm_get_def_cfg,
        .ops.singleton.mod_init = (mod_init)tmphm_init,
        .ops.singleton.mod_start = (mod_start)tmphm_start,
        .ops.singleton.mod_run = (mod_run)tmphm_run,
        .cfg_obj = &tmphm_cfg,
    },






};

////////////////////////////////////////////////////////////////////////////////
// Public (global) variables and externs
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Public (global) functions
////////////////////////////////////////////////////////////////////////////////

void app_main(void)
{
    // int32_t result;;
    int32_t rc;
    int32_t idx;
    struct mod_info* mod;



    wdg_start_init_hdw_wdg();

    lwl_enable(true);


    //
    // Invoke the init API on modules the use it.
    //

    setvbuf(stdout, NULL, _IONBF, 0);
    printc("\nInit: Init modules\n");

    for (idx = 0, mod = mods;
         idx < ARRAY_SIZE(mods);
         idx++, mod++) {
        if (mod->ops.singleton.mod_get_def_cfg != NULL &&
            mod->cfg_obj != NULL) {
            if (mod->instance == MOD_NO_INSTANCE) {
                rc = mod->ops.singleton.mod_get_def_cfg(mod->cfg_obj);
            } else {
                rc = mod->ops.multi_instance.mod_get_def_cfg(mod->instance,
                                                             mod->cfg_obj);
            }

        }
    }


    tmphm_cfg.i2c_instance_id = I2C_INSTANCE_3;


    for (idx = 0, mod = mods;
         idx < ARRAY_SIZE(mods);
         idx++, mod++) {
        if (mod->ops.singleton.mod_init != NULL) {
            if (mod->instance == MOD_NO_INSTANCE) {
                rc = mod->ops.singleton.mod_init(mod->cfg_obj);
            } else {
                rc = mod->ops.multi_instance.mod_init(mod->instance,
                                                      mod->cfg_obj);
            }

        }
    }

    //
    // Invoke the start API on modules the use it.
    //

    printc("Init: Start modules\n");

    for (idx = 0, mod = mods;
         idx < ARRAY_SIZE(mods);
         idx++, mod++) {
        if (mod->ops.singleton.mod_start != NULL) {
            if (mod->instance == MOD_NO_INSTANCE) {
                rc = mod->ops.singleton.mod_start();
            } else {
                rc = mod->ops.multi_instance.mod_start(mod->instance);
            }

        }
                
    }

    rc = cmd_register(&cmd_info);




    //
    // In the super loop invoke the run API on modules the use it.
    //

    wdg_init_successful();
    rc = wdg_start_hdw_wdg(CONFIG_WDG_HARD_TIMEOUT_MS);

    printc("Init: Enter super loop\n");
    while (1)
    {


        for (idx = 0, mod = mods;
             idx < ARRAY_SIZE(mods);
             idx++, mod++) {
            if (mod->ops.singleton.mod_run != NULL) {
                if (mod->instance == MOD_NO_INSTANCE) {
                    rc = mod->ops.singleton.mod_run();
                } else {
                    rc = mod->ops.multi_instance.mod_run(mod->instance);
                }
                if (rc < 0) {
                	printc("Run Error\n");
                }

            }
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



    if (clear) {
        printc("Clearing loop stat\n");

    }
    return 0;
}
