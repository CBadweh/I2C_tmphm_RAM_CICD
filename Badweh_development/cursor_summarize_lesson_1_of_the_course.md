# Summarize lesson 1 of the course
_Exported on 10/23/2025 at 20:55:57 EDT from Cursor (1.7.54)_

---

**User**

Summarize @I2C Lesson 1. Introduction to the Course.txt compredhensively

---

**Cursor**

## Summary of I2C Lesson 1: Introduction to the Course

**Course Overview:**
- Title: "I2C Theory and Practice on Bare Metal STM32" by Gene Schrader
- Duration: 4 videos, ~2.25 hours total

**Course Objectives:**
1. Learn I2C interface theory and operation
2. Design an I2C driver at register level with interrupts for STM32 MCU
3. Achieve production-quality designs and code

**Course Structure (3 Parts):**

1. **I2C Theory of Operation** - Fundamentals of the protocol

2. **I2C Driver Module (Bare Metal)**
   - Requirements for minimum viable product
   - State machine-based driver design
   - Implementation code
   - Console demo with logic analyzer

3. **Higher-Level Module (SHT31 Sensor)**
   - Temperature/humidity sensor from Adafruit
   - State machine-based design
   - Implementation and demo

**Prerequisites:**
- MCU and embedded programming knowledge (or completion of prerequisite YouTube course on "super loop and module pattern")
- Minimal STM32 Cube experience needed (just pin assignment)
- C programming proficiency
- Basic electronics (understanding high/low signals)

**Hardware:**
- Nucleo F401RE board (STM32F401RE MCU)
- Adafruit SHT31-D temperature/humidity sensor
- Low-cost logic analyzer (compatible with Saleae Logic 2 software)
- Uses I2C3 interface

**Software (all free):**
- STM32CubeIDE
- Saleae Logic 2
- Infrastructure software from GitHub repo

**Important Notes:**
- STM32 MCUs have at least two different I2C hardware implementations—this driver won't work on all STM32 variants
- Code reuses module API from base course
- Developed on Windows 10, but tools support Linux/Mac
- GitHub repos available for source code and course materials

---

**User**

Summarize @I2C Lesson 2. I2C Introduction and Theory of Operation.txt  compredhensively

---

**Cursor**

## Summary of I2C Lesson 2: I2C Introduction and Theory of Operation

**Course Topics Covered:**
1. Introduction (advantages/disadvantages)
2. Terminology and basic concepts
3. Open drain signals
4. I2C circuit connections
5. Step-by-step bus operation details
6. Additional I2C features and procedures

---

## **Introduction to I2C**

**What is I2C:**
- Stands for Inter-Integrated Circuit (pronounced "I2C" or "I squared C")
- Serial communication method for MCUs to communicate with sensors, memory, I/O extenders, displays
- Typically local to a board or closely connected boards (≤1 meter)

**Advantages vs SPI/Async Serial:**
- Only 2 signals (plus ground) to communicate with multiple devices
- Built-in error detection via ACK signals
- Automatic message framing

**Disadvantages:**
- **Slower speed** (biggest issue) due to open drain signals
- Requires pull-up resistors (minor cost/board space)
- Possible address conflicts

---

## **Terminology and Concepts**

**Transaction:** Transfer of information on I2C bus between devices; can consist of one or more read/write operations with distinct start/end markers.

**Addresses:**
- Each device needs a unique address (normally 7-bit, 10-bit also possible)
- First byte in transaction contains target device address
- Selected device participates; others stay silent

**Master and Slave:**
- **Master:** Initiates transaction (typically MCU)
- **Slave:** Reacts to transaction (typically sensors, external devices)
- Master sends address to select particular slave
- Single master most common (multi-master possible but complex)
- Master has no address in a transaction

---

## **Open Drain Signals**

**How It Works:**
- Signal defaults to HIGH via pull-up resistor (3.3V or 5V)
- Any device can pull signal LOW (overpowers pull-up resistor)
- Devices can both control AND read the signal level
- Enables signal sharing between multiple devices
- I2C protocol provides organized rules for when devices control vs read signals

---

## **I2C Circuit Example**

**Two Signals Required:**
- **SDA:** Serial Data (transmit/receive)
- **SCL:** Serial Clock

**Typical Setup:**
- Master = MCU (only one)
- Slaves = sensors, A/D converters, memory, I/O expanders
- Each slave must have unique address on its bus

**Address Conflict Solution:**
Example: If temperature sensor only supports addresses 0x44 or 0x45, but you need 3 sensors, add a second I2C bus. Same addresses work on different buses.

---

## **Bus Operations (Step-by-Step)**

1. **Acquire Bus:** Both SDA and SCL must be HIGH (idle)
2. **Start Condition:** Master signals all slaves to wake up and listen
3. **Send Address + R/W Bit:** 
   - Master sends 7-bit slave address + 1 read/write bit
   - Matching slave sends ACK (LOW signal)
   - No match = NACK (absence of ACK) → transaction stops
4. **Data Transfer:**
   - **Write:** Master sends bytes, slave ACKs each byte
   - **Read:** Slave sends bytes, master ACKs each byte (NACKs last byte to end)
   - Transfer repeats for each byte
5. **Stop Condition:** Master signals transaction is over, bus returns to idle

**Signal Details:**
- **Clock (SCL):** Rising edge = sample the data bit
- **Data (SDA):** Changes after falling edge of clock
- **Start Condition:** SDA falling edge while SCL is HIGH
- **Stop Condition:** SDA rising edge while SCL is HIGH
- Master controls clock (almost always); slave can control SDA during ACK/NACK

---

## **Additional I2C Features**

**Multi-Master:**
- Requires bus arbitration (check if busy before use)
- Hardware detects collisions when both masters start simultaneously
- One master backs off; the other continues

**Bus Speeds:**
- Standard Mode: up to 100 kbps
- Fast Mode: up to 400 kbps  
- Even faster modes exist

**Clock Stretching:**
- Slave can pull SCL LOW to request extra processing time
- Master detects and waits until SCL goes HIGH again
- Master can also slow clock as needed
- Mostly transparent hardware feature

**Combined Format (Repeated Start):**
- Multiple operations in single transaction (typically write then read)
- After first operation, generate another START instead of STOP
- Creates single atomic operation (no intervening transactions)
- Example: Write memory address, then read value from that address

**10-Bit Addresses:**
- Extension of 7-bit addresses (not commonly used)
- Can coexist with 7-bit devices on same bus
- Uses reserved 7-bit pattern (11110xx) to signal 10-bit addressing
- Requires two address bytes

**General Call Address (Address 0x00):**
- Reserved for broadcasting to all slaves
- Standard defines procedures like soft reset
- Master cannot confirm if slaves received/executed command

---

## **Key Takeaways**

- I2C requires only 2 MCU pins for multiple devices (major advantage)
- Slower than SPI due to open drain (major disadvantage)
- Master-slave architecture with address-based device selection
- ACK/NACK provides positive confirmation of communication
- Open drain allows multiple devices to share signals with organized protocol rules
- I2C spec defines signal patterns, but device-specific data content varies by manufacturer

---

**User**

Summarize @I2C Lesson 3. I2C Module Requirements and Design.txt  compredhensively

---

**Cursor**

## Summary of I2C Lesson 3: I2C Module Requirements and Design

**Lesson Overview:**
- Part 1 of 2-part series on I2C driver implementation
- Focuses on requirements and design (implementation/demo in Part 2)
- Specific to STM32 MCU but kept generic where possible
- Note: Different STM32 MCUs have different I2C hardware implementations

---

## **Minimum Viable Product Requirements**

**I2C-Specific Requirements:**
1. **Master operation only** - Single master (the MCU)
2. **Simple operations** - Read-only and write-only (no combined format like write+read in single transaction)
3. **7-bit addresses** - 10-bit addresses rarely used

**Non-Functional Requirements:**
4. **Shared bus usage** - Support multiple user modules sharing one I2C bus
5. **Super loop compatibility** - Support standard core API for super loop systems
6. **Non-blocking APIs** - Critical requirement
7. **Test/Debug capabilities:**
   - Console command-line interface for I2C transactions
   - Performance measurements for long runs

---

## **Major Design Tasks**

The design process is **iterative**, not strictly sequential:

1. **Understand theory of operation** - Don't need to be expert initially; learn while coding
2. **Read documentation** - Data sheet/reference manual repeatedly; understanding improves each pass
3. **High-level design decisions** - API design, DMA usage, etc. (document decisions and rationale)
4. **Actual design and coding** - State machine design, timers, coding with multiple iterations/refinements
5. **Iterative testing** - Get testable code early (e.g., start condition, interrupts)
6. **Reference other implementations** - HAL driver, Linux driver (separate boilerplate from core algorithms)

---

## **High-Level Design Decisions**

**1. Register Access Approach:**
- Use **STM32 Low Level (LL) library** for register manipulation
- Rationale: LL does the menial grunt work; register-level access useful for learning but not long-term
- LL provides one-line functions/macros for register operations
- Portability: Create wrappers if/when porting to non-STM32 parts

**2. Data Transfer Method:**
- **Interrupt-based** (not DMA, at least initially)
- DMA can be added later if performance needed
- Learning from interrupt-based design helps with future DMA implementation

---

**User**

Summarize @I2C Lesson 4. I2C Module Implementation and Demo.txt  compredhensively. include the related code

The source code is in @i2c.c @app_main.c 

don't make things up

---

**Cursor**

## Summary of I2C Lesson 4: I2C Module Implementation and Demo

**Lesson Overview:**
Part 2 of the I2C driver design/implementation series, focusing on actual code implementation and demonstration with a logic analyzer.

---

## **Topics Covered**
1. IDE configuration for I2C and code generation
2. Overall software structure
3. Module code highlights
4. IDE-generated initialization code
5. Integration into module startup and super loop
6. Demo with logic analyzer (multiple use cases)

---

## **IDE Configuration**

**I2C3 Setup:**
- Used I2C3 instance (MCU has 3 I2C interfaces)
- SDA configured on pin PB4
- Default bit rate: 100 kbps (standard mode)
- **Low Level (LL) library selected** (not HAL) in Project Manager → Advanced Settings

**Code Generation:**
- IDE generates initialization code in `main.c`
- Configures GPIO pins and I2C peripheral registers

---

## **Software Structure Overview**

**Layered Architecture (bottom to top):**

**System Code & Libraries (Gray boxes):**
- Power-up code & interrupt vector table
- Standard C libraries (libc, libm) - uses newlib-nano (lightweight)
- C runtime code (compiler-specific)

**Hardware Access Libraries (Light brown):**
- **CMSIS library** - CPU and core peripherals (interrupt controller, FPU)
- **LL library** - Non-core peripherals (UARTs, I2C)

**IDE-Provided Startup Code (Light green):**
- Generated by IDE based on configured peripherals
- Initializes hardware peripherals

**Module Startup & Super Loop (Bright blue):**
- Initializes all modules through 2-stage init process
- Runs super loop invoking module APIs

**Hardware-Dependent Modules (Bluish gray):**
- Use CMSIS/LL APIs
- **I2C module** (highlighted) - provides hardware abstraction
- If ported to different MCU, these modules need modification

**Hardware-Independent Modules (Top level):**
- **tmphm module** (temp/humidity) - uses I2C module for abstraction
- Should work on different MCUs without changes

---

## **I2C Module Code Highlights**

### **State Enumeration**

```106:114:tmphm/modules/i2c/i2c.c
enum states {
    STATE_IDLE,
    STATE_MSTR_WR_GEN_START,
    STATE_MSTR_WR_SENDING_ADDR,
    STATE_MSTR_WR_SENDING_DATA,
    STATE_MSTR_RD_GEN_START,
    STATE_MSTR_RD_SENDING_ADDR,
    STATE_MSTR_RD_READING_DATA,
};
```

### **Per-Instance State Structure**

```121:137:tmphm/modules/i2c/i2c.c
// Per-instance i2c state information.
struct i2c_state {
    struct i2c_cfg cfg;
    I2C_TypeDef* i2c_reg_base;
    int32_t guard_tmr_id;

    uint8_t* msg_bfr;
    uint32_t msg_len;
    uint32_t msg_bytes_xferred;

    uint16_t dest_addr;

    bool reserved;
    enum states state;
    enum i2c_errors last_op_error;
    enum states last_op_error_state;
};
```

**Key Data Structure:**
- Configuration parameters
- Pointer to peripheral registers
- Message buffer info (pointer, length, bytes transferred)
- Reservation flag (simple boolean)
- Current state and error tracking

### **Simple API Example: Reserve**

```390:402:tmphm/modules/i2c/i2c.c
int32_t i2c_reserve(enum i2c_instance_id instance_id)
{
    if (instance_id >= I2C_NUM_INSTANCES ||
        i2c_states[instance_id].i2c_reg_base == NULL)
        return MOD_ERR_BAD_INSTANCE;
    if (i2c_states[instance_id].reserved) {
        INC_SAT_U16(cnts_u16[CNT_RESERVE_FAIL]);
        return MOD_ERR_RESOURCE;
    } else {
        i2c_states[instance_id].reserved = true;
    }
    return 0;
}
```

**Reservation System:**
- Very simple (just sets boolean flag)
- Honor-based system
- Discussed weaknesses in prompts (see below)

### **Interrupt Handlers**

```548:560:tmphm/modules/i2c/i2c.c
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
```

**Interrupt Handling:**
- Overrides weak default implementations in vector table
- Separate handlers for event (EV) and error (ER) interrupts
- All call common `i2c_interrupt()` function

### **Interrupt Handler - State Machine Implementation**

**Informal State Machine:**
- Uses switch statements (not formal table-driven approach)
- Switch on current state, handle expected events
- Track which status bits were handled

**Example: Write Generating Start Condition**

```663:672:tmphm/modules/i2c/i2c.c
        switch (st->state) {
            case STATE_MSTR_WR_GEN_START:
                if (sr1 & LL_I2C_SR1_SB) {
                    // SB is cleared by reading SR1 followed by writing address
                    // to DR.

                    st->i2c_reg_base->DR = st->dest_addr << 1;
                    st->state = STATE_MSTR_WR_SENDING_ADDR;
                }
                sr1_handled_mask = LL_I2C_SR1_SB;
```

**sr1_handled_mask Technique:**
- Tracks which status bits have been considered
- Later checks for unexpected bits

```752:755:tmphm/modules/i2c/i2c.c
        // Check for unexpected events.
        sr1 &= ~sr1_handled_mask;
        if (sr1 & INTERRUPT_EVT_MASK)
            op_stop_fail(st, I2C_ERR_INTR_UNEXPECT, CNT_INTR_UNEXPECT);
```

**Error Handling:**
- Simple approach: end transaction on unexpected events
- Generate stop condition, disable hardware, return to IDLE

---

## **IDE-Generated Initialization Code**

Located in `Core/Src/main.c` (not shown in provided files, but described in lesson):

**Two Main Tasks:**
1. **GPIO Pin Setup:**
   - Enable clock for GPIO ports
   - Fill data structures for SDA and SCL pins
   - Call LL function to write to registers

2. **I2C Peripheral Setup:**
   - Enable peripheral clock
   - Configure settings (e.g., clock stretching)
   - Fill data structure with configuration (bit rate, etc.)
   - Call LL function to write to registers

**Note:** "Not a lot of code" - relatively straightforward

---

## **Module Integration in app_main.c**

### **Configuration Structure**

```172:172:tmphm/app1/app_main.c
    struct i2c_cfg i2c_cfg;
```

### **Initialization Phase**

```261:271:tmphm/app1/app_main.c
    result = i2c_get_def_cfg(I2C_INSTANCE_3, &i2c_cfg);
    if (result < 0) {
        log_error("i2c_get_def_cfg error %d\n", result);
        INC_SAT_U16(cnts_u16[CNT_INIT_ERR]);
    } else {
        result = i2c_init(I2C_INSTANCE_3, &i2c_cfg);
        if (result < 0) {
            log_error("i2c_init error %d\n", result);
            INC_SAT_U16(cnts_u16[CNT_INIT_ERR]);
        }
    }
```

### **Start Phase**

```334:338:tmphm/app1/app_main.c
    result = i2c_start(I2C_INSTANCE_3);
    if (result < 0) {
        log_error("i2c_start 3 error %d\n", result);
        INC_SAT_U16(cnts_u16[CNT_START_ERR]);
    }
```

### **Super Loop**

```360:379:tmphm/app1/app_main.c
    while (1)
    {
        stat_dur_restart(&stat_loop_dur);

        result = console_run();
        if (result < 0)
            INC_SAT_U16(cnts_u16[CNT_RUN_ERR]);

//        result = gps_run();
//        if (result < 0)
//            INC_SAT_U16(cnts_u16[CNT_RUN_ERR]);

        result = tmr_run();
        if (result < 0)
            INC_SAT_U16(cnts_u16[CNT_RUN_ERR]);

        result = tmphm_run(TMPHM_INSTANCE_1);
        if (result < 0)
            INC_SAT_U16(cnts_u16[CNT_RUN_ERR]);
    }
```

**Note:** I2C module has **no run function** - operates entirely via interrupts

---

## **Demo with Logic Analyzer**

**Setup:**
- Saleae Logic 2 software (top of screen)
- MCU console (bottom of screen)
- Sampling at 2 mega samples/second
- **Trigger:** Falling edge on SDA + HIGH on SCL = START condition
- I2C analyzer configured (identifies SDA/SCL channels)

### **Console Test Commands**

```1008:1018:tmphm/modules/i2c/i2c.c
    // Handle help case.
    if (argc == 2) {
        printc("Test operations and param(s) are as follows:\n"
               "  Reserve I2C, usage: i2c test reserve <instance-id>\n"
               "  Release I2C, usage: i2c test reserve <instance-id>\n");
        printc("  Start write, usage: i2c test write <instance-id> <addr> [<bytes> ...]\n"
               "  Start read, usage: i2c test read <instance-id> <addr> <num-bytes>\n"
               "  Get op status/error, usage: i2c test status <instance-id>\n");
        printc("  Bus busy, usage: i2c test busy <instance-id>\n"
               "  Print msg buffer, usage: i2c test msg <instance-id>\n");
        return 0;
    }
```

### **Demo 1: Basic Write**

**Command:** `i2c test write 0 44 <byte1> <byte2>`
- Temperature sensor at address 0x44
- Sends measurement start command (2 data bytes)
- **First attempt:** Failed - forgot to reserve!
- **Second attempt:** `i2c test reserve 0` → Success

**Logic Analyzer Shows:**
- Green dot = START condition
- Address + R/W bit → ACKed
- 2 data bytes → ACKed
- Orange dot = STOP condition

### **Demo 2: Basic Read**

**Command:** `i2c test read 0 44 6`
- Reads 6 bytes from sensor
- Same structure as write
- **Key difference:** Last byte was NACKed (as per I2C read protocol)
- Rising clock edges marked (when SDA is sampled)

### **Demo 3: Error Case - Non-Existent Address**

**Command:** `i2c test read 0 11 2`
- Address 0x11 doesn't exist on bus
- **Result:** Address was NACKed (signal HIGH when ACK expected)
- Driver received interrupt with AF (address failure) error bit
- Generated STOP condition
- Transaction failed

**Status Check:** `i2c test status 0`
- Module error code: -7 (MOD_ERR_PERIPH)
- Detailed error: 6 (address failure)

### **Demo 4: Status Commands**

**General Status:** `i2c status`
- Shows all instances
- Reserved flag, current state, destination address
- Message length, bytes transferred
- Last operation error and error state
- Register base address

**Performance Measurements:** `i2c pm`
- Shows error counters (reserve fail, bus busy, guard timer, etc.)
- 16-bit counters (saturate at max value)
- In demo: 1 ACK failure, many reserve failures (from tmphm module trying to access bus while reserved)

---

## **Key Takeaways from Code**

### **State Machine Characteristics:**
- **Informal implementation** with switch statements
- More complexity than state diagram shows
- Includes "special instructions" from reference manual for short reads and last bytes
- Comprehensive error checking

### **Design Philosophy:**
- Simple reservation system (honor-based)
- Guard timer for overall transaction (not per-state)
- Generic error handling (stop transaction on unexpected events)
- Performance measurement counters for debugging

### **Hardware Abstraction:**
- I2C module provides MCU-independent interface
- Higher-level modules (tmphm) use I2C module, remain hardware-independent
- LL library isolates register access

---

## **Prompts/Discussion from Lesson**

### **Prompt 1: Data Corruption Prevention**

**Question:** What variables/registers modified at both base and interrupt level? How is simultaneous modification prevented?

**Answer:**
- **Modified by both:** Fields in `i2c_state` structure (msg_bytes_xferred, last_op_error, last_op_error_state) and I2C registers (CR1, CR2)
- **Prevention:** `start_op()` immediately returns if not in IDLE state; when IDLE, I2C interrupts are disabled
- No simultaneous access possible

### **Prompt 2: Guard Timer Weaknesses**

**Question:** Weaknesses of guard timer implementation? Improvements?

**Answer:**
- **Weakness:** Single timeout value for entire transaction
- Must accommodate longest possible transaction
- Short errors (like start condition failure) wait unnecessarily long

**Improvement:** Per-state timeout values
- Update guard timer period in each state
- Appropriate timeout for each state's expected duration

### **Prompt 3: Reservation Scheme Weaknesses**

**Question:** Weaknesses and improvements?

**Weaknesses:**
1. **Honor system** - User B could skip reservation and use bus directly (module only checks *someone* reserved, not *who*)
2. **Forgotten release** - Bug in user code could lock bus forever
3. **No fairness** - First to call `i2c_reserve()` after release gets it (no queue)

**Improvements:**
1. **Unique reservation ID/key:**
   - Module provides unique ID when reservation granted
   - User must pass ID to `i2c_read()`/`i2c_write()`
   - Module verifies correct (most recent) key

2. **Maximum use time:**
   - User provides max time when reserving (e.g., 100ms)
   - Module auto-releases if not released in time

3. **Queue system:**
   - Users request reservation, granted in order
   - Possibly using callback functions

**Note:** "Comes down to engineering judgment" - simple system works if all users play by rules

---

## **Summary**

**Minimal viable I2C driver supports:**
- Single master operation
- 7-bit addressing
- Simple reads/writes

**Design is iterative:**
- Close reading of reference manual
- High-level design decisions
- State machine development
- Coding and testing

**State machine:**
- Models waiting points in I2C transaction
- Handles expected and unexpected events

**Tools essential for development:**
- Console test commands for manual testing
- Logic analyzer for seeing bus activity during debugging

```c
enum states {
    STATE_IDLE,
    STATE_MSTR_WR_GEN_START,
    STATE_MSTR_WR_SENDING_ADDR,
    STATE_MSTR_WR_SENDING_DATA,
    STATE_MSTR_RD_GEN_START,
    STATE_MSTR_RD_SENDING_ADDR,
    STATE_MSTR_RD_READING_DATA,
};
```

```c
// Per-instance i2c state information.
struct i2c_state {
    struct i2c_cfg cfg;
    I2C_TypeDef* i2c_reg_base;
    int32_t guard_tmr_id;

    uint8_t* msg_bfr;
    uint32_t msg_len;
    uint32_t msg_bytes_xferred;

    uint16_t dest_addr;

    bool reserved;
    enum states state;
    enum i2c_errors last_op_error;
    enum states last_op_error_state;
};
```

```c
int32_t i2c_reserve(enum i2c_instance_id instance_id)
{
    if (instance_id >= I2C_NUM_INSTANCES ||
        i2c_states[instance_id].i2c_reg_base == NULL)
        return MOD_ERR_BAD_INSTANCE;
    if (i2c_states[instance_id].reserved) {
        INC_SAT_U16(cnts_u16[CNT_RESERVE_FAIL]);
        return MOD_ERR_RESOURCE;
    } else {
        i2c_states[instance_id].reserved = true;
    }
    return 0;
}
```

```c
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
```

```c
        switch (st->state) {
            case STATE_MSTR_WR_GEN_START:
                if (sr1 & LL_I2C_SR1_SB) {
                    // SB is cleared by reading SR1 followed by writing address
                    // to DR.

                    st->i2c_reg_base->DR = st->dest_addr << 1;
                    st->state = STATE_MSTR_WR_SENDING_ADDR;
                }
                sr1_handled_mask = LL_I2C_SR1_SB;
```

```c
        // Check for unexpected events.
        sr1 &= ~sr1_handled_mask;
        if (sr1 & INTERRUPT_EVT_MASK)
            op_stop_fail(st, I2C_ERR_INTR_UNEXPECT, CNT_INTR_UNEXPECT);
```

```c
    struct i2c_cfg i2c_cfg;
```

```c
    result = i2c_get_def_cfg(I2C_INSTANCE_3, &i2c_cfg);
    if (result < 0) {
        log_error("i2c_get_def_cfg error %d\n", result);
        INC_SAT_U16(cnts_u16[CNT_INIT_ERR]);
    } else {
        result = i2c_init(I2C_INSTANCE_3, &i2c_cfg);
        if (result < 0) {
            log_error("i2c_init error %d\n", result);
            INC_SAT_U16(cnts_u16[CNT_INIT_ERR]);
        }
    }
```

```c
    result = i2c_start(I2C_INSTANCE_3);
    if (result < 0) {
        log_error("i2c_start 3 error %d\n", result);
        INC_SAT_U16(cnts_u16[CNT_START_ERR]);
    }
```

```c
    while (1)
    {
        stat_dur_restart(&stat_loop_dur);

        result = console_run();
        if (result < 0)
            INC_SAT_U16(cnts_u16[CNT_RUN_ERR]);

//        result = gps_run();
//        if (result < 0)
//            INC_SAT_U16(cnts_u16[CNT_RUN_ERR]);

        result = tmr_run();
        if (result < 0)
            INC_SAT_U16(cnts_u16[CNT_RUN_ERR]);

        result = tmphm_run(TMPHM_INSTANCE_1);
        if (result < 0)
            INC_SAT_U16(cnts_u16[CNT_RUN_ERR]);
    }
```

```c
    // Handle help case.
    if (argc == 2) {
        printc("Test operations and param(s) are as follows:\n"
               "  Reserve I2C, usage: i2c test reserve <instance-id>\n"
               "  Release I2C, usage: i2c test reserve <instance-id>\n");
        printc("  Start write, usage: i2c test write <instance-id> <addr> [<bytes> ...]\n"
               "  Start read, usage: i2c test read <instance-id> <addr> <num-bytes>\n"
               "  Get op status/error, usage: i2c test status <instance-id>\n");
        printc("  Bus busy, usage: i2c test busy <instance-id>\n"
               "  Print msg buffer, usage: i2c test msg <instance-id>\n");
        return 0;
    }
```

---

**User**

Summarize @I2C Lesson 5. Temperature and Humidity Module Design and Implementation.txt  compredhensively. include the related code

The source code is in @i2c.c @app_main.c @tmphm.c 

don't make things up

---

**Cursor**

## Summary of I2C Lesson 5: Temperature and Humidity Module Design and Implementation

**Lesson Overview:**
Design and implementation of a software module for the Sensirion SHT31-D temperature/humidity sensor with I2C interface, demonstrating how to build a higher-level device driver using the I2C module from Lesson 4.

---

## **Module Requirements**

**Module Name:** `tmphm` (Temperature Humidity)

1. **Background sampling** - Perform automatic temperature/humidity sampling using SHT31-D sensor
2. **Default sample rate** - 1 second (configurable)
3. **CRC checking** - Verify received sensor data integrity using built-in CRC support
4. **Non-blocking get API** - Provide latest sample value and age of sample
5. **No on-demand measurement** - Module doesn't support request-based measurements (future feature)
6. **Multiple sensor support** - Support multiple sensors on same or different I2C buses
7. **Test/Debug support** - Console commands for status, testing, and debugging

---

## **Theory of Operation - SHT31-D Sensor**

### **I2C Addressing (Table 7, Page 9)**
- **ADDR pin LOW** → Address 0x44
- **ADDR pin HIGH** → Address 0x45
- Adafruit board has pull-down resistor → Address defaults to 0x44
- Very limited addressing (common issue leading to address conflicts)

### **One-Shot Measurement Command (Table 8, Page 10)**

**Command Structure:**
- 2 bytes: MSB + LSB
- Determines repeatability level (high/medium/low)
- Determines clock stretching enabled/disabled

**Selected Command:**

```160:160:tmphm/modules/tmphm/tmphm.c
const char sensor_i2c_cmd[2] = {0x2c, 0x06 };
```
This enables clock stretching with high repeatability.

**Message Sequences:**
1. **Write Command:** START → Address + W → MSB → LSB → STOP
2. **Read Result (clock stretching disabled):** 
   - If measurement not ready: Address is NACKed → Try again later
   - If ready: Address ACKed → Read 6 bytes
3. **Read Result (clock stretching enabled):**
   - Device ACKs address but pulls clock LOW
   - Master waits automatically (hardware handles it)
   - When measurement done, device releases clock
   - Master continues reading data

### **Measurement Data Format**

**6 bytes total:**
- Byte 0-1: Temperature (16-bit)
- Byte 2: Temperature CRC
- Byte 3-4: Humidity (16-bit)  
- Byte 5: Humidity CRC

### **CRC Calculation (Table 19, Page 14)**

**Parameters:**
- Polynomial: 0x31 (x^8 + x^5 + x^4 + 1)
- Initialization: 0xFF
- Final XOR: 0x00
- Example: 0xBEEF → 0x92

**Purpose:** Detect data transmission errors with very high probability

### **Data Conversion Formulas**

**Temperature (°C):**
```
temp_degC = -45 + (175 * raw_temp / 65535)
```

**Relative Humidity (%):**
```
rh_percent = (100 * raw_hum / 65535)
```

**Integer Implementation (avoiding floating point):**

```339:346:tmphm/modules/tmphm/tmphm.c
                    int32_t temp = (msg[0] << 8) + msg[1];
                    uint32_t hum = (msg[3] << 8) + msg[4];
                    temp = -450 +
                        (1750 * temp + divisor/2) / divisor;
                    hum = (1000 * hum + divisor/2) / divisor;
                    st->last_meas.temp_deg_c_x10 = temp;
                    st->last_meas.rh_percent_x10 = hum;
```
Results multiplied by 10 to preserve resolution (22.4°C stored as 224)

### **Measurement Duration (Table 4, Page 5)**
- High repeatability: **15 milliseconds** (longest)
- Module waits this long before attempting to read result

---

## **Design Decisions**

**Single Decision: Use I2C Module for Communication**

**Advantages of Layered Approach:**
1. **Hides complexity** - tmphm doesn't need to know I2C protocol details
2. **Hardware abstraction** - tmphm is portable to different MCUs (only I2C module needs modification)
3. **Bus sharing** - Multiple modules can share same I2C bus via reservation scheme (e.g., tmphm + display module)
4. **Test/Debug features** - I2C module's console commands help during tmphm development

---

## **State Machine Design**

### **States**

```67:73:tmphm/modules/tmphm/tmphm.c
enum states {
    STATE_IDLE,
    STATE_RESERVE_I2C,
    STATE_WRITE_MEAS_CMD,
    STATE_WAIT_MEAS,
    STATE_READ_MEAS_VALUE
};
```

### **Events**

**Unlike I2C module (interrupt-driven), tmphm uses polling:**
1. **Timer callback** - Periodic measurement timer expires (only non-polling event)
2. **Polling events** (checked in `run()` function called by super loop):
   - Result of `i2c_reserve()`
   - Result of `i2c_get_op_status()`
   - Time calculation (waiting for measurement completion)

### **State Machine Flow**

**IDLE → RESERVE_I2C:**
- **Event:** Periodic timer expires (e.g., every 1 second)
- **Action:** Move to RESERVE_I2C state

**RESERVE_I2C:**
- **Polling:** Call `i2c_reserve()` each super loop iteration
- **Failure:** Stay in state (another module using bus)
- **Success:** Call `i2c_write()` with measurement command → STATE_WRITE_MEAS_CMD

**WRITE_MEAS_CMD:**
- **Polling:** Call `i2c_get_op_status()`
- **In Progress:** Stay in state
- **Failure:** Release I2C, return to IDLE
- **Success:** Record start time → STATE_WAIT_MEAS

**WAIT_MEAS:**
- **Polling:** Calculate elapsed time since write completed
- **Wait condition:** If elapsed time < 15ms, stay in state
- **Time up:** Call `i2c_read()` to read 6 bytes → STATE_READ_MEAS_VALUE

**READ_MEAS_VALUE:**
- **Polling:** Call `i2c_get_op_status()`
- **In Progress:** Stay in state
- **Failure:** Release I2C, return to IDLE
- **Success:** 
  - Check CRC on temperature and humidity data
  - If CRC good: Convert raw data, save as last_meas, record timestamp
  - If CRC bad: Increment error counter
  - Release I2C, return to IDLE

### **Guard Timers & Error Handling**

**No explicit guard timers in tmphm:**
- Depends on I2C module's guard timers
- Alternative: Use periodic timer as implicit guard timer (detects stuck state machine if timer expires in non-IDLE state)

**Continuous Measurement Failure Handling Options:**
1. Print warnings (useless in field)
2. Send alarm to management system (if available)
3. Reset sensor chip via software
4. Reset MCU (often resets entire board - "big hammer" approach)

**Engineering judgment:** Simple board reset often used for rare scenarios

---

## **Module API and Implementation**

### **Configuration Structure**

```78:87:tmphm/modules/tmphm/tmphm.c
// Per-instance tmphm state information.
struct tmphm_state {
    struct tmphm_cfg cfg;
    struct tmphm_meas last_meas;
    int32_t tmr_id;
    uint32_t i2c_op_start_ms;
    uint32_t last_meas_ms;
    uint8_t msg_bfr[I2C_MSG_BFR_LEN];
    bool got_meas;
    enum states state;
};
```

**Configuration parameters:**
- I2C instance ID (which bus sensor is on)
- I2C address (0x44 or 0x45)
- Sample time (default 1 second)
- Measurement time (default 15-16ms)

### **Measurement Data Format**

Values stored as integers × 10 to preserve decimal resolution:
- `temp_deg_c_x10`: 20.0°C = 200
- `rh_percent_x10`: 45.0% = 450

### **Core Module Functions**

Standard super loop interface:
- `tmphm_init()` - Initialize module
- `tmphm_start()` - Start operation (registers timer callback)
- `tmphm_run()` - Called from super loop (implements state machine)

### **Get Last Measurement API**

```372:391:tmphm/modules/tmphm/tmphm.c
int32_t tmphm_get_last_meas(enum tmphm_instance_id instance_id,
                            struct tmphm_meas* meas, uint32_t* meas_age_ms)
{
    struct tmphm_state* st;

    if (instance_id >= TMPHM_NUM_INSTANCES)
        return MOD_ERR_BAD_INSTANCE;
    if (meas == NULL)
        return MOD_ERR_ARG;

    st = &tmphm_states[instance_id];
    if (!st->got_meas)
        return MOD_ERR_UNAVAIL;

    *meas = st->last_meas;
    if (meas_age_ms != NULL)
        *meas_age_ms = tmr_get_ms() - st->last_meas_ms;

    return 0;
}
```

Returns measurement and its age. Age should be < sample time if working properly.

### **Run Function - State Machine Implementation**

```259:361:tmphm/modules/tmphm/tmphm.c
int32_t tmphm_run(enum tmphm_instance_id instance_id)
{
    struct tmphm_state* st;
    int32_t rc;
    // ChatGPT
//    log_debug("tmphm_run: instance=%d, state=%d\n", instance_id, tmphm_states[instance_id].state);


    if (instance_id >= TMPHM_NUM_INSTANCES)
        return MOD_ERR_BAD_INSTANCE;

    st = &tmphm_states[instance_id];

    switch (st->state) {
        case STATE_IDLE:
            break;

        case STATE_RESERVE_I2C:
        	// ChatGPT
//            log_debug("tmphm: attempting to reserve I2C instance %d\n", st->cfg.i2c_instance_id);
            rc = i2c_reserve(st->cfg.i2c_instance_id);
            if (rc == 0) {
                memcpy(st->msg_bfr, sensor_i2c_cmd, sizeof(sensor_i2c_cmd));
                rc = i2c_write(st->cfg.i2c_instance_id, st->cfg.i2c_addr, st->msg_bfr,
                               sizeof(sensor_i2c_cmd));
                if (rc == 0) {
                    st->state = STATE_WRITE_MEAS_CMD;
                } else {
                    INC_SAT_U16(cnts_u16[CNT_WRITE_INIT_FAIL]);
                    i2c_release(st->cfg.i2c_instance_id);
                    st->state = STATE_IDLE;
                }
            } else {
                INC_SAT_U16(cnts_u16[CNT_RESERVE_FAIL]);
            }
            break;

        case STATE_WRITE_MEAS_CMD:
            rc = i2c_get_op_status(st->cfg.i2c_instance_id);
            if (rc != MOD_ERR_OP_IN_PROG) {
                if (rc == 0) {
                    st->i2c_op_start_ms = tmr_get_ms();
                    st->state = STATE_WAIT_MEAS;
                } else {
                    INC_SAT_U16(cnts_u16[CNT_WRITE_OP_FAIL]);
                    i2c_release(st->cfg.i2c_instance_id);
                    st->state = STATE_IDLE;
                }
            }
            break;

        case STATE_WAIT_MEAS:
            if (tmr_get_ms() - st->i2c_op_start_ms >= st->cfg.meas_time_ms) {
                rc = i2c_read(st->cfg.i2c_instance_id, st->cfg.i2c_addr,
                              st->msg_bfr, I2C_MSG_BFR_LEN);
                if (rc == 0) {
                    st->state = STATE_READ_MEAS_VALUE;
                } else {
                    INC_SAT_U16(cnts_u16[CNT_READ_INIT_FAIL]);
                    i2c_release(st->cfg.i2c_instance_id);
                    st->state = STATE_IDLE;
                }
            }
            break;

        case STATE_READ_MEAS_VALUE:
            rc = i2c_get_op_status(st->cfg.i2c_instance_id);
            if (rc != MOD_ERR_OP_IN_PROG) {
                if (rc == 0) {
                    const uint32_t divisor = 65535;
                    uint8_t* msg = st->msg_bfr;
                    // ChatGPT
//                    log_info("tmphm: Read raw bytes: %02x %02x %02x %02x %02x %02x\n",
//                             msg[0], msg[1], msg[2], msg[3], msg[4], msg[5]);


                    if (crc8(&msg[0], 2) != msg[2] ||
                        crc8(&msg[3], 2) != msg[5]) {
                        INC_SAT_U16(cnts_u16[CNT_CRC_FAIL]);
                    } else {
                        int32_t temp = (msg[0] << 8) + msg[1];
                        uint32_t hum = (msg[3] << 8) + msg[4];
                        temp = -450 +
                            (1750 * temp + divisor/2) / divisor;
                        hum = (1000 * hum + divisor/2) / divisor;
                        st->last_meas.temp_deg_c_x10 = temp;
                        st->last_meas.rh_percent_x10 = hum;
                        st->last_meas_ms = tmr_get_ms();
                        st->got_meas = true;
                        log_info("temp=%ld degC*10 hum=%d %%*10\n",
                                 temp, hum);
                    }
                    
                } else {
                    INC_SAT_U16(cnts_u16[CNT_READ_OP_FAIL]);
                }
                i2c_release(st->cfg.i2c_instance_id);
                st->state = STATE_IDLE;
            }
            break;
    }
    return 0;
}
```

Called at high rate from super loop. Follows state diagram closely.

### **Timer Callback**

```397:412:tmphm/modules/tmphm/tmphm.c
static enum tmr_cb_action tmr_callback(int32_t tmr_id, uint32_t user_data)
{
    enum tmphm_instance_id instance_id = (enum tmphm_instance_id)user_data;
    struct tmphm_state* st;

    log_trace("tmr_callback(tmr_id=%d user_data=%lu)\n", tmr_id, user_data);
    if (instance_id >= TMPHM_NUM_INSTANCES)
        return TMR_CB_RESTART;

    st = &tmphm_states[instance_id];
    if (st->state == STATE_IDLE)
        st->state = STATE_RESERVE_I2C;
    else
        INC_SAT_U16(cnts_u16[CNT_TASK_OVERRUN]);
    return TMR_CB_RESTART;
}
```

Triggers new measurement cycle. If not IDLE, logs overrun error (state machine stuck).

### **CRC8 Implementation**

```547:571:tmphm/modules/tmphm/tmphm.c
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
```

**Not highly optimized** but simple and sufficient for 2-byte data. Common code found on web.

---

## **Integration in app_main.c**

### **Initialization**

```273:284:tmphm/app1/app_main.c
    result = tmphm_get_def_cfg(TMPHM_INSTANCE_1, &tmphm_cfg);
    if (result < 0) {
        log_error("tmphm_get_def_cfg error %d\n", result);
        INC_SAT_U16(cnts_u16[CNT_INIT_ERR]);
    } else {
        tmphm_cfg.i2c_instance_id = I2C_INSTANCE_3;
        result = tmphm_init(TMPHM_INSTANCE_1, &tmphm_cfg);
        if (result < 0) {
            log_error("tmphm_init error %d\n", result);
            INC_SAT_U16(cnts_u16[CNT_INIT_ERR]);
        }
    }
```

Configures tmphm to use I2C_INSTANCE_3.

### **Start Phase**

```340:344:tmphm/app1/app_main.c
    result = tmphm_start(TMPHM_INSTANCE_1);
    if (result < 0) {
        log_error("tmphm_start 1 error %d\n", result);
        INC_SAT_U16(cnts_u16[CNT_START_ERR]);
    }
```

### **Super Loop**

```376:378:tmphm/app1/app_main.c
        result = tmphm_run(TMPHM_INSTANCE_1);
        if (result < 0)
            INC_SAT_U16(cnts_u16[CNT_RUN_ERR]);
```

Called continuously to advance state machine.

---

## **Demonstrations**

### **Demo 1: Normal Operation**

**Setup:** Logic analyzer capturing I2C traffic, sampling every 1 second

**Observed:**
- Write operation: 2 bytes (command)
- 16.7ms later: Read operation: 6 bytes (measurement result)
- Continuous measurements every second

### **Demo 2: Clock Stretching**

**Command:** `tmphm test meastime 0 10` (reduce wait from 16ms to 10ms)

**Result:**
- Write command sent
- Read attempted after ~9.8ms (too soon)
- **Clock held LOW by sensor for ~3ms** (clock stretching)
- Master waited automatically
- When sensor released clock, data transfer continued
- Total gap: 3 milliseconds

**Observation:** Clock stretching is transparent to software, handled by hardware

### **Demo 3: CRC Verification**

**Command:** `tmphm test crc8 0x8f 0x98`

**Result:** CRC mismatch (wrong bytes entered)

**Command:** `tmphm test crc8 0x8f 0x9b` (correct bytes)

**Result:** CRC = 0xe8 (matches received data)

**Purpose:** Demonstrates CRC detects data corruption

### **Demo 4: Clock Stretching Forever (Error Case)**

**Setup:**
1. Reserve I2C bus: `i2c test reserve 0` (blocks tmphm from running)
2. Read without first sending measurement command: `i2c test read 0 44 6`

**Result:**
- Sensor ACKed address
- **Sensor held clock LOW indefinitely** (stuck waiting for measurement command that was never sent)
- Bus completely stuck - cannot communicate
- Status: `i2c_get_op_status()` returns MOD_ERR_PERIPH (error 2 = bus busy)

**Recovery:**
- Only solution: Hardware reset of sensor chip
- Demonstrated by grounding reset pin manually
- Clock released, bus functional again

**Implications for Production:**
- "Not very nice" behavior from sensor
- In 24/7 systems, unexpected conditions happen
- Need recovery mechanism:
  - Option 1: MCU reset (if it resets entire board)
  - Option 2: GPIO connected to sensor reset pin for software-controlled reset
- **Better to discover issues early than in field**

### **Demo 5: Viewing Actual Temperature/Humidity**

**Method 1:** `tmphm test lastmeas 0`
- Output: `Temp=22.4 C Hum=49.7 % age=885 ms`

**Method 2:** Enable logging
- Prints every sample: `temp=224 degC*10 hum=497 %*10`
- Shows raw values (÷10 for actual)
- Temperature stable at 22.4-22.5°C
- Humidity varies slightly around 49-50%

---

## **Key Takeaways**

### **Layered Design Benefits:**
1. **Simplifies implementation** - tmphm doesn't handle I2C complexity
2. **Hardware abstraction** - Portable across different MCUs
3. **Bus sharing** - Multiple modules can coexist via reservation
4. **Reusable test infrastructure** - I2C debug commands aid tmphm development

### **State Machine Usage:**
- Orchestrates sequence of I2C transactions
- Poll-based (unlike interrupt-driven I2C module)
- Simple, clear flow matching measurement procedure

### **Data Integrity:**
- CRC provides high-confidence error detection
- Critical for reliable sensor data in real-world applications

### **Murphy's Law in Embedded:**
- Experiment with unexpected I2C operations
- Devices may behave poorly under unusual conditions
- Need robust recovery mechanisms for production
- Find problems early through testing

---

## **Prompts/Discussion**

### **Prompt 1: Detecting Duplicate Sensors**

**Question:** Two SHT31-D sensors with same address on single bus - how to detect?

**Answer:** 
- Measurement data will likely be corrupted (unless both sensors return identical values)
- **CRC will fail** due to data corruption from multiple devices responding
- Refers back to Lesson 4 prompt about open drain combining bits

### **Prompt 2: Three Reset Methods**

**Question:** Per datasheet, what are three ways to reset sensor?

**Answer:**
1. **Soft reset** - I2C command
2. **Pulse nRESET pin** - Hardware reset (used in demo)
3. **Power cycle** - Remove/restore power

### **Prompt 3: Recommended Reset Method**

**Question:** Recommended method without removing power? Why?

**Answer:**
- **Pulse nRESET pin** (not soft reset)
- **Reason:** More reliable - doesn't depend on I2C communication working
- Demonstrated in demo when sensor held clock LOW (I2C communication impossible)

### **Prompt 4: Combined Format Support**

**Question:** Does sensor support combined format messages?

**Answer:**
- **Yes**
- Look for transaction with multiple START conditions before STOP
- Examples:
  - **Fetch Data command** (Table 11)
  - **Read Status Register command** (Table 17)
- Both are write followed by read

---

## **Summary**

Using generic I2C module simplifies device-specific module implementation and provides advantages like simple bus sharing. State machine orchestrates sequence of I2C transactions for measurement task. Sensor uses CRCs to verify data integrity across I2C bus. Demo showed importance of experimenting with I2C devices to understand behavior under unexpected conditions - Murphy's Law requires thinking about recovery mechanisms for stuck devices in production systems.

```c
const char sensor_i2c_cmd[2] = {0x2c, 0x06 };
```

```plaintext
temp_degC = -45 + (175 * raw_temp / 65535)
```

```plaintext
rh_percent = (100 * raw_hum / 65535)
```

```c
                    int32_t temp = (msg[0] << 8) + msg[1];
                    uint32_t hum = (msg[3] << 8) + msg[4];
                    temp = -450 +
                        (1750 * temp + divisor/2) / divisor;
                    hum = (1000 * hum + divisor/2) / divisor;
                    st->last_meas.temp_deg_c_x10 = temp;
                    st->last_meas.rh_percent_x10 = hum;
```

```c
enum states {
    STATE_IDLE,
    STATE_RESERVE_I2C,
    STATE_WRITE_MEAS_CMD,
    STATE_WAIT_MEAS,
    STATE_READ_MEAS_VALUE
};
```

```c
// Per-instance tmphm state information.
struct tmphm_state {
    struct tmphm_cfg cfg;
    struct tmphm_meas last_meas;
    int32_t tmr_id;
    uint32_t i2c_op_start_ms;
    uint32_t last_meas_ms;
    uint8_t msg_bfr[I2C_MSG_BFR_LEN];
    bool got_meas;
    enum states state;
};
```

```c
int32_t tmphm_get_last_meas(enum tmphm_instance_id instance_id,
                            struct tmphm_meas* meas, uint32_t* meas_age_ms)
{
    struct tmphm_state* st;

    if (instance_id >= TMPHM_NUM_INSTANCES)
        return MOD_ERR_BAD_INSTANCE;
    if (meas == NULL)
        return MOD_ERR_ARG;

    st = &tmphm_states[instance_id];
    if (!st->got_meas)
        return MOD_ERR_UNAVAIL;

    *meas = st->last_meas;
    if (meas_age_ms != NULL)
        *meas_age_ms = tmr_get_ms() - st->last_meas_ms;

    return 0;
}
```

```c
int32_t tmphm_run(enum tmphm_instance_id instance_id)
{
    struct tmphm_state* st;
    int32_t rc;
    // ChatGPT
//    log_debug("tmphm_run: instance=%d, state=%d\n", instance_id, tmphm_states[instance_id].state);


    if (instance_id >= TMPHM_NUM_INSTANCES)
        return MOD_ERR_BAD_INSTANCE;

    st = &tmphm_states[instance_id];

    switch (st->state) {
        case STATE_IDLE:
            break;

        case STATE_RESERVE_I2C:
        	// ChatGPT
//            log_debug("tmphm: attempting to reserve I2C instance %d\n", st->cfg.i2c_instance_id);
            rc = i2c_reserve(st->cfg.i2c_instance_id);
            if (rc == 0) {
                memcpy(st->msg_bfr, sensor_i2c_cmd, sizeof(sensor_i2c_cmd));
                rc = i2c_write(st->cfg.i2c_instance_id, st->cfg.i2c_addr, st->msg_bfr,
                               sizeof(sensor_i2c_cmd));
                if (rc == 0) {
                    st->state = STATE_WRITE_MEAS_CMD;
                } else {
                    INC_SAT_U16(cnts_u16[CNT_WRITE_INIT_FAIL]);
                    i2c_release(st->cfg.i2c_instance_id);
                    st->state = STATE_IDLE;
                }
            } else {
                INC_SAT_U16(cnts_u16[CNT_RESERVE_FAIL]);
            }
            break;

        case STATE_WRITE_MEAS_CMD:
            rc = i2c_get_op_status(st->cfg.i2c_instance_id);
            if (rc != MOD_ERR_OP_IN_PROG) {
                if (rc == 0) {
                    st->i2c_op_start_ms = tmr_get_ms();
                    st->state = STATE_WAIT_MEAS;
                } else {
                    INC_SAT_U16(cnts_u16[CNT_WRITE_OP_FAIL]);
                    i2c_release(st->cfg.i2c_instance_id);
                    st->state = STATE_IDLE;
                }
            }
            break;

        case STATE_WAIT_MEAS:
            if (tmr_get_ms() - st->i2c_op_start_ms >= st->cfg.meas_time_ms) {
                rc = i2c_read(st->cfg.i2c_instance_id, st->cfg.i2c_addr,
                              st->msg_bfr, I2C_MSG_BFR_LEN);
                if (rc == 0) {
                    st->state = STATE_READ_MEAS_VALUE;
                } else {
                    INC_SAT_U16(cnts_u16[CNT_READ_INIT_FAIL]);
                    i2c_release(st->cfg.i2c_instance_id);
                    st->state = STATE_IDLE;
                }
            }
            break;

        case STATE_READ_MEAS_VALUE:
            rc = i2c_get_op_status(st->cfg.i2c_instance_id);
            if (rc != MOD_ERR_OP_IN_PROG) {
                if (rc == 0) {
                    const uint32_t divisor = 65535;
                    uint8_t* msg = st->msg_bfr;
                    // ChatGPT
//                    log_info("tmphm: Read raw bytes: %02x %02x %02x %02x %02x %02x\n",
//                             msg[0], msg[1], msg[2], msg[3], msg[4], msg[5]);


                    if (crc8(&msg[0], 2) != msg[2] ||
                        crc8(&msg[3], 2) != msg[5]) {
                        INC_SAT_U16(cnts_u16[CNT_CRC_FAIL]);
                    } else {
                        int32_t temp = (msg[0] << 8) + msg[1];
                        uint32_t hum = (msg[3] << 8) + msg[4];
                        temp = -450 +
                            (1750 * temp + divisor/2) / divisor;
                        hum = (1000 * hum + divisor/2) / divisor;
                        st->last_meas.temp_deg_c_x10 = temp;
                        st->last_meas.rh_percent_x10 = hum;
                        st->last_meas_ms = tmr_get_ms();
                        st->got_meas = true;
                        log_info("temp=%ld degC*10 hum=%d %%*10\n",
                                 temp, hum);
                    }
                    
                } else {
                    INC_SAT_U16(cnts_u16[CNT_READ_OP_FAIL]);
                }
                i2c_release(st->cfg.i2c_instance_id);
                st->state = STATE_IDLE;
            }
            break;
    }
    return 0;
}
```

```c
static enum tmr_cb_action tmr_callback(int32_t tmr_id, uint32_t user_data)
{
    enum tmphm_instance_id instance_id = (enum tmphm_instance_id)user_data;
    struct tmphm_state* st;

    log_trace("tmr_callback(tmr_id=%d user_data=%lu)\n", tmr_id, user_data);
    if (instance_id >= TMPHM_NUM_INSTANCES)
        return TMR_CB_RESTART;

    st = &tmphm_states[instance_id];
    if (st->state == STATE_IDLE)
        st->state = STATE_RESERVE_I2C;
    else
        INC_SAT_U16(cnts_u16[CNT_TASK_OVERRUN]);
    return TMR_CB_RESTART;
}
```

```c
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
```

```c
    result = tmphm_get_def_cfg(TMPHM_INSTANCE_1, &tmphm_cfg);
    if (result < 0) {
        log_error("tmphm_get_def_cfg error %d\n", result);
        INC_SAT_U16(cnts_u16[CNT_INIT_ERR]);
    } else {
        tmphm_cfg.i2c_instance_id = I2C_INSTANCE_3;
        result = tmphm_init(TMPHM_INSTANCE_1, &tmphm_cfg);
        if (result < 0) {
            log_error("tmphm_init error %d\n", result);
            INC_SAT_U16(cnts_u16[CNT_INIT_ERR]);
        }
    }
```

```c
    result = tmphm_start(TMPHM_INSTANCE_1);
    if (result < 0) {
        log_error("tmphm_start 1 error %d\n", result);
        INC_SAT_U16(cnts_u16[CNT_START_ERR]);
    }
```

```c
        result = tmphm_run(TMPHM_INSTANCE_1);
        if (result < 0)
            INC_SAT_U16(cnts_u16[CNT_RUN_ERR]);
```

