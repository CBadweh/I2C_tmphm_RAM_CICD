#ifndef _CONSOLE_CMD_H_
#define _CONSOLE_CMD_H_

/*
 * @brief Combined interface declaration of console and cmd modules.
 *
 * This file combines the interfaces for both console and command modules.
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

#include <stdarg.h>
#include <stdint.h>

#include "ttys.h"

// ============================================================================
// CONSOLE MODULE INTERFACE
// ============================================================================

struct console_cfg
{
    enum ttys_instance_id ttys_instance_id;
};

// Core console module interface functions.
int32_t console_get_def_cfg(struct console_cfg* cfg);
int32_t console_init(struct console_cfg* cfg);
int32_t console_run(void);

// Console output APIs.
int	printc(const char* fmt, ...)
    __attribute__((__format__ (__printf__, 1, 2)));
int	vprintc(const char* fmt, va_list args);

// ============================================================================
// CMD MODULE INTERFACE
// ============================================================================

// Number of clients supported.
#define CMD_MAX_CLIENTS  10

// Function signature for a command handler function.
typedef int32_t (*cmd_func)(int32_t argc, const char** argv);

// Information about a single command, provided by the client.
struct cmd_cmd_info {
    const char* const name; // Name of command
    const cmd_func func;    // Command function
    const char* const help; // Command help string
};

// Information provided by the client.
// - Command base name
// - Command set info.
struct cmd_client_info {
    const char* const name;          // Client name (first command line token)
    const int32_t num_cmds;          // Number of commands.
    const struct cmd_cmd_info* const cmds; // Pointer to array of command info
};

struct cmd_arg_val {
    char type;
    union {
        void*       p;
        uint8_t*    p8;
        uint16_t*   p16;
        uint32_t*   p32;
        int32_t     i;
        uint32_t    u;
        const char* s;
    } val;
};
    
struct cmd_cfg {
    // FUTURE
};

// Core cmd module interface functions.
int32_t cmd_init(struct cmd_cfg* cfg);

// Other cmd APIs.
// Note: cmd_register() keeps a copy of the client_info pointer.
int32_t cmd_register(const struct cmd_client_info* client_info);
int32_t cmd_execute(char* bfr);
int32_t cmd_parse_args(int32_t argc, const char** argv, const char* fmt,
                       struct cmd_arg_val* arg_vals);

#endif // _CONSOLE_CMD_H_
