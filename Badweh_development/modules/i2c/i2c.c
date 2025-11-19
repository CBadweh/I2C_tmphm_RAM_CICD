/*
 * @brief I2C bus driver implementation with error detection
 *
 * Simplified I2C driver for STM32F401RE with explicit state machine.
 * Supports reserve/release resource management, write/read operations,
 * and automated error detection. All state transitions driven by interrupts.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include "stm32f4xx.h"
#include "stm32f4xx_ll_i2c.h"

#include "tmr.h"
#include "console.h"
#include "cmd.h"
#include "module.h"
#include "i2c.h"

////////////////////////////////////////////////////////////////////////////////
// MACROS - Interrupt control
////////////////////////////////////////////////////////////////////////////////

#define INTERRUPT_ENABLE_MASK (LL_I2C_CR2_ITEVTEN | LL_I2C_CR2_ITBUFEN | \
                               LL_I2C_CR2_ITERREN)
#define DISABLE_ALL_INTERRUPTS(st) st->i2c_reg_base->CR2 &= ~INTERRUPT_ENABLE_MASK
#define ENABLE_ALL_INTERRUPTS(st) st->i2c_reg_base->CR2 |= INTERRUPT_ENABLE_MASK

#define INTERRUPT_ERR_MASK_BASE (LL_I2C_SR1_BERR | LL_I2C_SR1_ARLO | \
                                 LL_I2C_SR1_AF   | LL_I2C_SR1_OVR)
#ifdef I2C_ISR_TIMEOUT
#define INTERRUPT_ERR_MASK_TIMEOUT I2C_ISR_TIMEOUT
#else
#define INTERRUPT_ERR_MASK_TIMEOUT 0U
#endif
#ifdef I2C_ISR_PECERR
#define INTERRUPT_ERR_MASK_PEC I2C_ISR_PECERR
#else
#define INTERRUPT_ERR_MASK_PEC 0U
#endif
#define INTERRUPT_ERR_MASK (INTERRUPT_ERR_MASK_BASE | \
                            INTERRUPT_ERR_MASK_TIMEOUT | \
                            INTERRUPT_ERR_MASK_PEC)

////////////////////////////////////////////////////////////////////////////////
// STATE MACHINE - The heart of the driver
////////////////////////////////////////////////////////////////////////////////

enum states {
    STATE_IDLE,                     // Ready for new operation
    STATE_MSTR_WR_GEN_START,        // Write: Generating START condition
    STATE_MSTR_WR_SENDING_ADDR,     // Write: Sending address + W bit
    STATE_MSTR_WR_SENDING_DATA,     // Write: Sending data bytes
    STATE_MSTR_RD_GEN_START,        // Read: Generating START condition
    STATE_MSTR_RD_SENDING_ADDR,     // Read: Sending address + R bit
    STATE_MSTR_RD_READING_DATA,     // Read: Receiving data bytes
};

enum interrupt_type {
    INTER_TYPE_EVT,                 // Event interrupt
    INTER_TYPE_ERR,                 // Error interrupt
};

////////////////////////////////////////////////////////////////////////////////
// STATE STRUCTURE - Holds all info for one I2C instance
////////////////////////////////////////////////////////////////////////////////

struct i2c_state {
    struct i2c_cfg cfg;             // Configuration
    I2C_TypeDef* i2c_reg_base;      // Hardware registers (I2C3)
    int32_t guard_tmr_id;           // Timer ID (unused in happy path)
    
    uint8_t* msg_bfr;               // Data buffer
    uint32_t msg_len;               // Total bytes to transfer
    uint32_t msg_bytes_xferred;     // Bytes transferred so far
    
    uint16_t dest_addr;             // Slave address
    
    bool reserved;                  // Is bus reserved?
    enum states state;              // Current state

    enum i2c_errors last_op_error;  // ← ADD THIS LINE

};

////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

static void i2c_interrupt(enum i2c_instance_id instance_id,
                          enum interrupt_type inter_type);
static enum tmr_cb_action tmr_callback(int32_t tmr_id, uint32_t user_data);
static int32_t cmd_i2c_test(int32_t argc, const char** argv);
static void op_stop_fail(struct i2c_state* st, enum i2c_errors error);

////////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
////////////////////////////////////////////////////////////////////////////////

static struct i2c_state i2c_states[I2C_NUM_INSTANCES];

// Auto test state tracking
static bool auto_test_active = false;

#ifdef ENABLE_FAULT_INJECTION
// Fault injection flags (for testing error paths)
// Only compiled in Debug builds - completely removed from Release builds
static bool fault_inject_wrong_addr = false;  // Simulate wrong I2C address
static bool fault_inject_nack = false;        // Simulate NACK (unplugged sensor)
static bool fault_inject_timeout = false;     // Simulate timeout (stuck operation)
#endif

// Command registration
static struct cmd_cmd_info cmds[] = {
    {
        .name = "test",
        .func = cmd_i2c_test,
        .help = "Run test, usage: i2c test [auto|not_reserved] (enter no args for help)",
    },
};

static struct cmd_client_info cmd_info = {
    .name = "i2c",
    .num_cmds = ARRAY_SIZE(cmds),
    .cmds = cmds,
    .log_level_ptr = NULL,  // No log level support
    .num_u16_pms = 0,
    .u16_pms = NULL,
    .u16_pm_names = NULL,
};

////////////////////////////////////////////////////////////////////////////////
// PUBLIC API FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/*
 * @brief Get default I2C configuration
 *
 * @param[in] instance_id I2C instance identifier
 * @param[out] cfg Configuration structure to populate with defaults
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 */
int32_t i2c_get_def_cfg(enum i2c_instance_id instance_id, struct i2c_cfg* cfg)
{
    cfg->transaction_guard_time_ms = CONFIG_I2C_DFLT_TRANS_GUARD_TIME_MS;
    return 0;
}

/*
 * @brief Initialize I2C instance
 *
 * @param[in] instance_id I2C instance identifier
 * @param[in] cfg Configuration structure with timing parameters
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 *
 * @note Must be called before i2c_start(). Called once at system startup.
 */
int32_t i2c_init(enum i2c_instance_id instance_id, struct i2c_cfg* cfg)
{
    struct i2c_state* st = &i2c_states[instance_id];
    
    memset(st, 0, sizeof(*st));
    st->cfg = *cfg;
    st->i2c_reg_base = I2C3;  // Hardcoded for I2C3
    
    return 0;
}

/*
 * @brief Start I2C instance and enable interrupts
 *
 * @param[in] instance_id I2C instance identifier
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 *
 * @note Must be called after i2c_init(). Enables NVIC interrupts and registers console commands.
 */
int32_t i2c_start(enum i2c_instance_id instance_id)
{
    struct i2c_state* st = &i2c_states[instance_id];
    int32_t result;

    // Get a timer (for structure, not used in happy path)
    st->guard_tmr_id = tmr_inst_get_cb(0, tmr_callback, (uint32_t)instance_id);
    if (st->guard_tmr_id < 0)
        return MOD_ERR_RESOURCE;
    

    // Disable peripheral and interrupts initially
    LL_I2C_Disable(st->i2c_reg_base);
    DISABLE_ALL_INTERRUPTS(st);

    // Enable I2C3 interrupts in NVIC
    NVIC_SetPriority(I2C3_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(I2C3_EV_IRQn);
    NVIC_SetPriority(I2C3_ER_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(I2C3_ER_IRQn);

    // Register commands
    result = cmd_register(&cmd_info);
    if (result < 0) {
        return MOD_ERR_RESOURCE;
    }

    return 0;
}

/*
 * @brief Run I2C module state machine
 *
 * Polls automated test state machine if active. Called continuously from super loop.
 *
 * @param[in] instance_id I2C instance identifier
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 */
int32_t i2c_run(enum i2c_instance_id instance_id)
{
    if (auto_test_active) {
        int32_t rc = i2c_run_auto_test();
        if (rc > 0) {
            // Test completed
            auto_test_active = false;
            printc(">> I2C auto test completed\n\n");
        } else if (rc < 0) {
            // Test failed
            auto_test_active = false;
            printc(">> I2C auto test failed\n\n");
        }
        // If rc == 0, test is still in progress, continue polling
    }
    return 0;
}

/*
 * @brief Reserve I2C bus for exclusive access
 *
 * @param[in] instance_id I2C instance identifier
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 *
 * @note Must be called before i2c_write() or i2c_read(). Prevents other modules from using the bus.
 */
int32_t i2c_reserve(enum i2c_instance_id instance_id)
 {
     // Validate instance ID
     if (instance_id >= I2C_NUM_INSTANCES)
         return MOD_ERR_BAD_INSTANCE;
     
     struct i2c_state* st = &i2c_states[instance_id];
     
     // Check if already reserved
     if (st->reserved)
         return MOD_ERR_RESOURCE;
     
     st->reserved = true;
     return 0;  // Success
 }
/*
 * @brief Release I2C bus for other modules to use
 *
 * @param[in] instance_id I2C instance identifier
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 *
 * @note Must be called after operation completes. Call i2c_get_op_status() first to verify completion.
 */
int32_t i2c_release(enum i2c_instance_id instance_id)
 {
     // Validate instance ID
     if (instance_id >= I2C_NUM_INSTANCES)
         return MOD_ERR_BAD_INSTANCE;
     
     struct i2c_state* st = &i2c_states[instance_id];
     
     // Only release if it was reserved
     if (!st->reserved)
         return MOD_ERR_STATE;
     
     st->reserved = false;
     return 0;  // Success
 }

/*
 * @brief Start I2C write operation
 *
 * Initiates asynchronous write transaction. Operation completes in interrupt handler.
 * Poll i2c_get_op_status() to check completion.
 *
 * @param[in] instance_id I2C instance identifier
 * @param[in] dest_addr 7-bit slave device address
 * @param[in] msg_bfr Buffer containing data to write
 * @param[in] msg_len Number of bytes to write
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 *
 * @note Bus must be reserved with i2c_reserve() before calling. Driver must be in IDLE state.
 */
int32_t i2c_write(enum i2c_instance_id instance_id, uint32_t dest_addr, 
    uint8_t* msg_bfr, uint32_t msg_len){
// Validate instance ID
if (instance_id >= I2C_NUM_INSTANCES)
return MOD_ERR_BAD_INSTANCE;

struct i2c_state* st = &i2c_states[instance_id];

// Must be reserved
if (!st->reserved)
return MOD_ERR_NOT_RESERVED;

// Must be idle
if (st->state != STATE_IDLE)
return MOD_ERR_STATE;

// Save operation parameters
#ifdef ENABLE_FAULT_INJECTION
// Fault injection: use wrong address if enabled (simulates non-existent device)
st->dest_addr = fault_inject_wrong_addr ? 0x45 : dest_addr;
#else
st->dest_addr = dest_addr;  // Zero overhead in production builds
#endif
st->msg_bfr = msg_bfr;
st->msg_len = msg_len;
st->msg_bytes_xferred = 0;
st->last_op_error = I2C_ERR_NONE;  // ← Initialize error to NONE

// Set initial state
st->state = STATE_MSTR_WR_GEN_START;

// Enable I2C peripheral
LL_I2C_Enable(st->i2c_reg_base);

// Generate START condition
LL_I2C_GenerateStartCondition(st->i2c_reg_base);

// Enable interrupts (rest happens in interrupt handler!)
ENABLE_ALL_INTERRUPTS(st);

// Arm guard timer to catch stalled transactions
if (st->guard_tmr_id >= 0) {
#ifdef ENABLE_FAULT_INJECTION
    // Fault injection: use 1ms timeout if enabled (forces timeout before operation completes)
    uint32_t timeout_ms = fault_inject_timeout ? 1 : st->cfg.transaction_guard_time_ms;
    tmr_inst_start(st->guard_tmr_id, timeout_ms);
#else
    // Zero overhead in production builds
    tmr_inst_start(st->guard_tmr_id, st->cfg.transaction_guard_time_ms);
#endif
}

return 0;  // Success - operation started
}

/*
 * @brief Start I2C read operation
 *
 * Initiates asynchronous read transaction. Operation completes in interrupt handler.
 * Poll i2c_get_op_status() to check completion.
 *
 * @param[in] instance_id I2C instance identifier
 * @param[in] dest_addr 7-bit slave device address
 * @param[out] msg_bfr Buffer to store received data
 * @param[in] msg_len Number of bytes to read
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 *
 * @note Bus must be reserved with i2c_reserve() before calling. Driver must be in IDLE state.
 */
int32_t i2c_read(enum i2c_instance_id instance_id, uint32_t dest_addr,uint8_t* msg_bfr, uint32_t msg_len){
// Validate instance ID
if (instance_id >= I2C_NUM_INSTANCES)
return MOD_ERR_BAD_INSTANCE;

struct i2c_state* st = &i2c_states[instance_id];

// Must be reserved
if (!st->reserved)
return MOD_ERR_NOT_RESERVED;

// Must be idle
if (st->state != STATE_IDLE)
return MOD_ERR_STATE;

// Save operation parameters
#ifdef ENABLE_FAULT_INJECTION
// Fault injection: use wrong address if enabled (simulates non-existent device)
st->dest_addr = fault_inject_wrong_addr ? 0x45 : dest_addr;
#else
st->dest_addr = dest_addr;  // Zero overhead in production builds
#endif
st->msg_bfr = msg_bfr;
st->msg_len = msg_len;
st->msg_bytes_xferred = 0;
st->last_op_error = I2C_ERR_NONE;  // ← Initialize error to NONE

// Set initial state
st->state = STATE_MSTR_RD_GEN_START;

// Enable I2C peripheral
LL_I2C_Enable(st->i2c_reg_base);

// Generate START condition
LL_I2C_GenerateStartCondition(st->i2c_reg_base);

// Enable interrupts (rest happens in interrupt handler!)
ENABLE_ALL_INTERRUPTS(st);

// Arm guard timer to catch stalled transactions
if (st->guard_tmr_id >= 0) {
#ifdef ENABLE_FAULT_INJECTION
    // Fault injection: use 1ms timeout if enabled (forces timeout before operation completes)
    uint32_t timeout_ms = fault_inject_timeout ? 1 : st->cfg.transaction_guard_time_ms;
    tmr_inst_start(st->guard_tmr_id, timeout_ms);
#else
    // Zero overhead in production builds
    tmr_inst_start(st->guard_tmr_id, st->cfg.transaction_guard_time_ms);
#endif
}

return 0;  // Success - operation started
}

/*
 * @brief Get status of current I2C operation
 *
 * @param[in] instance_id I2C instance identifier
 *
 * @return 0 for success, MOD_ERR_OP_IN_PROG if operation still in progress,
 *         -1 if operation failed, else a "MOD_ERR" value. See code for details.
 *
 * @note If operation failed, call i2c_get_error() for detailed error code.
 *       After successful completion, call i2c_release() to free the bus.
 */
int32_t i2c_get_op_status(enum i2c_instance_id instance_id)
{
    // Validate instance ID
    if (instance_id >= I2C_NUM_INSTANCES)
        return MOD_ERR_BAD_INSTANCE;
    
    struct i2c_state* st = &i2c_states[instance_id];
    
    if (st->state != STATE_IDLE)
        return MOD_ERR_OP_IN_PROG;  // Still working
    
    // Operation complete - check if it succeeded or failed
    if (st->last_op_error == I2C_ERR_NONE)
        return 0;  // Success!
    else
        return MOD_ERR_PERIPH;  // Failed (use i2c_get_error() for details)
}

// FAILURE: Clean up and return to IDLE with error
static void op_stop_fail(struct i2c_state* st, enum i2c_errors error)
{
    // Disable all interrupts
    DISABLE_ALL_INTERRUPTS(st);
    
    // Always send STOP to release bus
    LL_I2C_GenerateStopCondition(st->i2c_reg_base);
    
    // Disable peripheral
    LL_I2C_Disable(st->i2c_reg_base);

    // Disarm guard timer so it cannot fire after cleanup
    if (st->guard_tmr_id >= 0)
        tmr_inst_start(st->guard_tmr_id, 0);
    
    // Record error and return to IDLE
    st->last_op_error = error;
    st->state = STATE_IDLE;
}


/*
 * @brief Get detailed error code from last failed operation
 *
 * @param[in] instance_id I2C instance identifier
 *
 * @return Error code from enum i2c_errors, or I2C_ERR_INVALID_INSTANCE if instance invalid
 *
 * @note Call this after i2c_get_op_status() returns -1 to get specific failure reason.
 */
enum i2c_errors i2c_get_error(enum i2c_instance_id instance_id)
{
    if (instance_id >= I2C_NUM_INSTANCES)
        return I2C_ERR_INVALID_INSTANCE;
    
    return i2c_states[instance_id].last_op_error;
}
////////////////////////////////////////////////////////////////////////////////
// INTERRUPT HANDLERS
////////////////////////////////////////////////////////////////////////////////

void I2C3_EV_IRQHandler(void)
{
    i2c_interrupt(I2C_INSTANCE_3, INTER_TYPE_EVT);
}

void I2C3_ER_IRQHandler(void)
{
    i2c_interrupt(I2C_INSTANCE_3, INTER_TYPE_ERR);
}

////////////////////////////////////////////////////////////////////////////////
// INTERRUPT HANDLER - THE HEART OF THE DRIVER
// 
// NO HELPER FUNCTIONS - all cleanup code inlined directly where it happens
// You can see the COMPLETE flow for each state in one place
////////////////////////////////////////////////////////////////////////////////

static void i2c_interrupt(enum i2c_instance_id instance_id,
                          enum interrupt_type inter_type)
{
    struct i2c_state* st = &i2c_states[instance_id];
    uint16_t sr1 = st->i2c_reg_base->SR1;  // Read status

    if (inter_type == INTER_TYPE_EVT) {
        switch (st->state) {
        
            // ===== WRITE SEQUENCE: 3 States =====
            
            case STATE_MSTR_WR_GEN_START:
                // HW sent START? Check SB (Start Bit) flag
                if (sr1 & LL_I2C_SR1_SB) {
                    // Send 7-bit address + W bit (0)
                    st->i2c_reg_base->DR = st->dest_addr << 1;
                    st->state = STATE_MSTR_WR_SENDING_ADDR;
                }
                break;

            case STATE_MSTR_WR_SENDING_ADDR:
                // Slave ACKed the address?
                if (sr1 & LL_I2C_SR1_ADDR) {
                    (void)st->i2c_reg_base->SR2;  // Clear ADDR flag
                    
                    // Start sending data bytes
                    st->state = STATE_MSTR_WR_SENDING_DATA;
                    st->i2c_reg_base->DR = st->msg_bfr[st->msg_bytes_xferred++];
                }
                break;

            case STATE_MSTR_WR_SENDING_DATA:
                // HW ready for next byte?
                if (sr1 & (LL_I2C_SR1_TXE | LL_I2C_SR1_BTF)) {
                    
                    if (st->msg_bytes_xferred < st->msg_len) {
                        // More bytes to send
                        st->i2c_reg_base->DR = st->msg_bfr[st->msg_bytes_xferred++];
                        
                    } else if (sr1 & LL_I2C_SR1_BTF) {
                        // All bytes sent → DONE! Clean up inline (no helper function)
                        DISABLE_ALL_INTERRUPTS(st);
                        LL_I2C_GenerateStopCondition(st->i2c_reg_base);
                        LL_I2C_Disable(st->i2c_reg_base);
                        if (st->guard_tmr_id >= 0)
                            tmr_inst_start(st->guard_tmr_id, 0);
                        st->state = STATE_IDLE;
                        st->last_op_error = I2C_ERR_NONE;  // ← ADD THIS LINE
                    }
                }
                break;

            // ===== READ SEQUENCE: 3 States =====
            
            case STATE_MSTR_RD_GEN_START:
                // HW sent START?
                if (sr1 & LL_I2C_SR1_SB) {
                    // Send 7-bit address + R bit (1)
                    st->i2c_reg_base->DR = (st->dest_addr << 1) | 1;
                    st->state = STATE_MSTR_RD_SENDING_ADDR;
                }
                break;

            case STATE_MSTR_RD_SENDING_ADDR:
                // Slave ACKed the address?
                if (sr1 & LL_I2C_SR1_ADDR) {
                    // Set ACK/NACK mode
                    if (st->msg_len == 1) {
                        LL_I2C_AcknowledgeNextData(st->i2c_reg_base, LL_I2C_NACK);
                    } else {
                        LL_I2C_AcknowledgeNextData(st->i2c_reg_base, LL_I2C_ACK);
                    }
                    
                    (void)st->i2c_reg_base->SR2;  // Clear ADDR flag
                    
                    // For single byte: generate STOP early
                    if (st->msg_len == 1) {
                        LL_I2C_GenerateStopCondition(st->i2c_reg_base);
                    }
                    
                    st->state = STATE_MSTR_RD_READING_DATA;
                }
                break;

            case STATE_MSTR_RD_READING_DATA:
                // HW received data?
                if (sr1 & LL_I2C_SR1_RXNE) {
                    // Read byte from DR register
                    st->msg_bfr[st->msg_bytes_xferred++] = st->i2c_reg_base->DR;
                    
                    if (st->msg_bytes_xferred >= st->msg_len) {
                        // All bytes received → DONE! Clean up inline (no helper function)
                        DISABLE_ALL_INTERRUPTS(st);
                        if (st->msg_len > 1) {  // STOP already sent for single byte
                            LL_I2C_GenerateStopCondition(st->i2c_reg_base);
                        }
                        LL_I2C_Disable(st->i2c_reg_base);
                        if (st->guard_tmr_id >= 0)
                            tmr_inst_start(st->guard_tmr_id, 0);
                        st->state = STATE_IDLE;
                        st->last_op_error = I2C_ERR_NONE;  // ← ADD THIS LINE
                        
                    } else if (st->msg_bytes_xferred == st->msg_len - 1) {
                        // Next byte is the last one → NACK it and send STOP
                        LL_I2C_AcknowledgeNextData(st->i2c_reg_base, LL_I2C_NACK);
                        LL_I2C_GenerateStopCondition(st->i2c_reg_base);
                    }
                }
                break;

            default:
                break;
        }
    } else if (inter_type == INTER_TYPE_ERR) {
#ifdef ENABLE_FAULT_INJECTION
        // FAULT INJECTION: Force NACK error if enabled (simulates unplugged sensor)
        // Only compiled in Debug builds - zero overhead in production
        if (fault_inject_nack) {
            // Clear error flags (required by hardware!)
            st->i2c_reg_base->SR1 &= ~(sr1 & INTERRUPT_ERR_MASK);

            // Force ACK_FAIL error
            op_stop_fail(st, I2C_ERR_ACK_FAIL);
            return;
        }
#endif
        // Classify the error
        enum i2c_errors i2c_error;
        
        if (sr1 & I2C_SR1_AF)
            i2c_error = I2C_ERR_ACK_FAIL;  // Slave didn't ACK
        else if (sr1 & I2C_SR1_BERR)
            i2c_error = I2C_ERR_BUS_ERR;   // Bus error
        else
            i2c_error = I2C_ERR_INTR_UNEXPECT;  // Unknown
        
        // Clear error flags (required by hardware!)
        st->i2c_reg_base->SR1 &= ~(sr1 & INTERRUPT_ERR_MASK);

        
        // Abort operation and clean up
        op_stop_fail(st, i2c_error);
        return;
    }
}

static enum tmr_cb_action tmr_callback(int32_t tmr_id, uint32_t user_data)
{
    (void)tmr_id;

    enum i2c_instance_id instance_id = (enum i2c_instance_id)user_data;

    if (instance_id >= I2C_NUM_INSTANCES)
        return TMR_CB_NONE;

    struct i2c_state* st = &i2c_states[instance_id];

    if (st->state != STATE_IDLE)
        op_stop_fail(st, I2C_ERR_GUARD_TMR);

    return TMR_CB_NONE;
}

/*
 * @brief Run automated I2C test sequence
 *
 * State machine that performs: reserve → write → read → release.
 * Tests communication with SHT31-D sensor at address 0x44.
 *
 * @return 1 if test completed successfully, 0 if test still in progress,
 *         negative value if test failed, else a "MOD_ERR" value. See code for details.
 *
 * @note Call repeatedly from super loop until return value is non-zero.
 *       Positive return indicates completion, negative indicates failure.
 */
int32_t i2c_run_auto_test(void)
{
    enum i2c_instance_id instance_id = I2C_INSTANCE_3;
    static uint32_t test_state = 0;
    static uint32_t msg_len = 0;
    static uint8_t msg_bfr[7];
    int32_t rc;

    switch (test_state) {
        case 0:  // Reserve the I2C bus
            rc = i2c_reserve(instance_id);
            if (rc != 0){
                printc("I2C_RESERVE_FAIL\n");
                return rc;
            }
            test_state = 1;
            return 0;  // Continue

        case 1:  // Write command to sensor
            msg_bfr[0] = 0x2c;  // High repeatability measurement
            msg_bfr[1] = 0x06;
            rc = i2c_write(instance_id, 0x44, msg_bfr, 2);
            if (rc != 0){
                printc("I2C_WRITE_FAIL\n");
                i2c_release(instance_id);
                test_state = 0;
                return rc;  // Test failed
            }
            test_state = 2;
            return 0;  // Continue

        case 2:  // Wait for write to complete
            rc = i2c_get_op_status(instance_id);
            if (rc == MOD_ERR_OP_IN_PROG)
                return 0;  // Still busy
            if (rc != 0){
                enum i2c_errors err = i2c_get_error(instance_id);
                printc("I2C_WRITE_FAIL: %d\n", err);
                i2c_release(instance_id);
                test_state = 0;
                return rc;  // Test failed
            }
            test_state = 3;
            return 0;  // Continue

        case 3:  // Read temperature/humidity data
            msg_len = 6;  // temp(2) + CRC + hum(2) + CRC
            rc = i2c_read(instance_id, 0x44, msg_bfr, msg_len);
            if (rc != 0){
                printc("Read start failed: %d\n", (int)rc);
                i2c_release(instance_id);
                test_state = 0;
                return rc;
            }
            test_state = 4;
            return 0;  // Continue

        case 4:  // Wait for read to complete
            rc = i2c_get_op_status(instance_id);
            if (rc == MOD_ERR_OP_IN_PROG)
                return 0;  // Still busy
            if (rc != 0){
                enum i2c_errors err = i2c_get_error(instance_id);
                printc("I2C_READ_FAIL: %d\n", err);
                i2c_release(instance_id);
                test_state = 0;
                return rc;
            }
            test_state = 5;
            return 0;  // Continue

        case 5:  // Release the bus
            rc = i2c_release(instance_id);
            if (rc != 0) {
                printc("I2C_RELEASE_FAIL: %d\n", (int)rc);
                return rc;
            }
            test_state = 0;  // Reset for next test
            return 1;  // Test complete

        default:
            test_state = 0;
            return 1;
    }
}

/*
 * @brief Test error detection for "not reserved" condition
 *
 * Verifies that i2c_write() and i2c_read() correctly return MOD_ERR_NOT_RESERVED
 * when called without first reserving the bus. Also verifies proper sequence still works.
 *
 * @return 1 if all tests passed, else 1 if any test failed
 */
int32_t i2c_test_not_reserved(void)
{
    enum i2c_instance_id instance_id = I2C_INSTANCE_3;
    uint8_t test_buffer[2] = {0x2c, 0x06};
    int32_t rc;

    printc("\n========================================\n");
    printc("  TEST: Not Reserved Error Detection\n");
    printc("========================================\n");

    // Test 1: Try to write WITHOUT reserving first
    printc("\n[TEST 1] Calling i2c_write() WITHOUT i2c_reserve()...\n");
    rc = i2c_write(instance_id, 0x44, test_buffer, 2);

    if (rc == MOD_ERR_NOT_RESERVED) {
        printc("  ✓ PASS: Correctly returned MOD_ERR_NOT_RESERVED (%d)\n", (int)rc);
    } else {
        printc("  ✗ FAIL: Expected MOD_ERR_NOT_RESERVED, got %d\n", (int)rc);
        return 1;  // Test failed
    }

    // Test 2: Try to read WITHOUT reserving first
    printc("\n[TEST 2] Calling i2c_read() WITHOUT i2c_reserve()...\n");
    rc = i2c_read(instance_id, 0x44, test_buffer, 2);

    if (rc == MOD_ERR_NOT_RESERVED) {
        printc("  ✓ PASS: Correctly returned MOD_ERR_NOT_RESERVED (%d)\n", (int)rc);
    } else {
        printc("  ✗ FAIL: Expected MOD_ERR_NOT_RESERVED, got %d\n", (int)rc);
        return 1;  // Test failed
    }

    // Test 3: Verify that proper sequence still works
    printc("\n[TEST 3] Verifying proper sequence (reserve → write) still works...\n");
    rc = i2c_reserve(instance_id);
    if (rc != 0) {
        printc("  ✗ FAIL: i2c_reserve() failed: %d\n", (int)rc);
        return 1;
    }

    rc = i2c_write(instance_id, 0x44, test_buffer, 2);
    if (rc == 0) {
        printc("  ✓ PASS: Proper sequence works (reserved → write succeeded)\n");
        // Clean up: release the bus
        i2c_release(instance_id);
    } else {
        printc("  ✗ FAIL: Write failed after reserve: %d\n", (int)rc);
        i2c_release(instance_id);
        return 1;
    }

    printc("\n========================================\n");
    printc("  All tests passed!\n");
    printc("========================================\n\n");

    return 1;  // All tests passed
}

////////////////////////////////////////////////////////////////////////////////
// COMMAND HANDLERS
////////////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_FAULT_INJECTION
/*
 * @brief Toggle wrong address fault injection
 *
 * Toggles the wrong address fault injection flag. When enabled, I2C operations
 * use address 0x45 instead of the actual address, simulating a non-existent device.
 *
 * @return 0 for success
 */
int32_t i2c_test_wrong_addr(void)
{
    // Toggle the fault injection flag
    fault_inject_wrong_addr = !fault_inject_wrong_addr;

    printc("\n========================================\n");
    printc("  Fault Injection: Wrong Address\n");
    printc("========================================\n");
    printc("  Status: %s\n", fault_inject_wrong_addr ? "ENABLED" : "DISABLED");
    if (fault_inject_wrong_addr) {
        printc("  Next I2C operation will use address 0x45 instead of actual address\n");
        printc("  This simulates addressing a non-existent device\n");
    } else {
        printc("  Normal addressing restored\n");
    }
    printc("========================================\n\n");
    return 0;
}

/*
 * @brief Toggle NACK fault injection
 *
 * Toggles the NACK fault injection flag. When enabled, any I2C error
 * is forced to become ACK_FAIL, simulating an unplugged sensor.
 *
 * @return 0 for success
 */
int32_t i2c_test_nack(void)
{
    // Toggle the fault injection flag
    fault_inject_nack = !fault_inject_nack;

    printc("\n========================================\n");
    printc("  Fault Injection: NACK (Unplugged Sensor)\n");
    printc("========================================\n");
    printc("  Status: %s\n", fault_inject_nack ? "ENABLED" : "DISABLED");
    if (fault_inject_nack) {
        printc("  Next I2C error will be forced to ACK_FAIL\n");
        printc("  This simulates an unplugged or non-responsive sensor\n");
    } else {
        printc("  Normal error handling restored\n");
    }
    printc("========================================\n\n");
    return 0;
}

/*
 * @brief Toggle timeout fault injection
 *
 * Toggles the timeout fault injection flag. When enabled, I2C operations
 * use a 1ms timeout instead of the normal guard time, forcing timeout errors.
 *
 * @return 0 for success
 */
int32_t i2c_test_timeout(void)
{
    // Toggle the fault injection flag
    fault_inject_timeout = !fault_inject_timeout;

    printc("\n========================================\n");
    printc("  Fault Injection: Timeout\n");
    printc("========================================\n");
    printc("  Status: %s\n", fault_inject_timeout ? "ENABLED" : "DISABLED");
    if (fault_inject_timeout) {
        printc("  Next I2C operation will use 1ms timeout instead of %dms\n", 
               CONFIG_I2C_DFLT_TRANS_GUARD_TIME_MS);
        printc("  This simulates a stuck operation (sensor crashed, bus stuck)\n");
    } else {
        printc("  Normal timeout restored\n");
    }
    printc("========================================\n\n");
    return 0;
}
#endif  // ENABLE_FAULT_INJECTION

/*
 * Command handler for "i2c test" command
 */
static int32_t cmd_i2c_test(int32_t argc, const char** argv)
{
    // Handle help case (just "i2c test" with no arguments)
    if (argc == 2) {
        printc("Test operations:\n"
               "  Run auto test: i2c test auto\n"
               "  Test not reserved: i2c test not_reserved\n");
#ifdef ENABLE_FAULT_INJECTION
        printc("  Toggle wrong addr fault: i2c test wrong_addr\n");
        printc("  Toggle NACK fault: i2c test nack\n");
        printc("  Toggle timeout fault: i2c test timeout\n");
#endif
        return 0;
    }
    
    if (argc < 3) {
        printc("Insufficient arguments\n");
        return MOD_ERR_BAD_CMD;
    }
    
    // Case-insensitive comparison (simple implementation)
    const char* op = argv[2];
    int match_auto = 0;
    int match_not_reserved = 0;
#ifdef ENABLE_FAULT_INJECTION
    int match_wrong_addr = 0;
    int match_nack = 0;
    int match_timeout = 0;
#endif
    
    // Simple case-insensitive comparison
    if ((op[0] == 'a' || op[0] == 'A') &&
        (op[1] == 'u' || op[1] == 'U') &&
        (op[2] == 't' || op[2] == 'T') &&
        (op[3] == 'o' || op[3] == 'O') &&
        op[4] == '\0') {
        match_auto = 1;
    } else if ((op[0] == 'n' || op[0] == 'N') &&
               (op[1] == 'o' || op[1] == 'O') &&
               (op[2] == 't' || op[2] == 'T') &&
               op[3] == '_' &&
               (op[4] == 'r' || op[4] == 'R') &&
               (op[5] == 'e' || op[5] == 'E') &&
               (op[6] == 's' || op[6] == 'S') &&
               (op[7] == 'e' || op[7] == 'E') &&
               (op[8] == 'r' || op[8] == 'R') &&
               (op[9] == 'v' || op[9] == 'V') &&
               (op[10] == 'e' || op[10] == 'E') &&
               (op[11] == 'd' || op[11] == 'D') &&
               op[12] == '\0') {
        match_not_reserved = 1;
    }
#ifdef ENABLE_FAULT_INJECTION
    else if ((op[0] == 'w' || op[0] == 'W') &&
             (op[1] == 'r' || op[1] == 'R') &&
             (op[2] == 'o' || op[2] == 'O') &&
             (op[3] == 'n' || op[3] == 'N') &&
             (op[4] == 'g' || op[4] == 'G') &&
             op[5] == '_' &&
             (op[6] == 'a' || op[6] == 'A') &&
             (op[7] == 'd' || op[7] == 'D') &&
             (op[8] == 'd' || op[8] == 'D') &&
             (op[9] == 'r' || op[9] == 'R') &&
             (op[10] == 'r' || op[10] == 'R') &&
             op[11] == '\0') {
        match_wrong_addr = 1;
    } else if ((op[0] == 'n' || op[0] == 'N') &&
               (op[1] == 'a' || op[1] == 'A') &&
               (op[2] == 'c' || op[2] == 'C') &&
               (op[3] == 'k' || op[3] == 'K') &&
               op[4] == '\0') {
        match_nack = 1;
    } else if ((op[0] == 't' || op[0] == 'T') &&
               (op[1] == 'i' || op[1] == 'I') &&
               (op[2] == 'm' || op[2] == 'M') &&
               (op[3] == 'e' || op[3] == 'E') &&
               (op[4] == 'o' || op[4] == 'O') &&
               (op[5] == 'u' || op[5] == 'U') &&
               (op[6] == 't' || op[6] == 'T') &&
               op[7] == '\0') {
        match_timeout = 1;
    }
#endif
    
    if (match_auto) {
        // Start auto test
        auto_test_active = true;
        printc("\n>> Starting I2C auto test...\n");
        // Call once to start the state machine
        i2c_run_auto_test();
        return 0;
    } else if (match_not_reserved) {
        // Run not_reserved test immediately (it's synchronous)
        return i2c_test_not_reserved();
#ifdef ENABLE_FAULT_INJECTION
    } else if (match_wrong_addr) {
        // Toggle wrong address fault injection
        return i2c_test_wrong_addr();
    } else if (match_nack) {
        // Toggle NACK fault injection
        return i2c_test_nack();
    } else if (match_timeout) {
        // Toggle timeout fault injection
        return i2c_test_timeout();
#endif
    } else {
        printc("Unknown test operation '%s'\n", op);
        printc("Valid operations: auto, not_reserved");
#ifdef ENABLE_FAULT_INJECTION
        printc(", wrong_addr, nack, timeout");
#endif
        printc("\n");
        return MOD_ERR_BAD_CMD;
    }
}
