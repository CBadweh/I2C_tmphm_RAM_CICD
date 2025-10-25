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
#include "module.h"
#include "tmr.h"
#include "tmphm.h"

////////////////////////////////////////////////////////////////////////////////
// STATE MACHINE - The heart of the driver
////////////////////////////////////////////////////////////////////////////////

enum states {
    STATE_IDLE,              // Waiting for timer to trigger
    STATE_RESERVE_I2C,       // Acquiring I2C bus
    STATE_WRITE_MEAS_CMD,    // Sending measurement command
    STATE_WAIT_MEAS,         // Waiting 15ms for sensor
    STATE_READ_MEAS_VALUE    // Reading result
};

// SHT31-D measurement command from datasheet Table 8:
// 0x2C = High repeatability measurement
// 0x06 = Clock stretching enabled
const char sensor_i2c_cmd[2] = {0x2c, 0x06};

// Buffer size for sensor data:
// Byte 0: Temperature MSB
// Byte 1: Temperature LSB  
// Byte 2: Temperature CRC-8
// Byte 3: Humidity MSB
// Byte 4: Humidity LSB
// Byte 5: Humidity CRC-8
#define I2C_MSG_BFR_LEN 6

////////////////////////////////////////////////////////////////////////////////
// STATE STRUCTURE - Holds all info for the sensor
////////////////////////////////////////////////////////////////////////////////

struct tmphm_state {
    struct tmphm_cfg cfg;              // Configuration
    struct tmphm_meas last_meas;       // Last measurement result
    
    int32_t tmr_id;                    // Timer ID for periodic sampling
    uint32_t i2c_op_start_ms;          // When did I2C operation start?
    uint32_t last_meas_ms;             // When was last measurement taken?
    
    uint8_t msg_bfr[I2C_MSG_BFR_LEN];  // Buffer for I2C data
    
    bool got_meas;                     // Have we gotten at least one measurement?
    enum states state;                 // Current state
};

////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

static enum tmr_cb_action tmr_callback(int32_t tmr_id, uint32_t user_data);
static uint8_t crc8(const uint8_t *data, int len);

////////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
////////////////////////////////////////////////////////////////////////////////

static struct tmphm_state st;  // Single instance state
static int32_t log_level = LOG_INFO;  // Logging level (required by log macros)

////////////////////////////////////////////////////////////////////////////////
// PUBLIC API FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/*
 * Get default configuration
 * This sets up the concrete values that will be used throughout the driver
 */
int32_t tmphm_get_def_cfg(enum tmphm_instance_id instance_id, struct tmphm_cfg* cfg)
{
    cfg->i2c_instance_id = I2C_INSTANCE_3;  // Use I2C bus #3 (the only one we have)
    cfg->i2c_addr = 0x44;                   // SHT31-D sensor address (could be 0x44 or 0x45)
    cfg->sample_time_ms = 1000;             // Take reading every 1000ms = 1 second
    cfg->meas_time_ms = 15;                 // Sensor needs 15ms to complete measurement (from datasheet)
    return 0;
}

/*
 * Initialize TMPHM
 */
int32_t tmphm_init(enum tmphm_instance_id instance_id, struct tmphm_cfg* cfg)
{
    memset(&st, 0, sizeof(st));
    st.cfg = *cfg;
    return 0;
}

/*
 * Start TMPHM - Get timer and begin periodic sampling
 */
int32_t tmphm_start(enum tmphm_instance_id instance_id)
{
    // Get a periodic timer that fires every sample_time_ms
    st.tmr_id = tmr_inst_get_cb(st.cfg.sample_time_ms, tmr_callback, 0);
    if (st.tmr_id < 0)
        return MOD_ERR_RESOURCE;
    
    return 0;
}

/*
 * Run TMPHM - THE HEART OF THE DRIVER
 * Called repeatedly from super loop. Advances state machine.
 */
int32_t tmphm_run(enum tmphm_instance_id instance_id)
{
    int32_t rc;

    switch (st.state) {
        
        // ------ IDLE: Waiting for timer to trigger ------
        case STATE_IDLE:
            // Nothing to do - timer callback will move us to RESERVE_I2C
            break;

        // ------ RESERVE: Get exclusive I2C access ------
        case STATE_RESERVE_I2C:
            // Reserve I2C bus (st.cfg.i2c_instance_id = I2C_INSTANCE_3 = bus #3)
            rc = i2c_reserve(st.cfg.i2c_instance_id);  // i2c_reserve(3)
            if (rc == 0) {  // 0 = success
                // Got the bus! Send measurement command
                // Copy command bytes: {0x2c, 0x06} to buffer
                memcpy(st.msg_bfr, sensor_i2c_cmd, sizeof(sensor_i2c_cmd));  // 2 bytes
                
                // Write to sensor (bus=3, addr=0x44, data={0x2c,0x06}, len=2)
                rc = i2c_write(st.cfg.i2c_instance_id,  // bus 3
                              st.cfg.i2c_addr,           // 0x44
                              st.msg_bfr,                // {0x2c, 0x06}
                              sizeof(sensor_i2c_cmd));   // 2 bytes
                if (rc == 0) {  // 0 = success
                    st.state = STATE_WRITE_MEAS_CMD;
                } else {
                    // Write failed, release and go back to IDLE
                    i2c_release(st.cfg.i2c_instance_id);  // i2c_release(3)
                    st.state = STATE_IDLE;
                }
            }
            // If reserve failed, stay in RESERVE state and try again next loop
            break;

        // ------ WRITE: Waiting for write to complete ------
        case STATE_WRITE_MEAS_CMD:
            // Check write status (on bus 3)
            rc = i2c_get_op_status(st.cfg.i2c_instance_id);  // i2c_get_op_status(3)
            if (rc != MOD_ERR_OP_IN_PROG) {  // MOD_ERR_OP_IN_PROG = -4 (still working)
                // Write complete!
                if (rc == 0) {  // 0 = success
                    // Success - start waiting for sensor measurement
                    st.i2c_op_start_ms = tmr_get_ms();  // Record current time
                    st.state = STATE_WAIT_MEAS;
                } else {
                    // Write failed, release and go back to IDLE
                    i2c_release(st.cfg.i2c_instance_id);  // i2c_release(3)
                    st.state = STATE_IDLE;
                }
            }
            // Still in progress? Just wait
            break;

        // ------ WAIT: Sensor is measuring, wait 15ms ------
        case STATE_WAIT_MEAS:
            // Has enough time passed? (st.cfg.meas_time_ms = 15)
            if (tmr_get_ms() - st.i2c_op_start_ms >= st.cfg.meas_time_ms) {  // >= 15ms?
                // 15ms has elapsed, read the result
                // Read 6 bytes from sensor (bus=3, addr=0x44, len=6)
                rc = i2c_read(st.cfg.i2c_instance_id,  // bus 3
                             st.cfg.i2c_addr,           // 0x44
                             st.msg_bfr,                // store in buffer
                             I2C_MSG_BFR_LEN);          // 6 bytes
                if (rc == 0) {  // 0 = success
                    st.state = STATE_READ_MEAS_VALUE;
                } else {
                    // Read start failed, release and go back to IDLE
                    i2c_release(st.cfg.i2c_instance_id);  // i2c_release(3)
                    st.state = STATE_IDLE;
                }
            }
            break;

        // ------ READ: Waiting for read to complete ------
        case STATE_READ_MEAS_VALUE:
            // Check read status (on bus 3)
            rc = i2c_get_op_status(st.cfg.i2c_instance_id);  // i2c_get_op_status(3)
            if (rc != MOD_ERR_OP_IN_PROG) {  // MOD_ERR_OP_IN_PROG = -4
                // Read complete!
                if (rc == 0) {  // 0 = success
                    // SUCCESS! Validate CRC and convert data
                    uint8_t* msg = st.msg_bfr;  // msg[0..5] = 6 bytes from sensor
                    
                    // Validate CRC-8 for temperature and humidity
                    // msg[0,1] = temp raw (MSB, LSB), msg[2] = temp CRC
                    // msg[3,4] = hum raw (MSB, LSB),  msg[5] = hum CRC
                    if (crc8(&msg[0], 2) == msg[2] &&    // temp CRC valid?
                        crc8(&msg[3], 2) == msg[5]) {    // hum CRC valid?
                        
                        // CRC valid - convert raw data to temperature/humidity
                        const uint32_t divisor = 65535;  // 2^16 - 1
                        
                        // Combine MSB and LSB to get 16-bit raw value (0x0000 to 0xFFFF)
                        int32_t temp = (msg[0] << 8) + msg[1];  // 16-bit raw temp
                        uint32_t hum = (msg[3] << 8) + msg[4];  // 16-bit raw humidity
                        
                        // Temperature formula from datasheet: T[°C] = -45 + 175 × (raw/65535)
                        // Stored as ×10 for integer math: -450 + 1750 × (raw/65535)
                        // Example: raw=32768 (mid-range) → -450 + 1750×0.5 = 425 = 42.5°C
                        temp = -450 + (1750 * temp + divisor/2) / divisor;
                        
                        // Humidity formula from datasheet: H[%] = 100 × (raw/65535)
                        // Stored as ×10 for integer math: 1000 × (raw/65535)
                        // Example: raw=32768 (mid-range) → 1000×0.5 = 500 = 50.0%
                        hum = (1000 * hum + divisor/2) / divisor;
                        
                        // Store result (temp and hum are now in units of ×10)
                        st.last_meas.temp_deg_c_x10 = temp;  // e.g., 235 = 23.5°C
                        st.last_meas.rh_percent_x10 = hum;   // e.g., 450 = 45.0%
                        st.last_meas_ms = tmr_get_ms();
                        st.got_meas = true;
                        
                        // Print result (e.g., "temp=235 degC*10 hum=450 %*10")
                        log_info("temp=%ld degC*10 hum=%lu %%*10\n", temp, hum);
                    } else {
                        // CRC failed - ignore this measurement
                        log_error("CRC error\n");
                    }
                }
                // Read complete (success or fail), release I2C and go to IDLE
                i2c_release(st.cfg.i2c_instance_id);  // i2c_release(3)
                st.state = STATE_IDLE;
            }
            // Still in progress? Just wait
            break;
    }
    return 0;
}

/*
 * Get last measurement
 */
int32_t tmphm_get_last_meas(enum tmphm_instance_id instance_id,
                            struct tmphm_meas* meas, uint32_t* meas_age_ms)
{
    if (meas == NULL)
        return MOD_ERR_ARG;
    
    if (!st.got_meas)
        return MOD_ERR_UNAVAIL;  // No measurement yet

    *meas = st.last_meas;
    if (meas_age_ms != NULL)
        *meas_age_ms = tmr_get_ms() - st.last_meas_ms;

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/*
 * Timer callback - Triggers new measurement cycle
 * Called by timer module every 1000ms (1 second)
 * 
 * This is the "heartbeat" that kicks off each measurement cycle.
 * It just changes the state - the actual work happens in tmphm_run()
 */
static enum tmr_cb_action tmr_callback(int32_t tmr_id, uint32_t user_data)
{
    // If IDLE, start a new measurement cycle
    if (st.state == STATE_IDLE)
        st.state = STATE_RESERVE_I2C;  // Move to next state, tmphm_run() will handle it
    else
        // State != IDLE means previous measurement still in progress!
        // This shouldn't happen if timing is correct (measurement takes ~20ms, timer is 1000ms)
        log_error("Timer overrun - previous measurement not done!\n");
    
    return TMR_CB_RESTART;  // Keep timer running (fire again in 1000ms)
}

/*
 * CRC-8 calculation for data validation
 * 
 * Parameters from SHT31-D datasheet page 14:
 * - Polynomial: x^8 + x^5 + x^4 + 1 (0x31)
 * - Initialization: 0xFF
 * - Final XOR: 0x00 (none)
 * - Example: 0xBEEF => 0x92
 */
static uint8_t crc8(const uint8_t *data, int len)
{
    const uint8_t polynomial = 0x31;
    uint8_t crc = 0xff;
    uint32_t idx1, idx2;

    for (idx1 = len; idx1 > 0; idx1--) {
        crc ^= *data++;
        for (idx2 = 8; idx2 > 0; idx2--) {
            crc = (crc & 0x80) ? (crc << 1) ^ polynomial : (crc << 1);
        }
    }
    return crc;
}

