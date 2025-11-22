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
#include "cmd.h"
#include "console.h"
#include "i2c.h"
#include "module.h"
#include "tmphm.h"
#include "ttys.h"
#include "tmr.h"

////////////////////////////////////////////////////////////////////////////////
// Common macros
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Private (static) function declarations
////////////////////////////////////////////////////////////////////////////////

//static int32_t cmd_main_status();

////////////////////////////////////////////////////////////////////////////////
// Private (static) variables
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// Public (global) variables and externs
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Public (global) functions
////////////////////////////////////////////////////////////////////////////////

void app_main(void)
{
    int32_t result;;
    struct console_cfg console_cfg;
    struct i2c_cfg i2c_cfg;
    struct ttys_cfg ttys_cfg;
    struct tmphm_cfg tmphm_cfg;

   
    setvbuf(stdout, NULL, _IONBF, 0);
    printc("\nInit: Init modules\n");
    result = ttys_get_def_cfg(TTYS_INSTANCE_UART2, &ttys_cfg);
    result = ttys_init(TTYS_INSTANCE_UART2, &ttys_cfg);


    result = ttys_get_def_cfg(TTYS_INSTANCE_UART6, &ttys_cfg);
    result = ttys_init(TTYS_INSTANCE_UART6, &ttys_cfg);


    result = cmd_init(NULL);


    result = console_get_def_cfg(&console_cfg);
    result = console_init(&console_cfg);

    result = tmr_init(NULL);

    result = i2c_get_def_cfg(I2C_INSTANCE_3, &i2c_cfg);
    result = i2c_init(I2C_INSTANCE_3, &i2c_cfg);


    result = tmphm_get_def_cfg(TMPHM_INSTANCE_1, &tmphm_cfg);
    tmphm_cfg.i2c_instance_id = I2C_INSTANCE_3;
    result = tmphm_init(TMPHM_INSTANCE_1, &tmphm_cfg);

    //
    // Invoke the start API on modules the use it.
    //

    printc("Init: Start modules\n");

    result = ttys_start(TTYS_INSTANCE_UART2);
    result = ttys_start(TTYS_INSTANCE_UART6);
    result = tmr_start();


    result = i2c_start(I2C_INSTANCE_3);

    result = tmphm_start(TMPHM_INSTANCE_1);
    printc("Init: Enter super loop\n");

    while (1)
    {
        result = console_run();
        result = tmr_run();
        result = tmphm_run(TMPHM_INSTANCE_1);

    }
}


