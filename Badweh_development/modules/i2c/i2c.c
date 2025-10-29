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
#include "cmd.h"
#include "console.h"
#include "lwl.h"
#include "i2c.h"

////////////////////////////////////////////////////////////////////////////////
// MACROS - Interrupt control
////////////////////////////////////////////////////////////////////////////////

// LWL (Lightweight Logging) - uses global definitions from lwl.h

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

static int32_t cmd_i2c_test(int32_t argc, const char** argv);

////////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES - One state per I2C instance
////////////////////////////////////////////////////////////////////////////////

static struct i2c_state i2c_states[I2C_NUM_INSTANCES];

// Data structure with console command info.
static struct cmd_cmd_info cmds[] = {
    {
        .name = "test",
        .func = cmd_i2c_test,
        .help = "Run test, usage: i2c test [<op> [<arg>]] (enter no op/arg for help)",
    }
};

// Data structure passed to cmd module for console interaction.
static struct cmd_client_info cmd_info = {
    .name = "i2c",
    .num_cmds = 1,
    .cmds = cmds,
};

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

    // Register command with console
    int32_t result = cmd_register(&cmd_info);
    if (result < 0) {
        return MOD_ERR_RESOURCE;
    }

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
    LWL("I2C_RESERVE", 1, LWL_1(instance_id));
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
    LWL("I2C_RELEASE", 1, LWL_1(instance_id));
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

/*
 * AUTOMATED TEST - Run button-triggered I2C test sequence
 * Returns: 0 = in progress, 1 = test complete
 */
int32_t i2c_run_auto_test(void)
{
    enum i2c_instance_id instance_id = I2C_INSTANCE_3;
    static uint32_t test_state = 0;
    static uint32_t msg_len = 0;
    static uint8_t msg_bfr[7];
    int32_t rc;

    switch (test_state) {
        case 0:  // Reserve
            printc("\n=== I2C AUTO TEST START ===\n");
            rc = i2c_reserve(instance_id);
            if (rc == 0) {
                printc("[1/6] Reserve: OK\n");
                test_state = 1;
            } else {
                printc("[1/6] Reserve: FAIL (%ld)\n", rc);
                test_state = 0;
                return 1;
            }
            return 0;

        case 1:  // Write
            msg_bfr[0] = 0x2c;
            msg_bfr[1] = 0x06;
            rc = i2c_write(instance_id, 0x44, msg_bfr, 2);
            if (rc == 0) {
                printc("[2/6] Write started: OK\n");
                test_state = 2;
            } else {
                printc("[2/6] Write: FAIL (%ld)\n", rc);
                i2c_release(instance_id);
                test_state = 0;
                return 1;
            }
            return 0;

        case 2:  // Wait for write
            rc = i2c_get_op_status(instance_id);
            if (rc == 0) {
                printc("[3/6] Write complete: OK\n");
                test_state = 3;
            } else if (rc == MOD_ERR_OP_IN_PROG) {
                return 0;  // Still waiting
            } else {
                printc("[3/6] Write status: FAIL (%ld)\n", rc);
                i2c_release(instance_id);
                test_state = 0;
                return 1;
            }
            return 0;

        case 3:  // Read
            msg_len = 6;
            rc = i2c_read(instance_id, 0x44, msg_bfr, msg_len);
            if (rc == 0) {
                printc("[4/6] Read started: OK\n");
                test_state = 4;
            } else {
                printc("[4/6] Read: FAIL (%ld)\n", rc);
                i2c_release(instance_id);
                test_state = 0;
                return 1;
            }
            return 0;

        case 4:  // Wait for read
            rc = i2c_get_op_status(instance_id);
            if (rc == 0) {
                printc("[5/6] Read complete: OK\n");
                printc("  Data: ");
                for (int32_t i = 0; i < msg_len; i++) {
                    printc("0x%02x ", msg_bfr[i]);
                }
                printc("\n");
                test_state = 5;
            } else if (rc == MOD_ERR_OP_IN_PROG) {
                return 0;  // Still waiting
            } else {
                printc("[5/6] Read status: FAIL (%ld)\n", rc);
                i2c_release(instance_id);
                test_state = 0;
                return 1;
            }
            return 0;

        case 5:  // Release
            rc = i2c_release(instance_id);
            printc("[6/6] Release: %s\n", rc == 0 ? "OK" : "FAIL");
            printc("=== I2C AUTO TEST DONE ===\n\n");
            test_state = 0;
            return 1;

        default:
            test_state = 0;
            return 1;
    }
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
    LWL("I2C_OP_START", 3, LWL_1(instance_id), LWL_2(dest_addr), LWL_1(msg_len));

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
            	LWL("i2c_state", 1, LWL_1(st->state));
            	LWL("i2c_start", 0);
                if (sr1 & LL_I2C_SR1_SB) {
                    LWL("I2C_WR_START", 2, LWL_1(st->dest_addr), LWL_1(st->msg_len));
                    // Write address with W bit (bit 0 = 0)
                    st->i2c_reg_base->DR = st->dest_addr << 1;
                    st->state = STATE_MSTR_WR_SENDING_ADDR;
                }
                break;

            case STATE_MSTR_WR_SENDING_ADDR:
                // Address ACKed?
            	LWL("i2c_state", 1, LWL_1(st->state));
                if (sr1 & LL_I2C_SR1_ADDR) {
                    LWL("I2C_WR_ADDR_ACK", 1, LWL_1(st->dest_addr));
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
            	LWL("i2c_state", 1, LWL_1(st->state));
                if (sr1 & (LL_I2C_SR1_TXE | LL_I2C_SR1_BTF)) {
                    if (st->msg_bytes_xferred < st->msg_len) {
                        // Send next byte
                        st->i2c_reg_base->DR = st->msg_bfr[st->msg_bytes_xferred++];
                    } else {
                        // All bytes sent, wait for BTF before STOP
                        if (sr1 & LL_I2C_SR1_BTF) {
                            LWL("I2C_WR_DONE", 1, LWL_1(st->msg_len));
                            op_stop_success(st, true);
                        }
                    }
                }
                break;

            // ------ READ STATES ------
            
            case STATE_MSTR_RD_GEN_START:
                // START condition sent?
            	LWL("i2c_state", 1, LWL_1(st->state));
                if (sr1 & LL_I2C_SR1_SB) {
                    LWL("I2C_RD_START", 2, LWL_1(st->dest_addr), LWL_1(st->msg_len));
                    // Write address with R bit (bit 0 = 1)
                    st->i2c_reg_base->DR = (st->dest_addr << 1) | 1;
                    st->state = STATE_MSTR_RD_SENDING_ADDR;
                }
                break;

            case STATE_MSTR_RD_SENDING_ADDR:
                // Address ACKed?
            	LWL("i2c_state", 1, LWL_1(st->state));
                if (sr1 & LL_I2C_SR1_ADDR) {
                    LWL("I2C_RD_ADDR_ACK", 1, LWL_1(st->dest_addr));
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
            	LWL("i2c_state", 1, LWL_1(st->state));
                if (sr1 & LL_I2C_SR1_RXNE) {
                    // Read the byte
                    st->msg_bfr[st->msg_bytes_xferred++] = st->i2c_reg_base->DR;
                    
                    // Check if this was the last byte
                    if (st->msg_bytes_xferred >= st->msg_len) {
                        LWL("I2C_RD_DONE", 1, LWL_1(st->msg_len));
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

        LWL("I2C_ERROR", 2, LWL_1(i2c_error), LWL_1(sr1));
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
    LWL("I2C_SUCCESS", 2, LWL_1(st->msg_bytes_xferred), LWL_1(st->msg_len));
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
    	LWL("I2C_FAIL", 2, LWL_1(error), LWL_1(st->state));
        st->last_op_error = error;
    }
    
    st->state = STATE_IDLE;
}

////////////////////////////////////////////////////////////////////////////////
// Console Test Command
////////////////////////////////////////////////////////////////////////////////

/*
 * @brief Console command function for "i2c test".
 *
 * @param[in] argc Number of arguments, including "i2c"
 * @param[in] argv Argument values, including "i2c"
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 *
 * Command usage: i2c test [<op> [<arg>]]
 *
 * Test operations (uses I2C3 only):
 *   reserve
 *   release
 *   write <addr> [<bytes> ...]
 *   read <addr> <num-bytes>
 *   status
 *   busy
 *   msg
 */
static int32_t cmd_i2c_test(int32_t argc, const char** argv)
{
    struct cmd_arg_val arg_vals[8];
    int32_t rc = 0;
    int32_t idx;
    
#define MAX_MSG_LEN 7
    static uint32_t msg_len;
    enum i2c_instance_id instance_id = I2C_INSTANCE_3;
    static uint8_t msg_bfr[MAX_MSG_LEN];

    // Handle help case.
    if (argc == 2) {
        printc("I2C Test operations (using I2C3):\n");
        printc("  reserve                    - Reserve I2C3 bus\n");
        printc("  release                    - Release I2C3 bus\n");
        printc("  write <addr> [<bytes> ...]  - Write to I2C3\n");
        printc("  read <addr> <num-bytes>      - Read from I2C3\n");
        printc("  status                     - Get operation status\n");
        printc("  busy                       - Check if bus busy\n");
        printc("  msg                        - Print message buffer\n");
        printc("\nExample workflow:\n");
        printc("  i2c test reserve              - Reserve bus\n");
        printc("  i2c test write 0x44 0x2c 0x06 - Write to SHT31-D\n");
        printc("  i2c test status                - Check if done\n");
        printc("  i2c test read 0x44 6           - Read 6 bytes\n");
        printc("  i2c test status                - Check if done\n");
        printc("  i2c test msg                   - View data\n");
        printc("  i2c test release               - Release bus\n");
        return 0;
    }

    if (argc <= 2) {
        printc("No operation specified. Try 'i2c test' for help.\n");
        return MOD_ERR_BAD_CMD;
    }

    if (strcasecmp(argv[2], "reserve") == 0) {
        rc = i2c_reserve(instance_id);
        printc("Reserve result: %ld\n", rc);
    } 
    else if (strcasecmp(argv[2], "release") == 0) {
        rc = i2c_release(instance_id);
        printc("Release result: %ld\n", rc);
    } 
    else if (strcasecmp(argv[2], "write") == 0) {
        if (argc < 4) {
            printc("Usage: i2c test write <addr> [<bytes> ...]\n");
            return MOD_ERR_BAD_CMD;
        }
        
        rc = cmd_parse_args(argc-3, argv+3, "u[u[u[u[u[u]]]]]", arg_vals);
        if (rc < 1) {
            return MOD_ERR_BAD_CMD;
        }
        
        // First arg is address, rest are data bytes
        uint32_t addr = arg_vals[0].val.u;
        for (idx = 1; idx < rc; idx++) {
            msg_bfr[idx-1] = arg_vals[idx].val.u;
        }
        
        rc = i2c_write(instance_id, addr, msg_bfr, rc-1);
        printc("Write started: %ld\n", rc);
    } 
    else if (strcasecmp(argv[2], "read") == 0) {
        if (argc < 5) {
            printc("Usage: i2c test read <addr> <num-bytes>\n");
            return MOD_ERR_BAD_CMD;
        }
        
        rc = cmd_parse_args(argc-3, argv+3, "uu", arg_vals);
        if (rc < 2) {
            printc("Invalid command rc=%ld\n", rc);
            return MOD_ERR_BAD_CMD;
        }
        
        uint32_t addr = arg_vals[0].val.u;
        uint32_t num_bytes = arg_vals[1].val.u;
        
        if (num_bytes > MAX_MSG_LEN) {
            printc("Message length limited to %d\n", MAX_MSG_LEN);
            return MOD_ERR_ARG;
        }
        
        msg_len = num_bytes;
        rc = i2c_read(instance_id, addr, msg_bfr, msg_len);
        printc("Read started: %ld\n", rc);
    } 
    else if (strcasecmp(argv[2], "status") == 0) {
        rc = i2c_get_op_status(instance_id);
        enum i2c_errors err = i2c_get_error(instance_id);
        
        if (rc == MOD_ERR_OP_IN_PROG) {
            printc("Status: OPERATION IN PROGRESS\n");
        } else if (rc == 0) {
            printc("Status: SUCCESS (error code: %d)\n", err);
        } else {
            printc("Status: ERROR - rc=%ld (i2c error: %d)\n", rc, err);
        }
        return 0;
    } 
    else if (strcasecmp(argv[2], "busy") == 0) {
        rc = i2c_bus_busy(instance_id);
        if (rc < 0) {
            printc("Error checking bus: %ld\n", rc);
        } else if (rc) {
            printc("Bus is BUSY\n");
        } else {
            printc("Bus is IDLE\n");
        }
        return 0;
    } 
    else if (strcasecmp(argv[2], "msg") == 0) {
        printc("Message buffer (length %lu):\n", msg_len);
        for (idx = 0; idx < msg_len; idx++) {
            printc("  [%ld] = 0x%02x\n", idx, msg_bfr[idx]);
        }
        return 0;
    } 
    else {
        printc("Invalid operation '%s'\n", argv[2]);
        return MOD_ERR_BAD_CMD;
    }
    
    if (rc != 0 && rc != MOD_ERR_OP_IN_PROG) {
        printc("Return code: %ld\n", rc);
    }
    
    return 0;
}
