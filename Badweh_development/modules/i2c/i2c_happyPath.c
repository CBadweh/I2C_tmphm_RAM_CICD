/*
 * HAPPY PATH I2C DRIVER - Maximum Simplicity Version
 * 
 * ALL ABSTRACTIONS REMOVED - Every step is explicit and visible:
 * - No helper functions (start_op, op_stop_success)
 * - No error handling
 * - All code inlined for direct, linear reading
 * 
 * LEARNING PATH:
 * 1. Reserve/Release (resource sharing)
 * 2. Write/Read APIs (see entire flow in one place)
 * 3. State machine (7 states)
 * 4. Interrupt handler (drives state transitions, see cleanup inline)
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include "stm32f4xx.h"
#include "stm32f4xx_ll_i2c.h"

#include "tmr.h"
#include "console.h"
#include "i2c.h"

////////////////////////////////////////////////////////////////////////////////
// MACROS - Interrupt control
////////////////////////////////////////////////////////////////////////////////

#define INTERRUPT_ENABLE_MASK (LL_I2C_CR2_ITEVTEN | LL_I2C_CR2_ITBUFEN | \
                               LL_I2C_CR2_ITERREN)
#define DISABLE_ALL_INTERRUPTS(st) st->i2c_reg_base->CR2 &= ~INTERRUPT_ENABLE_MASK
#define ENABLE_ALL_INTERRUPTS(st) st->i2c_reg_base->CR2 |= INTERRUPT_ENABLE_MASK

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
};

////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

static void i2c_interrupt(enum i2c_instance_id instance_id,
                          enum interrupt_type inter_type);

////////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
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
    struct i2c_state* st = &i2c_states[instance_id];
    
    memset(st, 0, sizeof(*st));
    st->cfg = *cfg;
    st->i2c_reg_base = I2C3;  // Hardcoded for I2C3
    
    return 0;
}

/*
 * Start I2C instance - Enable interrupts
 */
int32_t i2c_start(enum i2c_instance_id instance_id)
{
    struct i2c_state* st = &i2c_states[instance_id];

    // Get a timer (for structure, not used in happy path)
    st->guard_tmr_id = tmr_inst_get_cb(0, NULL, 0);

    // Disable peripheral and interrupts initially
    LL_I2C_Disable(st->i2c_reg_base);
    DISABLE_ALL_INTERRUPTS(st);

    // Enable I2C3 interrupts in NVIC
    NVIC_SetPriority(I2C3_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(I2C3_EV_IRQn);

    return 0;
}

/*
 * Run function - Does nothing
 */
int32_t i2c_run(enum i2c_instance_id instance_id)
{
    return 0;
}

/*
 * RESERVE BUS - Get exclusive access
 */
int32_t i2c_reserve(enum i2c_instance_id instance_id)
{
    i2c_states[instance_id].reserved = true;
}

/*
 * RELEASE BUS - Free for others to use
 */
int32_t i2c_release(enum i2c_instance_id instance_id)
{
    i2c_states[instance_id].reserved = false;
}

/*
 * WRITE - Start write operation (NO HELPER FUNCTIONS - all inline)
 * 
 * You can read the ENTIRE write setup in one place:
 * 1. Save parameters (address, buffer, length)
 * 2. Set state to WR_GEN_START
 * 3. Enable peripheral
 * 4. Generate START
 * 5. Enable interrupts
 * 
 * Then the interrupt handler takes over (see i2c_interrupt below)
 */
int32_t i2c_write(enum i2c_instance_id instance_id, uint32_t dest_addr,
               uint8_t* msg_bfr, uint32_t msg_len)
{
    struct i2c_state* st = &i2c_states[instance_id];
    
    // Save operation parameters
    st->dest_addr = dest_addr;
    st->msg_bfr = msg_bfr;
    st->msg_len = msg_len;
    st->msg_bytes_xferred = 0;

    // Set initial state
    st->state = STATE_MSTR_WR_GEN_START;

    // Enable I2C peripheral
    LL_I2C_Enable(st->i2c_reg_base);
    
    // Generate START condition
    LL_I2C_GenerateStartCondition(st->i2c_reg_base);

    // Enable interrupts (rest happens in interrupt handler!)
    ENABLE_ALL_INTERRUPTS(st);
}

/*
 * READ - Start read operation (NO HELPER FUNCTIONS - all inline)
 * 
 * You can read the ENTIRE read setup in one place:
 * 1. Save parameters (address, buffer, length)
 * 2. Set state to RD_GEN_START
 * 3. Enable peripheral
 * 4. Generate START
 * 5. Enable interrupts
 * 
 * Then the interrupt handler takes over (see i2c_interrupt below)
 */
int32_t i2c_read(enum i2c_instance_id instance_id, uint32_t dest_addr,
              uint8_t* msg_bfr, uint32_t msg_len)
{
    struct i2c_state* st = &i2c_states[instance_id];
    
    // Save operation parameters
    st->dest_addr = dest_addr;
    st->msg_bfr = msg_bfr;
    st->msg_len = msg_len;
    st->msg_bytes_xferred = 0;

    // Set initial state
    st->state = STATE_MSTR_RD_GEN_START;

    // Enable I2C peripheral
    LL_I2C_Enable(st->i2c_reg_base);
    
    // Generate START condition
    LL_I2C_GenerateStartCondition(st->i2c_reg_base);

    // Enable interrupts (rest happens in interrupt handler!)
    ENABLE_ALL_INTERRUPTS(st);
}

/*
 * GET STATUS - Poll this to check if operation complete
 * Returns: 0 = complete, MOD_ERR_OP_IN_PROG = still working
 */
int32_t i2c_get_op_status(enum i2c_instance_id instance_id)
{
    struct i2c_state* st = &i2c_states[instance_id];
    
    if (st->state == STATE_IDLE)
        return 0;  // Complete!
    else
        return MOD_ERR_OP_IN_PROG;  // Still working
}

////////////////////////////////////////////////////////////////////////////////
// INTERRUPT HANDLERS
////////////////////////////////////////////////////////////////////////////////

void I2C3_EV_IRQHandler(void)
{
    i2c_interrupt(I2C_INSTANCE_3, INTER_TYPE_EVT);
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
                    st->state = STATE_IDLE;
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
                    st->state = STATE_IDLE;
                    
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
}

/*
 * AUTOMATED TEST - Run button-triggered I2C test sequence
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
            i2c_reserve(instance_id);
            test_state = 1;
            return 0;  // Continue

        case 1:  // Write command to sensor
            msg_bfr[0] = 0x2c;  // High repeatability measurement
            msg_bfr[1] = 0x06;
            i2c_write(instance_id, 0x44, msg_bfr, 2);
            test_state = 2;
            return 0;  // Continue

        case 2:  // Wait for write to complete
            rc = i2c_get_op_status(instance_id);
            if (rc == MOD_ERR_OP_IN_PROG)
                return 0;  // Still busy
            test_state = 3;
            return 0;  // Continue

        case 3:  // Read temperature/humidity data
            msg_len = 6;  // temp(2) + CRC + hum(2) + CRC
            i2c_read(instance_id, 0x44, msg_bfr, msg_len);
            test_state = 4;
            return 0;  // Continue

        case 4:  // Wait for read to complete
            rc = i2c_get_op_status(instance_id);
            if (rc == MOD_ERR_OP_IN_PROG)
                return 0;  // Still busy
            test_state = 5;
            return 0;  // Continue

        case 5:  // Release the bus
            i2c_release(instance_id);
            test_state = 0;  // Reset for next test
            return 1;  // Test complete

        default:
            test_state = 0;
            return 1;
    }
}
