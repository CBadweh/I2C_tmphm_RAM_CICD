# Gene's I2C & Temperature/Humidity Sensor Course - Comprehensive Summary

**Course:** I2C Theory and Practice on Bare Metal STM32
**Instructor:** Gene Schrader
**Summarized by:** Claude (Anthropic)
**Date:** December 2025

---

## Table of Contents

1. [Lesson 1: Introduction to the Course](#lesson-1-introduction-to-the-course)
2. [Lesson 2: I2C Introduction and Theory of Operation](#lesson-2-i2c-introduction-and-theory-of-operation)
3. [Lesson 3: I2C Module Requirements and Design](#lesson-3-i2c-module-requirements-and-design)
4. [Lesson 4: I2C Module Implementation and Demo](#lesson-4-i2c-module-implementation-and-demo)
5. [Lesson 5: Temperature and Humidity Module Design and Implementation](#lesson-5-temperature-and-humidity-module-design-and-implementation)
6. [Summary and Best Practices](#summary-and-best-practices)

---

## Lesson 1: Introduction to the Course

### Lesson Objectives

The course aims to provide comprehensive knowledge about:
- The I2C interface and its operation
- Designing an I2C driver at the register level with interrupts for STM32 MCU
- Developing production-quality designs and code
- Building practical bare-metal embedded software

**Why This Matters:** I2C is fundamental for communicating with external sensors, memory, displays, and I/O expanders in embedded systems. Understanding both theory and implementation is essential for professional embedded engineers.

### Course Structure

The course consists of three major parts (~2.25 hours total):

1. **I2C Theory of Operation**
   - Understanding the protocol fundamentals
   - How I2C compares to other serial interfaces

2. **I2C Driver Module** (Bare-Metal Embedded)
   - Requirements for minimum viable product
   - Driver module design based on state machine
   - Module implementation (code)
   - Demo using console and logic analyzer

3. **Higher-Level Sensor Module** (SHT31 Temperature/Humidity)
   - Requirements definition
   - Driver module design (state machine based)
   - Module implementation
   - Demo from console

### Hardware Setup

**MCU Platform:**
- **Board:** Nucleo-F401RE
- **MCU:** STM32F401RE
- **I2C Interface:** I2C3 (chosen for pin availability)
- **Pins:** SDA on PB4, SCL on PA8

**Sensor:**
- **Module:** Adafruit SHT31-D breakout
- **Sensor Chip:** Sensirion SHT31-D (temperature & humidity)
- **Interface:** I2C

**Test Equipment:**
- **Logic Analyzer:** Low-cost clone compatible with Saleae Logic 2
- **Purpose:** View I2C signals on the wire for debugging

**Connections:**
- Sensor: Power (VDD), Ground (GND), SDA, SCL from MCU
- Logic Analyzer: Connected to SDA, SCL, and Ground
- Both Nucleo and Logic Analyzer connect to laptop via USB

### Software Infrastructure

**Development Tools (All Free):**
- STM32CubeIDE
- STM32CubeMX (integrated with IDE)
- Saleae Logic 2 software
- ARM GCC compiler
- newlib-nano (lightweight C library)

**Base Infrastructure:**
Based on previous "MCU Base Course" which developed:
- Software infrastructure layer using super loop pattern
- Module API pattern for consistent interfaces
- Infrastructure modules (reused in this course)

**Module API Pattern:**
All modules follow standard interface:
```c
module_get_def_cfg()  // Get default configuration
module_init()         // Phase 1 initialization
module_start()        // Phase 2 initialization
module_run()          // Called from super loop (if needed)
```

### Prerequisites

**Required Knowledge:**
- Understanding of MCUs and embedded programming
- Basic C programming
- Concept of high/low signals (1s and 0s)
- Either the base MCU course or equivalent experience

**Optional:**
- STM32Cube experience (minimal usage in course)
- Electronics knowledge (not much needed beyond digital signals)

### Implementation Highlights

**GitHub Repositories:**
1. **Source Code:** Modified from base course repo, includes all I2C code
2. **Course Materials:** I2C specification, sensor datasheet, etc.

**Key Points:**
- Uses same module API from base course for easy integration
- Layered software architecture with reuse
- Hardware setup functional despite being "ugly" (prototype wiring)
- Different STM32 MCUs have different I2C hardware - this driver won't work on all

---

## Lesson 2: I2C Introduction and Theory of Operation

### Lesson Objectives

- Understand I2C protocol fundamentals and terminology
- Learn advantages and disadvantages compared to SPI and UART
- Master open-drain signals and their electrical characteristics
- Understand detailed step-by-step bus operations
- Learn about advanced I2C features

**Why This Matters:** Understanding the theory is essential for implementing drivers, debugging issues, and making informed design decisions.

### I2C Fundamentals

**What is I2C?**
- **Full Name:** Inter-Integrated Circuit
- **Pronunciation:** "I-two-C" or "I-squared-C"
- **Type:** Serial communication protocol
- **Common Uses:** MCU communication with:
  - External sensors
  - Memory (EEPROM)
  - I/O expanders
  - Displays
  - Other devices

**Bus Structure:**
- **SDA (Serial Data):** Transmit and receive data
- **SCL (Serial Clock):** Clock signal
- **Ground:** Common reference (not counted as a signal)
- **Two-wire interface** (plus ground)

### Comparison with Other Protocols

**Advantages of I2C:**

1. **Minimal Pin Usage**
   - Only 2 signals (SDA, SCL) to communicate with multiple devices
   - With SPI: Need more signals, often one dedicated signal per device
   - Biggest advantage for I2C

2. **Built-in Error Detection**
   - ACK signals provide positive confirmation of correct operation
   - With SPI: Some error conditions can go undetected
   - Increases reliability

3. **Message Framing**
   - Protocol handles start/stop conditions
   - vs. Asynchronous serial: Must write code to determine message boundaries
   - Simplifies software implementation

**Disadvantages of I2C:**

1. **Lower Speed** (Biggest disadvantage)
   - Due to open-drain bus signals (electrical limitation)
   - Typically slower than SPI
   - Trade-off for reduced pin count

2. **Pull-up Resistors Required**
   - Minor cost and board space
   - Not a major issue but adds components

3. **Possible Address Conflicts**
   - Limited 7-bit address space (128 addresses)
   - Some reserved addresses reduce available space
   - Can be problematic with certain device combinations

**Typical Applications:**
- I2C and SPI: Local to board or closely connected boards (< 1 meter)
- CAN and some async serial: Can span 100+ meters
- Different protocols for different use cases

### I2C Terminology and Concepts

**Transaction:**
- Complete transfer of information between devices
- Has distinct start and stop markers
- Can consist of one or more read/write operations

**Addressing (7-bit):**
- Each device on bus must have unique address
- First byte in transaction contains:
  - 7-bit address (bits 7-1)
  - Read/Write bit (bit 0)
- Master sends address → matching slave responds
- Address conflicts occur with duplicate addresses on same bus

**Master and Slave:**
- **Master:** Initiates transactions (typically MCU)
  - Sends address to select slave
  - Controls clock signal
  - No address needed (doesn't identify itself)
- **Slave:** Reacts to master requests (sensors, memory, etc.)
  - Each has unique address
  - Responds when addressed
- **Single Master:** Most common configuration
- **Multi-Master:** Possible but requires bus arbitration (covered briefly)

### Open Drain Signals - Critical Concept

**Why Open Drain?**
Enables multiple devices to share signals safely.

**How It Works:**

*Default State (High):*
- Pull-up resistor connects signal to VDD (3.3V or 5V)
- Signal naturally "pulled up" to high level
- High = logic 1

*Pulling Low:*
- Device closes conceptual "switch" to ground
- Connects signal to VSS/GND (0V)
- Overpowers pull-up resistor
- Low = logic 0
- **Key:** When device releases (opens switch), pull-up brings signal high again

*Reading:*
- All devices can always read signal level
- Both those controlling it and others

*Multiple Devices:*
- Any device can pull signal low
- If one pulls low, signal is low (even if others try to keep it high)
- **Low takes precedence over high**
- Enables safe sharing without conflicts

**Bus Interface (Simplified):**
```
Each device has:
- Reading capability: Always able to sense signal level
- Control capability: Conceptual switch to pull to ground
  - Open: No influence (pull-up makes signal high)
  - Closed: Pulls signal low
```

**Protocol Coordination:**
- I2C specification defines rules for who controls signals when
- Very organized, detailed steps for each operation
- Prevents conflicts through careful protocol design

### I2C Bus Operations - Detailed

**Example Circuit:**
- MCU (master)
- Multiple slaves (sensors, A/D converter, etc.)
- Two I2C buses shown (reason: address conflict resolution)
- Each bus has:
  - SDA and SCL signals
  - Pull-up resistors on each signal
  - Multiple devices connected in parallel

**Address Conflict Example:**
Temperature sensor limited to addresses 0x44 or 0x45 (common limitation).
- Need 3 sensors → only 2 addresses available
- **Solution:** Use two I2C buses
  - Bus 1: Sensors at 0x44 and 0x45
  - Bus 2: Third sensor at 0x45
  - Different buses = no conflict

**Step-by-Step Bus Operation:**

*High-Level Sequence (Abstract):*

1. **Acquire Bus**
   - Check SDA and SCL both high (idle state)
   - Single master: Usually immediate
   - May need short wait/retry after previous transaction

2. **Start Condition**
   - Master generates special signal pattern
   - Tells all slaves: "Wake up and listen"
   - Beginning of transaction

3. **Send Address + R/W Bit**
   - Master sends 7-bit slave address
   - Plus 1-bit read/write indicator
   - Total: 8 bits

4. **Wait for ACK/NACK**
   - All slaves compare address with their own
   - Matching slave: Sends ACK (acknowledge)
   - No match: No response (absence of ACK = NACK)
   - NACK → Master stops transaction (no one to talk to)

5. **Data Transfer** (if ACK received)
   - **Write Operation:**
     - Master sends data bytes
     - Slave ACKs each byte
     - Master stops when done (or slave can NACK to stop early)

   - **Read Operation:**
     - Slave sends data bytes
     - Master ACKs each byte
     - Master NACKs last byte to signal end

6. **Stop Condition**
   - Master generates special signal pattern
   - Transaction complete, bus returns to idle

**Signal-Level Details:**

*Signal Diagram Basics:*
- **SDA:** Data signal (high/low shows bit values)
- **SCL:** Clock signal (rising/falling edges)
- **Time:** Moves left to right
- **Sampling:** Data sampled on rising edge of clock
- **Changes:** Data can change after falling edge of clock

*Start Condition:*
- Unique pattern: SDA falls while SCL is high
- Unique because data normally doesn't change when clock is high

*Stop Condition:*
- Unique pattern: SDA rises while SCL is high
- Also unique for same reason

*Address + R/W Byte:*
- 7 address bits (MSB first)
- 1 R/W bit (0 = write, 1 = read)
- Clock rising edge: Sample each bit (7 clock pulses + 1 for R/W)

*ACK/NACK Bit:*
- 9th clock pulse
- Slave controls SDA (master releases)
- Low = ACK, High = NACK
- **Control Transfer:** Master controls SDA for address, slave takes over for ACK

*Data Bytes:*
- 8 bits per byte (MSB first typically)
- After each byte: ACK/NACK bit
- Control switches between sender and receiver for ACK

*Important:*
- **Master always controls SCL** (almost always - clock stretching exception)
- Open drain allows control to switch between devices

### Advanced I2C Features

**Multi-Master:**
- Multiple masters can use same bus
- **Arbitration:**
  - Master checks if bus busy before starting
  - If both start simultaneously (collision):
    - Hardware detection mechanism
    - Exactly one master backs off
    - Other continues successfully

**Bus Speeds (Modes):**
- **Standard Mode:** Up to 100 kbps (original spec)
- **Fast Mode:** Up to 400 kbps
- **Even Faster Modes:** High Speed, Ultra Fast (less common)
- Speed = bits per second ≈ clock frequency

**Clock Stretching:**
- **Slave Stretching:**
  - Slave needs time to process data
  - Pulls SCL low (holds clock)
  - Master detects and waits
  - Slave releases when ready
  - Open drain enables this

- **Master Stretching:**
  - Master can simply stop clock
  - Slaves typically don't notice
  - Resume when ready

- **Software:** Usually automatic, hardware handles it

**Combined Format (Write + Read):**
- Two or more operations in single transaction
- **Most Common:** Write then read
  - Write: Specify what to read (memory address, register number)
  - Read: Get the data

- **Implementation:**
  - Normal start + write operation
  - **Repeated Start** instead of stop
  - Second operation (read)
  - Stop condition at end

- **Benefit:**
  - Slightly more efficient
  - **Atomic Operation:** No other transaction can interrupt
  - Critical in multi-master systems

**10-bit Addressing:**
- Extension of 7-bit addresses (not commonly used)
- **Reserved Pattern:** 7-bit addresses starting with 11110xx are reserved
- **Mechanism:**
  - First byte: Reserved pattern (11110) + first 2 bits of 10-bit address
  - Second byte: Remaining 8 bits of 10-bit address
- **Coexistence:** Can mix 7-bit and 10-bit devices on same bus
- No 7-bit device will match reserved pattern

**General Call Address (0x00):**
- Special reserved address
- **Purpose:** Broadcast to all slaves
- All slaves recognize this address
- **Use Case:** Standard procedures like soft reset
  - Master sends: General call address + reset command
  - All slaves reset (if they support it)
- **Note:** Standard defines some command meanings (unusual for I2C)

### Key Insights from Lesson

**What the Spec Covers:**
- All the protocol details (start, stop, addressing, ACK/NACK, etc.)
- **What it doesn't cover:** Meaning of data bytes (device-specific)

**Practical Implications:**
- Open drain = slower but fewer pins
- Understanding signal timing crucial for debugging
- Logic analyzer shows exactly what's happening
- Protocol is organized and methodical

**Quote from Instructor:**
"Once you get the idea of the clock signal and how it tells you when to sample the data signal, you've learned a key concept in digital sequential logic."

---

## Lesson 3: I2C Module Requirements and Design

### Lesson Objectives

- Define minimum viable product (MVP) requirements
- Make high-level design decisions
- Design the module API
- Create state machine for I2C operations

**Why This Matters:** Professional development starts with clear requirements and thoughtful design. This prevents scope creep and creates maintainable code.

### Minimum Viable Product Requirements

**Functional Requirements (I2C-specific):**

1. **Master Operation Only**
   - Single master on bus (MCU is the master)
   - No multi-master support (simplifies design)

2. **Simple Read and Write Operations**
   - Read-only transactions
   - Write-only transactions
   - No combined format (write+read) initially
   - Rationale: Some devices support combined format but don't require it

3. **7-bit Address Support Only**
   - No 10-bit addressing
   - Rationale: 10-bit addresses rarely used

**Non-Functional Requirements (Internal Quality):**

4. **Shared Bus Support**
   - Multiple user modules can share I2C interface
   - Reservation system for resource management
   - Essential for multi-device systems

5. **Super Loop Integration**
   - Support standard core API (init, start, run)
   - Pattern from base course
   - Enables integration with other modules

6. **Non-blocking APIs**
   - Critical requirement for super loop
   - All operations return immediately
   - Status polling for completion

7. **Test and Debug Support**
   - Console commands for manual testing
   - Performance measurements for monitoring
   - Status reporting for troubleshooting
   - Essential for production support

**Evolution:**
- Requirements can change during development
- This is normal and expected
- Document changes as they occur

### Major Design Tasks

**Iterative Process (Not Strictly Sequential):**

1. **Understand Theory of Operation**
   - Don't need to be expert initially
   - Learn more while coding
   - Continuous learning process

2. **Read MCU Documentation**
   - Reference manual and/or datasheet
   - Read repeatedly (not all clear first time)
   - Refer back throughout development
   - Some aspects may remain unclear even when done

3. **High-Level Design Decisions**
   - API design
   - Use of DMA vs. interrupts
   - Initialization approach
   - Document decisions and rationale

4. **Detailed Design and Coding**
   - State machine design
   - Timer management
   - Actual implementation
   - Continuous refinement and refactoring
   - May add/remove states as understanding grows

5. **Iterative Testing**
   - Get testable code early
   - Example: Generate start condition, verify with logic analyzer
   - Prioritize interrupt implementation (core of driver)
   - Jump around - not strictly linear

6. **Reference Other Implementations**
   - HAL driver (from ST)
   - Linux driver (for this same I2C peripheral)
   - Compare different approaches
   - Separate framework overhead from core algorithms
   - Use judiciously

### Critical Design Decisions

**Decision 1: Register-Level with LL Library**

*Choice:* Use STM32 Low-Level (LL) library for register access

*Rationale:*
- LL handles grunt work of register bit/field manipulation
- Cleaner than raw register access
- Simple wrappers if porting needed
- Good middle ground between HAL (too much abstraction) and raw (too tedious)

*Implementation:*
- Find LL API for desired register operation
- Usually one-line function or macro
- Review code occasionally to understand mechanism

*Portability:*
- If porting to non-STM32 with same I2C peripheral:
  - Create simple wrapper functions
  - Isolated within .c file
  - Don't overthink until actually needed

**Decision 2: Interrupt-Based Transfer**

*Choice:* Use interrupts (not DMA) for data transfer

*Rationale:*
- Good learning foundation for DMA later
- Adequate performance for typical use cases
- Less complex than DMA
- Knowledge transfers to DMA implementation

*Future:*
- DMA can be added if performance becomes issue
- Little wasted work - understanding still applies

**Decision 3: IDE-Generated Initialization**

*Choice:* Use STM32CubeMX generated initialization code

*Rationale:*
- Generated code is simple and functional
- Not worth taking over immediately
- Avoid work that doesn't add value now

*Future Enhancement:*
- Move initialization into I2C module later
- Add API support for configuration (bit rate, etc.)
- Currently bit rate set in IDE tool

### API Design

**Core Module Interface (Standard Pattern):**

```c
int32_t i2c_get_def_cfg(enum i2c_instance_id instance_id,
                        struct i2c_cfg* cfg);
int32_t i2c_init(enum i2c_instance_id instance_id,
                 struct i2c_cfg* cfg);
int32_t i2c_start(enum i2c_instance_id instance_id);
// Note: No i2c_run() - module is fully interrupt-driven
```

**I2C Operations API:**

```c
// Resource management
int32_t i2c_reserve(enum i2c_instance_id instance_id);
int32_t i2c_release(enum i2c_instance_id instance_id);

// I2C transactions (non-blocking)
int32_t i2c_write(enum i2c_instance_id instance_id,
                  uint32_t dest_addr,
                  uint8_t* msg_bfr,
                  uint32_t msg_len);

int32_t i2c_read(enum i2c_instance_id instance_id,
                 uint32_t dest_addr,
                 uint8_t* msg_bfr,
                 uint32_t msg_len);

// Status checking (polling model)
int32_t i2c_get_op_status(enum i2c_instance_id instance_id);
```

**Configuration Structure:**

```c
struct i2c_cfg {
    uint32_t transaction_guard_time_ms;  // Guard timer timeout
};
```

**Design Patterns:**

*Reservation System:*
- `i2c_reserve()` - Get exclusive access
- `i2c_release()` - Free for others
- Ensures only one user at a time
- Simple but effective

*Poll Model vs. Callback:*
- Poll: User calls `i2c_get_op_status()` to check completion
- Alternative: Callback function when done
- Poll model fine for super loop systems

*Non-blocking Guarantee:*
- All functions return immediately
- Actual work happens in interrupts
- Status polling shows progress

**Console Commands:**

```
i2c status     // Dump module state, counters, errors
i2c test       // Various test operations
  - reserve/release
  - write/read operations
  - get_op_status
  - etc.
```

### State Machine Design

**General Principles:**

*Developing States:*
1. Write down procedure steps (e.g., "read 6 bytes from sensor")
2. Find natural waiting points → these become states
3. Example state: "Waiting for ACK after sending address"

*Events:*
- **Expected Events:**
  - Interrupts with specific status bits
  - Timer expirations
- **Unexpected Events:**
  - Error interrupt bits
  - Unexpected interrupt in given state
  - Guard timer timeout
- Must handle both types

*Event Handling Strategy:*
- Some error handling may be state-dependent
- **Goal:** Common error handler for most cases
- Keeps code simpler and more maintainable
- This driver: Most errors → generate stop, disable HW, go to idle

*Guard Timers:*
- **Purpose:** Prevent state machine from getting stuck
- **Implementation:** Single timer covers entire transaction
- **Operation:**
  - Start: When read/write operation begins
  - Stop: When operation completes (success or error)
  - Timeout: Emergency cleanup, return to idle
- Add if any doubt about getting stuck

### Simplified State Machine for Read Operation

**States:**
- `IDLE` - Waiting for operation request
- `GENERATE_START` - Initiating start condition
- `SENDING_ADDRESS` - Transmitting address + R/W bit
- `READING_DATA` - Receiving data bytes
- (Back to `IDLE` when complete)

**Transitions and Actions:**

```
IDLE
  Event: i2c_read() called (API function)
  Actions:
    - Enable hardware
    - Set START bit in control register 1
    - Set bytes_to_read variable
    - Start guard timer
  Next State: GENERATE_START

GENERATE_START
  Event: Interrupt with SB (Start Bit) set
  Actions:
    - Write address + read bit to data register
  Next State: SENDING_ADDRESS

SENDING_ADDRESS
  Event: Interrupt with ADDR bit set (ACK received)
  Actions: (none - ready to receive)
  Next State: READING_DATA

READING_DATA
  Event: Interrupt with receive byte ready
  Actions:
    - Read byte from data register
    - Store in receive buffer
    - Decrement bytes_to_read
    - If bytes_to_read == 1:
        Clear ACK bit, set STOP bit
    - If bytes_to_read == 0:
        Cancel guard timer
        Disable hardware
  Next State: IDLE (when bytes_to_read == 0)

ANY_STATE (except IDLE)
  Event: Unexpected interrupt OR guard timer timeout
  Actions:
    - Set STOP bit (generate stop condition)
    - Cancel guard timer
    - Disable hardware
    - Record error
  Next State: IDLE
```

**Write Operation:**
Very similar pattern, different data direction.

**Key Points:**
- State = waiting point in protocol sequence
- Expected events drive normal flow
- Unexpected events have common cleanup path
- Guard timer provides safety net

### Special Considerations

**Special Instructions from Reference Manual:**

*Discovery:*
- Reference manual has cryptic "special instructions"
- For very short reads and last few bytes of read
- Not obvious, seems to conflict with earlier text
- Easy to miss

*Validation:*
- Checked HAL driver: implements special instructions
- Checked Linux driver: implements same way
- Identical implementation in both

*Decision:*
- Implement special instructions despite code working without them
- Assumption: Cover rare edge cases
- Better safe than sorry

*Lesson:*
- Read reference manual very carefully
- Comparison with other drivers valuable
- Some details are subtle and critical

**Error Handling Philosophy:**

*General Approach:*
- Generate stop condition (clean bus state)
- Cancel guard timer
- Disable hardware
- Return to idle state
- Record error for debugging

*Simplicity:*
- Most errors handled identically
- Minimal state-specific logic
- Easy to understand and maintain

*Robustness:*
- Guard timer catches stuck conditions
- ACK/NACK naturally drive transitions
- Defensive programming

### Key Takeaways from Design Lesson

**Professional Development:**
- Written requirements essential (even for small projects)
- MVP approach prevents over-engineering
- Iteration is normal and expected

**Modular Architecture:**
- Shared bus support needed for real products
- Reservation system simple but effective
- Non-blocking critical for super loop

**State Machines:**
- Natural fit for sequential protocols
- Clear structure aids understanding
- Systematic approach to error handling
- Makes code reviewable and testable

**Test & Debug:**
- Console commands enable field support
- Performance measurements catch long-term issues
- Logic analyzer essential during development

---

## Lesson 4: I2C Module Implementation and Demo

### Lesson Objectives

- Understand practical implementation of state machine
- See IDE configuration and code generation
- Learn software architecture layers
- Use logic analyzer for debugging

**Why This Matters:** Seeing theory applied in practice solidifies understanding and demonstrates professional embedded development techniques.

### IDE Configuration (STM32CubeMX)

**I2C3 Peripheral Setup:**

*Selection:*
- Enable I2C3 in peripheral list
- Initially chose I2C3 for pin availability on connectors
- Stayed with it even when using breadboard

*Pin Configuration:*
- SDA: PB4 (selected explicitly)
- SCL: PA8 (assigned automatically)
- "Pin" them to prevent reassignment during further configuration

*Parameters:*
- Use defaults:
  - Bit rate: 100 kbps (Standard Mode)
  - Clock stretching: Enabled
  - Other timing parameters: Auto-calculated

*Code Generation:*
- Project Manager → Advanced Settings
- Change I2C from HAL to LL (Low-Level)
- Generate code

**Generated Initialization Code:**

*Location:* `main.c` in `MX_I2C3_Init()` function

*Two Main Tasks:*

1. **GPIO Pin Setup:**
   - Enable GPIO port clock
   - Fill `LL_GPIO_InitTypeDef` structure:
     - Pin number
     - Mode (alternate function)
     - Pull-up configuration
     - Speed
     - Alternate function number
   - Call `LL_GPIO_Init()` to apply settings
   - Repeat for both SDA and SCL pins

2. **I2C Peripheral Setup:**
   - Enable I2C peripheral clock
   - Fill `LL_I2C_InitTypeDef` structure:
     - Clock speed
     - Duty cycle
     - Own address (not used in master mode)
     - Ack mode
     - Etc.
   - Call `LL_I2C_Init()` to apply
   - Enable clock stretching (separate call)

*Simplicity:*
- Very straightforward structure-based approach
- LL functions handle register writes
- Not worth reimplementing immediately

### Software Architecture - Layered Design

**Layer 1: System Code & Libraries (Gray)**

*Power-up Code:*
- First instructions executed at power-on
- Minimal setup for C runtime
- Sets up interrupt vector table

*Standard C Libraries:*
- libc (newlib or newlib-nano - lightweight)
- libm (math library)

*C Runtime:*
- Compiler-specific low-level code
- Rarely thought about but included for completeness

**Layer 2: Hardware Access Libraries (Light Brown)**

*CMSIS (Cortex Microcontroller Software Interface Standard):*
- For CPU and core peripherals
- Interrupt controller (NVIC)
- Floating-point unit
- Other ARM Cortex-M features

*LL Library (Low-Level):*
- For non-core peripherals
- UART, I2C, GPIO, timers, etc.
- Register-level access with clean API

**Layer 3: IDE-Provided Startup Code (Light Green)**

*Generated by STM32CubeMX:*
- Based on peripheral configuration
- Initializes hardware before main()
- Calls initialization functions like `MX_I2C3_Init()`

*Execution Flow:*
- Power-up code → Clock setup → Peripheral init → main()

**Layer 4: Module Startup & Super Loop (Bright Blue)**

*Location:* `app_main.c`

*Responsibilities:*
- Initialize all modules (phase 1 and 2)
- Run super loop
- Hardware-independent coordination

*Pattern:*
```c
void app_main(void) {
    // Phase 1: Get configs and init
    i2c_get_def_cfg(instance, &cfg);
    i2c_init(instance, &cfg);
    // ... other modules ...

    // Phase 2: Start modules
    i2c_start(instance);
    // ... other modules ...

    // Super loop
    while (1) {
        console_run();
        tmphm_run();  // Uses I2C via API
        // ... other modules ...
    }
}
```

**Layer 5: Hardware-Dependent Modules (Bluish Gray)**

*Examples:*
- **I2C Module:** Uses LL APIs, provides hardware abstraction
- **TTYS Module:** UART interface wrapper
- **TMR Module:** Timer services

*Key Characteristic:*
- Use CMSIS or LL APIs (hardware-specific)
- Provide hardware-independent API to users
- **Value:** Hardware abstraction layer
- If moving to different MCU: Update these modules, higher layers unchanged

**Layer 6: Hardware-Independent Modules (Top)**

*Examples:*
- **tmphm Module:** Temperature/humidity sensor driver (covered in Lesson 5)
- **Console Module:** User interface
- **Command Module:** Command parsing

*Key Characteristic:*
- Use lower module APIs only
- No direct hardware access
- Portable across different MCUs
- Business logic layer

**Benefits of Layered Architecture:**
- Clear separation of concerns
- Hardware abstraction enables portability
- Reusable modules
- Testable components
- Professional structure

### I2C Module Implementation Details

**Key Data Structures:**

*State Enumeration:*
```c
enum i2c_state {
    I2C_STATE_IDLE,
    I2C_STATE_WRITE_GENERATING_START,
    I2C_STATE_WRITE_SENDING_ADDRESS,
    I2C_STATE_WRITE_SENDING_DATA,
    I2C_STATE_READ_GENERATING_START,
    I2C_STATE_READ_SENDING_ADDRESS,
    I2C_STATE_READ_READING_DATA,
};
```

*Instance State Structure:*
```c
struct i2c_state {
    struct i2c_cfg cfg;              // Configuration
    I2C_TypeDef* i2c_dev;            // Pointer to peripheral registers
    uint8_t* msg_bfr;                // Message buffer
    uint32_t msg_bytes_transferred;  // Bytes sent/received so far
    uint32_t msg_len;                // Total message length
    bool reserved;                   // Reservation flag
    enum i2c_state state;            // Current state
    enum i2c_errors last_error;      // Last error code
    // ... performance counters, etc.
};
```

*One instance per I2C peripheral (e.g., I2C1, I2C2, I2C3)*

**Core API Implementation:**

*i2c_reserve() Example:*
```c
int32_t i2c_reserve(enum i2c_instance_id instance_id) {
    // Validate instance
    if (instance_id >= I2C_NUM_INSTANCES)
        return ERROR_INVALID_INSTANCE;

    // Check if already reserved
    if (state[instance_id].reserved)
        return ERROR_BUSY;

    // Grant reservation
    state[instance_id].reserved = true;
    return SUCCESS;
}
```

*Simple but effective resource management*

**Interrupt Handling:**

*Interrupt Vector Table:*
- Contains weak symbols for ISRs
- Default implementation: Tight loop (bad if hit)
- Module overrides with actual implementation

*ISR Functions:*
```c
// Instance-specific handlers
void I2C1_EV_IRQHandler(void) {
    i2c_interrupt(I2C_INSTANCE_1, EVENT_INTERRUPT);
}

void I2C3_EV_IRQHandler(void) {
    i2c_interrupt(I2C_INSTANCE_3, EVENT_INTERRUPT);
}

// Similar for error interrupts
```

*Common Handler:*
All ISRs call `i2c_interrupt()` with instance ID

**State Machine Implementation (Informal Style):**

*Characteristics:*
- Uses switch statement on state
- Called "informal" (vs. formal table-driven)
- Multiple switch statements in some cases

*Example:*
```c
void i2c_interrupt(enum i2c_instance_id inst, interrupt_type type) {
    uint32_t handled_bits = 0;
    uint32_t sr1 = LL_I2C_ReadReg(i2c_dev, SR1);

    switch (state[inst].state) {

    case I2C_STATE_WRITE_GENERATING_START:
        if (sr1 & I2C_SR1_SB) {  // Start bit generated?
            // Write address + write bit to data register
            LL_I2C_TransmitData8(i2c_dev, (addr << 1) | 0);
            state[inst].state = I2C_STATE_WRITE_SENDING_ADDRESS;
            handled_bits |= I2C_SR1_SB;
        }
        break;

    case I2C_STATE_WRITE_SENDING_ADDRESS:
        if (sr1 & I2C_SR1_ADDR) {  // Address acknowledged?
            // Clear ADDR by reading SR1 then SR2 (required by hardware)
            uint32_t dummy = LL_I2C_ReadReg(i2c_dev, SR1);
            dummy = LL_I2C_ReadReg(i2c_dev, SR2);
            state[inst].state = I2C_STATE_WRITE_SENDING_DATA;
            handled_bits |= I2C_SR1_ADDR;
        }
        break;

    // ... other states ...
    }

    // Check for unexpected status bits
    if (sr1 & ~handled_bits) {
        // Unexpected interrupt - error handling
        LL_I2C_GenerateStopCondition(i2c_dev);
        cancel_guard_timer(inst);
        disable_hardware(inst);
        state[inst].state = I2C_STATE_IDLE;
        state[inst].last_error = I2C_ERR_INTR_UNEXPECT;
    }
}
```

*Bit Tracking Technique:*
- Keep track of "handled" status bits
- At end, check if any unexpected bits set
- Catches unusual conditions
- Simple error detection

*Informal vs. Formal State Machines:*
- **Informal:** Switch statements, flexible
- **Formal:** Function tables, every state/event pair defined
- **Formal Advantage:** Forces consideration of all combinations
- **Informal Risk:** May miss some state/event cases
- Must be careful with informal approach

**Complexity Beyond State Diagram:**
- Diagram shows basic flow
- Code has more details:
  - Special instructions for short reads
  - Register sequence requirements (SR1 then SR2)
  - Exact timing of bit clearing
  - Error bit checking
- Diagram accurate but simplified

### Demo with Logic Analyzer

**Setup:**

*Logic Analyzer Configuration:*
- **Software:** Saleae Logic 2
- **Hardware:** Low-cost clone analyzer
- **Sampling Rate:** 2 MHz (had issues with higher rates)
- **Channels:**
  - Channel 0: SDA
  - Channel 1: SCL
  - Others: Hidden
- **Trigger:** Falling edge on SDA while SCL high (= Start condition)
- **Analyzer:** I2C protocol analyzer (auto-decodes)

*Console:*
- Bottom of screen
- Enter I2C test commands

**Demo 1: Successful Write Operation**

*Setup:*
- Sensor at address 0x44 connected
- Command to start measurement

*Console Commands:*
```
i2c test reserve 0         # Reserve I2C instance 0
i2c test write 0 0x44 0x2C 0x06  # Write command to sensor
```

*First Attempt Error:*
- Forgot to reserve → error message
- Shows reservation enforcement working

*After Reserve:*
```
i2c test reserve 0         # Reserve successful
i2c test write 0 0x44 0x2C 0x06  # Write successful
```

*Logic Analyzer Shows:*
1. **Trigger Point (T):** Start condition (green dot)
2. **Address + W:**
   - Value: 0x88 (0x44 << 1 | write_bit)
   - Decoded as: Address 0x44, Write
   - ACK received (SDA low on 9th clock)
3. **Data Byte 1:** 0x2C, ACK
4. **Data Byte 2:** 0x06, ACK
5. **Stop Condition:** Orange dot

*Timing:*
- Rising clock edges marked with arrows
- These are sampling points for data
- Visible confirmation of theory

**Demo 2: Successful Read Operation**

*Console Command:*
```
i2c test read 0 0x44 6  # Read 6 bytes from sensor
```

*Logic Analyzer Shows:*
1. **Start condition**
2. **Address + R:**
   - Value: 0x89 (0x44 << 1 | read_bit)
   - Decoded as: Address 0x44, Read
   - ACK received
3. **6 Data bytes:** All from slave to master
4. **Last byte:** NACK from master (SDA high)
   - Signals end of read (matches theory)
5. **Stop condition**

*Key Observation:*
- ACK/NACK bit: No arrow on rising clock edge
- Analyzer doesn't mark it the same way
- Actually helps identify ACK/NACK bit

**Demo 3: Address NACK Error**

*Console Command:*
```
i2c test read 0 0x11 2  # Try non-existent address 0x11
```

*Logic Analyzer Shows:*
1. **Start condition**
2. **Address + R:** 0x23 (0x11 << 1 | read_bit)
3. **NACK:** SDA stays high on 9th clock
   - No slave with this address
   - Analyzer marks as "NACK"
4. **Stop condition:** Immediate (no data transfer)

*Console Status Check:*
```
i2c test get_op_status 0
→ Module error: -7 (peripheral error)
→ Detailed error: 6 (address failure - AF)
```

*How It Works:*
- Hardware sets AF (address failure) bit in status register
- Interrupt occurs with error bit
- Error handler generates stop, reports error

**Demo 4: Performance Measurements**

*Console Command:*
```
i2c status
```

*Shows:*
- **Instance Info:** Reserved/free status
- **Last Operation:** Type, address, result
- **Error Counters:**
  - Address failures: 1 (from demo 3)
  - Reserve failures: 65535 (saturated)
  - Other errors: 0
- **Base Address:** Peripheral register address

*Reserve Failures Explanation:*
- Another module (tmphm) continuously trying to use I2C
- Blocked by our manual reservation
- Counter saturates at 16-bit max
- Normal behavior in this test scenario

*Value:*
- Long-term monitoring
- Catch intermittent issues
- Field debugging

### Key Implementation Insights

**Race Condition Prevention:**

*Problem:*
- `i2c_write()` / `i2c_read()` (base level) modify state variables
- `i2c_interrupt()` (interrupt level) modifies same variables
- Simultaneous access could corrupt data

*Solution:*
- `i2c_write()` / `i2c_read()` return immediately if not in IDLE
- In IDLE state: Interrupts disabled for that peripheral
- When operation starts: State changes, interrupts enabled
- No simultaneous access possible

*Clean Design:*
- No need for critical sections
- State machine structure prevents conflicts
- Simple and effective

**Guard Timer Weaknesses:**

*Current Implementation:*
- Single timer for entire transaction
- Same timeout value regardless of stage

*Problem:*
- Quick failures must wait full timeout
- Example: Start condition failure (quick) waits same time as full multi-byte read (slow)
- Timeout set for worst case

*Improvement:*
- State-specific timeout values
- Short timeout for start condition
- Longer timeout for data transfer
- More responsive error detection

**Reservation Scheme Weaknesses:**

*Issue 1: Honor System*
- Module only checks that someone has reserved
- Doesn't verify it's the correct user
- Misbehaving module could skip reserve, call read/write directly

*Issue 2: Forgotten Release*
- User module bug could fail to release
- I2C locked forever
- No recovery mechanism

*Issue 3: No Fairness*
- First to call `reserve()` after release gets it
- No queuing
- No prioritization

*Possible Improvements:*

1. **Reservation Key:**
   - `reserve()` returns unique ID
   - User must pass ID to `read()`/`write()`
   - Module verifies ID matches current reservation
   - Prevents unauthorized use

2. **Auto-Release:**
   - User specifies max time when reserving
   - Module auto-releases if not freed in time
   - Prevents permanent lockup
   - Requires timer management

3. **Queue System:**
   - Users request reservation (with callback)
   - Module grants in order
   - Callback when granted
   - Complex, possibly overkill

*Engineering Judgment:*
- Current system simple and effective
- Improvements add complexity
- Depends on application requirements
- Consider actual risks in your system

### Why This Implementation Matters

**Professional Development:**
- Logic analyzer essential for I2C development
- Console commands enable rapid testing
- Performance counters for long-term reliability
- Layered architecture enables incremental development

**Real-World Applicability:**
- Interrupt-driven typical for production
- Error handling simple and robust
- Hardware abstraction enables reuse
- Module pattern provides clean interfaces

**Learning Value:**
- Theory applied in practice
- Informal state machine implementation
- Debugging with logic analyzer
- Importance of test/debug infrastructure

---

## Lesson 5: Temperature and Humidity Module Design and Implementation

### Lesson Objectives

- Design device-specific driver using generic I2C module
- Implement state machine for sensor measurement cycle
- Handle CRC error detection for data integrity
- Demonstrate benefits of hardware abstraction

**Why This Matters:** Shows how low-level driver (I2C) enables higher-level functionality (sensor reading) with clean separation of concerns.

### Module Requirements

**Functional Requirements:**

1. **Background Sampling**
   - Continuously sample temperature and humidity
   - Sensor: Sensirion SHT31-D
   - No on-demand measurements (future enhancement)

2. **Default Sample Rate**
   - 1 sample per second (configurable)

3. **CRC Checking**
   - Verify data integrity using sensor-provided CRCs
   - Reject corrupted data

4. **Non-blocking API**
   - Get latest measurement at any time
   - Also get age of measurement

5. **Multiple Instances**
   - Support multiple sensors
   - Same or different I2C buses

**Non-Functional (Test & Debug):**

*Console Commands:*
- **Status:** Module state, last measurement
- **Test:** Various operations:
  - Trigger measurement
  - Test CRC calculation
  - Change sample time
  - Change measurement wait time
- **Get Measurement:** Retrieve latest data
- **Performance Measurements:** Sample counts, error counts

### Sensor Theory of Operation (SHT31-D)

**I2C Addressing:**

*ADDR Pin Control (from datasheet Table 7):*
- **ADDR pin low:** Address = 0x44
- **ADDR pin high:** Address = 0x45
- Only two choices (limited flexibility)

*Adafruit Board:*
- ADDR pin has pull-down resistor
- Default address: 0x44

*Implications:*
- Address conflicts possible
- Same issue mentioned in Lesson 2
- Need multiple I2C buses or hardware solutions for more than 2 sensors

**Measurement Commands (Datasheet Table 8):**

*Command Structure:*
- 2 bytes: MSB, LSB
- Different commands for different modes

*Repeatability Levels:*
- **High Repeatability:** Best accuracy/precision
- **Medium Repeatability:** Moderate
- **Low Repeatability:** Fastest/lowest power

*Clock Stretching:*
- **Enabled:** 0x2C 0x06 (high repeatability) - chosen for implementation
- **Disabled:** Different command codes

*Why High Repeatability with Clock Stretch:*
- Best data quality
- Clock stretch handled automatically by hardware
- Software doesn't need to do anything

**Measurement Procedure:**

*Step 1: Send Command*
- **I2C Write:** 2 bytes (command)
- **Transaction:** START + ADDR(W) + MSB + LSB + STOP
- Example: START + 0x88 + 0x2C + 0x06 + STOP

*Step 2: Wait for Measurement*
- Sensor performs measurement
- **Duration:** Up to 15ms for high repeatability (from Table 4)
- Software should wait at least this long

*Step 3: Read Result*
- **I2C Read:** 6 bytes
- **Transaction:** START + ADDR(R) + 6 bytes + STOP

*Clock Stretching Behavior (from datasheet):*

- **Without clock stretch command:**
  - If not ready: Sensor NACKs address
  - Master must retry later
  - Software complexity

- **With clock stretch command:**
  - Send: START + ADDR(R)
  - If not ready: Sensor pulls SCL low (stretches)
  - Master waits automatically
  - When ready: Sensor releases SCL
  - Data transfer proceeds
  - **No software action needed**

*Implementation Choice:*
- Use clock-stretch command
- Wait 15ms before reading (avoid stretch in normal case)
- If still not ready: Clock stretch handles it automatically

**Data Format (6 Bytes):**

From datasheet:
1. Temperature MSB
2. Temperature LSB
3. Temperature CRC
4. Humidity MSB
5. Humidity LSB
6. Humidity CRC

*Two measurement pairs, each with CRC*

**CRC-8 Calculation:**

*Parameters (from Datasheet Table 19):*
- **Polynomial:** 0x31 (x^8 + x^5 + x^4 + 1)
- **Initialization:** 0xFF
- **Final XOR:** 0x00 (no XOR)
- **Input:** 2 data bytes
- **Output:** 1 CRC byte

*Purpose:*
- Data integrity verification
- Detects transmission errors
- High probability of catching corruption

*Algorithm (standard CRC-8, code from web):*
```c
uint8_t crc8(uint8_t* data, uint32_t len, uint8_t polynomial) {
    uint8_t crc = 0xFF;  // Initialization value

    for (uint32_t i = 0; i < len; i++) {
        crc ^= data[i];  // XOR with data byte

        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80)  // MSB set?
                crc = (crc << 1) ^ polynomial;
            else
                crc = (crc << 1);
        }
    }

    return crc;  // No final XOR for this sensor
}
```

*Usage:*
```c
uint8_t temp_crc = crc8(&data[0], 2, 0x31);
if (temp_crc != data[2]) {
    // CRC mismatch - data corrupted!
}
```

*Notes:*
- Not the most efficient implementation
- Simple and clear
- Only processing a few bytes
- Efficiency not critical here

**Data Conversion:**

*Raw to Physical Units (from datasheet):*

**Temperature (°C):**
```
Temp_C = -45 + 175 * (raw_value / 65535)
```

**Temperature (°F):**
```
Temp_F = -49 + 315 * (raw_value / 65535)
```

**Relative Humidity (%):**
```
RH_percent = 100 * (raw_value / 65535)
```

*Avoiding Floating Point:*

Embedded systems often avoid floating point for:
- Code size
- Execution speed
- Determinism

*Integer Implementation:*
- Multiply by 10 to preserve one decimal place
- Store as integer

**Example - Temperature:**
```c
// Actual: 22.4°C
// Raw value: Let's say 30000

// Floating point version:
// temp_c = -45 + 175 * (30000 / 65535.0) = 22.4

// Integer version (multiply by 10):
temp_c_x10 = -450 + (1750 * raw_value) / 65535;
// Result: 224 (represents 22.4°C)
```

**Example - Humidity:**
```c
// Actual: 49.7%
// Raw value: Let's say 32500

// Integer version (multiply by 10):
rh_x10 = (1000 * raw_value) / 65535;
// Result: 497 (represents 49.7%)
```

*Order of Operations:*
- Multiply before divide to preserve precision
- Integer math requires care

### Design Decisions

**Use I2C Module for Communication**

*Benefit 1: Hides I2C Complexity*
- Protocol details
- Interrupt handling
- Register manipulation
- Start/stop conditions
- ACK/NACK handling

*Benefit 2: Hardware Abstraction*
- tmphm module portable to other MCUs
- Only I2C module needs changes for new hardware
- Sensor logic stays the same

*Benefit 3: Bus Sharing*
- Reservation system built-in
- Multiple modules can coexist
- Display module, sensor module, etc.
- No mutual awareness needed

*Benefit 4: Test & Debug Features*
- I2C console commands available
- Performance counters
- Status reporting
- Aids tmphm development

*Layered Architecture Value:*
```
tmphm module (sensor logic)
     ↓ uses API
I2C module (protocol handler)
     ↓ uses LL
Hardware registers
```

### API Design

**Configuration Structure:**

```c
struct tmphm_cfg {
    enum i2c_instance_id i2c_instance_id;  // Which I2C bus
    uint32_t i2c_addr;                      // Sensor address (0x44 or 0x45)
    uint32_t sample_time_ms;                // Sample interval (default 1000ms)
    uint32_t meas_time_ms;                  // Measurement wait (default 15ms)
};
```

**Measurement Structure:**

```c
struct tmphm_meas {
    int16_t temp_deg_c_x10;     // Temperature in 0.1°C units (can be negative)
    uint16_t rh_percent_x10;    // Humidity in 0.1% units (always positive)
};
```

*Examples:*
- 22.4°C stored as 224
- -10.5°C stored as -105
- 49.7% RH stored as 497

**API Functions:**

*Core Module Interface (Standard Pattern):*
```c
int32_t tmphm_get_def_cfg(enum tmphm_instance_id instance_id,
                          struct tmphm_cfg* cfg);
int32_t tmphm_init(enum tmphm_instance_id instance_id,
                   struct tmphm_cfg* cfg);
int32_t tmphm_start(enum tmphm_instance_id instance_id);
int32_t tmphm_run(enum tmphm_instance_id instance_id);  // Called from super loop!
```

*Application Interface:*
```c
int32_t tmphm_get_last_meas(enum tmphm_instance_id instance_id,
                            struct tmphm_meas* meas,
                            uint32_t* meas_age_ms);
```

*Difference from I2C Module:*
- tmphm HAS `run()` function (I2C doesn't)
- `run()` drives state machine via polling
- Called repeatedly from super loop

### State Machine Design

**Events - All Polling Based:**

*vs. I2C Module:*
- I2C: Interrupt-driven events
- tmphm: Polling-based events

*Event Types:*

1. **Periodic Timer Callback**
   - Timer expires → start new measurement
   - Only interrupt in this module

2. **i2c_reserve() Result**
   - Poll: Call `i2c_reserve()`, check return value
   - Success/failure

3. **i2c_get_op_status() Result**
   - Poll: Call `i2c_get_op_status()`, check return
   - In progress / success / failure

4. **Time Elapsed**
   - Calculate: `current_time - start_time`
   - Compare with threshold

*Polling in Super Loop:*
- `tmphm_run()` called very frequently
- Overhead low (just checks and returns quickly)
- Appropriate for super loop design

**State Machine States and Transitions:**

```
IDLE
  ↓ Event: Periodic timer expired (start next sample)
  → RESERVE_I2C

RESERVE_I2C
  Actions:
    - Call i2c_reserve()

  ↓ Event: Reserve failed
  → RESERVE_I2C (stay, keep trying - normal if bus busy)

  ↓ Event: Reserve success
  → WRITE_MEAS_COMMAND
    Actions:
      - Call i2c_write(addr, [0x2C, 0x06], 2)
      - Record time

WRITE_MEAS_COMMAND
  Actions:
    - Call i2c_get_op_status()

  ↓ Event: In progress
  → WRITE_MEAS_COMMAND (stay, keep waiting)

  ↓ Event: Failure
  → IDLE
    Actions:
      - Call i2c_release()
      - Try again next sample period

  ↓ Event: Success
  → WAIT_MEASUREMENT
    Actions:
      - Record measurement start time

WAIT_MEASUREMENT
  Actions:
    - Calculate elapsed time

  ↓ Event: elapsed < meas_time_ms
  → WAIT_MEASUREMENT (stay, keep waiting)

  ↓ Event: elapsed >= meas_time_ms
  → READ_MEAS_VALUE
    Actions:
      - Call i2c_read(addr, buffer, 6)

READ_MEAS_VALUE
  Actions:
    - Call i2c_get_op_status()

  ↓ Event: In progress
  → READ_MEAS_VALUE (stay, keep waiting)

  ↓ Event: Failure
  → IDLE
    Actions:
      - Call i2c_release()

  ↓ Event: Success, CRC invalid
  → IDLE
    Actions:
      - Discard data
      - Increment error counter
      - Call i2c_release()

  ↓ Event: Success, CRC valid
  → IDLE
    Actions:
      - Convert raw data
      - Save measurement and timestamp
      - Call i2c_release()
```

**Key Points:**

*No Guard Timers:*
- Relies on I2C module's guard timers
- I2C ensures operations don't hang forever
- tmphm trusts lower layer

*Alternative Guard Timer:*
- Could use sample period as quasi-guard
- If next sample timer fires while not IDLE → stuck detected
- Duration longer than ideal but simple

*Always Release I2C:*
- Critical: Release before returning to IDLE
- Every path that goes to IDLE includes `i2c_release()`
- Failure to release = permanent lockup

*Error Recovery:*
- Simple: Return to IDLE, try again next period
- Robust for transient errors
- Continuous failures need higher-level handling

### Implementation Highlights

**tmphm_run() Function:**

*Called from Super Loop:*
- Very frequently (many times per second)
- Drives state machine
- Returns quickly if nothing to do

*Structure:*
```c
int32_t tmphm_run(enum tmphm_instance_id inst) {
    switch (state[inst].state) {

    case TMPHM_STATE_IDLE:
        // Nothing to do (waiting for timer callback)
        break;

    case TMPHM_STATE_RESERVE_I2C:
        if (i2c_reserve(state[inst].cfg.i2c_instance_id) == SUCCESS) {
            result = i2c_write(
                state[inst].cfg.i2c_instance_id,
                state[inst].cfg.i2c_addr,
                measurement_command,  // [0x2C, 0x06]
                2
            );
            if (result == SUCCESS) {
                state[inst].state = TMPHM_STATE_WRITE_MEAS_COMMAND;
            }
        }
        // Else stay in this state, try again next call
        break;

    case TMPHM_STATE_WRITE_MEAS_COMMAND:
        status = i2c_get_op_status(state[inst].cfg.i2c_instance_id);
        if (status == IN_PROGRESS) {
            break;  // Keep waiting
        }
        if (status != SUCCESS) {
            // Failed - cleanup and retry next sample
            i2c_release(state[inst].cfg.i2c_instance_id);
            state[inst].state = TMPHM_STATE_IDLE;
            break;
        }
        // Success - start waiting for measurement
        state[inst].meas_start_time = tmr_get_ms();
        state[inst].state = TMPHM_STATE_WAIT_MEASUREMENT;
        break;

    case TMPHM_STATE_WAIT_MEASUREMENT:
        elapsed = tmr_get_ms() - state[inst].meas_start_time;
        if (elapsed >= state[inst].cfg.meas_time_ms) {
            result = i2c_read(
                state[inst].cfg.i2c_instance_id,
                state[inst].cfg.i2c_addr,
                read_buffer,
                6
            );
            if (result == SUCCESS) {
                state[inst].state = TMPHM_STATE_READ_MEAS_VALUE;
            }
        }
        break;

    case TMPHM_STATE_READ_MEAS_VALUE:
        status = i2c_get_op_status(state[inst].cfg.i2c_instance_id);
        if (status == IN_PROGRESS) {
            break;
        }

        i2c_release(state[inst].cfg.i2c_instance_id);  // Always release

        if (status != SUCCESS) {
            state[inst].state = TMPHM_STATE_IDLE;
            break;
        }

        // Check CRCs
        if (crc8(&buffer[0], 2, 0x31) != buffer[2] ||
            crc8(&buffer[3], 2, 0x31) != buffer[5]) {
            // CRC failure - discard measurement
            increment_crc_error_counter();
            state[inst].state = TMPHM_STATE_IDLE;
            break;
        }

        // Convert raw data to physical units (integer math)
        temp_raw = (buffer[0] << 8) | buffer[1];
        rh_raw = (buffer[3] << 8) | buffer[4];

        state[inst].last_meas.temp_deg_c_x10 =
            -450 + (1750 * temp_raw) / 65535;
        state[inst].last_meas.rh_percent_x10 =
            (1000 * rh_raw) / 65535;
        state[inst].last_meas_time = tmr_get_ms();

        state[inst].state = TMPHM_STATE_IDLE;
        break;
    }

    return SUCCESS;
}
```

*Implementation Notes:*
- Each state does its check and acts
- Returns quickly if nothing to do
- Advances state when ready
- Always releases I2C before IDLE

### Demo Scenarios

**Demo 1: Normal Operation**

*Logic Analyzer Shows:*
1. **Write Transaction:**
   - START + 0x88 (addr 0x44 + write bit)
   - Data: 0x2C, 0x06
   - STOP

2. **Delay:** ~16ms (slightly more than 15ms spec)

3. **Read Transaction:**
   - START + 0x89 (addr 0x44 + read bit)
   - 6 data bytes received
   - STOP

*Console Output:*
```
tmphm test get_last_meas 0
→ Temp: 22.4°C (raw: 224)
→ RH: 49.7% (raw: 497)
→ Age: 885ms
```

*Observations:*
- Clean transactions
- Age less than 1 second (sample period)
- Normal operation

**Demo 2: Clock Stretching**

*Setup:*
Reduce wait time to force clock stretching:
```
tmphm test meas_time 0 10  # Set wait to 10ms (too short)
```

*Logic Analyzer Shows:*
1. Write transaction (normal)
2. **Delay:** ~10ms (software wait)
3. Read attempt:
   - START + ADDR(R)
   - **Clock held low for ~3ms** (by sensor)
   - **Clock released** (measurement done)
   - Data transfer proceeds normally
4. STOP

*Observations:*
- Gap visible in timeline
- SCL stuck low during gap
- Automatically handled by hardware
- No software intervention needed
- Demonstrates clock stretching in action

**Demo 3: CRC Verification**

*Manual CRC Test:*

Read actual measurement bytes from last demo:
- Humidity bytes: 0x8F, 0x98
- CRC byte: (let's say 0xBB)

Test CRC calculation:
```
tmphm test crc8 0x8F 0x98
→ CRC: 0xBB (matches!)
```

Test with wrong byte:
```
tmphm test crc8 0x8F 0x9B  # Changed 0x98 to 0x9B
→ CRC: 0xE8 (mismatch - error would be detected)
```

*Demonstrates:*
- CRC calculation working correctly
- Would catch transmission errors
- Data integrity protection

**Demo 4: Serious Error - Stuck Bus**

*Scenario:*
What if we try to read measurement without sending command first?

*Setup:*
```
i2c test reserve 0         # Block tmphm from using bus
i2c test read 0 0x44 6     # Try to read (no command sent first)
```

*Logic Analyzer Shows:*
1. START condition
2. ADDR(R) byte sent
3. **Clock goes low and STAYS LOW**
4. **Never recovers**
5. Bus completely stuck

*Explanation:*
- Sensor not expecting read (no measurement started)
- Sensor holds clock low indefinitely
- Not designed for this scenario
- No timeout on sensor side

*Bus Status:*
- No further I2C communication possible
- Bus locked up
- Other devices can't use it

*Recovery:*
Manual intervention required - reset sensor via GPIO:
```
// Physically pulse sensor reset pin to ground
// (done manually in demo)

i2c test release 0         # Release reservation
// tmphm resumes normal operation
```

*Lessons Learned:*

1. **Hardware Can Misbehave:**
   - Sensors have unexpected failure modes
   - Not all documented
   - Testing reveals issues

2. **Need Recovery Mechanisms:**
   - **Software Reset:** Send I2C reset command (if supported)
   - **GPIO Reset:** Control sensor reset pin from MCU
   - **Watchdog Timer:** Reset MCU if stuck (resets everything)

3. **Production Considerations:**
   - Can't rely on manual intervention
   - Must design for recovery
   - Consider watchdog
   - Consider GPIO-controlled reset

4. **Testing is Critical:**
   - Try edge cases
   - Try error scenarios
   - Find issues early
   - Better in lab than field

### Error Handling Philosophy

**Guard Timers - Decision Points:**

*Current Implementation:*
- No tmphm-level guard timers
- Relies on I2C module timers
- Simple, fewer moving parts

*Alternatives:*

1. **Explicit Guard Timer:**
   - Start timer when leaving IDLE
   - Stop when returning to IDLE
   - Timeout → error recovery
   - **Pro:** Catches stuck conditions
   - **Con:** More complexity

2. **Sample Timer as Guard:**
   - If next sample starts while not IDLE → stuck
   - **Pro:** No extra timer
   - **Con:** Longer timeout (up to sample period)

*Engineering Judgment:*
- Depends on criticality
- Cost of failure
- Complexity tolerance
- Resource availability

**Continuous Failures - Response Options:**

*Detection:*
- Count consecutive failed measurements
- Track time since last success
- Threshold determines "continuous failure"

*Response Options:*

1. **Print Warnings**
   - Logs error messages
   - **Pro:** Simple
   - **Con:** Useless in field (no one watching)

2. **Send Alarm**
   - Alert to management system
   - **Pro:** Visibility
   - **Con:** Requires management infrastructure
   - **Applicability:** Only if system exists

3. **Software Reset**
   - Reset sensor via I2C command
   - Reset I2C peripheral
   - **Pro:** May fix transient issues
   - **Con:** May not work (if I2C stuck)

4. **Hardware Reset**
   - GPIO pulse to sensor reset pin
   - **Pro:** More reliable than software reset
   - **Con:** Requires hardware design (GPIO connection)

5. **MCU Reset**
   - Reset entire MCU (often resets board)
   - **Pro:** "Nuclear option" - usually fixes everything
   - **Pro:** Simple to implement
   - **Con:** Disrupts everything
   - **Common:** Often used as last resort

*Decision Factors:*
- How critical is this measurement?
- What else is affected by reset?
- What are failure modes?
- How rare is the scenario?

*Quote from Instructor:*
"Using the big hammer" or "Go big or go home"

*Reality:*
- Spend significant time on error handling
- Often more time than normal operation
- No clear correct answer many times
- Part of embedded engineering

### Why This Module Matters

**Layered Architecture Benefits:**

*Separation of Concerns:*
- tmphm: Sensor measurement logic
- I2C: Protocol and bus management
- Clear boundaries

*Portability:*
- Change MCU → update I2C module only
- tmphm unchanged
- Business logic preserved

*Testability:*
- Can test I2C independently
- Can test tmphm with I2C simulator
- Unit testing easier

*Maintainability:*
- Clear responsibilities
- Easier to understand
- Easier to modify

**Data Integrity:**

*Why CRC Matters:*
- I2C operates in electrically noisy environment
- EMI, power supply noise, etc.
- Transmission errors possible
- CRC catches corruption

*Real-World:*
- Not theoretical concern
- Actual systems see errors
- CRC prevents bad data propagation
- Critical for reliability

**State Machine Versatility:**

*Different Event Types:*
- I2C module: Interrupt-driven
- tmphm module: Polling-driven
- Both valid for different scenarios

*Super Loop Enables Polling:*
- `run()` called frequently
- Low overhead
- Simple event model
- Appropriate for coordination tasks

**Real-World Considerations:**

*Hardware Surprises:*
- Devices misbehave in unexpected ways
- Documentation may not cover all cases
- Testing reveals issues
- Need defensive design

*Recovery Design:*
- Detection is not enough
- Must have recovery mechanism
- Consider various levels:
  - Software retry
  - Software reset
  - Hardware reset
  - MCU reset

*Testing Value:*
- Edge cases reveal problems
- Error scenarios must be tested
- Logic analyzer invaluable
- Find issues in development, not field

---

## Summary and Best Practices

### Key Takeaways

**I2C Protocol Understanding:**
- Two-wire serial bus (SDA, SCL) with address-based device selection
- Open-drain signals enable multi-device sharing with simple hardware
- ACK/NACK provides built-in error detection
- Slower than SPI but more pin-efficient
- Master-slave architecture with master controlling bus
- Understanding open-drain is crucial for troubleshooting
- Clock stretching allows devices to request more time

**Driver Design Principles:**
- State machines are natural fit for sequential protocols
- Interrupt-driven design efficient for bare-metal systems
- Guard timers essential to prevent stuck conditions
- Comprehensive error handling for production quality
- Test/debug infrastructure (console commands, performance counters) critical
- Layered architecture provides hardware abstraction
- Module pattern enables clean interfaces and reusability

**Software Architecture:**
- Layered design provides hardware abstraction and portability
- Module pattern with standard API enables integration
- Super loop architecture works well for polling-based coordination
- Reservation systems enable resource sharing
- Non-blocking APIs essential for cooperative multitasking
- Clear separation between hardware-dependent and independent layers

**Development Practices:**
- Requirements drive design, even for small projects
- MVP approach prevents over-engineering and scope creep
- Iterative approach: theory → design → implement → test
- Reference manual study is ongoing throughout development
- Logic analyzer is indispensable tool for debugging
- Test commands enable flexible, rapid testing
- Comparison with other implementations provides validation

**Production Considerations:**
- Error recovery mechanisms must be designed in, not added later
- Guard timers prevent indefinite hangs
- Data integrity checking (CRC) catches communication errors
- Performance measurements catch intermittent issues
- Hardware reset capabilities important for stuck peripherals
- Watchdog timers provide last-resort recovery
- Edge case testing reveals real-world issues

### Course Value

**What This Course Provides:**

1. **Deep Understanding**
   - I2C protocol theory and practice
   - Why things work the way they do
   - Trade-offs in design decisions

2. **Production-Quality Code**
   - Examples suitable for real products
   - Industry-standard practices
   - Robust error handling

3. **Professional Design Patterns**
   - State machines for sequential protocols
   - Layered architecture
   - Module-based organization
   - API design principles

4. **Practical Debugging Skills**
   - Logic analyzer usage
   - Interpreting protocol signals
   - Finding and fixing issues

5. **Real-World Insights**
   - Error handling philosophy
   - Edge cases and failure modes
   - Recovery mechanisms
   - Engineering judgment

6. **Reusable Infrastructure**
   - Module pattern
   - Console commands
   - Performance measurements
   - Super loop framework

### Best Practices Demonstrated

**Requirements and Design:**
- Document requirements explicitly
- Define MVP scope
- Make and record design decisions
- Iterate and refine

**Implementation:**
- Use state machines for protocol drivers
- Implement comprehensive error handling
- Build in test/debug features from start
- Use hardware abstraction layers
- Follow consistent API patterns

**Testing and Debugging:**
- Use logic analyzer for protocol verification
- Implement console commands for manual testing
- Add performance measurements
- Test error scenarios, not just happy path
- Find issues early through systematic testing

**Error Handling:**
- Plan for failures
- Implement guard timers
- Add data integrity checks
- Design recovery mechanisms
- Consider all levels: retry, reset, restart

**Code Quality:**
- Clear, readable structure
- Consistent naming conventions
- Appropriate comments
- Production-ready, not just demo code

### Recommended Next Steps

**Immediate:**
1. Study the GitHub source code in detail
2. Experiment with the code on actual hardware
3. Try different sensors or devices
4. Modify and extend the code

**Intermediate:**
1. Implement additional I2C devices
2. Add combined format support (write+read)
3. Experiment with different bus speeds
4. Add more comprehensive error handling

**Advanced:**
1. Consider adding DMA support for higher throughput
2. Investigate formal state machine implementations
3. Apply these patterns to other protocols (SPI, UART)
4. Port to different MCU families

**Professional Development:**
1. Review I2C specification in depth
2. Study other driver implementations
3. Practice with logic analyzer on other protocols
4. Apply module patterns to your own projects

### Final Thoughts

**Quote from Course:**
"This is similar to what you would see in industry" - referring to code quality and structure.

**What Makes This Course Valuable:**
- Not just theory or just practice - complete integration
- Production quality, not toy examples
- Real hardware, real tools, real debugging
- Professional development practices
- Honest discussion of trade-offs and limitations

**Embedded Development Reality:**
- Spend significant time on error handling
- Hardware behaves unexpectedly
- Documentation has gaps
- Testing reveals surprises
- Engineering judgment required
- "Go big or go home" sometimes means MCU reset

**Key Philosophy:**
- Start with clear requirements
- Design before coding
- Test early and often
- Handle errors comprehensively
- Build in debug support
- Think about production, not just demo

---

## Appendix: Quick Reference

### I2C Signal Patterns

**Start Condition:**
- SDA falls while SCL is high
- Signals beginning of transaction

**Stop Condition:**
- SDA rises while SCL is high
- Signals end of transaction

**Data Sampling:**
- Data sampled on rising edge of SCL
- Data can change after falling edge of SCL

**ACK/NACK:**
- ACK: SDA low on 9th clock pulse
- NACK: SDA high on 9th clock pulse

### Common I2C Speeds

- **Standard Mode:** 100 kbps
- **Fast Mode:** 400 kbps
- **Fast Mode Plus:** 1 Mbps
- **High Speed:** 3.4 Mbps

### STM32 I2C Status Register Bits

**SR1 (Status Register 1):**
- **SB:** Start bit generated
- **ADDR:** Address sent/matched
- **BTF:** Byte transfer finished
- **TXE:** Transmit buffer empty
- **RXNE:** Receive buffer not empty
- **AF:** Acknowledge failure
- **BERR:** Bus error

### Module API Pattern

```c
// Get default configuration
module_get_def_cfg(instance, &cfg);

// Phase 1 initialization
module_init(instance, &cfg);

// Phase 2 initialization
module_start(instance);

// Called from super loop (if needed)
module_run(instance);
```

### Console Command Examples

**I2C Module:**
```
i2c status                    # Show module status
i2c test reserve 0            # Reserve instance 0
i2c test write 0 0x44 0x2C 0x06  # Write to address 0x44
i2c test read 0 0x44 6        # Read 6 bytes from 0x44
i2c test get_op_status 0      # Check operation status
i2c test release 0            # Release instance 0
```

**tmphm Module:**
```
tmphm test get_last_meas 0    # Get latest measurement
tmphm test meas_time 0 15     # Set measurement wait to 15ms
tmphm test crc8 0x8F 0x98     # Test CRC calculation
```

---

*This comprehensive summary was created by analyzing Gene Schrader's excellent I2C course transcripts and source code. The course demonstrates professional-grade embedded software development practices and is highly recommended for anyone working with I2C or bare-metal embedded systems.*

**Course Resources:**
- Video course available on YouTube
- Source code on GitHub
- Course materials (I2C spec, datasheets) on GitHub
- Hardware: Nucleo-F401RE, Adafruit SHT31-D, logic analyzer

**Target Audience:**
- Embedded systems engineers
- Students learning embedded programming
- Anyone working with I2C peripherals
- Developers seeking production-quality code examples
