# Most Basic I2C Driver Code Reference

## Core Data Structures

```c
// 7-State Machine - The heart of the driver
enum states {
    STATE_IDLE,                     // Ready for new operation
    STATE_MSTR_WR_GEN_START,        // Write: Generating START condition
    STATE_MSTR_WR_SENDING_ADDR,     // Write: Sending address + W bit
    STATE_MSTR_WR_SENDING_DATA,     // Write: Sending data bytes
    STATE_MSTR_RD_GEN_START,        // Read: Generating START condition
    STATE_MSTR_RD_SENDING_ADDR,     // Read: Sending address + R bit
    STATE_MSTR_RD_READING_DATA,     // Read: Receiving data bytes
};

// I2C Instance State - Everything needed for one I2C bus
struct i2c_state {
    struct i2c_cfg cfg;             // Configuration (guard timeout)
    I2C_TypeDef* i2c_reg_base;      // Hardware registers (I2C3)
    int32_t guard_tmr_id;           // Timeout timer ID
    
    uint8_t* msg_bfr;               // Data buffer pointer
    uint32_t msg_len;               // Total bytes to transfer
    uint32_t msg_bytes_xferred;     // Bytes transferred so far
    
    uint16_t dest_addr;             // Slave address (7-bit)
    
    bool reserved;                  // Is bus reserved?
    enum states state;              // Current state machine state
    enum i2c_errors last_op_error;  // Last error code
};

// Error codes
enum i2c_errors {
    I2C_ERR_NONE,                   // Success
    I2C_ERR_INVALID_INSTANCE,
    I2C_ERR_BUS_BUSY,
    I2C_ERR_GUARD_TMR,              // Operation timeout
    I2C_ERR_ACK_FAIL,               // Slave didn't ACK
    I2C_ERR_BUS_ERR,                // Bus error
};
```

## Essential Pattern Implementation

### Initialization Sequence

```c
// Step 1: Get default config
struct i2c_cfg cfg;
i2c_get_def_cfg(I2C_INSTANCE_3, &cfg);

// Step 2: Initialize (called once at startup)
i2c_init(I2C_INSTANCE_3, &cfg);

// Step 3: Start (enable interrupts, get timer)
i2c_start(I2C_INSTANCE_3);
```

### Non-Blocking Operation Flow

**Critical Concept:** Operations are **NON-BLOCKING**. They start the operation and return immediately. You must poll for completion.

```c
// Reserve bus (mutual exclusion)
i2c_reserve(I2C_INSTANCE_3);

// Start write operation (returns immediately!)
uint8_t data[] = {0x2c, 0x06};
i2c_write(I2C_INSTANCE_3, 0x44, data, 2);

// Poll until complete
int32_t status;
do {
    status = i2c_get_op_status(I2C_INSTANCE_3);
} while (status == MOD_ERR_OP_IN_PROG);

// Check result
if (status == 0) {
    // Success!
} else {
    // Failed - check error code
    enum i2c_errors err = i2c_get_error(I2C_INSTANCE_3);
}

// Release bus when done
i2c_release(I2C_INSTANCE_3);
```

### Core Write State Machine

```c
static int32_t start_op(enum i2c_instance_id instance_id, uint32_t dest_addr,
                        uint8_t* msg_bfr, uint32_t msg_len,
                        enum states init_state)
{
    struct i2c_state* st = &i2c_states[instance_id];
    
    // Must be reserved and idle
    if (!st->reserved || st->state != STATE_IDLE)
        return MOD_ERR_STATE;
    
    // Start guard timer (100ms timeout)
    tmr_inst_start(st->guard_tmr_id, st->cfg.transaction_guard_time_ms);
    
    // Save operation parameters
    st->dest_addr = dest_addr;
    st->msg_bfr = msg_bfr;
    st->msg_len = msg_len;
    st->msg_bytes_xferred = 0;
    st->last_op_error = I2C_ERR_NONE;
    
    // Set state (WRITE or READ)
    st->state = init_state;
    
    // Enable peripheral
    LL_I2C_Enable(st->i2c_reg_base);
    
    // Hardware generates START condition
    LL_I2C_GenerateStartCondition(st->i2c_reg_base);
    
    // Enable interrupts - rest happens in interrupt handler!
    ENABLE_ALL_INTERRUPTS(st);
    
    return 0;  // SUCCESS: Operation STARTED (not finished!)
}
```

### Interrupt-Driven State Machine

**Critical Concept:** Each interrupt advances the state machine by ONE step.

```c
static void i2c_interrupt(enum i2c_instance_id instance_id,
                          enum interrupt_type inter_type,
                          IRQn_Type irq_type)
{
    struct i2c_state* st = &i2c_states[instance_id];
    uint16_t sr1 = st->i2c_reg_base->SR1;  // Read status
    
    if (inter_type == INTER_TYPE_EVT) {
        switch (st->state) {
        
        // ===== WRITE SEQUENCE =====
        
        case STATE_MSTR_WR_GEN_START:
            if (sr1 & LL_I2C_SR1_SB) {  // START sent?
                // Send address with W bit (bit 0 = 0)
                st->i2c_reg_base->DR = st->dest_addr << 1;
                st->state = STATE_MSTR_WR_SENDING_ADDR;
            }
            break;
            
        case STATE_MSTR_WR_SENDING_ADDR:
            if (sr1 & LL_I2C_SR1_ADDR) {  // Address ACKed?
                // Clear ADDR by reading SR2 (hardware requirement!)
                (void)st->i2c_reg_base->SR2;
                
                if (st->msg_len > 0) {
                    st->state = STATE_MSTR_WR_SENDING_DATA;
                    // Send first byte if TXE ready
                    if (sr1 & LL_I2C_SR1_TXE) {
                        st->i2c_reg_base->DR = st->msg_bfr[st->msg_bytes_xferred++];
                    }
                } else {
                    op_stop_success(st, true);  // Zero-byte write
                }
            }
            break;
            
        case STATE_MSTR_WR_SENDING_DATA:
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
            
        // ===== READ SEQUENCE =====
        
        case STATE_MSTR_RD_GEN_START:
            if (sr1 & LL_I2C_SR1_SB) {  // START sent?
                // Send address with R bit (bit 0 = 1)
                st->i2c_reg_base->DR = (st->dest_addr << 1) | 1;
                st->state = STATE_MSTR_RD_SENDING_ADDR;
            }
            break;
            
        case STATE_MSTR_RD_SENDING_ADDR:
            if (sr1 & LL_I2C_SR1_ADDR) {  // Address ACKed?
                if (st->msg_len == 1) {
                    // Single byte: NACK it immediately
                    LL_I2C_AcknowledgeNextData(st->i2c_reg_base, LL_I2C_NACK);
                } else {
                    // Multi-byte: ACK them
                    LL_I2C_AcknowledgeNextData(st->i2c_reg_base, LL_I2C_ACK);
                }
                // Clear ADDR by reading SR2
                (void)st->i2c_reg_base->SR2;
                
                if (st->msg_len == 1) {
                    // Generate STOP now for single byte
                    LL_I2C_GenerateStopCondition(st->i2c_reg_base);
                }
                st->state = STATE_MSTR_RD_READING_DATA;
            }
            break;
            
        case STATE_MSTR_RD_READING_DATA:
            if (sr1 & LL_I2C_SR1_RXNE) {  // Data received?
                // Read the byte
                st->msg_bfr[st->msg_bytes_xferred++] = st->i2c_reg_base->DR;
                
                if (st->msg_bytes_xferred >= st->msg_len) {
                    // Done!
                    op_stop_success(st, st->msg_len > 1);
                } else if (st->msg_bytes_xferred == st->msg_len - 1) {
                    // Next byte is last: NACK it
                    LL_I2C_AcknowledgeNextData(st->i2c_reg_base, LL_I2C_NACK);
                    LL_I2C_GenerateStopCondition(st->i2c_reg_base);
                }
            }
            break;
        }
    }
    else if (inter_type == INTER_TYPE_ERR) {
        // Handle errors
        enum i2c_errors i2c_error = I2C_ERR_INTR_UNEXPECT;
        
        if (sr1 & I2C_SR1_AF)
            i2c_error = I2C_ERR_ACK_FAIL;  // Slave didn't respond
        else if (sr1 & I2C_SR1_BERR)
            i2c_error = I2C_ERR_BUS_ERR;
            
        // Clear error flags
        st->i2c_reg_base->SR1 &= ~(sr1 & INTERRUPT_ERR_MASK);
        
        op_stop_fail(st, i2c_error);
    }
}
```

## Helper Functions

```c
// SUCCESS: Clean up and return to IDLE
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

// FAILURE: Clean up and return to IDLE with error
static void op_stop_fail(struct i2c_state* st, enum i2c_errors error)
{
    DISABLE_ALL_INTERRUPTS(st);
    LL_I2C_GenerateStopCondition(st->i2c_reg_base);
    
    // Cancel guard timer
    tmr_inst_start(st->guard_tmr_id, 0);
    
    LL_I2C_Disable(st->i2c_reg_base);
    
    st->last_op_error = error;
    st->state = STATE_IDLE;
}

// TIMEOUT: Called by timer module if operation takes > 100ms
static enum tmr_cb_action tmr_callback(int32_t tmr_id, uint32_t user_data)
{
    enum i2c_instance_id instance_id = (enum i2c_instance_id)user_data;
    struct i2c_state* st = &i2c_states[instance_id];
    
    op_stop_fail(st, I2C_ERR_GUARD_TMR);
    
    return TMR_CB_NONE;
}
```

## Integration Example

### In app_main.c

```c
#include "i2c.h"

void app_main(void)
{
    struct i2c_cfg i2c_cfg;
    
    // Initialize I2C
    i2c_get_def_cfg(I2C_INSTANCE_3, &i2c_cfg);
    i2c_init(I2C_INSTANCE_3, &i2c_cfg);
    i2c_start(I2C_INSTANCE_3);
    
    // ... rest of initialization ...
}
```

### Typical Usage Pattern (SHT31-D Temperature Sensor)

```c
void read_temperature(void)
{
    uint8_t tx_data[2] = {0x2c, 0x06};  // Measurement command
    uint8_t rx_data[6];                 // Temp + humidity data
    int32_t status;
    
    // 1. Reserve bus
    if (i2c_reserve(I2C_INSTANCE_3) != 0) {
        // Bus already reserved
        return;
    }
    
    // 2. Send measurement command
    i2c_write(I2C_INSTANCE_3, 0x44, tx_data, 2);
    
    // 3. Wait for write to complete
    do {
        status = i2c_get_op_status(I2C_INSTANCE_3);
    } while (status == MOD_ERR_OP_IN_PROG);
    
    if (status != 0) {
        // Write failed
        i2c_release(I2C_INSTANCE_3);
        return;
    }
    
    // 4. Read result (6 bytes)
    i2c_read(I2C_INSTANCE_3, 0x44, rx_data, 6);
    
    // 5. Wait for read to complete
    do {
        status = i2c_get_op_status(I2C_INSTANCE_3);
    } while (status == MOD_ERR_OP_IN_PROG);
    
    // 6. Release bus
    i2c_release(I2C_INSTANCE_3);
    
    if (status == 0) {
        // Success! Process rx_data
        // rx_data[0-1]: Temperature MSB/LSB
        // rx_data[2]: Temp CRC
        // rx_data[3-4]: Humidity MSB/LSB
        // rx_data[5]: Humidity CRC
    }
}
```

## Dependencies

### Required Modules

```c
// Timer module - provides timeout protection
int32_t tmr_inst_get_cb(uint32_t inst, tmr_cb_func cb, uint32_t user_data);
int32_t tmr_inst_start(int32_t tmr_id, uint32_t period_ms);

enum tmr_cb_action {
    TMR_CB_NONE,
    // ... others
};
```

### STM32 Hardware Layer (ST LL drivers)

```c
// From stm32f4xx_ll_i2c.h
void LL_I2C_Enable(I2C_TypeDef* I2Cx);
void LL_I2C_Disable(I2C_TypeDef* I2Cx);
void LL_I2C_GenerateStartCondition(I2C_TypeDef* I2Cx);
void LL_I2C_GenerateStopCondition(I2C_TypeDef* I2Cx);
void LL_I2C_AcknowledgeNextData(I2C_TypeDef* I2Cx, uint32_t TypeAcknowledge);
uint32_t LL_I2C_IsActiveFlag_BUSY(I2C_TypeDef* I2Cx);

// Status flags (in SR1 register)
#define LL_I2C_SR1_SB       // START bit sent
#define LL_I2C_SR1_ADDR     // Address sent and ACKed
#define LL_I2C_SR1_TXE      // Transmit register empty
#define LL_I2C_SR1_BTF      // Byte transfer finished
#define LL_I2C_SR1_RXNE     // Receive register not empty
#define I2C_SR1_AF          // ACK failure
#define I2C_SR1_BERR        // Bus error
```

## Key Concepts

### 🎯 Critical Understanding Points

1. **Non-Blocking Design**: `i2c_write()` and `i2c_read()` return immediately. Always poll with `i2c_get_op_status()`.

2. **Reserve/Release Pattern**: Multiple clients can share the bus. Always reserve before use, release when done.

3. **Interrupt-Driven State Machine**: Hardware generates interrupts when events occur. Each interrupt advances state by one step.

4. **Guard Timer Protection**: Every operation has a 100ms timeout. If it fires, operation failed.

5. **SR2 Read Required**: After ADDR flag, must read SR2 to clear it. This is STM32 hardware requirement!

### ⚠️ Common Pitfalls

1. **Forgetting to poll**: Starting a write/read doesn't mean it's done. You MUST poll `i2c_get_op_status()`.

2. **Not reserving bus**: Calling write/read without reserve will fail with `MOD_ERR_NOT_RESERVED`.

3. **Starting new operation while busy**: Check that state is IDLE before starting new operation.

4. **Reading wrong status register**: Must read SR1 in interrupt, read SR2 to clear ADDR flag.

### ✅ Best Practices

1. Always use reserve/release for bus sharing
2. Always poll operation status before proceeding
3. Check error codes when status != 0
4. Use guard timer to prevent infinite hangs
5. One operation at a time per bus instance

### 💡 Why This Design?

**Interrupt-driven vs Polling**: This driver uses interrupts to advance the state machine. This frees the CPU to do other work while I2C hardware handles communication. The alternative (bit-banging or busy-waiting) wastes CPU cycles.

**Non-blocking API**: Allows other code to run while I2C transaction happens. Perfect for multi-client systems where other tasks need CPU time.

**Reserve/Release**: Provides mutual exclusion without OS semaphores. Simple but effective for bare-metal systems.

---

**This pattern is portable across:**
- Different STM32 families (just change LL driver includes)
- Different I2C peripherals (I2C1, I2C2, I2C3)
- Different sensors/slaves (just change address and data)

**The core concept remains:** State machine + Interrupts + Non-blocking API = Efficient resource sharing.

---

## I2C Module State Variables

```
I2C Module State (struct i2c_state) ← Per-instance state (array of I2C_NUM_INSTANCES)

│
├── cfg (struct i2c_cfg) ← Configuration for this I2C instance
│   │
│   └── transaction_guard_time_ms (uint32_t) ← Timeout value in milliseconds
│       ├── Default: CONFIG_I2C_DFLT_TRANS_GUARD_TIME_MS (100ms)
│       └── Purpose: How long before guard timer fires and aborts operation
│
├── i2c_reg_base (I2C_TypeDef*) ← Pointer to hardware registers
│   │
│   ├── I2C3 ← For I2C_INSTANCE_3
│   ├── I2C2 ← For I2C_INSTANCE_2 (if enabled)
│   └── I2C1 ← For I2C_INSTANCE_1 (if enabled)
│       └── Purpose: Direct hardware access for interrupt handler
│
├── guard_tmr_id (int32_t) ← Timer ID for timeout protection
│   │
│   ├── Obtained from: tmr_inst_get_cb() during i2c_start()
│   ├── Started: When i2c_write() or i2c_read() begins
│   └── Stopped: When operation completes (success or failure)
│       └── Purpose: Prevent infinite hangs if slave doesn't respond
│
├── Transaction Buffer Tracking ← Non-blocking operation parameters
│   │
│   ├── msg_bfr (uint8_t*) ← Pointer to caller's data buffer
│   │   ├── Write: Points to data to send
│   │   └── Read: Points to where received data goes
│   │       └── ⚠️ CRITICAL: Caller must keep buffer valid until operation done!
│   │
│   ├── msg_len (uint32_t) ← Total bytes to transfer
│   │   ├── Write: Number of data bytes to send
│   │   └── Read: Number of data bytes to receive
│   │
│   └── msg_bytes_xferred (uint32_t) ← Progress counter
│       ├── Initialized: 0 at start_op()
│       ├── Incremented: Each time interrupt handler sends/receives byte
│       └── Purpose: Track how many bytes done vs msg_len
│
├── dest_addr (uint16_t) ← I2C slave address (7-bit)
│   │
│   ├── Examples: 0x44 (SHT31-D sensor), 0x50 (EEPROM)
│   ├── Shifted left by 1 before sending (address + R/W bit)
│   │   ├── Write: (dest_addr << 1) | 0 ← Address + Write bit
│   │   └── Read: (dest_addr << 1) | 1 ← Address + Read bit
│   └── Purpose: Which slave device we're talking to
│
├── reserved (bool) ← Mutual exclusion flag
│   │
│   ├── false ← Bus is free, anyone can use
│   ├── true ← Bus is reserved by a client
│   │
│   ├── Set by: i2c_reserve()
│   ├── Cleared by: i2c_release()
│   │
│   └── Purpose: Prevent multiple clients from conflicting
│       └── ✅ BEST PRACTICE: Reserve → Write/Read → Release sequence
│
├── state (enum states) ← Current state machine state
│   │
│   ├── STATE_IDLE (0) ← Ready for new operation
│   │   └── Transitions from: op_stop_success() or op_stop_fail()
│   │
│   ├── Write States ← Master write sequence (3 states)
│   │   ├── STATE_MSTR_WR_GEN_START (1) ← Generating START condition
│   │   │   └── Next: STATE_MSTR_WR_SENDING_ADDR when SB flag set
│   │   │
│   │   ├── STATE_MSTR_WR_SENDING_ADDR (2) ← Sending address + W bit
│   │   │   └── Next: STATE_MSTR_WR_SENDING_DATA when ADDR flag set
│   │   │
│   │   └── STATE_MSTR_WR_SENDING_DATA (3) ← Sending data bytes
│   │       └── Next: STATE_IDLE when all bytes sent and BTF set
│   │
│   └── Read States ← Master read sequence (3 states)
│       ├── STATE_MSTR_RD_GEN_START (4) ← Generating START condition
│       │   └── Next: STATE_MSTR_RD_SENDING_ADDR when SB flag set
│       │
│       ├── STATE_MSTR_RD_SENDING_ADDR (5) ← Sending address + R bit
│       │   └── Next: STATE_MSTR_RD_READING_DATA when ADDR flag set
│       │
│       └── STATE_MSTR_RD_READING_DATA (6) ← Receiving data bytes
│           └── Next: STATE_IDLE when all bytes received
│
└── last_op_error (enum i2c_errors) ← Last error code
    │
    ├── I2C_ERR_NONE (0) ← Operation succeeded
    ├── I2C_ERR_INVALID_INSTANCE ← Bad instance_id parameter
    ├── I2C_ERR_BUS_BUSY ← I2C bus stuck (SCL/SDA held low)
    ├── I2C_ERR_GUARD_TMR ← Operation timed out (>100ms)
    ├── I2C_ERR_ACK_FAIL ← Slave didn't ACK (most common error!)
    │   └── Causes: Wrong address, slave not powered, bad wiring
    ├── I2C_ERR_BUS_ERR ← Bus error (arbitration lost, etc.)
    └── I2C_ERR_INTR_UNEXPECT ← Unexpected interrupt
        │
        ├── Set by: op_stop_fail() when error occurs
        ├── Read by: i2c_get_error() to get detailed error info
        └── Purpose: Know WHY operation failed after i2c_get_op_status() != 0

Static Array ← Private storage for all I2C instances

static struct i2c_state i2c_states[I2C_NUM_INSTANCES];
    │
    ├── i2c_states[0] ← I2C_INSTANCE_1 (if CONFIG_I2C_HAVE_INSTANCE_1)
    ├── i2c_states[1] ← I2C_INSTANCE_2 (if CONFIG_I2C_HAVE_INSTANCE_2)
    └── i2c_states[2] ← I2C_INSTANCE_3 (if CONFIG_I2C_HAVE_INSTANCE_3)
        │
        └── Purpose: One state struct per hardware I2C peripheral

⚠️ CRITICAL EXECUTION FLOW:

1. start_op() fills state, enables interrupts, returns immediately
2. Hardware generates events → interrupts fire
3. i2c_interrupt() advances state machine by ONE step per interrupt
4. Caller polls i2c_get_op_status() until state == STATE_IDLE
5. Check last_op_error to see if succeeded or failed
```

---

## How to Request Visual Aids for Complex Concepts

When learning complex systems with multiple interacting components, use these prompt patterns to get effective visual diagrams:

### 📊 For State Machines

**Ask for:**
```
"Create a state diagram showing [module name] state machine with:
- All states and transitions
- Trigger conditions for each transition
- What happens in each state
- Entry/exit actions"
```

**Example:**
```
"Create a state diagram for the i2c_interrupt() state machine showing 
all 7 states, hardware flags that trigger transitions, and what 
register operations happen in each state"
```

### 🔄 For Interacting Components

**Ask for:**
```
"Show how [component A] and [component B] interact with:
- Separate boxes for each component
- Arrows showing data/control flow
- Labels on arrows explaining what's passed
- Timing or sequence if relevant"
```

**Example:**
```
"Show how i2c_run_auto_test() and i2c_interrupt() state machines 
interact - which functions trigger which states, and how status 
is communicated back"
```

### ⏱️ For Timing/Sequences

**Ask for:**
```
"Create a sequence diagram showing [operation] from start to finish:
- All participants (functions, hardware, modules)
- Order of function calls
- What triggers each step
- Return values and their meaning"
```

**Example:**
```
"Create a sequence diagram for a complete I2C write+read operation 
showing app_main(), test function, API calls, interrupt handler, 
and hardware events in time order"
```

### 🗺️ For Module Architecture

**Ask for:**
```
"Show the architecture of [module] with:
- Layers (application, driver, hardware)
- Public vs private interfaces
- Dependencies between components
- Data flow direction"
```

**Example:**
```
"Show I2C module architecture from app_main down to hardware,
including which functions are public API vs private helpers"
```

### 📈 For Call Graphs

**Ask for:**
```
"Create a call graph for [operation] showing:
- Which functions call which
- Color-code by purpose (init, operation, error handling)
- Show return paths
- Highlight critical paths"
```

**Example:**
```
"Create a call graph showing all functions involved in i2c_write()
from the application call down to hardware register writes"
```

### 🎯 Power Prompt Template

When you want comprehensive understanding:

```
"I need to understand [concept/module]. Please provide:

1. A state diagram showing [states/phases]
2. A diagram of how [component A] and [component B] interact
3. A sequence diagram showing [operation] timing
4. Highlight what's synchronous vs asynchronous
5. Show where data is stored between steps

Focus on the 'aha!' moments - what makes this pattern work."
```

### 💡 Tips for Better Visual Aids

**Be specific about:**
- ✅ **Level of detail**: "High-level overview" vs "Show register-level details"
- ✅ **What to include**: "Show only success path" vs "Include error handling"
- ✅ **Format preference**: "State machine" vs "Flowchart" vs "Sequence diagram"
- ✅ **What's confusing**: "I don't understand how polling detects completion"

**Examples of effective requests:**

1. **"Show me two state machines side-by-side: the high-level test sequence and the low-level protocol handler, with arrows showing how they communicate"**
   - ✅ Clear: Two components, their relationship, communication flow

2. **"Create a timing diagram showing when i2c_write() returns vs when the actual I2C transaction completes"**
   - ✅ Clear: Addresses asynchronous behavior confusion

3. **"Diagram the lifecycle of msg_bfr pointer: where it's stored, who uses it, when it can be freed"**
   - ✅ Clear: Focuses on a specific data flow concern

4. **"Show all the hardware flags (SB, ADDR, TXE, BTF) on a timeline with I2C bus signals above them"**
   - ✅ Clear: Correlates software events with hardware behavior

### 🎓 Learning Pattern

For any complex module:
1. **First**: Get overall architecture diagram
2. **Second**: Get state machine(s) diagram
3. **Third**: Get sequence diagram for one complete operation
4. **Fourth**: Get call graph showing function relationships
5. **Fifth**: Deep dive into specific confusing parts with targeted diagrams

**This multi-view approach builds complete mental models!**

---

## 🆘 When You're Overwhelmed - Start Here

### If you don't know where to begin, use these **emergency prompts**:

#### 🎯 Level 1: "I'm Lost" Prompt
```
"I'm trying to understand [module/file name] but I'm overwhelmed. 
Can you:
1. Explain in 3 sentences what this module does
2. Show me the simplest example of using it
3. Point out the 3 most important functions to understand first"
```

**Example:**
```
"I'm trying to understand i2c.c but I'm overwhelmed. 
Can you explain what it does, show the simplest usage example, 
and tell me which 3 functions are most important?"
```

#### 🎯 Level 2: "Walk Me Through One Thing" Prompt
```
"Walk me through ONE complete example of [specific operation] from 
start to finish. Show me:
- Which function I call first
- What happens step by step
- What I check to know it's done
- Just the success path, skip error handling for now"
```

**Example:**
```
"Walk me through ONE complete I2C write operation from start to finish. 
Show me which functions to call, what happens, and how I know it's done. 
Just the success path."
```

#### 🎯 Level 3: "Show Me the Pattern" Prompt
```
"What's the basic pattern for using [module]? 
Show me the typical sequence like:
Step 1: [...]
Step 2: [...]
Step 3: [...]

Use a real example with actual function names and values."
```

**Example:**
```
"What's the basic pattern for using the I2C module? 
Show me the typical reserve->write->read->release sequence 
with actual function names and example values for a sensor."
```

#### 🎯 Level 4: "Compare to Something I Know" Prompt
```
"Explain [new concept] by comparing it to [familiar concept].
What's similar? What's different?
Use an analogy if it helps."
```

**Example:**
```
"Explain the I2C non-blocking API by comparing it to a restaurant order. 
How is i2c_write() like placing an order? What's the 'polling' equivalent?"
```

#### 🎯 Level 5: "What Should I Ignore?" Prompt
```
"I'm trying to learn [concept] but the code has a lot of stuff. 
What can I safely ignore for now? What's the absolute minimum 
I need to understand to use this?"
```

**Example:**
```
"The I2C driver has console commands, error handling, logging, etc. 
What can I ignore to start? What's the minimum to understand 
for basic usage?"
```

### 📋 Simple Triage Process

When facing unfamiliar code, follow this order:

1. **Ask: "What does this DO?"** (High-level purpose)
   - Don't dive into HOW yet
   - Get a 1-sentence answer first

2. **Ask: "Show me ONE working example"** (Concrete usage)
   - See it work before understanding internals
   - Build from working code

3. **Ask: "What are the 3 key functions?"** (Critical path)
   - Not all functions are equal
   - Focus on the essential ones first

4. **Ask: "What's the typical sequence?"** (Usage pattern)
   - Initialize → Use → Cleanup pattern
   - Most modules follow this

5. **Ask: "What's different/special about this?"** (Key concepts)
   - What makes this non-obvious?
   - What's the "trick" that makes it work?

### 🔥 Panic Recovery Prompts

#### When code is too complex:
```
"Simplify [function/module] to the bare minimum. 
Remove error handling, logging, and edge cases. 
Show me the 10-line version that captures the core idea."
```

#### When you can't trace execution:
```
"Trace one execution path through [module] starting from [entry point].
Use an example like [specific values].
Show me each function call in order with what it does."
```

#### When there's too much state:
```
"What variables actually matter for [operation]?
Which ones can I ignore for now?
Draw me a simple diagram of just the critical data flow."
```

#### When you can't see the big picture:
```
"Draw me a simple block diagram with 3-5 boxes showing 
the main components and how they connect. 
Label the arrows with what data/control flows between them."
```

### 💡 Pro Tips for Getting Unstuck

1. **Start with usage, not implementation**
   - "Show me how to USE this" before "Show me how it WORKS"

2. **One thing at a time**
   - Don't ask about everything at once
   - Master one operation before the next

3. **Real examples over theory**
   - "Show me actual code" beats "Explain the concept"
   - Use concrete numbers (0x44, 2 bytes) not abstract (address, length)

4. **Success path first**
   - Ignore error handling initially
   - Add complexity after understanding the basics

5. **It's okay to skip details**
   - You don't need to understand every line
   - Focus on the interface, not the implementation

### 🎓 Remember: Learning is Iterative

**First pass:** Get it working (copy/paste example)  
**Second pass:** Understand the pattern (why this sequence?)  
**Third pass:** Understand the internals (how does it work?)  
**Fourth pass:** Understand edge cases (what can go wrong?)

**You don't need to do all four passes at once!** Start with pass 1, use it for a while, then come back for pass 2 when you're ready.

