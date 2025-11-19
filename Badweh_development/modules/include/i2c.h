#ifndef _I2C_H_
#define _I2C_H_

/*
 * @brief Interface declaration of i2c module.
 *
 * See implementation file for information about this module.
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

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// I2C Module Configuration (extracted for portability)
////////////////////////////////////////////////////////////////////////////////

// I2C timing configuration
#define CONFIG_I2C_DFLT_TRANS_GUARD_TIME_MS 100

// Fault injection support (only enabled in Debug builds for testing)
// In production (Release) builds, fault injection code is completely removed
#ifdef DEBUG
    #define ENABLE_FAULT_INJECTION
#endif

// Module error codes (subset used by I2C)
#define MOD_ERR_ARG          -1
#define MOD_ERR_RESOURCE     -2
#define MOD_ERR_STATE        -3
#define MOD_ERR_BAD_CMD      -4
#define MOD_ERR_BAD_INSTANCE -6
#define MOD_ERR_PERIPH       -7
#define MOD_ERR_NOT_RESERVED -8
#define MOD_ERR_OP_IN_PROG   -9

////////////////////////////////////////////////////////////////////////////////

enum i2c_errors {
    I2C_ERR_NONE,  // Must have value 0.
    I2C_ERR_INVALID_INSTANCE,
    I2C_ERR_BUS_BUSY,
    I2C_ERR_GUARD_TMR,
    I2C_ERR_PEC,
    I2C_ERR_TIMEOUT,
    I2C_ERR_ACK_FAIL,
    I2C_ERR_BUS_ERR,
    I2C_ERR_INTR_UNEXPECT,
};

// I2C numbering is based on the MCU hardware definition.
// Hardcoded for I2C3 only (connected to SHT31-D sensor)

enum i2c_instance_id {
    I2C_INSTANCE_3,      // I2C3 hardware peripheral
    I2C_NUM_INSTANCES    // Total number of instances (1)
};

struct i2c_cfg {
    uint32_t transaction_guard_time_ms;
};

// Core module interface functions.
int32_t i2c_get_def_cfg(enum i2c_instance_id instance_id, struct i2c_cfg* cfg);
int32_t i2c_init(enum i2c_instance_id instance_id, struct i2c_cfg* cfg);
int32_t i2c_start(enum i2c_instance_id instance_id);
int32_t i2c_run(enum i2c_instance_id instance_id);

// Other APIs (Happy Path - void return for reserve/release/write/read).
int32_t i2c_reserve(enum i2c_instance_id instance_id);
int32_t i2c_release(enum i2c_instance_id instance_id);

int32_t i2c_write(enum i2c_instance_id instance_id, uint32_t dest_addr,
               uint8_t* msg_bfr, uint32_t msg_len);
int32_t i2c_read(enum i2c_instance_id instance_id, uint32_t dest_addr,
              uint8_t* msg_bfr, uint32_t msg_len);

int32_t i2c_get_op_status(enum i2c_instance_id instance_id);

enum i2c_errors i2c_get_error(enum i2c_instance_id instance_id);

// Automated test (button-triggered)
int32_t i2c_run_auto_test(void);
int32_t i2c_test_not_reserved(void);

#ifdef ENABLE_FAULT_INJECTION
// Fault injection tests (for testing error paths)
// Only available in Debug builds - removed from Release builds
int32_t i2c_test_wrong_addr(void);
int32_t i2c_test_nack(void);
int32_t i2c_test_timeout(void);
#endif

#endif // _I2C_H_
