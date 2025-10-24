/*
 * SIMPLIFIED I2C DRIVER - For Learning the Critical Path
 * 
 * This is a stripped-down version focusing on the essential state machine
 * and interrupt handling. Non-essential features removed:
 * - Console commands (debugging only)
 * - Performance counters (nice to have)
 * - I2C1/I2C2 support (focus on I2C3 only)
 * - Complex logging
 * 
 * CRITICAL PATH TO UNDERSTAND:
 * 1. Reserve/Release (resource sharing)
 * 2. Write/Read APIs (non-blocking start)
 * 3. State machine (7 states)
 * 4. Interrupt handler (drives state transitions)
 * 5. Guard timer (timeout protection)
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include "stm32f4xx.h"
#include "stm32f4xx_ll_i2c.h"

#include "config.h"
#include "module.h"
#include "tmr.h"
#include "i2c.h"

////////////////////////////////////////////////////////////////////////////////
// MACROS - Interrupt control
////////////////////////////////////////////////////////////////////////////////

#define INTERRUPT_ENABLE_MASK (LL_I2C_CR2_ITEVTEN | LL_I2C_CR2_ITBUFEN | \
                               LL_I2C_CR2_ITERREN)
#define DISABLE_ALL_INTERRUPTS(st) st->i2c_reg_base->CR2 &= ~INTERRUPT_ENABLE_MASK
#define ENABLE_ALL_INTERRUPTS(st) st->i2c_reg_base->CR2 |= INTERRUPT_ENABLE_MASK

// Error flags from STM32 reference manual
#define INTERRUPT_ERR_MASK (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | \
                            I2C_SR1_OVR | I2C_SR1_PECERR | I2C_SR1_TIMEOUT)

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
    INTER_TYPE_EVT,                 // Event interrupt (SB, ADDR, TXE, RXNE, etc.)
    INTER_TYPE_ERR,                 // Error interrupt (BERR, AF, etc.)
};

////////////////////////////////////////////////////////////////////////////////
// STATE STRUCTURE - Holds all info for one I2C instance
////////////////////////////////////////////////////////////////////////////////

struct i2c_state {
    struct i2c_cfg cfg;             // Configuration
    I2C_TypeDef* i2c_reg_base;      // Hardware registers (I2C3)
    int32_t guard_tmr_id;           // Timeout timer ID
    
    uint8_t* msg_bfr;               // Data buffer
    uint32_t msg_len;               // Total bytes to transfer
    uint32_t msg_bytes_xferred;     // Bytes transferred so far
    
    uint16_t dest_addr;             // Slave address
    
    bool reserved;                  // Is bus reserved?
    enum states state;              // Current state
    enum i2c_errors last_op_error;  // Last error code
};

////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

static int32_t start_op(enum i2c_instance_id instance_id, uint32_t dest_addr,
                        uint8_t* msg_bfr, uint32_t msg_len,
                        enum states init_state);
static void i2c_interrupt(enum i2c_instance_id instance_id,
                          enum interrupt_type inter_type,
                          IRQn_Type irq_type);
static enum tmr_cb_action tmr_callback(int32_t tmr_id, uint32_t user_data);
static void op_stop_success(struct i2c_state* st, bool set_stop);
static void op_stop_fail(struct i2c_state* st, enum i2c_errors error);

////////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES - One state per I2C instance
////////////////////////////////////////////////////////////////////////////////

static struct i2c_state i2c_states[I2C_NUM_INSTANCES];

////////////////////////////////////////////////////////////////////////////////
// PUBLIC API FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

/*
 * Get default configuration
 */
int32_t i2c_get_def_cfg(enum i2c_instance_id instance_id, struct i2c_cfg* cfg)
{
    cfg->transaction_guard_time_ms = CONFIG_I2C_DFLT_TRANS_GUARD_TIME_MS;
    return 0;
}

/*
 * Initialize I2C instance - Called once at startup
 */
int32_t i2c_init(enum i2c_instance_id instance_id, struct i2c_cfg* cfg)
{
    struct i2c_state* st;

    if (instance_id >= I2C_NUM_INSTANCES)
        return MOD_ERR_BAD_INSTANCE;
    if (cfg == NULL)
        return MOD_ERR_ARG;

    st = &i2c_states[instance_id];
    memset(st, 0, sizeof(*st));
    st->cfg = *cfg;

    // Only supporting I2C3 in this simplified version
    if (instance_id == I2C_INSTANCE_3) {
        st->i2c_reg_base = I2C3;
    } else {
        return MOD_ERR_BAD_INSTANCE;
    }
    
    return 0;
}

/*
 * Start I2C instance - Enable interrupts, get guard timer
 */
int32_t i2c_start(enum i2c_instance_id instance_id)
{
    struct i2c_state* st;

    if (instance_id >= I2C_NUM_INSTANCES ||
        i2c_states[instance_id].i2c_reg_base == NULL)
        return MOD_ERR_BAD_INSTANCE;

    st = &i2c_states[instance_id];

    // Get a timer for timeout protection
    st->guard_tmr_id = tmr_inst_get_cb(0, tmr_callback, (uint32_t)instance_id);
    if (st->guard_tmr_id < 0)
        return MOD_ERR_RESOURCE;

    // Disable peripheral and interrupts initially
    LL_I2C_Disable(st->i2c_reg_base);
    DISABLE_ALL_INTERRUPTS(st);

    // Enable I2C3 interrupts in NVIC
    NVIC_SetPriority(I2C3_EV_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(I2C3_EV_IRQn);
    NVIC_SetPriority(I2C3_ER_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(I2C3_ER_IRQn);

    return 0;
}

/*
 * Run function - Required by module framework (does nothing for I2C)
 */
int32_t i2c_run(enum i2c_instance_id instance_id)
{
    return 0;
}

/*
 * RESERVE BUS - Get exclusive access
 * Returns: 0 = success, MOD_ERR_RESOURCE = already reserved
 */
int32_t i2c_reserve(enum i2c_instance_id instance_id)
{
    if (instance_id >= I2C_NUM_INSTANCES ||
        i2c_states[instance_id].i2c_reg_base == NULL)
        return MOD_ERR_BAD_INSTANCE;
        
    if (i2c_states[instance_id].reserved) {
        return MOD_ERR_RESOURCE;  // Already reserved by someone
    }
    
    i2c_states[instance_id].reserved = true;
    return 0;
}

/*
 * RELEASE BUS - Free for others to use
 */
int32_t i2c_release(enum i2c_instance_id instance_id)
{
    if (instance_id >= I2C_NUM_INSTANCES ||
        i2c_states[instance_id].i2c_reg_base == NULL)
        return MOD_ERR_BAD_INSTANCE;
        
    i2c_states[instance_id].reserved = false;
    return 0;
}

/*
 * WRITE - Non-blocking start of write operation
 * Returns immediately! Check status with i2c_get_op_status()
 */
int32_t i2c_write(enum i2c_instance_id instance_id, uint32_t dest_addr,
                  uint8_t* msg_bfr, uint32_t msg_len)
{
    return start_op(instance_id, dest_addr, msg_bfr, msg_len,
                    STATE_MSTR_WR_GEN_START);
}

/*
 * READ - Non-blocking start of read operation
 * Returns immediately! Check status with i2c_get_op_status()
 */
int32_t i2c_read(enum i2c_instance_id instance_id, uint32_t dest_addr,
                 uint8_t* msg_bfr, uint32_t msg_len)
{
    return start_op(instance_id, dest_addr, msg_bfr, msg_len,
                    STATE_MSTR_RD_GEN_START);
}

/*
 * GET ERROR - Get detailed error code
 */
enum i2c_errors i2c_get_error(enum i2c_instance_id instance_id)
{
   if (instance_id >= I2C_NUM_INSTANCES ||
        i2c_states[instance_id].i2c_reg_base == NULL)
       return I2C_ERR_INVALID_INSTANCE;

   return i2c_states[instance_id].last_op_error;
}

/*
 * GET STATUS - Poll this to check if operation complete
 * Returns: 0 = success, MOD_ERR_OP_IN_PROG = still working, MOD_ERR_PERIPH = failed
 */
int32_t i2c_get_op_status(enum i2c_instance_id instance_id)
{
    struct i2c_state* st;

    if (instance_id >= I2C_NUM_INSTANCES)
        return MOD_ERR_BAD_INSTANCE;

    st = &i2c_states[instance_id];
    
    if (!st->reserved)
        return MOD_ERR_NOT_RESERVED;
    else if (st->state == STATE_IDLE)
        return st->last_op_error == I2C_ERR_NONE ? 0 : MOD_ERR_PERIPH;
    else
        return MOD_ERR_OP_IN_PROG;  // Still working!
}

/*
 * BUS BUSY - Check if bus is busy (optional helper)
 */
int32_t i2c_bus_busy(enum i2c_instance_id instance_id)
{
   if (instance_id >= I2C_NUM_INSTANCES ||
        i2c_states[instance_id].i2c_reg_base == NULL)
        return MOD_ERR_BAD_INSTANCE;

   return LL_I2C_IsActiveFlag_BUSY(i2c_states[instance_id].i2c_reg_base);
}

////////////////////////////////////////////////////////////////////////////////
// INTERRUPT HANDLERS - Override weak defaults from startup code
////////////////////////////////////////////////////////////////////////////////

#if CONFIG_I2C_HAVE_INSTANCE_3 == 1

void I2C3_EV_IRQHandler(void)
{
    i2c_interrupt(I2C_INSTANCE_3, INTER_TYPE_EVT, I2C3_EV_IRQn);
}

void I2C3_ER_IRQHandler(void)
{
    i2c_interrupt(I2C_INSTANCE_3, INTER_TYPE_ERR, I2C3_ER_IRQn);
}

#endif

////////////////////////////////////////////////////////////////////////////////
// PRIVATE (STATIC) FUNCTIONS - The Real Work Happens Here
////////////////////////////////////////////////////////////////////////////////

/*
 * START OPERATION - Common code for read and write
 * This is NON-BLOCKING! It starts the operation and returns immediately.
 */
static int32_t start_op(enum i2c_instance_id instance_id, uint32_t dest_addr,
                        uint8_t* msg_bfr, uint32_t msg_len,
                        enum states init_state)
{
    struct i2c_state* st;

    if (instance_id >= I2C_NUM_INSTANCES ||
        i2c_states[instance_id].i2c_reg_base == NULL)
        return MOD_ERR_BAD_INSTANCE;

    st = &i2c_states[instance_id];
    
    // Must be reserved
    if (!st->reserved)
        return MOD_ERR_NOT_RESERVED;

    // Must be idle (no operation in progress)
    if (st->state != STATE_IDLE)
        return MOD_ERR_STATE;

    // Check if bus is busy
    if (LL_I2C_IsActiveFlag_BUSY(st->i2c_reg_base)) {
        st->last_op_error = I2C_ERR_BUS_BUSY;
        return MOD_ERR_PERIPH;
    }

    // Start guard timer (100ms timeout)
    tmr_inst_start(st->guard_tmr_id, st->cfg.transaction_guard_time_ms);

    // Save operation parameters
    st->dest_addr = dest_addr;
    st->msg_bfr = msg_bfr;
    st->msg_len = msg_len;
    st->msg_bytes_xferred = 0;
    st->last_op_error = I2C_ERR_NONE;

    // Set initial state
    st->state = init_state;

    // Enable I2C peripheral
    LL_I2C_Enable(st->i2c_reg_base);
    
    // Generate START condition (hardware will do this)
    LL_I2C_GenerateStartCondition(st->i2c_reg_base);

    // Enable interrupts (rest happens in interrupt handler!)
    ENABLE_ALL_INTERRUPTS(st);

    return 0;  // SUCCESS: Operation started (NOT finished!)
}

/*
 * I2C INTERRUPT HANDLER - THE HEART OF THE DRIVER
 * 
 * This is called by hardware when I2C events occur:
 * - START condition sent (SB flag)
 * - Address ACKed (ADDR flag)
 * - Data byte sent (TXE flag)
 * - Data byte received (RXNE flag)
 * - Errors (AF, BERR, etc.)
 * 
 * The state machine processes one step per interrupt.
 */
static void i2c_interrupt(enum i2c_instance_id instance_id,
                          enum interrupt_type inter_type,
                          IRQn_Type irq_type)
{
    struct i2c_state* st;
    uint16_t sr1;

    if (instance_id >= I2C_NUM_INSTANCES)
        return;

    st = &i2c_states[instance_id];

    // Safety check
    if (st->i2c_reg_base == NULL) {
        NVIC_DisableIRQ(irq_type);
        return;
    }
    
    // Read status register (clears some flags)
    sr1 = st->i2c_reg_base->SR1;

    // ========== EVENT INTERRUPTS ==========
    if (inter_type == INTER_TYPE_EVT)
    {
        switch (st->state) {
        
            // ------ WRITE STATES ------
            
            case STATE_MSTR_WR_GEN_START:
                // START condition sent?
                if (sr1 & LL_I2C_SR1_SB) {
                    // Write address with W bit (bit 0 = 0)
                    st->i2c_reg_base->DR = st->dest_addr << 1;
                    st->state = STATE_MSTR_WR_SENDING_ADDR;
                }
                break;

            case STATE_MSTR_WR_SENDING_ADDR:
                // Address ACKed?
                if (sr1 & LL_I2C_SR1_ADDR) {
                    // Clear ADDR flag by reading SR2
                    (void)st->i2c_reg_base->SR2;

                    if (st->msg_len == 0) {
                        // Zero-byte write, just stop
                        op_stop_success(st, true);
                    } else {
                        // Send first data byte
                        st->state = STATE_MSTR_WR_SENDING_DATA;
                        if (sr1 & LL_I2C_SR1_TXE) {
                            st->i2c_reg_base->DR = st->msg_bfr[st->msg_bytes_xferred++];
                        }
                    }
                }
                break;

            case STATE_MSTR_WR_SENDING_DATA:
                // Ready to send next byte?
                if (sr1 & (LL_I2C_SR1_TXE | LL_I2C_SR1_BTF)) {
                    if (st->msg_bytes_xferred < st->msg_len) {
                        // Send next byte
                        st->i2c_reg_base->DR = st->msg_bfr[st->msg_bytes_xferred++];
                    } else {
                        // All bytes sent, wait for BTF before STOP
                        if (sr1 & LL_I2C_SR1_BTF) {
                            op_stop_success(st, true);
                        }
                    }
                }
                break;

            // ------ READ STATES ------
            
            case STATE_MSTR_RD_GEN_START:
                // START condition sent?
                if (sr1 & LL_I2C_SR1_SB) {
                    // Write address with R bit (bit 0 = 1)
                    st->i2c_reg_base->DR = (st->dest_addr << 1) | 1;
                    st->state = STATE_MSTR_RD_SENDING_ADDR;
                }
                break;

            case STATE_MSTR_RD_SENDING_ADDR:
                // Address ACKed?
                if (sr1 & LL_I2C_SR1_ADDR) {
                    if (st->msg_len == 1) {
                        // Single byte read: NACK it
                        LL_I2C_AcknowledgeNextData(st->i2c_reg_base, LL_I2C_NACK);
                    } else {
                        // Multi-byte read: ACK them
                        LL_I2C_AcknowledgeNextData(st->i2c_reg_base, LL_I2C_ACK);
                    }
                    // Clear ADDR flag by reading SR2
                    (void)st->i2c_reg_base->SR2;
                    
                    if (st->msg_len == 1) {
                        // For single byte, generate STOP now
                        LL_I2C_GenerateStopCondition(st->i2c_reg_base);
                    }
                    st->state = STATE_MSTR_RD_READING_DATA;
                }
                break;

            case STATE_MSTR_RD_READING_DATA:
                // Data byte received?
                if (sr1 & LL_I2C_SR1_RXNE) {
                    // Read the byte
                    st->msg_bfr[st->msg_bytes_xferred++] = st->i2c_reg_base->DR;
                    
                    // Check if this was the last byte
                    if (st->msg_bytes_xferred >= st->msg_len) {
                        op_stop_success(st, st->msg_len > 1);  // STOP already sent for single byte
                    } else if (st->msg_bytes_xferred == st->msg_len - 1) {
                        // Next byte is last: NACK it
                        LL_I2C_AcknowledgeNextData(st->i2c_reg_base, LL_I2C_NACK);
                        LL_I2C_GenerateStopCondition(st->i2c_reg_base);
                    }
                }
                break;

            default:
                break;
        }
    }
    
    // ========== ERROR INTERRUPTS ==========
    else if (inter_type == INTER_TYPE_ERR) {
        enum i2c_errors i2c_error = I2C_ERR_INTR_UNEXPECT;

        // Clear error flags
        st->i2c_reg_base->SR1 &= ~(sr1 & INTERRUPT_ERR_MASK);

        // Identify specific error
        if (sr1 & I2C_SR1_TIMEOUT)
            i2c_error = I2C_ERR_TIMEOUT;
        else if (sr1 & I2C_SR1_PECERR)
            i2c_error = I2C_ERR_PEC;
        else if (sr1 & I2C_SR1_AF)
            i2c_error = I2C_ERR_ACK_FAIL;
        else if (sr1 & I2C_SR1_BERR)
            i2c_error = I2C_ERR_BUS_ERR;

        op_stop_fail(st, i2c_error);
    }
}

/*
 * GUARD TIMER CALLBACK - Operation timeout
 * If this fires, the I2C operation took too long (> 100ms)
 */
static enum tmr_cb_action tmr_callback(int32_t tmr_id, uint32_t user_data)
{
    struct i2c_state* st;
    enum i2c_instance_id instance_id = (enum i2c_instance_id)user_data;

    if (instance_id >= I2C_NUM_INSTANCES ||
        i2c_states[instance_id].i2c_reg_base == NULL)
        return TMR_CB_NONE;

    st = &i2c_states[instance_id];
    op_stop_fail(st, I2C_ERR_GUARD_TMR);

    return TMR_CB_NONE;
}

/*
 * OPERATION SUCCESS - Clean up and return to IDLE
 */
static void op_stop_success(struct i2c_state* st, bool set_stop)
{
    DISABLE_ALL_INTERRUPTS(st);
    
    if (set_stop)
        LL_I2C_GenerateStopCondition(st->i2c_reg_base);
    
    // Cancel guard timer
    tmr_inst_start(st->guard_tmr_id, 0);
    
    LL_I2C_Disable(st->i2c_reg_base);
    st->state = STATE_IDLE;
    st->last_op_error = I2C_ERR_NONE;
}

/*
 * OPERATION FAILED - Clean up and return to IDLE with error
 */
static void op_stop_fail(struct i2c_state* st, enum i2c_errors error)
{
    DISABLE_ALL_INTERRUPTS(st);
    LL_I2C_GenerateStopCondition(st->i2c_reg_base);
    
    // Cancel guard timer
    tmr_inst_start(st->guard_tmr_id, 0);
    
    LL_I2C_Disable(st->i2c_reg_base);

    // Record error (only first error per transaction)
    if (st->last_op_error == I2C_ERR_NONE) {
        st->last_op_error = error;
    }
    
    st->state = STATE_IDLE;
}
