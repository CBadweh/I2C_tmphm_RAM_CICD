/*
 * TMPHM DRIVER - Temperature/Humidity Module (SHT31-D Sensor)
 * 
 * DAY 3 COMPLETE VERSION with:
 * - Console commands for testing and status
 * - Performance counters for diagnostics
 * - Comprehensive LWL (Lightweight Logging) instrumentation
 * - Full error handling
 * 
 * CRITICAL PATH:
 * 1. Timer triggers measurement cycle every 1 second
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

#include "cmd.h"
#include "console.h"
#include "i2c.h"
#include "log.h"
#include "lwl.h"
#include "module.h"
#include "tmr.h"
#include "tmphm.h"
#ifdef CONFIG_WDG_PRESENT
#include "wdg.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// State Machine
////////////////////////////////////////////////////////////////////////////////

enum states {
    STATE_IDLE,
    STATE_RESERVE_I2C,
    STATE_WRITE_MEAS_CMD,
    STATE_WAIT_MEAS,
    STATE_READ_MEAS_VALUE
};

////////////////////////////////////////////////////////////////////////////////
// Performance Measurements
////////////////////////////////////////////////////////////////////////////////

enum tmphm_u16_pms {
    CNT_RESERVE_FAIL,
    CNT_WRITE_INIT_FAIL,
    CNT_WRITE_OP_FAIL,
    CNT_READ_INIT_FAIL,
    CNT_READ_OP_FAIL,
    CNT_TASK_OVERRUN,
    CNT_CRC_FAIL,

    NUM_U16_PMS
};

////////////////////////////////////////////////////////////////////////////////
// State Structure
////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////
// Private Function Declarations
////////////////////////////////////////////////////////////////////////////////

static enum tmr_cb_action my_callback(int32_t tmr_id, uint32_t user_data);
static uint8_t crc8(const uint8_t *data, int len);
static int32_t cmd_tmphm_status(int32_t argc, const char** argv);
static int32_t cmd_tmphm_test(int32_t argc, const char** argv);

////////////////////////////////////////////////////////////////////////////////
// Private Variables
////////////////////////////////////////////////////////////////////////////////

static struct tmphm_state st;  // The actual state variable
const char sensor_i2c_cmd[2] = {0x2c, 0x06};  // Sensor command bytes
static int32_t log_level = LOG_INFO;

// Storage for performance measurements
static uint16_t cnts_u16[NUM_U16_PMS];

// Names of performance measurements
static const char* cnts_u16_names[NUM_U16_PMS] = {
    "reserve fail",
    "write init fail",
    "write op fail",
    "read init fail",
    "read op fail",
    "task overrun",
    "crc error",
};

// Console command info
static struct cmd_cmd_info cmds[] = {
    {
        .name = "status",
        .func = cmd_tmphm_status,
        .help = "Get module status, usage: tmphm status",
    },
    {
        .name = "test",
        .func = cmd_tmphm_test,
        .help = "Run test, usage: tmphm test [<op> [<arg>]] (enter no op for help)",
    }
};

// Data structure passed to cmd module for console interaction
static struct cmd_client_info cmd_info = {
    .name = "tmphm",
    .num_cmds = ARRAY_SIZE(cmds),
    .cmds = cmds,
    .log_level_ptr = &log_level,
    .num_u16_pms = NUM_U16_PMS,
    .u16_pms = cnts_u16,
    .u16_pm_names = cnts_u16_names,
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
    int32_t rc;
    
    // Register console commands
    rc = cmd_register(&cmd_info);
    if (rc < 0) {
        log_error("tmphm_start: cmd error %d\n", rc);
        return rc;
    }
    
    // Get periodic timer with callback (fires every sample_time_ms)
    st.tmr_id = tmr_inst_get_cb(st.cfg.sample_time_ms, my_callback, 0);
    if (st.tmr_id < 0) {
        return MOD_ERR_RESOURCE;
    }
    
    // Register TMPHM watchdog with 5-second timeout (if watchdog present)
#ifdef CONFIG_WDG_PRESENT
    rc = wdg_register(CONFIG_TMPHM_WDG_ID, 5000);
    if (rc < 0) {
        log_error("tmphm_start: wdg_register error %d\n", rc);
        return rc;
    }
    log_info("tmphm_start: Registered watchdog %d with 5s timeout\n",
             CONFIG_TMPHM_WDG_ID);
#endif

    return 0;
}


int32_t tmphm_run(enum tmphm_instance_id instance_id)
{
    int32_t rc;
    
    switch (st.state) {
        
        case STATE_IDLE:
            // Waiting for timer to trigger next measurement cycle
            break;

        case STATE_RESERVE_I2C:
            LWL("TMPHM: Attempting I2C reserve", 0);
            
            rc = i2c_reserve(st.cfg.i2c_instance_id);
            if (rc == 0) {
                // Reserve succeeded - prepare and send measurement command
                memcpy(st.msg_bfr, sensor_i2c_cmd, sizeof(sensor_i2c_cmd));
                rc = i2c_write(st.cfg.i2c_instance_id, st.cfg.i2c_addr, st.msg_bfr, 2);
                if (rc == 0) {
                    LWL("TMPHM: Write started", 0);
                    st.state = STATE_WRITE_MEAS_CMD;
                } else {
                    LWL("TMPHM: Write init failed rc=%d", 4, LWL_4(rc));
                    INC_SAT_U16(cnts_u16[CNT_WRITE_INIT_FAIL]);
                    i2c_release(st.cfg.i2c_instance_id);
                    st.state = STATE_IDLE;
                }
            } else {
                LWL("TMPHM: Reserve failed rc=%d", 4, LWL_4(rc));
                INC_SAT_U16(cnts_u16[CNT_RESERVE_FAIL]);
                // Stay in RESERVE state, will retry next time through loop
            }
            break;

        case STATE_WRITE_MEAS_CMD:
            // Poll for write operation completion
            rc = i2c_get_op_status(st.cfg.i2c_instance_id);
            if (rc != MOD_ERR_OP_IN_PROG) {
                if (rc == 0) {
                    // Write succeeded - start timing for sensor measurement
                    LWL("TMPHM: Write complete, waiting for sensor", 0);
                    st.i2c_op_start_ms = tmr_get_ms();
                    st.state = STATE_WAIT_MEAS;
                } else {
                    LWL("TMPHM: Write op failed rc=%d", 4, LWL_4(rc));
                    INC_SAT_U16(cnts_u16[CNT_WRITE_OP_FAIL]);
                    i2c_release(st.cfg.i2c_instance_id);
                    st.state = STATE_IDLE;
                }
            }
            break;

        case STATE_WAIT_MEAS:
            // Wait for sensor to complete measurement (17ms typ)
            if (tmr_get_ms() - st.i2c_op_start_ms >= st.cfg.meas_time_ms) {
                LWL("TMPHM: Wait complete, starting read", 0);
                rc = i2c_read(st.cfg.i2c_instance_id, st.cfg.i2c_addr, st.msg_bfr, 6);
                if (rc == 0) {
                    st.state = STATE_READ_MEAS_VALUE;
                } else {
                    LWL("TMPHM: Read init failed rc=%d", 4, LWL_4(rc));
                    INC_SAT_U16(cnts_u16[CNT_READ_INIT_FAIL]);
                    i2c_release(st.cfg.i2c_instance_id);
                    st.state = STATE_IDLE;
                }
            }
            break;

        case STATE_READ_MEAS_VALUE:
            // Poll for read operation completion
            rc = i2c_get_op_status(st.cfg.i2c_instance_id);
            if (rc != MOD_ERR_OP_IN_PROG) {
                if (rc == 0) {
                    // Read succeeded - validate and process data
                    uint8_t* msg = st.msg_bfr;
                    
                    if (crc8(&msg[0], 2) != msg[2] || crc8(&msg[3], 2) != msg[5]) {
                        // CRC validation failed
                        LWL("TMPHM: CRC error", 0);
                        INC_SAT_U16(cnts_u16[CNT_CRC_FAIL]);
                    } else {
                        // CRC valid - convert raw data to temperature/humidity
                        const uint32_t divisor = 65535;
                        int32_t temp = (msg[0] << 8) + msg[1];
                        uint32_t hum = (msg[3] << 8) + msg[4];
                        
                        // Apply conversion formulas (datasheet pg 14)
                        temp = -450 + (1750 * temp + divisor/2) / divisor;
                        hum = (1000 * hum + divisor/2) / divisor;
                        
                        // Store measurement
                        st.last_meas.temp_deg_c_x10 = temp;
                        st.last_meas.rh_percent_x10 = hum;
                        st.last_meas_ms = tmr_get_ms();
                        st.got_meas = true;
                        
                        LWL("TMPHM: Got good measurement temp=%d hum=%d", 4, 
                            LWL_2(temp), LWL_2(hum));
                        
                        // Feed watchdog after successful measurement
#ifdef CONFIG_WDG_PRESENT
                        wdg_feed(CONFIG_TMPHM_WDG_ID);
#endif
                    }
                    // Always release I2C after read completes
                    i2c_release(st.cfg.i2c_instance_id);
                    st.state = STATE_IDLE;
                } else {
                    LWL("TMPHM: Read op failed rc=%d", 4, LWL_4(rc));
                    INC_SAT_U16(cnts_u16[CNT_READ_OP_FAIL]);
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
    // Validate parameter
    if (meas == NULL) {
        return MOD_ERR_ARG;
    }

    // Check if we have data
    if (!st.got_meas) {
        return MOD_ERR_UNAVAIL;
    }

    // Copy measurement to caller
    *meas = st.last_meas;

    // Calculate age (optional parameter)
    if (meas_age_ms != NULL) {
        *meas_age_ms = tmr_get_ms() - st.last_meas_ms;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Timer Callback - Triggers measurement cycle
////////////////////////////////////////////////////////////////////////////////

static enum tmr_cb_action my_callback(int32_t tmr_id, uint32_t user_data)
{
    if (st.state == STATE_IDLE) {
        // Start new measurement cycle
        LWL("TMPHM: Start measurement cycle", 0);
        st.state = STATE_RESERVE_I2C;
    } else {
        // Timer overrun - previous measurement still in progress
        LWL("TMPHM: Timer overrun state=%d", 1, LWL_1(st.state));
        INC_SAT_U16(cnts_u16[CNT_TASK_OVERRUN]);
    }

    return TMR_CB_RESTART;  // Fire again next cycle
}

////////////////////////////////////////////////////////////////////////////////
// Console Command Functions
////////////////////////////////////////////////////////////////////////////////

/*
 * @brief Console command function for "tmphm status".
 *
 * @param[in] argc Number of arguments, including "tmphm"
 * @param[in] argv Argument values, including "tmphm"
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 *
 * Command usage: tmphm status
 */
static int32_t cmd_tmphm_status(int32_t argc, const char** argv)
{
    printc("         Got  Last Last Meas Meas\n"
           "ID State Meas Temp Hum  Age  Time\n"
           "-- ----- ---- ---- ---- ---- ----\n");
    
    printc("%2d %5d %4d %4d %4u %4lu %4lu\n",
           0,  // instance_id (only have 1)
           st.state,
           st.got_meas,
           st.last_meas.temp_deg_c_x10,
           st.last_meas.rh_percent_x10,
           st.got_meas ? (tmr_get_ms() - st.last_meas_ms) : 0,
           st.cfg.meas_time_ms);
    
    return 0;
}

/*
 * @brief Console command function for "tmphm test".
 *
 * @param[in] argc Number of arguments, including "tmphm"
 * @param[in] argv Argument values, including "tmphm"
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 *
 * Command usage: tmphm test [<op> [<arg>]]
 */
static int32_t cmd_tmphm_test(int32_t argc, const char** argv)
{
    struct cmd_arg_val arg_vals[4];
    int32_t rc;
    uint32_t idx;
    enum tmphm_instance_id instance_id = TMPHM_INSTANCE_1;

    // Handle help case
    if (argc == 2) {
        printc("Test operations and param(s) are as follows:\n"
               "  Get last meas, usage: tmphm test lastmeas <instance-id>\n"
               "  Set meas time, usage: tmphm test meastime <instance-id> <time-ms>\n"
               "  Test crc8, usage: tmphm test crc8 byte1 ... (up to 4 bytes)\n");
        return 0;
    }

    if (argc < 3) {
        return MOD_ERR_BAD_CMD;
    }

    // Get instance ID (except for crc8 option)
    if (strcasecmp(argv[2], "crc8") != 0) {
        rc = cmd_parse_args(argc-3, argv+3, "u+", arg_vals);
        if (rc < 1)
            return MOD_ERR_BAD_CMD;
        instance_id = (enum tmphm_instance_id)arg_vals[0].val.u;
        if (instance_id >= TMPHM_NUM_INSTANCES) {
            printc("Bad instance\n");
            return MOD_ERR_BAD_INSTANCE;
        }
    }

    if (strcasecmp(argv[2], "lastmeas") == 0) {
        struct tmphm_meas meas;
        uint32_t meas_age_ms;
        rc = tmphm_get_last_meas(instance_id, &meas, &meas_age_ms);
        if (rc == 0)
            printc("Temp=%d.%d C Hum=%u.%u %% age=%lu ms\n",
                   meas.temp_deg_c_x10 / 10,
                   meas.temp_deg_c_x10 % 10,
                   meas.rh_percent_x10 / 10,
                   meas.rh_percent_x10 % 10,
                   meas_age_ms);
        else
            printc("tmphm_get_last_meas fails rc=%ld\n", rc);
    } else if (strcasecmp(argv[2], "meastime") == 0) {
        rc = cmd_parse_args(argc-4, argv+4, "u", arg_vals);
        if (rc != 1) {
            return MOD_ERR_BAD_CMD;
        }
        st.cfg.meas_time_ms = arg_vals[0].val.u;
        printc("Meas time set to %lu ms\n", st.cfg.meas_time_ms);
    } else if (strcasecmp(argv[2], "crc8") == 0) {
        uint8_t data[4];

        rc = cmd_parse_args(argc-3, argv+3, "u[u[u[u]]]", arg_vals);
        if (rc < 1) {
            return MOD_ERR_BAD_CMD;
        }
        for (idx = 0; idx < 4 && idx < rc; idx++)
            data[idx] = (uint8_t)arg_vals[idx].val.u;

        printc("crc8: 0x%02x\n", crc8(data, rc));
    } else {
        printc("Invalid operation '%s'\n", argv[2]);
        return MOD_ERR_BAD_CMD;
    }
    
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// CRC-8 Calculation
////////////////////////////////////////////////////////////////////////////////

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
