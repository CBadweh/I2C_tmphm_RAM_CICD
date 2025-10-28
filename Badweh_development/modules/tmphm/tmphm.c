/*
 * SIMPLIFIED TMPHM DRIVER - For Learning the Critical Path
 * 
 * This is a stripped-down version focusing on the essential state machine
 * and sensor interaction. Non-essential features removed:
 * - Console commands (debugging only)
 * - Performance counters (nice to have)
 * - Multiple instance support (focus on INSTANCE_1 only)
 * 
 * CRITICAL PATH TO UNDERSTAND:
 * 1. Timer triggers measurement cycle
 * 2. State machine reserves I2C bus
 * 3. Write measurement command to sensor (0x2c 0x06)
 * 4. Wait 15ms for sensor to measure
 * 5. Read 6 bytes from sensor
 * 6. Validate CRC-8
 * 7. Convert to temperature/humidity
 * 8. Release I2C and wait for next timer
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "i2c.h"
#include "log.h"
//#include "lwl.h"
#include "module.h"
#include "tmr.h"
#include "tmphm.h"

enum states {
    STATE_IDLE,
    STATE_RESERVE_I2C,
    STATE_WRITE_MEAS_CMD,
    STATE_WAIT_MEAS,
    STATE_READ_MEAS_VALUE
};

struct tmphm_state {
    struct tmphm_cfg cfg;              // Config (I2C instance, address, timings)
    struct tmphm_meas last_meas;       // Last measurement
    int32_t tmr_id;                    // Timer ID
    uint32_t i2c_op_start_ms;          // For timing the 15ms wait
    uint32_t last_meas_ms;             // When was last measurement?
    uint8_t msg_bfr[6];                // 6 bytes from sensor
    bool got_meas;                     // Have we gotten at least one?
    enum states state;                 // Current state
};


int32_t tmphm_get_def_cfg(enum tmphm_instance_id instance_id, struct tmphm_cfg* cfg){

    // Question 1: Which I2C bus are you using?
    cfg->i2c_instance_id = CONFIG_TMPHM_1_DFLT_I2C_INSTANCE;

    // Question 2: What's the sensor's I2C address? (Check SHT31-D datasheet)
    cfg->i2c_addr = CONFIG_TMPHM_1_DFLT_I2C_ADDR;

    // Question 3: How often should we take readings? (milliseconds)
    cfg->sample_time_ms = CONFIG_TMPHM_DFLT_SAMPLE_TIME_MS;

    // Question 4: How long does the sensor need to measure? (Check datasheet Table 4)
    cfg->meas_time_ms = CONFIG_TMPHM_DFLT_MEAS_TIME_MS;

    return 0;
}


int32_t tmphm_init(enum tmphm_instance_id instance_id, struct tmphm_cfg* cfg){


}


int32_t tmphm_start(enum tmphm_instance_id instance_id){


}


int32_t tmphm_run(enum tmphm_instance_id instance_id){


}

