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

static struct tmphm_state st;  // The actual state variable
const char sensor_i2c_cmd[2] = {0x2c, 0x06};  // Sensor command bytes
static enum tmr_cb_action my_callback(int32_t tmr_id, uint32_t user_data);
static uint8_t crc8(const uint8_t *data, int len);
static int32_t log_level = LOG_INFO;


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
    // Question 1: What should you do with the state structure first?
    // Hint: Start with a clean slate - zero everything
	memset(&st, 0, sizeof(st));

    // Question 2: Where do you store the configuration?
    // Hint: You have a `cfg` field in your state structure
	st.cfg = *cfg;

    // Question 3: What should the initial state be?
    // Hint: Are you measuring yet? No! So what state?
	st.state = STATE_IDLE;

	return 0;
}


int32_t tmphm_start(enum tmphm_instance_id instance_id)
{
    // Question 1: What resource do you need for periodic sampling?
    // Hint: Something that fires every sample_time_ms

    // Question 2: How do you get a periodic timer?
    // Hint: Look at timer module API (tmr.h) - what function gets a callback timer?


    // Question 3: What are the timer parameters?
    // Parameter 1: Period in ms - what value?
    // Parameter 2: Callback function - what function name?
    // Parameter 3: User data - what do you pass? (could be 0 for single instance)

    // Question 4: Where do you store the timer ID?
    // Hint: You'll need it later - there's a field for this
	st.tmr_id = tmr_inst_get_cb(st.cfg.sample_time_ms,my_callback, 0 );
    // Question 5: What if timer allocation fails?
    // Hint: Check if tmr_id < 0
	if(st.tmr_id < 0){
		return MOD_ERR_RESOURCE;
	}

    return 0;
}


int32_t tmphm_run(enum tmphm_instance_id instance_id){

    int32_t rc;  // For return codes

    switch (st.state) {

        case STATE_IDLE:
            // Do idle work
            break;

        case STATE_RESERVE_I2C:
            // Do reserve work
        	// Write
        	rc = i2c_reserve(st.cfg.i2c_instance_id);
        	if (rc == 0){
        		st.msg_bfr[0] = 0x2c;
        		st.msg_bfr[1] = 0x06;
        		rc = i2c_write(st.cfg.i2c_instance_id,st.cfg.i2c_addr, st.msg_bfr,2 );
        		if (rc ==  0){
        			st.state = STATE_WRITE_MEAS_CMD;
        		}
                i2c_release(st.cfg.i2c_instance_id);
                st.state = STATE_IDLE;
        	}

            break;

        case STATE_WRITE_MEAS_CMD:
            // Do write work
        	// if check the status register, move to measure
			rc = i2c_get_op_status(st.cfg.i2c_instance_id);
			if(rc != MOD_ERR_OP_IN_PROG){
				if(rc == 0){
					st.i2c_op_start_ms = tmr_get_ms();
					st.state = STATE_WAIT_MEAS;
				}
				else {
	        		i2c_release(st.cfg.i2c_instance_id);
	        		st.state = STATE_IDLE;
				}
			}
            break;

        case STATE_WAIT_MEAS:
            // Do wait work
        	if ( tmr_get_ms() - st.i2c_op_start_ms >= st.cfg.meas_time_ms){
        		rc = i2c_read(st.cfg.i2c_instance_id, st.cfg.i2c_addr,st.msg_bfr, 6);
        		if (rc == 0){
        			st.state = STATE_READ_MEAS_VALUE;
        		}else {
            		i2c_release(st.cfg.i2c_instance_id);
            		st.state = STATE_IDLE;
        		}
        	}
            break;

        case STATE_READ_MEAS_VALUE:
            // Do read work
        	rc = i2c_get_op_status(st.cfg.i2c_instance_id);
        	if (rc != MOD_ERR_OP_IN_PROG ){
        		if (rc == 0){
        			uint8_t* msg = st.msg_bfr;

        			if (crc8(&msg[0], 2) != msg[2] ||      // Check: either FAIL
        			    crc8(&msg[3], 2) != msg[5]) {
//        				log_error("CRC error\n");

        			} else {
                        // Step 4: Convert raw data
                        const uint32_t divisor = 65535;

                        // Combine MSB and LSB
                        int32_t temp = (msg[0] << 8) + msg[1];
                        uint32_t hum = (msg[3] << 8) + msg[4];

                        // Apply conversion formulas (from datasheet)
                        temp = -450 + (1750 * temp + divisor/2) / divisor;
                        hum = (1000 * hum + divisor/2) / divisor;

                        // Step 5: Store the result
                        // Question: Where do you store temperature?
                        st.last_meas.temp_deg_c_x10 = temp;
                        st.last_meas.rh_percent_x10 = hum;
                        st.last_meas_ms = tmr_get_ms();  // Current time
                        st.got_meas = true;  // We have valid data now!

//                        log_info("temp=%ld degC*10 hum=%lu %%*10\n", temp, hum);
        			}
    			    // Always release I2C after read completes
    			    i2c_release(st.cfg.i2c_instance_id);
    			    st.state = STATE_IDLE;

        		}else {
                    // Read failed
                    i2c_release(st.cfg.i2c_instance_id);
                    st.state = STATE_IDLE;
        		}
        	}


            break;
    }

    return 0;

}


int32_t tmphm_get_last_meas(enum tmphm_instance_id instance_id,
                            struct tmphm_meas* meas, uint32_t* meas_age_ms)
{
    // Step 1: Validate parameter
    // Question: What if meas pointer is NULL?
    if (meas == NULL) {
        return MOD_ERR_ARG;
    }

    // Step 2: Check if we have data
    // Question: What if no measurement taken yet?
    // Hint: Check st.got_meas flag
    if (!st.got_meas) {
        return MOD_ERR_UNAVAIL;  // What error code means "no data available"?
    }

    // Step 3: Copy measurement to caller
    // Question: How do you copy the struct?
    *meas = st.last_meas  ;  // What do you copy?

    // Step 4: Calculate age (optional parameter)
    // Question: What if caller doesn't want age?
    if (meas_age_ms != NULL) {
        // Age = current_time - measurement_time
        *meas_age_ms = tmr_get_ms() - st.last_meas_ms;
    }

    return 0;
}

static enum tmr_cb_action my_callback(int32_t tmr_id, uint32_t user_data){
    if(st.state == STATE_IDLE ){
    	st.state = STATE_RESERVE_I2C;
    }else{
    	log_error("Timer overrun!\n");  // ← Add this
    }

    return TMR_CB_RESTART;  // Fire again
}


static uint8_t crc8(const uint8_t *data, int len)
{
    //
    // Parameters comes from page 14 of the STH31-D datasheet (see @ref).
    // - Polynomial: x^8 + x^5 + x^4 + x^0 which is 100110001 binary, or 0x31 in
    //   "normal" representation
    // - Initialization: 0xff.
    // - Final XOR: 0x00 (i.e. none).
    // - Example: 0xbeef => 0x92.
    //
    const uint8_t polynomial = 0x31;
    uint8_t crc = 0xff;
    uint32_t idx1;
    uint32_t idx2;

    for (idx1 = len; idx1 > 0; idx1--)
    {
        crc ^= *data++;
        for (idx2 = 8; idx2 > 0; idx2--)
        {
            crc = (crc & 0x80) ? (crc << 1) ^ polynomial : (crc << 1);
        }
    }
    return crc;
}
