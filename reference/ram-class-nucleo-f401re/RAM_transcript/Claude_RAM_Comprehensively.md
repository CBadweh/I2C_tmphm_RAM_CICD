# Reliability, Availability, and Maintainability (RAM) Course - Comprehensive Summary

## Overview
This comprehensive summary covers the 7-lesson video course on Reliability, Availability, and Maintainability (RAM) theory and practice on STM32 microcontrollers by Eugene R. Schroeder. The course provides field-grade implementations of critical embedded systems techniques for handling faults and ensuring system reliability.

---

## Lesson 1: Introduction to the Course

### Objective
Introduce the RAM course structure, objectives, prerequisites, and development environment.

### Theory and Concepts

#### RAM Terminology
- **Reliability**: System's ability to function without failure
- **Availability**: Ability to function at any point in time (measured as % uptime)
- **Maintainability**: Ease with which problems can be corrected

#### Course Scope
- 7 videos, ~3 hours total runtime
- Focuses on embedded systems, particularly important for handling rare catastrophic events
- Separates university projects from real-world products
- Software-oriented (hardware also plays important role in RAM/safety)

### Key Ideas

1. **Target Audience**
   - Embedded professionals seeking RAM techniques
   - Students wanting broader embedded systems perspective
   - Requires C programming beyond beginner level
   - Builds on bare-metal embedded knowledge

2. **Course Techniques Covered**
   - Lightweight Logging (circular buffer flight recorder)
   - Fault Handling (panic mode processing)
   - Watchdogs (monitoring software health)
   - Stack Overflow Protection (memory safety)
   - Asserts and Audits (design discussion only)

3. **Software Architecture**
   - Based on super loop and module pattern
   - All new modules conform to established API
   - Modules provide debug/test capabilities
   - Field-grade designs suitable for production

### Important Notes

- **Hardware Used**: STM32 Nucleo F401RE board with Adafruit temp/humidity sensor
- **Software**: Free tools including STM32CubeIDE, Python for formatting tool
- **GitHub Repos**:
  - Source code repo (bare metal + RAM additions)
  - Course materials repo (documentation + IDE project zip)
- **Build Environment**: Windows 10 development (tools support Linux/MacOS)
- All code above purple line in architecture diagram is custom; below is vendor/standard libraries

---

## Lesson 2: Background on RAM

### Objective
Establish theoretical foundation and terminology for reliability, availability, and maintainability concepts.

### Theory and Concepts

#### Facts of Software Life

1. **Bugs are Inevitable**
   - Systems with complexity will almost always have bugs
   - New features produce new bugs
   - Bug fixes sometimes create new bugs ("whack-a-mole problem")
   - Products ship with known non-serious bugs
   - Unknown bugs are the real concern

2. **Defensive Programming**
   - Weaknesses in defensive programming ARE bugs
   - External devices/systems won't always behave as expected
   - Users won't behave as expected
   - Code must handle unexpected/wrong behavior reasonably
   - Analysis of potential failures is critical even for non-safety-critical systems

#### Key Definitions

**Reliability**
- Ability to function without failure
- Measured in failures per unit time (MTBF - Mean Time Between Failures)
- Hard to apply MTBF to software (no wear-out)
- This course assumes failures will occur and focuses on handling them

**Availability**
- Ability to function at any point in time
- Measured as % time system is functioning/up
- Affected by MTBF and MTTR (Mean Time To Repair)
- Course focuses on detection and automatic repair to reduce MTTR

**Maintainability**
- Ease with which problems can be corrected

Two perspectives:
1. Customer viewpoint: Features/tools to diagnose configuration, installation, hardware problems
2. Developer viewpoint: Features/tools to diagnose and fix software bugs

**Other Terms**
- **Safety**: Ability to not harm people, environment, or assets (not main focus)
- **Resiliency/Robustness**: Ability to function in face of adversity (relates to reliability/availability)
- **Non-functional Requirements**: How system must operate (reliability, availability, maintainability, security, safety)

### Key Ideas

#### Fault Handling Actions

1. **Detect Fault**
   - Addresses availability (must know when something is wrong)
   - Techniques: Watchdogs, CPU exceptions, application checks
   - Worst field events: system malfunctioning but doesn't know it

2. **Collect Information**
   - Addresses maintainability (helps developers diagnose/fix)
   - Data: Return address, CPU registers, fault registers, circular buffer logs

3. **Perform Recovery**
   - Policy depends on system nature and what's practical
   - **Failsafe Policy**: Shut down safely, stay idle, requires human intervention (safety-critical)
   - **Automatic Recovery**: Hardware/software reset, restart operation (availability-critical)

#### Fault Handling Design Principles

- Fault handling code must be highest quality
- Keep fault handling simple (not a place to be clever)
- Full hardware reset is simplest single recovery action
- Partial recovery (e.g., reinitialize I2C) requires escalation design
- Sometimes must avoid full reset - escalate only if needed

#### Panic Mode Operation

When fault detected, system integrity is questionable:
- Disable interrupts for all future operations
- Reset stack pointer to initial value
- Use polling (not interrupts/DMA) for hardware
- Special panic versions of console/flash output functions (polling, blocking)
- Reinitialize peripherals if needed
- Hardware watchdog as backup if fault handling gets stuck

### Implementation Concepts

**Maintainability Support (Non-Fault Scenarios)**
- Console with commands (window into system)
- Multi-level logging per module
- USB drive logging for field systems
- Performance Measurements (PMs): Counters for unusual events, alarms, displayed via console

### Demo Examples
None in this lesson (theory only)

### Discussion Prompts

**Q1: Why is reliability best during maintenance-only phase?**
- Only bug fixes (no new features) = fewer new bugs introduced
- Overall bug count decreases over time
- Seen in products where feature development stopped

**Q2: 99.999% (5-nines) availability = how much downtime/year?**
- Calculation: 365 days × 24 hrs × 60 min × 0.00001 = 5.25 minutes/year
- Often expected for communication network equipment
- Achieved using redundant hardware with hot-swapping

**Q3: System running at 50% capacity - availability treatment?**
- Pro-rate availability: n minutes at 50% = n/2 minutes downtime
- Important for service agreements
- Large distributed systems: stage updates to eliminate downtime

**Q4: All comm links down - treat as fault?**
- Normally no (assume external problem)
- But if system idle anyway, could reset after timeout (e.g., 15 minutes)
- Hedge against internal problem being the cause

---

## Lesson 3: Lightweight Logging (LWL)

### Objective
Implement low-overhead logging system for recording software activity without affecting operation, especially for post-fault analysis.

### Theory and Concepts

#### Lightweight Logging (LWL) Definition
- Mechanism to record software activity using minimal CPU time, real-time, and memory
- Objective: Enable on production systems running in field without affecting operation
- Ideally enabled always as part of normal product operation
- "Never know when you want logs - ideal is to always have them"

#### Major Use Case
- See what system was doing immediately before fault (watchdog trigger, exception, etc.)
- Comparable to aircraft flight recorder ("black box")
- Offline formatting tools decode raw data using source code

### Requirements

**Developer Usage**
- printf-like API (familiar to C programmers)
- Simple to add log statements to files
- Usable at base level and in interrupts (bare metal)

**Low Overhead Achievements**
- No runtime formatting of log messages (integers to text, etc.)
- Format strings NOT stored on system (no flash waste)
- Formatting done offline by Python tool
- Support integer-like types only (int, enum, bool, pointers)
- Strings and floats not required
- User specifies bytes needed per parameter (save circular buffer space)

**General Requirements**
- No source file editing during build (no preprocessor tricks)
- Uses macros (limits flexibility but simpler than build-time code generation)
- Non-blocking (super loop requirement)
- Support for test/debug (console commands)
- Storage to flash handled outside LWL module (fault module does this)

### Implementation

#### LWL ID Management
Each file defines ID range with hash-defines:
```c
#define LWL_BASE_ID 20
#define LWL_NUM 10
// File can use IDs 20-29
```

#### LWL Statement Syntax
```c
LWL("State=%u mask=0x%04x\n",
    3,                          // Total argument bytes (1+2)
    LWL_1(state_init),         // 1 byte for enum
    LWL_2(mask_un16));         // 2 bytes for uint16
```

Macros: LWL_1, LWL_2, LWL_3, LWL_4 specify byte count per argument

#### Data Structure Design

**Circular Buffer in RAM**
- Header area: buffer size, put index
- Put index points to next write location (after last record)
- May point mid-record if partial overwrite occurred

**Single Record Format**
```
| LWL_ID (1 byte) | Param1 (n bytes) | Param2 (m bytes) | ...
```
Parameters stored big-endian format

**Viewing Logs**
1. Dump LWL data in raw hexadecimal text (includes header with put index)
2. Decode offline using Python formatting tool (log_format.py)
3. Tool searches source code for LWL statements to get format strings
4. Tool uses put index to find oldest LWL record (handles wrap-around)

### Code Snippets

#### API (lwl.h)
```c
// Core functions
int32_t lwl_start(void);
void lwl_rec(uint8_t id, int32_t num_arg_bytes, ...);
void lwl_enable(bool on);
void lwl_dump(void);
uint8_t* lwl_get_buffer(uint32_t* len);

// Macro using __COUNTER__ to generate IDs
#define LWL(fmt, num_arg_bytes, ...) LWL_CNT(__COUNTER__, fmt, num_arg_bytes, ##__VA_ARGS__)

// Argument macros (convert to bytes)
#define LWL_1(a) (uint32_t)(a)
#define LWL_2(a) (uint32_t)(a) >> 8,  (uint32_t)(a)
#define LWL_3(a) (uint32_t)(a) >> 16, (uint32_t)(a) >> 8,  (uint32_t)(a)
#define LWL_4(a) (uint32_t)(a) >> 24, (uint32_t)(a) >> 16, (uint32_t)(a) >> 8, (uint32_t)(a)
```

#### Critical Section in lwl_rec()
```c
// Start critical section to prevent concurrent logging
__disable_irq();

// Write LWL ID
lwl_data.buff[lwl_data.idx_put] = id;
lwl_data.idx_put = (lwl_data.idx_put + 1) % LWL_BUFFER_SIZE;

// Write argument bytes
for (int i = 0; i < num_arg_bytes; i++) {
    lwl_data.buff[lwl_data.idx_put] = va_arg(ap, uint32_t);
    lwl_data.idx_put = (lwl_data.idx_put + 1) % LWL_BUFFER_SIZE;
}

__enable_irq();
// End critical section
```

### Demo Example

**Goal**: Show LWL logging during normal operation and after test command

**Why Important**: Demonstrates:
- Continuous logging with minimal overhead
- Time context (uptime seconds, 100ms ticks)
- Application-specific logs (temperature/humidity measurements)
- Post-test analysis

**Demo Flow**:
1. Reset board, observe temp/humidity sampling logs
2. Execute `lwl status` - shows put index changing as logging occurs
3. Execute `lwl test` - generates 4 test logs then disables logging
4. Execute `lwl dump` - outputs raw hexadecimal data
5. Run `log_format.py` on dumped data
6. View formatted output showing:
   - Uptime ticks and seconds
   - Temperature/humidity measurement starts/ends
   - Test log statements (last entries before logging disabled)

**Related Code**:
Timer module logs every 1 second (uptime) and 100ms (tick):
```c
// In SysTick_Handler (1ms interrupt)
if (uptime_ms % 1000 == 0) {
    uptime_s++;
    LWL("uptime_s=%lu\n", 4, LWL_4(uptime_s));
}
if (uptime_ms % 100 == 0) {
    LWL("tick_100ms\n", 0);
}
```

Temperature/humidity module logs:
```c
LWL("start meas\n", 0);  // When starting measurement
LWL("temp=%.1f hum=%.1f\n", 8, LWL_4(temp), LWL_4(humidity));  // On completion
```

### Important Notes

1. **__COUNTER__ Macro**: Special GCC feature - increments each time accessed, used to auto-generate LWL IDs
2. **Critical Section Size**: Trade-off between minimizing disabled-interrupt time and simplicity
3. **Circular Buffer Wrap**: Modulo operation handles automatic wrap-around
4. **Formatting Tool**: Python script `log_format.py` searches source directory for LWL statements

### Discussion Prompts

**Q1: How to improve LWL if build system could modify source?**
- No hash-defines for ID management (system handles it)
- No manual num_arg_bytes parameter (system calculates)
- Optimize record creation (inline assembly, special cases)
- More efficient handling of zero-argument logs

**Q2: Real-time LWL viewer implementation?**
- Write raw LWL data to UART/serial in addition to circular buffer
- Laptop program reads serial port, decodes, and displays
- Advantage: Compact data format = minimal serial bandwidth

**Q3: Avoid filling buffer with same repeated message?**
- Keep last LWL ID in header
- Discard new record if ID matches previous
- Option: Count discarded records
- Insert special record with discard count when different ID appears
- Example: "Previous message occurred 1000 times"

**Q4: Timestamp with every log record?**
- Advantage: Precise time for every log
- Disadvantage: Wasted space for logs where timestamp not useful
- Compromise: Periodic timestamp logs (done in this implementation)
- Could use 2-byte millisecond timestamps for intervals

---

## Lesson 4: Fault Handling and Fault Module

### Objective
Implement comprehensive fault handling module as the core software for RAM course, handling all fault types and saving fault data to flash.

### Theory and Concepts

#### Fault Module Purpose
- Central handler for all fault types regardless of detection method
- Collects fault data and saves to flash for post-mortem analysis
- Implements panic mode operation
- Coordinates with flash module for non-volatile storage

### Requirements

**Supported Fault Types**
1. CPU-detected faults:
   - Invalid pointer usage
   - Invalid memory operations (write to read-only, stack guard)
   - Integer divide by zero
   - Illegal instruction

2. Watchdog trigger

3. Application-detected faults (serious data structure corruption)

**Fault Data Collection**
Write to flash AND console:
- Fault type
- Program counter and stack pointer (if available)
- Selected CPU registers
- ARM system registers containing fault information
- Lightweight log buffer (shows pre-fault activity)

**Flash Management**
- If flash already contains fault data, do NOT overwrite
- Still write to console for current fault
- Prevents flash wear-out in continuous reboot scenarios

**Recovery Action**
- After writing fault data, reset MCU
- Recovery type (failsafe vs. automatic) depends on post-reset behavior

**Offline Analysis**
- Formatting tool (Python) decodes raw binary fault data
- Makes data human-readable with function names, line numbers

### MCU Reset Mechanism

**Reset Pin Characteristics**
- Both input (external reset button) and output (MCU-initiated)
- Uses open-drain/open-collector hardware technique
- MCU-initiated reset can reset external hardware (important feature)

**Reset Line Connections**
- MCU with pull-up resistor
- Reset button
- External devices (temp/humidity sensor, I/O expander)
- Voltage supervisor (monitors power, holds reset until stable)

**Why External Device Reset Matters**
- Ensures I/O expander outputs go off safely
- Complex devices can get stuck, only hardware reset recovers
- Seen in I2C devices that require reset to recover from lock-up

### Implementation

#### Panic Mode Design
When fault detected:
1. Disable interrupts for all operations
2. Reset stack pointer to top of RAM (ensure good stack)
3. Use polling (not interrupts/DMA) for hardware
4. Special panic versions of console/flash functions (blocking, polling)
5. Disable MPU (precaution)
6. Hardware watchdog as backup (in case fault handling gets stuck)

#### Fault Data Format
Binary format organized in sections:
1. **Generic Fault Data Section**
   - Magic number + length
   - Fault type and parameter
   - Register values (R0-R3, R12, LR, PC, xPSR)
   - System fault registers (HFSR, CFSR, MMFAR, BFAR, etc.)

2. **LWL Buffer Section**
   - Magic number + length
   - Complete lightweight log buffer

3. **End Marker Section**
   - Magic number (indicates complete data set)

Section sizes must be multiple of minimum flash write size (8 or 16 bytes)

#### Exception Processing (ARM Cortex-M4)

**Vector Table**
- Exception number indexes into vector table to get handler address
- Index 0: Initial stack pointer value
- Exceptions 1-15: CPU faults and system exceptions
- Exceptions 16+: Peripheral interrupts

**Key Exceptions**
- HardFault (3)
- MemManage (4): MPU violations, stack guard hits
- BusFault (5)
- UsageFault (6): Divide by zero, illegal instruction
- Unexpected interrupts from unused peripherals

**Exception Stack Frame**
When exception occurs, CPU automatically pushes to stack:
- R0-R3 (general purpose registers)
- R12 (general purpose)
- LR (Link Register / R14)
- PC (Return address / R15)
- xPSR (Processor Status Register)
- Possibly FPU registers if active
- Padding word if needed for 8-byte alignment

**Fault Handler Flow**
1. Initial handler (assembly) saves stack pointer to R0
2. Assembly handler resets stack pointer to safe value
3. Assembly handler jumps to C secondary handler (fault_exception_handler)
4. C handler validates saved stack pointer
5. If valid, copies exception stack frame to fault data structure
6. Collects additional fault information from system registers
7. Calls common fault handler

### Flash Module

#### Writing to Flash Requirements
- Must erase first (can't overwrite like RAM)
- Erase operates on pages/sections/blocks (variable sizes on STM32)
- Write must be on even boundary (multiple of 8 or 16 bytes)
- Write data must be multiple of 8 or 16 bytes

#### Flash API
```c
int32_t flash_start(void);
int32_t flash_panic_erase_page(uint32_t page_num);
int32_t flash_panic_write(uint32_t addr, uint32_t len_bytes, const uint8_t* data);
```

#### Flash Page Reservation (Linker Script)
For STM32F401RE:
- Last page too large, so use 2nd page after vector table
- Modified linker script to create 3 flash sections:
  1. ISR_VECTOR (16KB) - Vector table
  2. FAULT_DATA (16KB) - Fault data storage
  3. FLASH (480KB) - Remaining flash for code/data

### Code Snippets

#### Fault Type Enumeration (fault.h)
```c
enum fault_type {
    FAULT_TYPE_WDG = 1,        // Watchdog triggered
    FAULT_TYPE_EXCEPTION,      // CPU exception
};
```

#### Fault Data Structure
```c
struct fault_data {
    uint32_t magic;           // Section identifier
    enum fault_type type;     // Fault type
    uint32_t fault_param;     // Type-specific parameter
    uint32_t r0, r1, r2, r3;  // General registers
    uint32_t r12;             // General register
    uint32_t lr;              // Link register
    uint32_t pc;              // Program counter
    uint32_t xpsr;            // Status register
    uint32_t hfsr;            // HardFault Status
    uint32_t cfsr;            // Configurable Fault Status
    uint32_t mmfar;           // MemManage Fault Address
    uint32_t bfar;            // BusFault Address
    // ... additional system registers
};
```

#### Exception Handler (Assembly - startup file)
```asm
Default_Handler:
    MRS r0, MSP                  ; Move Main Stack Pointer to R0 (1st argument)
    LDR r1, =_estack            ; Load initial stack pointer value
    MSR MSP, r1                  ; Reset stack pointer to safe value
    B fault_exception_handler    ; Branch to C handler
```

#### Exception Handler (C)
```c
void fault_exception_handler(uint32_t sp)
{
    // Enter panic mode
    __disable_irq();
    ARM_MPU_Disable();

    // Collect fault data
    fault_data.type = FAULT_TYPE_EXCEPTION;
    fault_data.fault_param = (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk);

    // Validate and copy exception stack frame
    if (sp >= RAM_START && sp <= RAM_END) {
        memcpy(&fault_data.r0, (void*)sp, sizeof(exception_stack_frame));
    }

    // Collect system fault registers
    fault_data.hfsr = SCB->HFSR;
    fault_data.cfsr = SCB->CFSR;
    fault_data.mmfar = SCB->MMFAR;
    // ...

    common_fault_handler();
}
```

#### Common Fault Handler
```c
static void common_fault_handler(void)
{
    // Record fault data sections
    record_fault_data(&fault_data, sizeof(fault_data));
    record_lwl_buffer();
    record_end_marker();

    // Reset MCU
    NVIC_SystemReset();
}
```

#### Flash Write with Erase Check
```c
static void record_fault_data(void* data, uint32_t len)
{
    bool write_flash = true;

    // Check if flash already has fault data (magic number present)
    if (*(uint32_t*)FAULT_DATA_FLASH_ADDR == FAULT_DATA_MAGIC) {
        write_flash = false;  // Don't overwrite existing data
    }

    if (write_flash) {
        if (data_offset == 0) {
            flash_panic_erase_page(FAULT_DATA_PAGE);
        }
        flash_panic_write(FAULT_DATA_FLASH_ADDR + data_offset, len, data);
        data_offset += len;
    }

    // Always write to console
    console_panic_dump_hex(data, len);
}
```

### Demo Example

**Goal**: Trigger fault using bad pointer, analyze fault data

**Why Important**:
- Demonstrates complete fault handling flow
- Shows fault data collection and storage
- Proves formatting tool decodes data correctly
- Illustrates flash preservation on subsequent faults

**Demo Flow**:
1. Execute `fault test pointer` command - attempts to write to invalid address (0xFFFFFFFF)
2. System detects fault, dumps data to console, resets
3. Copy console output to file (raw1.txt)
4. Run formatting tool: `python log_format.py <source_dir> <raw_file>`
5. Analyze decoded output:
   - Fault type: EXCEPTION (2)
   - Fault parameter: 3 (HardFault exception number)
   - Registers: R2 contains 0xBAD (value being written)
   - PC: 0x800C974 (exact line in fault test function)
   - LWL trace: Shows system activity before fault
6. Verify flash storage: `fault data` command dumps from flash
7. Test no-overwrite: `fault data erase` clears flash for next fault

**Related Code** (fault.c test command):
```c
static int32_t cmd_fault_test_pointer(void)
{
    uint32_t bad_value = 0xBAD;
    *(volatile uint32_t*)0xFFFFFFFF = bad_value;  // Triggers fault
    return 0;  // Never reached
}
```

### Important Notes

1. **Linker Script Modification**: Required to reserve flash page for fault data
2. **Assembly Handler**: Minimal assembly code collects stack pointer, resets to safe value, jumps to C
3. **Stack Validation**: Check SP within valid RAM range before dereferencing
4. **Flash Wear Prevention**: Don't overwrite existing fault data (continuous reboot protection)
5. **Formatting Tool**: Python `log_format.py` needs source directory to find LWL statements
6. **Console vs Flash**: Both receive fault data, but console always written, flash conditionally

### Discussion Prompts

**Q1: Store multiple fault data dumps in flash?**
- If page can hold multiple instances, append faults sequentially
- Erase logic more complex (when page full)
- Could reserve multiple pages for more history
- Trade-off: Flash space vs. historical fault data

**Q2: Stack backtrace alternatives (since ARM difficult)?**
- Search stack backwards for valid flash addresses (likely return addresses)
- Save first 10 found values in fault data
- Formatting tool converts addresses to function names + line numbers
- Manual analysis to reconstruct backtrace

**Q3: Entire RAM image dump analysis?**
- Linux equivalent: Core dump file
- Debuggers (GDB) can offline-analyze core dumps at source level
- Similar approach for MCU with sufficient storage
- Author implemented this on large embedded system - "invaluable"

**Q4: Resetting stack pointer too drastic?**
- Simple and safe, but potentially wasteful
- Alternative 1: Check current SP has sufficient remaining stack
- Alternative 2: Reserve separate RAM for fault handling stack
- Completely avoid touching normal stack

---

## Lesson 5: Watchdogs

### Objective
Implement watchdog system to detect faults in software execution and trigger recovery, crucial for safety and availability.

### Theory and Concepts

#### Watchdog Definition
- Software or hardware mechanism to detect software faults
- Software must periodically "feed" watchdog (signal "alive and well")
- If software doesn't feed within timeout, watchdog "triggers"
- Trigger indicates something wrong in system

#### Watchdog Types

**Software Watchdog**
- Implemented in code
- Can take software actions when triggered:
  - Save system state information
  - Controlled shutdown
  - Call fault module

**Hardware Watchdog**
- Dedicated MCU peripheral
- Typically just does immediate hardware reset ("boom reset")
- Some support "early warning interrupt" before reset
- STM32F401RE doesn't have early warning feature

#### Hardware Watchdog Features (Varies by MCU)

**Clock Type**
- Trade-off between accuracy and reliability
- More accurate clocks less reliable in fault conditions

**Timeout Types**
1. **Simple Timeout**: Feed before time expires
2. **Windowed Timeout**: Feed within precise time window (not too early, not too late)

#### System Initialization Handling
- Initialization different from steady-state
- System can get stuck before infrastructure running
- May need special handling for initialization watchdog
- This course: Allows N failed initialization attempts before giving up

### Watchdog Usage Strategy

**Multiple Software Watchdogs**
- Each ensures specific critical work is performed
- Not just that code executes, but that WORK gets done
- Examples:
  - Watchdog for reading inputs every 100ms
  - Watchdog for control loop writing outputs
  - Watchdog for main loop execution

**Key Principle**: "Watchdogs ensure critical work is done, not simply that code executes"

**Hierarchy**
- Single hardware watchdog checks that software watchdogs are working
- Hardware watchdog is "checker of the checkers"
- If software watchdog system fails, hardware watchdog resets MCU

### Requirements

**Software Watchdog Module**
- Support multiple independent software watchdogs
- Monitor different system parts
- Runtime registration (no compile-time dependencies)
- Single module can register for trigger notifications (fault module)

**Hardware Watchdog Usage**
- Backup only (when software watchdogs fail)
- Support single initialization watchdog
- After N consecutive failed inits, disable watchdog (allow system up for debug)
- N can be set to infinity (always try)

**Module Requirements**
- Super loop compatible (standard API, non-blocking)
- Logging on errors
- Console commands for status and testing

### Implementation

#### Conceptual Flow

**Normal Operation**
```
Application Code → Feeds Software Watchdog
                ↓
    Software Watchdogs Checked Periodically (100ms)
                ↓
         All OK? → Feed Hardware Watchdog
```

**Software Watchdog Fault**
```
Application Code → Stops Feeding Watchdog
                ↓
    Software Watchdog Triggers
                ↓
    Notify Fault Module → Save Info → Reset
```

**Software Watchdog System Failure**
```
Periodic Check Stops Working
                ↓
    Hardware Watchdog Not Fed
                ↓
        Hardware Watchdog Resets MCU
```

**Fault Module Gets Stuck**
```
Software Watchdog Triggers → Fault Module Called
                ↓
        Fault Module Hangs
                ↓
    Hardware Watchdog Not Fed
                ↓
        Hardware Watchdog Resets MCU
```

#### Initialization Watchdog Design

**Challenge**: Count failed initializations across MCU resets

**Solution**: No-init RAM block
- Added to linker script (not initialized by startup code)
- Contains consecutive failed init counter
- Persists across MCU resets (unlike normal globals in .data/.bss)

**Counter Reset Conditions**
1. Successful initialization
2. Invalid no-init block data (e.g., after power cycle)
3. Previous reset NOT due to hardware watchdog

**Linker Script Addition**
```ld
.noinit.vars (NOLOAD) :
{
    . = ALIGN(32);
    *(.noinit.vars)
} >RAM
```

Position: After .bss, before heap

**Variable Declaration**
```c
struct no_init_vars {
    uint32_t magic;
    uint32_t consecutive_failed_inits;
    uint32_t check;
} __attribute__((section(".noinit.vars")));
```

### Code Snippets

#### API (wdg.h)
```c
typedef void (*wdg_triggered_cb)(uint32_t wdg_id);

// Core module API
int32_t wdg_init(struct wdg_cfg* cfg);
int32_t wdg_start(void);

// Software watchdog API
int32_t wdg_register(uint32_t wdg_id, uint32_t period_ms);
int32_t wdg_feed(uint32_t wdg_id);
int32_t wdg_register_triggered_cb(wdg_triggered_cb triggered_cb);

// Hardware watchdog API
void wdg_start_init_hdw_wdg(void);       // Start hdw wd for init (may not actually start)
void wdg_init_successful(void);          // Signal init succeeded
int32_t wdg_start_hdw_wdg(uint32_t timeout_ms);  // Start/restart hdw wd
void wdg_feed_hdw(void);                 // Feed hardware watchdog
```

#### Data Structures
```c
struct soft_watchdog {
    uint32_t period_ms;
    uint32_t last_feed_time_ms;
};

struct wdg_state {
    struct soft_watchdog watchdogs[NUM_WATCHDOGS];
    wdg_triggered_cb triggered_cb;
};

struct no_init_vars {
    uint32_t magic;
    uint32_t consecutive_failed_inits;
    uint32_t check;
} __attribute__((section(".noinit.vars")));
```

#### Module Start (Starts Periodic Timer)
```c
int32_t wdg_start(void)
{
    return tmr_start(TMR_WDG_CHECK,
                     WDG_CHECK_PERIOD_MS,  // 10ms
                     wdg_check_callback,
                     NULL);
}
```

#### Client Registration
```c
int32_t wdg_register(uint32_t wdg_id, uint32_t period_ms)
{
    if (wdg_id >= NUM_WATCHDOGS)
        return -1;

    wdg_state.watchdogs[wdg_id].period_ms = period_ms;
    wdg_state.watchdogs[wdg_id].last_feed_time_ms = tmr_get_ms();
    return 0;
}
```

#### Feed Watchdog
```c
int32_t wdg_feed(uint32_t wdg_id)
{
    if (wdg_id >= NUM_WATCHDOGS)
        return -1;

    wdg_state.watchdogs[wdg_id].last_feed_time_ms = tmr_get_ms();
    return 0;
}
```

#### Initialization Watchdog Start Logic
```c
void wdg_start_init_hdw_wdg(void)
{
    validate_no_init_vars();  // Reinitialize if invalid

    // If last reset NOT watchdog, clear fail counter
    if (!(RCC->CSR & RCC_CSR_IWDGRSTF)) {
        no_init_vars.consecutive_failed_inits = 0;
    }

    // Decide whether to start watchdog
    if (no_init_vars.consecutive_failed_inits < MAX_CONSECUTIVE_INIT_FAILS) {
        no_init_vars.consecutive_failed_inits++;  // Assume will fail
        wdg_start_hdw_wdg(INIT_TIMEOUT_MS);
    }
}
```

#### Periodic Check Callback
```c
static void wdg_check_callback(union tmr_cb_data* data)
{
    uint32_t now_ms = tmr_get_ms();
    bool watchdog_triggered = false;

    // Check all software watchdogs
    for (uint32_t i = 0; i < NUM_WATCHDOGS; i++) {
        if (wdg_state.watchdogs[i].period_ms > 0) {
            uint32_t elapsed = now_ms - wdg_state.watchdogs[i].last_feed_time_ms;
            if (elapsed > wdg_state.watchdogs[i].period_ms) {
                // Watchdog triggered!
                if (wdg_state.triggered_cb) {
                    wdg_state.triggered_cb(i);
                }
                watchdog_triggered = true;
                break;
            }
        }
    }

    // If no software watchdog triggered, feed hardware watchdog
    if (!watchdog_triggered) {
        wdg_feed_hdw();
    }
}
```

#### Hardware Watchdog Start (Low-level)
```c
int32_t wdg_start_hdw_wdg(uint32_t timeout_ms)
{
    // Calculations for clock divider and reload value
    // ... (complex but clear calculations based on clock rate)

    // Configure IWDG registers
    LL_IWDG_EnableWriteAccess(IWDG);
    LL_IWDG_SetPrescaler(IWDG, prescaler);
    LL_IWDG_SetReloadCounter(IWDG, reload_value);

    // Important: Stop watchdog when debugger stops CPU
    LL_DBGMCU_APB1_GRP1_FreezePeriph(LL_DBGMCU_APB1_GRP1_IWDG_STOP);

    LL_IWDG_ReloadCounter(IWDG);  // Feed it
    LL_IWDG_Enable(IWDG);

    return 0;
}
```

### Demo Example

**Goal**: Demonstrate watchdog triggering due to temp/humidity module failure, and hardware watchdog reset

**Why Important**:
- Shows software watchdog monitoring critical work
- Proves fault detection and recovery flow
- Demonstrates hardware watchdog as backup

**Setup**: Temp/humidity module has watchdog (5 second period), fed on successful measurement

**Demo Flow**:

1. **Normal Operation**
   - Reset board, observe temp/hum samples
   - Execute `wdg status` multiple times
   - See "last feed" timestamp updating every ~1 second

2. **Trigger Software Watchdog**
   - Execute `i2c test reserve` (reserves I2C bus)
   - Temp/hum module can't get measurements
   - Stops feeding its watchdog
   - After 5 seconds: Fault module output appears
   - System resets automatically
   - After reset: I2C unreserved, temp/hum works again

3. **Trigger Hardware Watchdog**
   - Execute `wdg test no_feed_hdw`
   - Sets flag to stop feeding hardware watchdog
   - Temp/hum still taking samples
   - After hardware timeout: System resets directly
   - No fault module involvement (reset too fast)

**Related Code** (tmphm.c):
```c
// Registration in init
wdg_register(WDG_ID_TMPHM, 5000);  // 5 second timeout

// Feed on successful measurement
if (measurement_successful) {
    wdg_feed(WDG_ID_TMPHM);
}
```

### Important Notes

1. **Debugger Consideration**: DBGMCU register setting stops watchdog when debugger halts (prevents resets during debugging)
2. **Timeout Values**: Demo uses short values (5 sec) for speed; production often uses larger safety margins
3. **__attribute__((section()))**: GCC extension to place variables in specific linker sections
4. **No-Init Variables**: Must validate magic/check values to detect power cycles
5. **Callback Function**: Fault module registers to receive watchdog trigger notifications

### Discussion Prompts

**Q1: How to support client busy period (can't feed watchdog temporarily)?**

Options:
1. API to increase timeout period temporarily (client restores when done)
   - Con: Must predict busy duration accurately
2. API to stop/start watchdog checking
   - More flexible than option 1
3. Use timeout handler feature (next question) to intercept trigger

**Q2: Client handles trigger itself before system reset?**

Implementation:
- Add `handler` parameter to `wdg_register()`
- Callback function pointer called when watchdog triggers
- Handler return value determines action:
  - Ignore trigger (handler dealt with it)
  - Pass to fault module (normal flow)
- Allows client to resolve issue without system reset

**Q3: (Not explicitly stated but implied) Why hierarchy of watchdogs?**
- Software watchdogs can fail (code bug, stuck loop)
- Hardware watchdog independent, more reliable
- Multiple layers of protection
- Hardware watchdog as "last resort" safety net

---

## Lesson 6: Stack Overflow Protection

### Objective
Implement stack overflow detection and prevention using memory protection unit (MPU), plus techniques to measure actual stack usage.

### Theory and Concepts

#### Stack Overflow Problem
- Occurs when stack extends into other memory areas
- CPU generally doesn't detect overflow
- Results in random memory corruption
- System exhibits "weird" behavior - very confusing to debug
- Can waste significant debugging time before realizing root cause

#### Stack Basics
**Stack Usage**
- Temporary storage for functions and interrupt handlers
- Stores: Return addresses, saved registers, local variables
- Sometimes arguments (when registers insufficient)
- Interrupts use stack same way as functions
- In this system: Single stack for all code (some systems have separate interrupt stack)

**Stack Growth**
- Starts at highest RAM address (stack pointer initialized to top of RAM)
- Grows downward (toward lower addresses) as used
- Stack pointer decrements before each push
- Shrinks (pointer moves up) as functions return
- Unused space needed between stack and other RAM users (globals, heap)

**Estimation Challenge**
- Hard to predict maximum stack usage
- Would need to know:
  - Stack usage per function
  - Call depth (nesting)
  - Interrupt nesting
  - Local variable sizes
- Compiler tools exist but have limitations

### Techniques

#### 1. Stack Guard with MPU
**MPU (Memory Protection Unit)**
- NOT MMU (Memory Management Unit for virtual memory/Linux)
- Hardware device in MCU placing restrictions on memory regions
- Defines read/write permissions per region
- Limited number of regions (8 for STM32F401RE)
- Minimum region size: 32 bytes (for this MCU)

**Stack Guard Creation**
- Use MPU to create small read-only region just beyond stack area
- If stack overflows and hits guard, CPU generates fault
- For STM32F401RE: MemManage fault exception

**Benefits**
1. **Safety**: Reset prevents dangerous behavior from corruption
2. **Availability**: Maximizes uptime by quickly recovering
3. **Detection**: Catch during testing before field deployment
4. **Wasted Memory**: Only 32 bytes (minimal)

**Best Practice**: Size stack larger than needed with safety margin; give spare RAM to stack

#### 2. High Water Mark Technique
**Concept**
- Fill stack with fixed pattern during init
- As stack used, pattern overwritten with real data
- Find first location NOT containing pattern = high water mark
- Indicates maximum stack usage up to that point

**Limitations**
- Can't guarantee maximum scenario occurred
- Provides useful data point
- Need to run system through typical operations

**Pattern Choice**
- Not common value (not 0, small integers)
- Recognizable in hex dump
- This implementation: 0xCAFEBAD

### RAM Memory Map

**Initial (IDE-generated)**
```
0x20000000: .data section (initialized globals)
            .bss section (zero-initialized globals)
            Heap (minimum size from project settings)
            Stack (minimum size from project settings)
            Unused RAM → given to stack
0x20018000: (End of 96KB RAM)
```

**Modified (with Stack Guard)**
```
0x20000000: .data section
            .bss section
            Heap
            Stack (minimum)
0x20017FE0: Stack Guard (32 bytes, read-only via MPU)
0x20017FFF:
            Unused RAM → given to stack
0x20018000: (End of RAM)
```

Stack grows down, hits guard if overflow

### Implementation

#### Linker Script Modification
```ld
MPU_MIN_BLOCK_SIZE = 32;

MEMORY
{
    RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 96K
    FLASH (rx) : ORIGIN = 0x8000000, LENGTH = 512K
}

SECTIONS
{
    .heap_stack :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        . = . + _Min_Heap_Size;
        . = ALIGN(MPU_MIN_BLOCK_SIZE);      /* NEW: Align for MPU */
        _sstart_guard = .;                   /* NEW: Stack guard start */
        . = . + MPU_MIN_BLOCK_SIZE;         /* NEW: Allocate 32 bytes */
        _send_guard = .;                     /* NEW: Stack guard end */
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } >RAM
}
```

Variables `_sstart_guard` and `_send_guard` accessible from C code

#### MPU Configuration (fault.c)
```c
#define FAULT_STACK_GUARD_REGION_NUM    0

void fault_start(void)
{
    // Fill stack with pattern (from current SP down to stack guard)
    uint32_t sp;
    __asm volatile ("MRS %0, MSP" : "=r" (sp));  // Get current stack pointer
    sp -= sizeof(uint32_t);  // Decrement before write (ARM stack behavior)

    for (uint32_t* p = (uint32_t*)sp; p >= (uint32_t*)&_sstart_guard; p--) {
        *p = FAULT_STACK_INIT_PATTERN;  // 0xCAFEBAD
    }

    // Configure MPU for stack guard
    LL_MPU_ConfigRegion(
        FAULT_STACK_GUARD_REGION_NUM,           // Region 0
        (uint32_t)&_sstart_guard,               // Start address
        MPU_MIN_BLOCK_SIZE,                      // Size: 32 bytes
        LL_MPU_REGION_PRIV_RO                   // Read-only permission
        // ... additional config flags
    );

    LL_MPU_Enable(LL_MPU_CTRL_PRIVILEGED_DEFAULT);
}
```

#### Stack Usage Measurement (fault.c console command)
```c
static int32_t cmd_fault_status(void)
{
    // Print stack limits
    printc("Stack: start=0x%lx end=0x%lx size=%lu bytes\n",
           (uint32_t)&_sstart_guard, _estack,
           _estack - (uint32_t)&_sstart_guard);

    // Find high water mark
    uint32_t* p = (uint32_t*)&_sstart_guard;
    while (*p == FAULT_STACK_INIT_PATTERN && p < (uint32_t*)_estack) {
        p++;
    }

    uint32_t stack_used = _estack - (uint32_t)p;
    printc("Stack high water mark: %lu bytes used\n", stack_used);

    return 0;
}
```

#### Stack Overflow Test (fault.c)
```c
static void test_overflow_stack(void)
{
    test_overflow_stack();  // Infinite recursion - uses stack indefinitely
}

static int32_t cmd_fault_test(int32_t argc, const char** argv)
{
    if (strcasecmp(argv[2], "overflow") == 0) {
        test_overflow_stack();
    }
    // ... other tests
    return 0;
}
```

### Demo Example

**Goal**: Show stack usage measurement and stack overflow fault detection

**Why Important**:
- Proves high water mark technique works
- Demonstrates MPU guard protection
- Shows fault data for stack overflow (MemManage fault)

**Demo Flow**:

1. **Measure Stack Usage**
   - Reset board, stop logging
   - Execute `fault status` - shows high water mark (e.g., 864 bytes)
   - Execute various commands (ttys status, dio status port a)
   - Check `fault status` again - higher usage (940, 1012 bytes)
   - Demonstrates high water mark increasing as stack used more
   - Executing same command again doesn't increase (already captured maximum)

2. **Trigger Stack Overflow**
   - Execute `fault test overflow`
   - Infinite recursion rapidly exceeds stack
   - System resets (fault module triggered)
   - Fault output shows type 2 (EXCEPTION), parameter 4 (MemManage fault)

3. **Analyze Fault Data**
   - Save console output to file
   - Run formatting tool: `python log_format.py <src_dir> <raw_file>`
   - Decoded output shows:
     - Fault type: EXCEPTION (2)
     - Fault parameter: 4 (MemManage exception)
     - MMFAR register: 0x20018238 (address that caused fault - in stack guard)
     - Stack pointer: 0x20018220 (right at edge of stack guard)
   - Stack guard region (from .map file): 0x20018220 - 0x20018240
   - Address in MMFAR confirms write to guard region
   - Exception stack frame contains pattern (stack wasn't usable for saving state)

### Important Notes

1. **Inline Assembly**: `__asm volatile ("MRS %0, MSP" : "=r" (sp))` - GCC extension to get stack pointer register
2. **Stack Decrement First**: ARM stacks decrement SP before push, so decrement before writing pattern
3. **MPU Region Alignment**: Must be on 32-byte boundary (enforced by linker with ALIGN directive)
4. **Pattern Visibility**: 0xCAFEBAD readable as "CAFE BAD" in hex dumps
5. **Stack Limits**: Available from linker script symbols `_sstart_guard` and `_estack`
6. **Map File**: Build output shows exact addresses of stack guard region

### Discussion Prompts

**Q1: Can stack guard miss overflow?**

**Yes, possible scenario:**
- Function has block of local variables ≥32 bytes
- Variables never written
- Block happens to align with stack guard
- No writes to guard = no fault triggered
- Program "jumps over" guard region

**Mitigation:**
- Larger guard region (reduces likelihood)
- GCC `-fstack-check` option
  - Probes stack to ensure writable
  - Prevents jump-over attacks (common in Linux exploits)
  - May work for small guard regions

**Q2: Keep running after stack overflow instead of reset?**

**Possible but very difficult:**
- System state unknown when guard hit
- Hard to recover reliably

**Option 1: Reset stack, jump to main**
- Reset SP to initial value
- Jump to main() entry point
- Globals/statics NOT reinitialized (unlike normal reset)
- Could be feature: Code might pick up where it left off
- Apollo lunar lander did this during moon landing!
  - Had resets due to issues
  - Kept running, picked up operation
  - Critical for successful landing

**Option 2: setjmp/longjmp**
- C functions for "time travel"
- `setjmp()` saves execution state
- `longjmp()` returns to that state
- Generally considered unsafe/"dodgy"
- Impractical for this use case

**Conclusion**: Reset is simplest, safest option

---

## Lesson 7: Asserts and Audits

### Objective
Discuss two additional techniques relevant to RAM - asserts and audits - without full implementation, to raise awareness for use in professional work.

### Theory and Concepts

#### Asserts
**Definition**
- Software statement declaring condition that should be true
- Example: `assert(pointer_var != NULL)`
- Originated in Unix

**Traditional Behavior**
- Failed assert aborts process (drastic)
- In embedded: Usually custom assert macros with various failure actions

**Common Assert Failure Actions**

1. **Log Error and Continue**
   ```c
   ASSERT_LOG(condition)
   // If fails: Log error with file name and line number, continue execution
   ```
   - Least disruptive
   - Problem noted but not fixed

2. **Return with Error Code**
   ```c
   ASSERT_RETURN(condition, error_code)
   // If fails: Return from function with specified error code
   ```
   - Let caller handle the error
   - Function exits immediately on assert failure

3. **Declare Fatal Fault**
   ```c
   ASSERT_FATAL(condition)
   // If fails: Call fault_detected(), system resets
   ```
   - Most drastic, comparable to Unix abort
   - Treats assert failure as critical fault

**Example Implementation**
```c
#define ASSERT_FATAL(cond) do { \
    if (!(cond)) { \
        fault_detected(FAULT_TYPE_ASSERT, \
                       (FILE_ID << 16) | __LINE__); \
    } \
} while (0)
```
- FILE_ID: Unique identifier per source file
- __LINE__: Predefined macro for line number
- Provides exact location of failed assert
- fault_detected() never returns (triggers full fault report and reset)

#### Typical Assert Usage

**Extremely Common**: Function argument validation
```c
int32_t process_data(uint8_t* data, uint32_t len, config_t* config)
{
    ASSERT_RETURN(data != NULL, ERR_NULL_PTR);
    ASSERT_RETURN(len > 0 && len <= MAX_LEN, ERR_INVALID_LEN);
    ASSERT_RETURN(config != NULL, ERR_NULL_PTR);

    // Function body...
}
```

**Developer Variation**
- Usage varies widely among projects and developers
- Some projects: Never used
- Some projects: Every function has asserts
- Some developers: Use heavily; others: Rarely

**Value Proposition**
- Fast and easy to write
- Provides exact failure location (file:line)
- Catches programming errors early
- Clear documentation of assumptions

### Asserts in Production Builds

#### The Debate

**Traditional Approach: Disable in Production**
- Asserts compiled out (macros become no-ops)
- Rationale: Served purpose in testing, save CPU/space
- Standard practice in many environments

**Alternative Approach: Keep in Production**
- Leave asserts active in release builds
- Depends heavily on product and application

**Arguments FOR Production Asserts (Safety-Critical)**

1. **Failsafe Justification**
   - Anything abnormal → go to safe state
   - Better safe than sorry
   - Minor issue might symptom of serious problem

2. **Unknown Unknowns**
   - If not seen in testing but appears in field → concerning
   - Indicates untested code path or condition
   - Warrants conservative response

3. **Example: Robotics**
   - Author's experience in factory robotics
   - Low tolerance for abnormalities
   - Most issues → shutdown for safety
   - Operator intervention required to restart

**Arguments AGAINST Production Asserts**

1. **Unnecessary Resets Hurt Availability**
   - Resets reduce uptime
   - Bad for customer perception
   - Many issues might be handled more gracefully

2. **Need Sophisticated Error Handling**
   - Asserts too primitive (binary: OK or FAIL)
   - Should use proper error handling, recovery
   - Assert not substitute for good design

3. **Field Complexity**
   - Field conditions far more complex than lab
   - Example: Complex communication systems
   - Asserts in production caused unnecessary resets
   - Some needed replacement with better error handling
   - "Pain experienced along the way"

4. **Performance Critical Systems**
   - Need every bit of CPU available
   - Asserts consume resources, do no productive work
   - Another factor to consider

**Conclusion**: Context-dependent decision
- Safety-critical: Lean toward keeping asserts
- Availability-critical: Lean toward sophisticated error handling
- Performance-critical: Consider resource cost
- Must think through implications for specific product

### Audits

#### Definition
Software that checks system integrity:
- Critical data structures and memory
- Configuration consistency
- Resource status
- File system space

#### What Audits Check

**Data Structure Integrity**
- Valid values in critical structures
- Pointer validity (not NULL, within bounds)
- Array indices within range
- Not exhaustive - focus on "big problem" issues

**Configuration Consistency**
- Compare working memory configuration vs. database
- Detect corruption or unintended changes

**Resource Health**
- Heap status
- Work queue status
- Orphaned nodes in tree structures

**File System Space**
- Critical in systems with file systems
- Monitor /var in Linux
- Example: Mars Spirit rover
  - File system filled during Earth-to-Mars trip
  - Background task slowly filled storage
  - System never tested for that duration
  - Caused major problem after landing
  - Fortunately fixed remotely from Earth
- Example: Author's experience
  - Systems running 6+ months start having problems
  - File system full
  - System behaves erratically
  - Common issue in field

#### Audit Purposes

1. **Find Invisible Problems**
   - Issues not immediately visible
   - Would cause incorrect operation
   - May cause stability problems in near future

2. **Preventive Maintenance**
   - Catch problems before they escalate
   - Reduce unexpected failures

#### Audit Scheduling

**Periodic Execution**
- Every N minutes or hours
- During low processing periods
- Balance thoroughness vs. overhead

#### Audit Types

**Correcting Audits**
- Detect AND fix problems automatically
- Examples:
  - Clean up file system
  - Restore configuration from database
  - Free orphaned resources

**Caution with Correcting Audits**
- Not always simple to change things in memory
- Could cause unexpected side effects
- Must think through implications

#### Audit Failure Actions

**1. Log Error/Warning**
- Simplest response
- Appropriate for minor issues
- Especially for correcting audits (problem already fixed)

**2. Set Alarm**
- For systems with alarm concept
- Notify operator to investigate
- Allows human decision

**3. Declare Fatal Fault**
- For serious problems
- System in "big trouble"
- Trigger automatic recovery (possibly full reset)

**Important**: Avoid trigger-happy audits
- Running automatically
- Must be very conservative
- Don't want constant resets

#### Integration with Fault Module

In this course's architecture:
```c
if (critical_audit_failure) {
    fault_detected(FAULT_TYPE_AUDIT, audit_id);
    // Never returns - triggers fault report and reset
}
```

Similar to ASSERT_FATAL macro approach

### Key Ideas

1. **Asserts**
   - Great tool for development/testing (universal agreement)
   - Production use: Context-dependent decision
   - Consider: Safety requirements, availability needs, performance constraints
   - Various failure action types suit different scenarios

2. **Audits**
   - Proactive system health monitoring
   - Catch problems before they cause visible failures
   - Can be correcting (auto-fix) or reporting
   - File system monitoring particularly important
   - Balance thoroughness with overhead

3. **Both Tools**
   - Complement other RAM techniques
   - Part of comprehensive robustness strategy
   - Should be considered but not always implemented
   - Author raising awareness rather than mandating use

### Important Notes

1. **No Implementation in Course**
   - This lesson is discussion/awareness only
   - No code demos or implementations shown
   - Up to developers to implement based on needs

2. **Real-World Examples**
   - Robotics: Low tolerance, assert-like behavior
   - Communication systems: Asserts caused problems
   - Mars rovers: File system audit would have helped
   - Shows importance varies by domain

3. **Integration with Course Modules**
   - Asserts could use fault_detected() API
   - Audits could use fault module for critical failures
   - Fits into existing RAM architecture

4. **Design Flexibility**
   - Multiple assert types for different severities
   - Audit actions range from logging to reset
   - Allows tailoring to specific product needs

### Discussion Prompts
None explicitly stated in this lesson (final lesson of course)

---

## Course Summary

### Complete RAM Technique Suite

The course covers a comprehensive set of techniques for embedded system reliability, availability, and maintainability:

1. **Lightweight Logging**: Flight recorder for post-fault analysis
2. **Fault Handling**: Panic mode processing, data collection, flash storage
3. **Watchdogs**: Multi-level monitoring ensuring critical work completion
4. **Stack Overflow Protection**: MPU guard + high water mark measurement
5. **Asserts**: Development and production error checking
6. **Audits**: Proactive system health monitoring

### Architecture Integration

All techniques integrate into super loop module architecture:
- Standard module API (init, start, run)
- Console commands for debug/test
- Performance measurements
- Logging support
- Field-grade implementations

### Key Principles

1. **Assume Failures Will Occur**: Focus on detection and handling
2. **Panic Mode**: Minimal dependencies, simple operations
3. **Information Collection**: Critical for maintainability
4. **Hardware Watchdog**: Ultimate backup
5. **Flash Storage**: Non-volatile fault data preservation
6. **Offline Analysis**: Formatting tools decode raw data

### Source Code Structure

**New Modules** (outlined in red in architecture):
- `lwl`: Lightweight logging module
- `fault`: Fault handling and stack protection
- `wdg`: Watchdog management
- `flash`: Flash write operations

**Existing Modules Enhanced**:
- Console, command, timer, dio, ttys
- Temperature/humidity sensor (demo platform)

**Hardware Specifics**:
- STM32F401RE Nucleo board
- ARM Cortex-M4 architecture
- 96KB RAM, 512KB Flash
- MPU support (8 regions, 32-byte minimum)
- Independent watchdog timer

### Practical Outcomes

**Development Benefits**:
- Faster debugging with LWL traces
- Immediate fault detection (vs. silent corruption)
- Stack usage visibility
- Watchdog prevents infinite hangs

**Field Benefits**:
- Automatic recovery from faults
- Diagnostic data for failures
- Reduced downtime (availability)
- Easier bug reproduction and fixing (maintainability)

**Safety Benefits**:
- Controlled fault handling
- Prevention of undefined behavior
- Stack overflow protection
- Multiple layers of monitoring

---

## Implementation Reference

### Critical Code Locations

**Lightweight Logging**
- Header: `modules/include/lwl.h`
- Implementation: `modules/lwl/lwl.c`
- Usage: Add LWL_BASE_ID and LWL_NUM to source files, use LWL() macro
- Tool: Python `log_format.py` in fault directory

**Fault Handling**
- Header: `modules/include/fault.h`
- Implementation: `modules/fault/fault.c`
- Assembly: Modified Default_Handler in startup file
- Linker: Modified to reserve flash page for fault data
- Stack protection: MPU configuration in fault_start()
- Console: `fault status`, `fault test`, `fault data`

**Watchdogs**
- Header: `modules/include/wdg.h`
- Implementation: `modules/wdg/wdg.c`
- Linker: .noinit.vars section for cross-reset counter
- Integration: Callback to fault module on trigger
- Console: `wdg status`, `wdg test`

**Flash**
- Header: `modules/include/flash.h`
- Implementation: `modules/flash/flash.c`
- Linker: Three flash sections (ISR_VECTOR, FAULT_DATA, FLASH)
- Operations: Page erase and multi-byte write

**Application Integration**
- Main: `app1/app_main.c`
- Module registration and startup in mods[] array
- Super loop runs all module run() functions

### Console Commands Summary

**LWL Commands**
```
lwl status           - Show logging status and put index
lwl dump             - Dump raw buffer (hex)
lwl test             - Generate test logs then disable
```

**Fault Commands**
```
fault status         - Show stack usage, fault counts
fault test pointer   - Trigger fault with bad pointer
fault test overflow  - Trigger stack overflow
fault data           - Dump fault data from flash
fault data erase     - Clear fault data in flash
```

**Watchdog Commands**
```
wdg status           - Show all watchdog states
wdg test no_feed_hdw - Stop feeding hardware watchdog
```

### Build Configuration

**Required Files Modified**
1. Linker script (.ld file)
   - .noinit.vars section
   - FAULT_DATA flash section
   - Stack guard region

2. Startup file (assembly)
   - Default_Handler modified
   - Calls fault_exception_handler

3. Config.h
   - Watchdog IDs
   - LWL configuration
   - Module enable flags

**Python Tool Requirements**
- Python 3.x installed
- log_format.py needs source directory path
- Searches recursively for LWL statements
- Generates formatted output from raw hex

---

## Additional Resources

**GitHub Repositories**
1. Source code repo: Contains all module code (bare metal + RAM enhancements)
2. Course materials repo: Documentation, IDE project ZIP, ARM reference manuals

**Reference Documents** (in course materials repo)
- ARM Cortex-M4 architecture reference manual
- STM32F401RE reference manual
- Application notes on fault handling
- README describing document sources

**Development Environment**
- STM32CubeIDE (free, multiplatform)
- Python 3.x for formatting tools
- Git for source control
- Optional: UART terminal for console (PuTTY, screen, etc.)

**Hardware Setup**
- STM32 Nucleo-F401RE board (~$15)
- Optional: Adafruit temp/humidity sensor for full demos
- USB cable for programming and console
- Reset button on board for testing

---

## Conclusion

This comprehensive RAM course provides production-ready implementations of critical embedded systems techniques. The modular architecture allows selective adoption of techniques based on specific product needs. All code follows sound design principles with complete test/debug support, making it suitable for both learning and adaptation to real products.

The key takeaway: Embedded systems must assume failures will occur and have robust mechanisms to detect, record, and recover from them. The combination of logging, fault handling, watchdogs, and stack protection provides multiple defensive layers ensuring system reliability, availability, and maintainability in the field.

---

# Production-Level Best Practices Guide

## Introduction

This guide extracts and organizes all production-level coding practices from the RAM course to help you elevate your code quality. These practices represent the difference between university projects and field-deployed products. They address the fundamental challenge: **software will have bugs, and systems must be resilient enough to detect, record, and recover from failures.**

Each practice is organized by its primary contribution to the RAM pillars (Reliability, Availability, Maintainability), with cross-cutting concerns covered separately. For each practice, you'll find:
- **Definition**: What the practice is
- **Why Important**: How it makes code production-ready
- **Pattern to Adopt**: Correct implementation with code examples
- **Anti-Pattern to Avoid**: Common mistakes to avoid
- **Course Reference**: Where this appears in the RAM lessons

---

## 1. Reliability Practices

Reliability = Ability to function without failure

### 1.1 Defensive Programming

#### Definition
Code that validates inputs, checks assumptions, and handles unexpected conditions gracefully rather than assuming everything will work correctly.

#### Why Important
External devices, user inputs, and even internal state won't always behave as expected. Defensive programming prevents cascading failures and makes code resilient to the unexpected. In the course context: "Weaknesses in defensive programming ARE bugs."

#### Pattern to Adopt
```c
// Validate all function inputs
int32_t process_buffer(uint8_t* data, uint32_t len, config_t* config)
{
    // Check for NULL pointers
    if (data == NULL || config == NULL) {
        return ERR_NULL_PTR;
    }

    // Check for valid ranges
    if (len == 0 || len > MAX_BUFFER_SIZE) {
        return ERR_INVALID_LENGTH;
    }

    // Check for valid state
    if (config->mode < MODE_MIN || config->mode > MODE_MAX) {
        return ERR_INVALID_MODE;
    }

    // Now safe to proceed
    for (uint32_t i = 0; i < len; i++) {
        data[i] = transform(data[i], config);
    }

    return 0;
}
```

#### Anti-Pattern to Avoid
```c
// NO VALIDATION - Assumes everything is correct
int32_t process_buffer(uint8_t* data, uint32_t len, config_t* config)
{
    // Dangerous: No NULL check, could crash
    for (uint32_t i = 0; i < len; i++) {
        data[i] = transform(data[i], config);  // What if data or config is NULL?
    }
    return 0;
}
```

#### Course Reference
Lesson 2 (Background) - "Defensive Programming" section emphasizes that code must handle unexpected behavior reasonably.

---

### 1.2 Fault Detection Mechanisms

#### Definition
Multiple layers of monitoring to detect when something goes wrong: hardware exceptions, watchdog timeouts, application-level checks, data integrity validation.

#### Why Important
"Worst field events: system malfunctioning but doesn't know it." Detection is the first step in any recovery strategy. Without detection, silent failures lead to dangerous or incorrect operation.

#### Pattern to Adopt
```c
// CPU Exception Detection (automatic)
void HardFault_Handler(void) {
    fault_exception_handler(sp);  // Handled by fault module
}

// Watchdog Detection (monitors critical work)
void control_loop_run(void)
{
    // Do critical work
    read_sensors();
    calculate_outputs();
    write_actuators();

    // Signal that work completed successfully
    wdg_feed(WDG_ID_CONTROL_LOOP);
}

// Application-Level Detection
int32_t parse_message(uint8_t* msg, uint32_t len)
{
    if (len < MIN_MSG_SIZE) {
        LOG_ERROR("Message too short: %u", len);
        return ERR_INVALID_MSG;
    }

    uint16_t checksum = calculate_checksum(msg, len - 2);
    uint16_t received_checksum = (msg[len-2] << 8) | msg[len-1];

    if (checksum != received_checksum) {
        LOG_ERROR("Checksum mismatch: calc=0x%04x rcvd=0x%04x",
                  checksum, received_checksum);
        return ERR_CHECKSUM;
    }

    // Proceed with valid message
    return process_message(msg, len);
}
```

#### Anti-Pattern to Avoid
```c
// No detection - silent failures
void control_loop_run(void)
{
    read_sensors();      // What if this hangs?
    calculate_outputs(); // What if sensors returned garbage?
    write_actuators();   // What if calculation overflowed?
    // No validation, no watchdog, no checks
}

// Ignoring errors
int32_t parse_message(uint8_t* msg, uint32_t len)
{
    // Blindly trust the data
    return process_message(msg, len);  // No length check, no checksum
}
```

#### Course Reference
Lesson 2 (Background) - "Fault Handling Actions: Detect Fault" section
Lesson 4 (Fault Handling) - CPU exception detection
Lesson 5 (Watchdogs) - Software and hardware watchdog detection

---

### 1.3 Input Validation

#### Definition
Explicit checking of all external inputs (function parameters, sensor readings, communication data) against valid ranges and formats before use.

#### Why Important
Invalid inputs are one of the most common sources of bugs and crashes. Validation at system boundaries prevents corruption from propagating through the system.

#### Pattern to Adopt
```c
// Sensor reading with validation
int32_t read_temperature(int16_t* temp_out)
{
    int16_t raw_temp;

    if (temp_out == NULL) {
        return ERR_NULL_PTR;
    }

    // Read from sensor
    int32_t rc = sensor_read(&raw_temp);
    if (rc != 0) {
        return rc;  // Communication error
    }

    // Validate reasonable range (-40°C to +125°C for typical sensor)
    if (raw_temp < -400 || raw_temp > 1250) {
        LOG_ERROR("Temperature out of range: %d", raw_temp);
        return ERR_OUT_OF_RANGE;
    }

    *temp_out = raw_temp;
    return 0;
}

// Command parsing with validation
int32_t cmd_set_mode(int32_t argc, const char** argv)
{
    if (argc < 2) {
        printc("Usage: mode <0-3>\n");
        return ERR_INVALID_ARGC;
    }

    char* endptr;
    long mode = strtol(argv[1], &endptr, 10);

    // Check for conversion errors
    if (*endptr != '\0') {
        printc("Invalid number: %s\n", argv[1]);
        return ERR_INVALID_ARG;
    }

    // Validate range
    if (mode < 0 || mode > 3) {
        printc("Mode must be 0-3, got %ld\n", mode);
        return ERR_OUT_OF_RANGE;
    }

    // Now safe to use
    set_operating_mode((uint8_t)mode);
    return 0;
}
```

#### Anti-Pattern to Avoid
```c
// No validation - trusting sensor blindly
int32_t read_temperature(int16_t* temp_out)
{
    sensor_read(temp_out);  // What if sensor returns 0xFFFF?
    return 0;               // What if pointer is NULL?
}

// No validation - dangerous conversion
int32_t cmd_set_mode(int32_t argc, const char** argv)
{
    int mode = atoi(argv[1]);  // No error checking
    set_operating_mode(mode);  // Could be any value!
    return 0;
}
```

#### Course Reference
Lesson 2 (Background) - "Defensive Programming" emphasizes handling unexpected inputs
Throughout course - All module APIs validate inputs before use

---

### 1.4 Assertion Usage

#### Definition
Software statements that declare conditions that should be true, providing immediate detection of programming errors with precise location information.

#### Why Important
Asserts catch violations of assumptions immediately, with exact file and line number. They document preconditions and invariants while providing runtime checking during development and (optionally) production.

#### Pattern to Adopt
```c
// Define multiple assert types for different severities
#define ASSERT_RETURN(cond, err) do { \
    if (!(cond)) { \
        LOG_ERROR("Assert failed at %s:%d", __FILE__, __LINE__); \
        return (err); \
    } \
} while (0)

#define ASSERT_FATAL(cond) do { \
    if (!(cond)) { \
        fault_detected(FAULT_TYPE_ASSERT, \
                       (FILE_ID << 16) | __LINE__); \
    } \
} while (0)

// Use asserts for parameter validation
int32_t add_to_queue(queue_t* q, item_t* item)
{
    ASSERT_RETURN(q != NULL, ERR_NULL_PTR);
    ASSERT_RETURN(item != NULL, ERR_NULL_PTR);
    ASSERT_RETURN(q->count < q->max_size, ERR_QUEUE_FULL);

    q->items[q->count++] = item;
    return 0;
}

// Use asserts for invariant checking
void process_state_machine(void)
{
    static enum state current_state = STATE_IDLE;

    switch (current_state) {
        case STATE_IDLE:
            // ...
            break;
        case STATE_ACTIVE:
            // ...
            break;
        case STATE_ERROR:
            // ...
            break;
        default:
            ASSERT_FATAL(0);  // Should never reach here
            break;
    }
}
```

#### Anti-Pattern to Avoid
```c
// No asserts - silent failures
int32_t add_to_queue(queue_t* q, item_t* item)
{
    q->items[q->count++] = item;  // Crash if q is NULL
                                   // Overflow if queue full
    return 0;
}

// Single assert type - no flexibility
#define ASSERT(cond) if (!(cond)) abort()  // Too drastic for all cases

// Asserts with side effects
ASSERT(initialize_device() == 0);  // BAD: Function only runs if asserts enabled!
```

#### Course Reference
Lesson 7 (Asserts and Audits) - Complete discussion of assert types, usage patterns, and production considerations

---

### 1.5 Data Integrity Checks

#### Definition
Mechanisms to verify that critical data structures and memory haven't been corrupted: checksums, magic numbers, bounds checking, redundancy.

#### Why Important
Memory corruption can occur from bugs, hardware faults, or cosmic rays (yes, really). Integrity checks catch corruption before it causes catastrophic failures.

#### Pattern to Adopt
```c
// Magic numbers for section identification
#define FAULT_DATA_MAGIC     0xFADE0001
#define LWL_BUFFER_MAGIC     0xFADE0002
#define END_MARKER_MAGIC     0xFADE0099

struct fault_data_section {
    uint32_t magic;      // Always first - identifies section
    uint32_t length;     // Section size for validation
    // ... actual data ...
};

// Validate before use
int32_t read_fault_data(void)
{
    struct fault_data_section* section = (struct fault_data_section*)FLASH_ADDR;

    // Check magic number
    if (section->magic != FAULT_DATA_MAGIC) {
        return ERR_NO_FAULT_DATA;  // Flash empty or corrupted
    }

    // Check length reasonable
    if (section->length < sizeof(*section) ||
        section->length > MAX_FAULT_DATA_SIZE) {
        return ERR_CORRUPT_DATA;
    }

    // Data appears valid
    return process_fault_data(section);
}

// No-init variables with validation
struct no_init_vars {
    uint32_t magic;
    uint32_t consecutive_failed_inits;
    uint32_t check;  // Redundancy for validation
} __attribute__((section(".noinit.vars")));

void validate_no_init_vars(void)
{
    const uint32_t expected_check =
        no_init_vars.magic ^ no_init_vars.consecutive_failed_inits;

    if (no_init_vars.magic != NO_INIT_MAGIC ||
        no_init_vars.check != expected_check) {
        // Data invalid (power cycle or corruption) - reinitialize
        no_init_vars.magic = NO_INIT_MAGIC;
        no_init_vars.consecutive_failed_inits = 0;
        no_init_vars.check = NO_INIT_MAGIC ^ 0;
    }
}
```

#### Anti-Pattern to Avoid
```c
// No integrity checks - blind trust
int32_t read_fault_data(void)
{
    struct fault_data_section* section = (struct fault_data_section*)FLASH_ADDR;
    return process_fault_data(section);  // Could be garbage!
}

// No validation of persistent data
struct no_init_vars {
    uint32_t consecutive_failed_inits;
} __attribute__((section(".noinit.vars")));

void use_no_init_vars(void)
{
    // Use directly without validation
    if (no_init_vars.consecutive_failed_inits > 10) {
        // Could be random value after power cycle!
    }
}
```

#### Course Reference
Lesson 4 (Fault Handling) - Magic numbers in binary fault data format
Lesson 5 (Watchdogs) - No-init variable validation with magic/check fields

---

### 1.6 Error Propagation

#### Definition
Consistent mechanisms for reporting errors up the call stack using return codes, allowing callers to handle failures appropriately.

#### Why Important
Errors must be communicated so callers can respond appropriately. Silent failures or ignored errors lead to undefined behavior. Consistent error handling makes code predictable and maintainable.

#### Pattern to Adopt
```c
// Define error codes
#define ERR_OK              0
#define ERR_INVALID_ARG     -1
#define ERR_NULL_PTR        -2
#define ERR_TIMEOUT         -3
#define ERR_NO_RESOURCE     -4

// Functions return error codes
int32_t initialize_sensor(sensor_t* sensor)
{
    if (sensor == NULL) {
        return ERR_NULL_PTR;
    }

    int32_t rc = i2c_write(sensor->addr, INIT_CMD);
    if (rc != 0) {
        return rc;  // Propagate I2C error
    }

    rc = wait_for_ready(sensor, 100);  // 100ms timeout
    if (rc != 0) {
        return rc;  // Propagate timeout or other error
    }

    return ERR_OK;
}

// Callers check and handle errors
int32_t app_startup(void)
{
    int32_t rc = initialize_sensor(&temp_sensor);
    if (rc != 0) {
        LOG_ERROR("Sensor init failed: %ld", rc);

        // Decide on recovery action
        if (rc == ERR_TIMEOUT) {
            // Maybe retry
            rc = initialize_sensor(&temp_sensor);
        }

        if (rc != 0) {
            // Still failed - take appropriate action
            enter_degraded_mode();
            return rc;
        }
    }

    // Continue with startup
    return ERR_OK;
}
```

#### Anti-Pattern to Avoid
```c
// Ignoring return values
void app_startup(void)
{
    initialize_sensor(&temp_sensor);  // What if it fails?
    start_sampling();                  // Will crash if sensor not initialized!
}

// Functions don't return status
void initialize_sensor(sensor_t* sensor)
{
    i2c_write(sensor->addr, INIT_CMD);
    wait_for_ready(sensor, 100);
    // No way to tell if these succeeded!
}

// Inconsistent error handling
int32_t func_a(void) {
    // Returns 0 on success, -1 on error
}

int32_t func_b(void) {
    // Returns 1 on success, 0 on error (inconsistent!)
}
```

#### Course Reference
Throughout course - All module APIs use int32_t return codes consistently
Module pattern establishes standard error handling approach

---

## 2. Availability Practices

Availability = Ability to function at any point in time (measured as % uptime)

### 2.1 Automatic Recovery

#### Definition
System's ability to detect faults and automatically reset/recover without human intervention, maximizing uptime.

#### Why Important
In the field, human intervention takes time (reduces availability). Automatic recovery minimizes MTTR (Mean Time To Repair), keeping systems operational. Course emphasizes this for availability-critical systems vs. failsafe for safety-critical.

#### Pattern to Adopt
```c
// Fault handler triggers automatic reset
void fault_detected(enum fault_type type, uint32_t fault_param)
{
    // Enter panic mode
    __disable_irq();
    ARM_MPU_Disable();

    // Collect fault information
    collect_fault_data(type, fault_param);

    // Write to flash and console
    record_fault_data();
    record_lwl_buffer();
    record_end_marker();

    // Automatic recovery: Reset MCU
    NVIC_SystemReset();
}

// Watchdog triggers automatic recovery
static void wdg_check_callback(union tmr_cb_data* data)
{
    // Check all software watchdogs
    for (uint32_t i = 0; i < NUM_WATCHDOGS; i++) {
        if (watchdog_expired(i)) {
            // Call fault module - will reset system
            if (wdg_state.triggered_cb) {
                wdg_state.triggered_cb(i);  // -> fault_detected()
            }
            return;
        }
    }

    // All watchdogs OK - feed hardware watchdog
    wdg_feed_hdw();
}

// Application restarts cleanly after reset
int main(void)
{
    // Hardware initialization
    HAL_Init();
    SystemClock_Config();

    // Check if previous reset was due to fault
    uint32_t reset_flags = fault_get_rcc_csr();
    if (reset_flags & RCC_CSR_IWDGRSTF) {
        // Hardware watchdog triggered - system recovered
    }

    // Initialize all modules and resume operation
    initialize_all_modules();

    // Super loop - system back online
    while (1) {
        run_all_modules();
    }
}
```

#### Anti-Pattern to Avoid
```c
// No recovery - system hangs forever
void fault_detected(enum fault_type type, uint32_t fault_param)
{
    printc("FAULT DETECTED!\n");
    while (1) {
        // Hang forever - requires power cycle
        // System completely unavailable
    }
}

// No watchdog - infinite loops undetected
void process_queue(void)
{
    while (queue_not_empty()) {
        process_item();  // If this hangs, system stuck forever
    }
    // No watchdog feeding, no timeout
}

// Failsafe when availability needed
void critical_fault(void)
{
    shutdown_all_peripherals();
    while (1) {
        // Waiting for human intervention
        // For availability-critical systems, should auto-recover!
    }
}
```

#### Course Reference
Lesson 2 (Background) - "Automatic Recovery" vs. "Failsafe Policy" discussion
Lesson 4 (Fault Handling) - Automatic reset after fault data collection
Lesson 5 (Watchdogs) - Watchdog hierarchy ensures recovery even if fault module fails

---

### 2.2 Watchdog Implementation

#### Definition
Multi-level monitoring system where critical tasks must periodically signal they're executing correctly, with automatic fault detection if signals stop.

#### Why Important
"Watchdogs ensure critical work is done, not simply that code executes." They catch infinite loops, hangs, and stuck states. Hierarchical design (software watchdogs monitored by hardware watchdog) provides defense in depth.

#### Pattern to Adopt
```c
// Register watchdogs for critical work
void module_init(void)
{
    // Register with 5 second timeout
    wdg_register(WDG_ID_MY_MODULE, 5000);
}

// Feed watchdog when critical work completes successfully
void module_run(void)
{
    // Perform critical work
    int32_t rc = read_sensors();
    if (rc != 0) {
        return;  // Don't feed watchdog - will trigger
    }

    rc = process_data();
    if (rc != 0) {
        return;  // Don't feed watchdog
    }

    rc = write_outputs();
    if (rc != 0) {
        return;  // Don't feed watchdog
    }

    // All critical work succeeded - signal watchdog
    wdg_feed(WDG_ID_MY_MODULE);
}

// Watchdog module checks periodically
static void wdg_check_callback(union tmr_cb_data* data)
{
    uint32_t now = tmr_get_ms();
    bool fault = false;

    for (uint32_t i = 0; i < NUM_WATCHDOGS; i++) {
        if (wdg_state.watchdogs[i].period_ms > 0) {
            uint32_t elapsed = now - wdg_state.watchdogs[i].last_feed_time_ms;
            if (elapsed > wdg_state.watchdogs[i].period_ms) {
                // Watchdog triggered - notify fault module
                if (wdg_state.triggered_cb) {
                    wdg_state.triggered_cb(i);
                }
                fault = true;
                break;
            }
        }
    }

    // Feed hardware watchdog only if all software watchdogs OK
    if (!fault) {
        wdg_feed_hdw();
    }
}

// Hardware watchdog as ultimate backup
int32_t wdg_start_hdw_wdg(uint32_t timeout_ms)
{
    // Configure hardware watchdog
    LL_IWDG_Enable(IWDG);
    LL_IWDG_SetReloadCounter(IWDG, reload_value);

    // Stop watchdog when debugger halts CPU
    LL_DBGMCU_APB1_GRP1_FreezePeriph(LL_DBGMCU_APB1_GRP1_IWDG_STOP);

    return 0;
}
```

#### Anti-Pattern to Avoid
```c
// Single watchdog for everything - not specific enough
void main_loop(void)
{
    wdg_feed(WDG_MAIN);  // Fed even if critical work didn't happen

    // Loop executes but maybe sensors aren't being read
    // or outputs aren't being written
}

// Feeding unconditionally - defeats purpose
void module_run(void)
{
    wdg_feed(WDG_ID_MY_MODULE);  // Feed first

    read_sensors();    // What if this hangs after feeding?
    process_data();    // Watchdog already satisfied!
}

// No hardware watchdog backup
// If software watchdog system has bug, no safety net
void wdg_check_callback(void)
{
    for (each watchdog) {
        check_watchdog();
    }
    // No hardware watchdog - if this callback stops running,
    // system hangs with no recovery
}

// Watchdog monitoring code execution, not work completion
void task(void)
{
    wdg_feed(WDG_TASK);  // Feed at start

    while (process_item()) {
        // If this loops forever, watchdog thinks everything is OK!
    }
}
```

#### Course Reference
Lesson 5 (Watchdogs) - Complete watchdog system design and implementation
"Watchdogs ensure critical work is done, not simply that code executes"

---

### 2.3 Graceful Degradation

#### Definition
System continues operating with reduced functionality when components fail, rather than completely shutting down.

#### Why Important
Maximizes availability by keeping system partially functional. Better to operate with degraded capability than to be completely offline. Allows system to continue serving users while issues are resolved.

#### Pattern to Adopt
```c
// Track component health
struct system_status {
    bool temp_sensor_ok;
    bool humidity_sensor_ok;
    bool display_ok;
    bool network_ok;
};

// Initialize with degraded mode option
int32_t tmphm_init(void)
{
    int32_t rc = i2c_init_device(TMPHM_ADDR);
    if (rc != 0) {
        LOG_WARN("Temp/humidity sensor not responding");
        system_status.temp_sensor_ok = false;
        system_status.humidity_sensor_ok = false;
        // Don't return error - continue with other init
    } else {
        system_status.temp_sensor_ok = true;
        system_status.humidity_sensor_ok = true;
    }

    return 0;  // System can still run without sensor
}

// Adapt behavior based on component status
void monitoring_run(void)
{
    if (system_status.temp_sensor_ok) {
        // Full functionality
        read_temperature(&temp);
        read_humidity(&hum);
        update_display(temp, hum);
    } else {
        // Degraded mode - use defaults or last known values
        LOG_WARN("Operating without temp sensor");
        update_display(DEFAULT_TEMP, DEFAULT_HUM);
    }

    if (system_status.network_ok) {
        send_telemetry();
    } else {
        // Buffer telemetry for later, or skip
        LOG_WARN("Network unavailable, buffering data");
    }
}

// Attempt recovery while degraded
void tmphm_run(void)
{
    static uint32_t retry_time = 0;

    if (!system_status.temp_sensor_ok) {
        // Retry initialization periodically
        if (tmr_get_ms() - retry_time > 60000) {  // Every 60 seconds
            int32_t rc = i2c_init_device(TMPHM_ADDR);
            if (rc == 0) {
                LOG_INFO("Temp sensor recovered!");
                system_status.temp_sensor_ok = true;
            }
            retry_time = tmr_get_ms();
        }
        return;
    }

    // Normal operation
    sample_sensors();
}
```

#### Anti-Pattern to Avoid
```c
// All-or-nothing - no degraded mode
int32_t system_init(void)
{
    if (temp_sensor_init() != 0) {
        return ERR_INIT_FAILED;  // Give up completely
    }
    if (display_init() != 0) {
        return ERR_INIT_FAILED;  // System could work without display!
    }
    if (network_init() != 0) {
        return ERR_INIT_FAILED;  // System could work offline!
    }
    return 0;
}

// No status tracking - can't adapt
void monitoring_run(void)
{
    // Blindly call functions that might fail
    read_temperature(&temp);   // Crash if sensor failed at init
    update_display(temp, hum);  // Crash if display failed
}

// Give up on first failure
void network_send(void)
{
    if (send_packet() != 0) {
        LOG_ERROR("Send failed");
        // Stop trying forever - should retry or buffer!
    }
}
```

#### Course Reference
Lesson 2 (Background) - Discussion of availability and reducing MTTR
Course design - Temperature/humidity sensor is optional, system runs without it

---

### 2.4 Resource Monitoring

#### Definition
Tracking usage of limited resources (stack, heap, flash, buffers) to detect exhaustion before it causes failures.

#### Why Important
Resource exhaustion causes hard-to-debug failures. Monitoring allows early detection and either preventive action or controlled degradation. "File system space" monitoring called out specifically as critical.

#### Pattern to Adopt
```c
// Stack usage monitoring - high water mark
void fault_start(void)
{
    // Fill stack with pattern
    uint32_t sp;
    __asm volatile ("MRS %0, MSP" : "=r" (sp));
    sp -= sizeof(uint32_t);

    for (uint32_t* p = (uint32_t*)sp;
         p >= (uint32_t*)&_sstart_guard;
         p--) {
        *p = FAULT_STACK_INIT_PATTERN;
    }
}

int32_t cmd_fault_status(void)
{
    // Find high water mark
    uint32_t* p = (uint32_t*)&_sstart_guard;
    while (*p == FAULT_STACK_INIT_PATTERN &&
           p < (uint32_t*)_estack) {
        p++;
    }

    uint32_t stack_used = _estack - (uint32_t)p;
    printc("Stack usage: %lu / %lu bytes (%.1f%%)\n",
           stack_used, STACK_SIZE,
           100.0 * stack_used / STACK_SIZE);

    // Warn if too high
    if (stack_used > STACK_SIZE * 0.8) {
        printc("WARNING: Stack usage over 80%%!\n");
    }

    return 0;
}

// Performance measurements for unusual events
struct module_stats {
    uint32_t errors;
    uint32_t timeouts;
    uint32_t retries;
    uint32_t max_queue_depth;
};

void track_queue_usage(uint32_t depth)
{
    if (depth > stats.max_queue_depth) {
        stats.max_queue_depth = depth;
    }

    if (depth > QUEUE_SIZE * 0.9) {
        LOG_WARN("Queue nearly full: %lu/%lu", depth, QUEUE_SIZE);
    }
}

// Flash space monitoring
void check_flash_usage(void)
{
    uint32_t used = get_flash_used_bytes();
    uint32_t total = get_flash_total_bytes();

    if (used > total * 0.95) {
        LOG_ERROR("Flash 95%% full - cleaning up old logs");
        cleanup_old_logs();
    }
}
```

#### Anti-Pattern to Avoid
```c
// No monitoring - stack just overflows
void recursive_function(int depth)
{
    char buffer[1024];  // Large local variable
    recursive_function(depth + 1);
    // No tracking of stack usage until it crashes
}

// No resource tracking
void add_to_buffer(item_t* item)
{
    buffer[count++] = item;
    // No check if buffer full
    // No tracking of max usage
}

// Ignoring resource limits
void write_log(const char* msg)
{
    append_to_flash(msg);
    // Never check flash space
    // System fails when flash full
}

// No performance measurements
void process_message(void)
{
    if (parse_failed) {
        // Error occurred but not counted
        // Can't analyze failure rates
    }
}
```

#### Course Reference
Lesson 6 (Stack Overflow Protection) - High water mark technique for stack monitoring
Lesson 7 (Asserts and Audits) - Audits check resource status, file system space
Lesson 2 (Background) - Performance Measurements (PMs) for tracking unusual events

---

### 2.5 Timeout Handling

#### Definition
Every potentially blocking operation has a maximum wait time, after which the system takes appropriate action (retry, error, degrade).

#### Why Important
Without timeouts, system can hang forever waiting for external events. Timeouts ensure system remains responsive and can detect communication failures or device lock-ups.

#### Pattern to Adopt
```c
// I2C operations with timeout
int32_t i2c_write_with_timeout(uint8_t addr, uint8_t* data,
                               uint32_t len, uint32_t timeout_ms)
{
    uint32_t start_time = tmr_get_ms();

    // Start transaction
    i2c_start_write(addr, data, len);

    // Wait for completion with timeout
    while (!i2c_transfer_complete()) {
        if (tmr_get_ms() - start_time > timeout_ms) {
            i2c_abort();
            return ERR_TIMEOUT;
        }
    }

    return i2c_get_status();
}

// Polling with timeout
int32_t wait_for_ready(sensor_t* sensor, uint32_t timeout_ms)
{
    uint32_t start = tmr_get_ms();
    uint8_t status;

    do {
        if (read_status_register(sensor, &status) != 0) {
            return ERR_COMM_FAIL;
        }

        if (status & STATUS_READY) {
            return 0;  // Success
        }

        if (tmr_get_ms() - start > timeout_ms) {
            return ERR_TIMEOUT;
        }

        // Small delay between polls
        delay_ms(10);
    } while (1);
}

// Watchdog IS a timeout mechanism
void critical_task_run(void)
{
    // Task has implicit timeout via watchdog
    // If task doesn't complete in time, watchdog triggers

    do_work();

    wdg_feed(WDG_ID_CRITICAL_TASK);  // Signal completion
}
```

#### Anti-Pattern to Avoid
```c
// Blocking forever - no timeout
int32_t i2c_write(uint8_t addr, uint8_t* data, uint32_t len)
{
    i2c_start_write(addr, data, len);

    while (!i2c_transfer_complete()) {
        // Wait forever if device doesn't respond
        // System stuck!
    }

    return 0;
}

// Busy-wait without timeout
void wait_for_ready(sensor_t* sensor)
{
    uint8_t status;
    while (1) {
        read_status_register(sensor, &status);
        if (status & STATUS_READY) {
            break;  // What if this never happens?
        }
    }
}

// No watchdog - infinite loop goes undetected
void process_loop(void)
{
    while (1) {
        process_item();  // If this hangs, no timeout
    }
}
```

#### Course Reference
Lesson 5 (Watchdogs) - Watchdog timeouts for critical tasks
Lesson 2 (Background) - Discussion of timeout handling as part of defensive programming

---

### 2.6 Initialization Watchdog with Retry Limit

#### Definition
Special watchdog during initialization that allows N consecutive failed attempts before giving up, balancing automatic recovery with debugging access.

#### Why Important
Systems can get stuck during initialization before normal monitoring is active. But unlimited retries waste time and prevent debugging. Limiting retries allows system to eventually stay up for diagnosis.

#### Pattern to Adopt
```c
// No-init RAM for cross-reset counter
struct no_init_vars {
    uint32_t magic;
    uint32_t consecutive_failed_inits;
    uint32_t check;
} __attribute__((section(".noinit.vars")));

// Start initialization watchdog
void wdg_start_init_hdw_wdg(void)
{
    validate_no_init_vars();

    // Check if last reset was watchdog
    if (!(RCC->CSR & RCC_CSR_IWDGRSTF)) {
        // Not watchdog reset - clear counter
        no_init_vars.consecutive_failed_inits = 0;
    }

    // Decide whether to start watchdog
    if (no_init_vars.consecutive_failed_inits < MAX_CONSECUTIVE_INIT_FAILS) {
        // Assume will fail (pessimistic)
        no_init_vars.consecutive_failed_inits++;
        update_check();

        // Start hardware watchdog for init
        wdg_start_hdw_wdg(INIT_TIMEOUT_MS);
    } else {
        LOG_ERROR("Too many failed inits (%lu), leaving watchdog off",
                  no_init_vars.consecutive_failed_inits);
    }
}

// Signal successful initialization
void wdg_init_successful(void)
{
    // Init succeeded - clear counter
    no_init_vars.consecutive_failed_inits = 0;
    update_check();

    // Restart hardware watchdog with normal timeout
    wdg_start_hdw_wdg(NORMAL_TIMEOUT_MS);
}

// In main
int main(void)
{
    wdg_start_init_hdw_wdg();  // Start init watchdog

    // Initialize all modules
    initialize_all();

    wdg_init_successful();  // Signal success

    // Normal operation
    while (1) {
        run_all();
    }
}
```

#### Anti-Pattern to Avoid
```c
// No initialization watchdog
int main(void)
{
    initialize_all();  // If this hangs, system stuck forever

    // Watchdog only started after init
    wdg_start();

    while (1) {
        run_all();
    }
}

// Unlimited retries - never gives up
void wdg_start_init_hdw_wdg(void)
{
    // Always start watchdog
    wdg_start_hdw_wdg(INIT_TIMEOUT_MS);
    // System resets forever if init keeps failing
    // Can never connect debugger to diagnose!
}

// No retry limit - opposite problem
void main(void)
{
    // Single attempt only
    if (initialize_all() != 0) {
        // Give up permanently - no auto-recovery
        while (1);
    }
}
```

#### Course Reference
Lesson 5 (Watchdogs) - "System Initialization Handling" section
No-init RAM variable technique for cross-reset counting

---

## 3. Maintainability Practices

Maintainability = Ease with which problems can be corrected

### 3.1 Module Pattern

#### Definition
Standardized code organization where each module has consistent API (init, start, run), configuration structure, and integration into build system.

#### Why Important
"Separates university projects from real-world products." Consistency makes code predictable, easier to understand, and simpler to maintain. New developers can quickly understand system architecture.

#### Pattern to Adopt
```c
// Standard module header (example: sensor.h)
#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <stdint.h>

// Configuration structure
struct sensor_cfg {
    uint32_t sample_period_ms;
    uint8_t i2c_addr;
};

// Core module interface
int32_t sensor_init(struct sensor_cfg* cfg);
int32_t sensor_start(void);
int32_t sensor_run(void);

// Module-specific APIs
int32_t sensor_read_temp(int16_t* temp_out);
int32_t sensor_read_humidity(uint16_t* hum_out);

#endif // _SENSOR_H_

// Implementation (sensor.c)
#include "sensor.h"

// Private state
static struct {
    uint32_t sample_period_ms;
    uint8_t i2c_addr;
    uint32_t last_sample_time;
    // ...
} sensor_state;

// Core module functions
int32_t sensor_init(struct sensor_cfg* cfg)
{
    if (cfg == NULL) {
        return ERR_NULL_PTR;
    }

    // Validate config
    // Store configuration
    sensor_state.sample_period_ms = cfg->sample_period_ms;
    sensor_state.i2c_addr = cfg->i2c_addr;

    // Initialize hardware
    // ...

    return 0;
}

int32_t sensor_start(void)
{
    // Start timers, register watchdogs, etc.
    wdg_register(WDG_ID_SENSOR, 5000);
    return tmr_start(TMR_SENSOR, sensor_state.sample_period_ms,
                     sensor_callback, NULL);
}

int32_t sensor_run(void)
{
    // Non-blocking periodic work
    // Called from main super loop
    return 0;
}

// Application integration
struct module {
    const char* name;
    int32_t (*init)(void*);
    int32_t (*start)(void);
    int32_t (*run)(void);
} mods[] = {
    { "cmd",    cmd_init,    cmd_start,    NULL },
    { "tmr",    tmr_init,    tmr_start,    tmr_run },
    { "sensor", sensor_init, sensor_start, sensor_run },
    // ...
};

int main(void)
{
    // Init phase
    for (int i = 0; i < NUM_MODULES; i++) {
        if (mods[i].init && mods[i].init(NULL) != 0) {
            ERROR("Module %s init failed", mods[i].name);
        }
    }

    // Start phase
    for (int i = 0; i < NUM_MODULES; i++) {
        if (mods[i].start && mods[i].start() != 0) {
            ERROR("Module %s start failed", mods[i].name);
        }
    }

    // Super loop
    while (1) {
        for (int i = 0; i < NUM_MODULES; i++) {
            if (mods[i].run) {
                mods[i].run();
            }
        }
    }
}
```

#### Anti-Pattern to Avoid
```c
// Inconsistent APIs across modules
void sensor_setup(uint8_t addr);        // Different name
int display_initialize(void);           // Different name
void network_begin(config_t* cfg);      // Different name

// No configuration structures
void sensor_setup(uint8_t addr, uint32_t period, bool filter, int mode);
// Adding parameters requires changing all call sites!

// Monolithic main - no module organization
int main(void)
{
    // Everything in main
    init_gpio();
    init_uart();
    setup_i2c(0x40, 100000);
    configure_adc(12, 1);
    // ... hundreds of lines ...

    while (1) {
        // All logic in one place
        read_sensors();
        process();
        write_outputs();
        handle_commands();
        // Hard to understand, maintain, or reuse
    }
}

// Mixed phases
int main(void)
{
    sensor_init();
    sensor_start();
    display_init();  // Starting sensor before initializing display!
    display_start();
    // Order dependencies fragile
}
```

#### Course Reference
Lesson 1 (Introduction) - "Software Architecture" based on module pattern
Throughout course - All modules follow init/start/run pattern consistently

---

### 3.2 Lightweight Logging

#### Definition
Low-overhead logging system that records software activity using minimal CPU/memory, storing only ID and parameters, with offline formatting.

#### Why Important
"Never know when you want logs - ideal is to always have them." Critical for post-mortem debugging. Low overhead allows always-on logging in production. Shows what system was doing immediately before faults.

#### Pattern to Adopt
```c
// In each source file, define ID range
#define LWL_BASE_ID 20
#define LWL_NUM 10

// Use LWL macro with format string and parameters
void process_command(uint8_t cmd_id, uint16_t param)
{
    LWL("cmd: id=%u param=0x%04x\n", 3,
        LWL_1(cmd_id), LWL_2(param));

    // Process command...

    if (result == SUCCESS) {
        LWL("cmd success: result=%u\n", 1, LWL_1(result));
    } else {
        LWL("cmd failed: err=%d\n", 1, LWL_1(error_code));
    }
}

// In interrupt context (safe because non-blocking)
void SysTick_Handler(void)
{
    uptime_ms++;

    if (uptime_ms % 1000 == 0) {
        LWL("tick_1s\n", 0);  // Timestamp marker
    }
}

// View logs after fault
void fault_detected(enum fault_type type, uint32_t param)
{
    // ... collect other fault data ...

    // Include complete LWL buffer
    uint32_t lwl_len;
    uint8_t* lwl_buf = lwl_get_buffer(&lwl_len);
    record_to_flash(lwl_buf, lwl_len);
}

// Format logs offline with Python tool
// $ python log_format.py source_dir raw_log.txt
// Output shows formatted messages with timestamps
```

#### Anti-Pattern to Avoid
```c
// printf everywhere - too slow for production
void process_command(uint8_t cmd_id, uint16_t param)
{
    printf("Processing command %u with param 0x%04x\n", cmd_id, param);
    // printf blocks, uses lots of CPU
    // Can't use in interrupts
    // Changes timing, hides bugs
}

// Logging disabled in production
#ifdef DEBUG
    log_message("Command processed");
#endif
// No logs when you need them most (in the field)!

// Huge log buffers consuming RAM
char log_buffer[100][256];  // 25KB just for logs!
// Embedded systems don't have memory to waste

// Complex logging inside logs
void log_state(void)
{
    char buffer[256];
    sprintf(buffer, "State: %d, Time: %lu, Temp: %.2f, Status: %s",
            state, time, temp, get_status_string());
    // Formatting takes CPU time, happens every log call
    // String conversion, function calls, dynamic formatting
}

// No offline tools - manual hex interpretation
// Output: 14 2A 01 3F
// What does this mean? Need to look up in code every time
```

#### Course Reference
Lesson 3 (Lightweight Logging) - Complete LWL design and implementation
Key features: printf-like API, non-blocking, low overhead, offline formatting

---

### 3.3 Console Commands for Debugging

#### Definition
Interactive command-line interface providing "window into system" for querying status, triggering tests, adjusting settings, and diagnosing issues.

#### Why Important
Essential for field debugging and development. Allows inspection of internal state without recompiling. Enables testing fault scenarios. Course emphasizes this as key maintainability feature.

#### Pattern to Adopt
```c
// Module provides console commands
static int32_t cmd_sensor_status(int32_t argc, const char** argv)
{
    printc("Sensor Status:\n");
    printc("  Temperature: %.1f C\n", last_temp / 10.0);
    printc("  Humidity: %.1f %%\n", last_hum / 10.0);
    printc("  Last sample: %lu ms ago\n",
           tmr_get_ms() - last_sample_time);
    printc("  Sample count: %lu\n", sample_count);
    printc("  Errors: %lu\n", error_count);
    return 0;
}

static int32_t cmd_sensor_test(int32_t argc, const char** argv)
{
    if (argc < 2) {
        printc("Usage: sensor test <test_name>\n");
        printc("  read    - Force immediate reading\n");
        printc("  reset   - Reset sensor\n");
        printc("  stress  - Rapid readings for stress test\n");
        return 0;
    }

    if (strcasecmp(argv[1], "read") == 0) {
        int32_t rc = sensor_read_now();
        printc("Read result: %ld\n", rc);
    } else if (strcasecmp(argv[1], "reset") == 0) {
        sensor_reset();
        printc("Sensor reset\n");
    }
    // ... other tests ...

    return 0;
}

// Register commands with command module
static struct cmd_cmd_info cmds[] = {
    { "status", cmd_sensor_status },
    { "test",   cmd_sensor_test },
};

int32_t sensor_init(struct sensor_cfg* cfg)
{
    // ... initialization ...

    // Register commands
    cmd_register("sensor", cmds, ARRAY_SIZE(cmds));
    return 0;
}

// Fault module example commands
// fault status - Show stack usage, reset reason
// fault test pointer - Trigger null pointer fault
// fault test overflow - Trigger stack overflow
// fault data - Dump fault data from flash
// fault data erase - Clear flash for next fault

// Watchdog example commands
// wdg status - Show all watchdog states
// wdg test no_feed_hdw - Stop feeding hardware watchdog

// LWL example commands
// lwl status - Show buffer status
// lwl dump - Output raw buffer
// lwl test - Generate test logs
```

#### Anti-Pattern to Avoid
```c
// No debug interface
// Only way to debug is recompiling with printfs
// Or connecting debugger (not possible in field)

// Commands only in debug builds
#ifdef DEBUG
void register_debug_commands(void) { ... }
#endif
// Commands most useful in production when problems occur!

// No status commands - only tests
static int32_t cmd_sensor_test(int32_t argc, const char** argv) { ... }
// Can trigger tests but can't see current state

// No parameter validation
static int32_t cmd_set_value(int32_t argc, const char** argv)
{
    int value = atoi(argv[1]);  // No argc check, no range check
    set_value(value);            // Crash if argc < 2, wrong if invalid
    return 0;
}

// Complex commands without help
static int32_t cmd_config(int32_t argc, const char** argv)
{
    // Takes many subcommands and parameters
    // No help text shown when used incorrectly
    // Users can't figure out syntax
}
```

#### Course Reference
Lesson 2 (Background) - "Console with commands" listed as maintainability tool
Throughout course - Every module provides status and test commands

---

### 3.4 Magic Numbers

#### Definition
Distinctive constant values placed at the start of data structures to identify section type and validate integrity.

#### Why Important
Binary data in flash or RAM can become corrupted or uninitialized. Magic numbers provide quick validation that data is what you expect. Enables robust parsing of binary formats.

#### Pattern to Adopt
```c
// Define magic numbers with distinctive pattern
#define FAULT_DATA_MAGIC     0xFADE0001
#define LWL_BUFFER_MAGIC     0xFADE0002
#define CONFIG_MAGIC         0xFADE0003
#define END_MARKER_MAGIC     0xFADE0099

// Use in data structures
struct fault_data_section {
    uint32_t magic;        // Always first field
    uint32_t length;       // Section size
    enum fault_type type;
    uint32_t fault_param;
    // ... rest of data ...
};

// Validate before using data
int32_t read_fault_data_from_flash(void)
{
    struct fault_data_section* section =
        (struct fault_data_section*)FAULT_DATA_FLASH_ADDR;

    // Check magic number
    if (section->magic != FAULT_DATA_MAGIC) {
        return ERR_NO_DATA;  // Flash empty or wrong data
    }

    // Check length reasonable
    if (section->length < sizeof(*section) ||
        section->length > MAX_SECTION_SIZE) {
        return ERR_CORRUPT_DATA;
    }

    // Data appears valid - use it
    process_fault_data(section);
    return 0;
}

// Multi-section format with magic numbers
void write_fault_report(void)
{
    // Section 1: Fault data
    struct fault_data_section fault_section = {
        .magic = FAULT_DATA_MAGIC,
        .length = sizeof(fault_section),
        // ...
    };
    flash_write(&fault_section);

    // Section 2: LWL buffer
    struct lwl_buffer_section lwl_section = {
        .magic = LWL_BUFFER_MAGIC,
        .length = sizeof(lwl_section) + buffer_size,
        // ...
    };
    flash_write(&lwl_section);

    // Section 3: End marker
    uint32_t end_marker = END_MARKER_MAGIC;
    flash_write(&end_marker);
}

// Parse multi-section format
void parse_fault_report(uint8_t* data, uint32_t len)
{
    uint32_t offset = 0;

    while (offset < len) {
        uint32_t magic = *(uint32_t*)(data + offset);

        switch (magic) {
            case FAULT_DATA_MAGIC:
                parse_fault_section(data + offset);
                offset += get_section_length(data + offset);
                break;

            case LWL_BUFFER_MAGIC:
                parse_lwl_section(data + offset);
                offset += get_section_length(data + offset);
                break;

            case END_MARKER_MAGIC:
                return;  // End of data

            default:
                LOG_ERROR("Unknown magic: 0x%08lx at offset %lu",
                          magic, offset);
                return;  // Corrupted data
        }
    }
}
```

#### Anti-Pattern to Avoid
```c
// No magic numbers - blind trust
struct fault_data {
    // No magic number
    enum fault_type type;
    uint32_t param;
    // ...
};

void read_fault_data_from_flash(void)
{
    struct fault_data* data = (struct fault_data*)FLASH_ADDR;
    // Use data without validation
    process_fault(data->type, data->param);
    // What if flash is empty (0xFF) or corrupted?
}

// Common values as magic (poor choice)
#define MAGIC 0x00000001  // Too common
#define MAGIC 0xFFFFFFFF  // Matches erased flash!

// Magic number not first
struct config {
    uint32_t version;
    uint32_t length;
    uint32_t magic;  // Too late! Already read version and length
    // ...
};

// No length field - can't validate size
struct data_section {
    uint32_t magic;
    // No length!
    uint8_t data[];  // How much data? Can't validate!
};
```

#### Course Reference
Lesson 4 (Fault Handling) - "Section sizes must be multiple of minimum flash write size"
Binary fault data format uses magic numbers for section identification

---

### 3.5 Critical Sections

#### Definition
Code regions where interrupts are disabled to ensure atomic operations on shared data, with minimal duration to reduce impact.

#### Why Important
Bare-metal systems with interrupts need to protect shared data from race conditions. Critical sections provide mutual exclusion. Must be minimal to maintain responsiveness.

#### Pattern to Adopt
```c
// Protect circular buffer write
void lwl_rec(uint8_t id, int32_t num_arg_bytes, ...)
{
    va_list ap;
    va_start(ap, num_arg_bytes);

    // Start critical section
    __disable_irq();

    // Write LWL ID
    lwl_data.buff[lwl_data.idx_put] = id;
    lwl_data.idx_put = (lwl_data.idx_put + 1) % LWL_BUFFER_SIZE;

    // Write argument bytes
    for (int i = 0; i < num_arg_bytes; i++) {
        lwl_data.buff[lwl_data.idx_put] = va_arg(ap, uint32_t);
        lwl_data.idx_put = (lwl_data.idx_put + 1) % LWL_BUFFER_SIZE;
    }

    // End critical section
    __enable_irq();

    va_end(ap);
}

// Protect flag checked by interrupt and main code
volatile bool data_ready = false;

void main_loop(void)
{
    // Read flag atomically
    __disable_irq();
    bool ready = data_ready;
    data_ready = false;
    __enable_irq();

    if (ready) {
        process_data();
    }
}

void UART_IRQHandler(void)
{
    receive_data();
    data_ready = true;  // Set flag
}

// Measure critical section duration (debugging)
void protected_operation(void)
{
    uint32_t start = DWT->CYCCNT;  // Cycle counter

    __disable_irq();
    // ... critical section ...
    __enable_irq();

    uint32_t cycles = DWT->CYCCNT - start;
    if (cycles > MAX_CRITICAL_SECTION_CYCLES) {
        LOG_WARN("Critical section took %lu cycles", cycles);
    }
}
```

#### Anti-Pattern to Avoid
```c
// No protection on shared data
volatile uint32_t counter = 0;

void main_loop(void)
{
    counter++;  // Read-modify-write not atomic!
    // Interrupt could occur between read and write
}

void Timer_IRQHandler(void)
{
    counter++;  // Race condition!
}

// Critical section too long
void long_operation(void)
{
    __disable_irq();

    // Huge critical section
    for (int i = 0; i < 1000; i++) {
        complex_calculation();  // Milliseconds with IRQs off!
    }
    parse_data();
    format_output();

    __enable_irq();
    // System unresponsive during this time
    // Watchdog might trigger
}

// Forgetting to re-enable
void buggy_function(void)
{
    __disable_irq();

    if (error_condition) {
        return;  // BUG: Forgot __enable_irq()!
    }

    // ... operation ...

    __enable_irq();
}

// Should use proper pairing
void better_function(void)
{
    __disable_irq();

    // ... operation ...

    __enable_irq();
    // All paths enable IRQs
}
```

#### Course Reference
Lesson 3 (Lightweight Logging) - Critical section in lwl_rec() function
Discussion of trade-off between minimizing disabled-interrupt time and simplicity

---

### 3.6 Consistent Error Handling and Return Codes

#### Definition
All functions use same convention for return codes (0 = success, negative = error), error codes defined in central location, errors propagated up call stack.

#### Why Important
Consistency eliminates confusion and bugs. Developers always know how to check for errors. Centralized error definitions prevent conflicts and aid debugging.

#### Pattern to Adopt
```c
// Central error code definitions (error.h)
#define ERR_OK              0
#define ERR_NULL_PTR        -1
#define ERR_INVALID_ARG     -2
#define ERR_INVALID_STATE   -3
#define ERR_TIMEOUT         -4
#define ERR_NO_RESOURCE     -5
#define ERR_COMM_FAIL       -6
// ... comprehensive list ...

// All module functions follow convention
int32_t module_operation(param_t* param)
{
    // Validate inputs
    if (param == NULL) {
        return ERR_NULL_PTR;
    }

    // Call lower-level function
    int32_t rc = lower_level_func();
    if (rc != 0) {
        LOG_ERROR("Lower level failed: %ld", rc);
        return rc;  // Propagate error
    }

    // Success
    return ERR_OK;
}

// Callers always check return codes
int32_t high_level_operation(void)
{
    int32_t rc;

    rc = operation1();
    if (rc != 0) {
        LOG_ERROR("Operation1 failed: %ld", rc);
        return rc;
    }

    rc = operation2();
    if (rc != 0) {
        LOG_ERROR("Operation2 failed: %ld", rc);
        cleanup_after_operation1();
        return rc;
    }

    return ERR_OK;
}

// Error to string helper
const char* error_to_string(int32_t error)
{
    switch (error) {
        case ERR_OK:           return "OK";
        case ERR_NULL_PTR:     return "Null pointer";
        case ERR_INVALID_ARG:  return "Invalid argument";
        case ERR_TIMEOUT:      return "Timeout";
        // ...
        default:               return "Unknown error";
    }
}
```

#### Anti-Pattern to Avoid
```c
// Inconsistent return conventions
int func_a(void) {
    return 0;  // 0 = success
}

int func_b(void) {
    return 1;  // 1 = success (inconsistent!)
}

bool func_c(void) {
    return true;  // Different type!
}

// No centralized error codes
#define ERROR -1  // In file1.c
#define ERROR -1  // In file2.c
#define ERROR 1   // In file3.c (different meaning!)

// Ignoring errors
void operation(void)
{
    int32_t rc = critical_function();
    // Not checking rc!

    continue_anyway();  // Might crash or produce wrong results
}

// No error propagation
int32_t high_level(void)
{
    int32_t rc = low_level();
    if (rc != 0) {
        LOG_ERROR("Error occurred");
        // Not returning error code!
    }
    return 0;  // Caller thinks everything succeeded!
}

// Error codes positive and negative (confusing)
#define ERR_TIMEOUT   -1
#define ERR_OVERFLOW   1  // Why positive? Inconsistent!
#define SUCCESS        0
```

#### Course Reference
Throughout course - All modules use int32_t return codes consistently
Module APIs demonstrate standard error handling pattern

---

### 3.7 Documentation Standards

#### Definition
Code documented with clear headers, function descriptions, parameter documentation, and inline comments explaining non-obvious logic.

#### Why Important
Code is read far more often than written. Good documentation helps future maintainers (including yourself) understand design decisions and usage. Course code demonstrates production documentation standards.

#### Pattern to Adopt
```c
// File header (fault.h example from course)
#ifndef _FAULT_H_
#define _FAULT_H_

/*
 * @brief Interface declaration of fault module.
 *
 * See implementation file for information about this module.
 *
 * MIT License
 *
 * Copyright (c) 2021 Eugene R Schroeder
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 */

// Type definitions clearly separated
////////////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////////////

enum fault_type {
    FAULT_TYPE_WDG = 1,        // Watchdog triggered
    FAULT_TYPE_EXCEPTION,      // CPU exception
};

// Public API clearly marked
////////////////////////////////////////////////////////////////////////////////
// Public (global) function declarations
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Initialize fault module
 * @param cfg Configuration structure (may be NULL for defaults)
 * @return 0 on success, negative error code on failure
 */
int32_t fault_init(struct fault_cfg* cfg);

/**
 * @brief Start fault module operation
 *
 * Fills stack with pattern and configures MPU guard region.
 * Must be called after fault_init() and before any other modules start.
 *
 * @return 0 on success, negative error code on failure
 */
int32_t fault_start(void);

/**
 * @brief Report fault to fault module
 *
 * DOES NOT RETURN. Collects fault data, writes to flash and console,
 * then resets MCU.
 *
 * @param type Type of fault (watchdog, exception, application)
 * @param fault_param Type-specific parameter (watchdog ID, exception number, etc.)
 */
void fault_detected(enum fault_type type, uint32_t fault_param);

// Implementation comments explain why
void fault_start(void)
{
    // Fill stack with pattern for high water mark detection.
    // We fill from current stack pointer down to guard region.
    // Must use inline assembly to get stack pointer without
    // disturbing it with function calls.
    uint32_t sp;
    __asm volatile ("MRS %0, MSP" : "=r" (sp));
    sp -= sizeof(uint32_t);  // Decrement before write (ARM convention)

    for (uint32_t* p = (uint32_t*)sp;
         p >= (uint32_t*)&_sstart_guard;
         p--) {
        *p = FAULT_STACK_INIT_PATTERN;
    }

    // Configure MPU region for stack guard.
    // This catches stack overflow by generating MemManage exception
    // when stack writes to guard region.
    ARM_MPU_SetRegion(/* ... */);
}
```

#### Anti-Pattern to Avoid
```c
// No file header
// No copyright, no license, no description

// No comments on non-obvious code
void x(uint8_t* d, uint32_t l)  // Cryptic names
{
    __disable_irq();
    for (int i = 0; i < l; i++) {
        b[idx] = d[i];
        idx = (idx + 1) % SZ;  // Why modulo? What's this for?
    }
    __enable_irq();
}

// Wrong or outdated comments
// Calculate temperature in Celsius
float temp = raw_value * 0.5;  // Actually returns Fahrenheit!

// Over-commenting obvious things
int32_t add(int32_t a, int32_t b)
{
    // Add a and b
    int32_t result = a + b;

    // Return the result
    return result;
}

// No function documentation
int32_t complex_function(void* ptr, uint32_t flags, callback_t cb);
// What does this do? What are valid flags? When is cb called?
```

#### Course Reference
Throughout course - All header files have comprehensive documentation
Implementation files explain design decisions and non-obvious logic

---

## 4. Cross-Cutting Practices

Practices that support multiple RAM pillars

### 4.1 Super Loop Architecture

#### Definition
Main program structure where initialization completes, then infinite loop calls non-blocking run functions for each module. No RTOS, no threads, deterministic execution.

#### Why Important
Simpler than RTOS for many embedded systems. Predictable timing. No context switching overhead or stack-per-task memory. Easier to debug. Course architecture demonstrates professional super loop design.

#### Pattern to Adopt
```c
// Module run functions are non-blocking
int32_t sensor_run(void)
{
    // Check if work needs doing
    if (!work_pending) {
        return 0;  // Nothing to do - return immediately
    }

    // Do small amount of work
    process_one_item();

    // Return for next module
    return 0;  // Non-blocking!
}

// Timer callbacks trigger work
static void sensor_timer_callback(union tmr_cb_data* data)
{
    // Set flag for run function
    work_pending = true;
}

// Main super loop
int main(void)
{
    // Phase 1: Initialize hardware
    HAL_Init();
    SystemClock_Config();

    // Phase 2: Initialize all modules
    for (int i = 0; i < NUM_MODULES; i++) {
        if (mods[i].init) {
            int32_t rc = mods[i].init(NULL);
            if (rc != 0) {
                // Handle init failure
                LOG_ERROR("Module %s init failed: %ld",
                          mods[i].name, rc);
            }
        }
    }

    // Phase 3: Start all modules (start timers, watchdogs, etc.)
    for (int i = 0; i < NUM_MODULES; i++) {
        if (mods[i].start) {
            int32_t rc = mods[i].start();
            if (rc != 0) {
                LOG_ERROR("Module %s start failed: %ld",
                          mods[i].name, rc);
            }
        }
    }

    wdg_init_successful();  // Signal initialization complete

    // Phase 4: Super loop - run forever
    while (1) {
        for (int i = 0; i < NUM_MODULES; i++) {
            if (mods[i].run) {
                mods[i].run();  // Non-blocking
            }
        }
    }
}

// Interrupts/timers trigger work, run functions execute it
void UART_RX_IRQHandler(void)
{
    uint8_t byte = UART->DR;

    // Put in buffer for processing
    rx_buffer[rx_write_idx++] = byte;

    // Set flag for run function to process
    data_received = true;

    // Return immediately - no processing in ISR
}

int32_t uart_run(void)
{
    if (data_received) {
        data_received = false;
        process_received_bytes();  // Process in run function
    }
    return 0;
}
```

#### Anti-Pattern to Avoid
```c
// Blocking operations in super loop
void sensor_run(void)
{
    delay_ms(100);  // BLOCKS entire system for 100ms!
    read_sensor();
    delay_ms(100);  // Nothing else can run!
}

// Infinite loops in run functions
void process_run(void)
{
    while (queue_not_empty()) {
        process_item();  // Other modules never get to run!
    }
}

// Heavy processing in interrupts
void UART_RX_IRQHandler(void)
{
    uint8_t data[256];
    int len = uart_receive(data, sizeof(data));

    // Bad: Complex processing in interrupt
    parse_message(data, len);
    update_database();
    send_response();

    // Interrupt runs too long
    // Blocks other interrupts
    // Increases latency
}

// No phases - mixed initialization
int main(void)
{
    module1_init();
    module1_start();  // Starting before others initialized!
    module2_init();
    module2_start();
    // Fragile ordering dependencies
}

// Busy waiting in super loop
int main(void)
{
    while (1) {
        // Constantly polling
        if (flag) {
            process();
        }
        // Wasting CPU even when nothing to do
        // Use interrupts + flags instead
    }
}
```

#### Course Reference
Lesson 1 (Introduction) - "Based on super loop and module pattern"
Throughout course - All modules designed for non-blocking super loop

---

### 4.2 Panic Mode Operation

#### Definition
Simplified operation mode when system integrity questionable after fault: disable interrupts, reset stack, use polling instead of interrupts/DMA, minimal dependencies.

#### Why Important
Normal operation assumes working system. After fault, can't trust anything. Panic mode ensures fault handling completes even if system damaged. "Keep fault handling simple - not a place to be clever."

#### Pattern to Adopt
```c
// Enter panic mode immediately
void fault_detected(enum fault_type type, uint32_t fault_param)
{
    // 1. Disable interrupts - all future operations use polling
    __disable_irq();

    // 2. Disable MPU - might be cause of fault
    ARM_MPU_Disable();

    // 3. Reset stack pointer to top of RAM - ensure good stack
    __asm volatile ("MSR MSP, %0" : : "r" (_estack));

    // 4. Use panic versions of functions (polling, blocking)
    console_panic_init();  // Reinitialize with polling
    flash_panic_init();

    // 5. Collect and record fault data
    // These functions use polling, not interrupts
    collect_fault_data(type, fault_param);
    record_to_flash();
    record_to_console();

    // 6. Hardware watchdog as backup
    // If we hang here, hardware watchdog will reset us
    wdg_start_hdw_wdg(5000);  // 5 second timeout

    // 7. Reset system
    NVIC_SystemReset();
}

// Panic console uses polling, not interrupts
void console_panic_send_char(char ch)
{
    // Wait for transmit ready (polling)
    while (!(USART2->SR & USART_SR_TXE)) {
        // Busy wait - OK in panic mode
    }

    USART2->DR = ch;

    // Wait for transmission complete
    while (!(USART2->SR & USART_SR_TC)) {
        // Blocking - OK in panic mode
    }
}

// Panic flash uses blocking writes
int32_t flash_panic_write(uint32_t addr, uint32_t len, const uint8_t* data)
{
    HAL_FLASH_Unlock();

    // Write using HAL (blocking)
    for (uint32_t i = 0; i < len; i += 8) {
        uint64_t dword = *(uint64_t*)(data + i);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr + i, dword);
        // Blocking until complete - OK in panic mode
    }

    HAL_FLASH_Lock();
    return 0;
}
```

#### Anti-Pattern to Avoid
```c
// Using normal functions in fault handler
void fault_detected(enum fault_type type, uint32_t fault_param)
{
    // Trying to use normal console (interrupt-based)
    printc("Fault occurred\n");  // Might hang if interrupts broken

    // Using DMA for flash write
    flash_write_dma(fault_data);  // DMA might not work

    // Calling complex functions with many dependencies
    update_database();  // Too complex, many failure points
    send_network_message();  // Network stack too complex
}

// Not resetting stack pointer
void fault_exception_handler(uint32_t sp)
{
    // Stack might be corrupted
    // Using it without resetting is dangerous

    struct fault_data data;  // Large local variable
    // Might overflow already-corrupted stack!
}

// Depending on interrupts/timers
void fault_detected(enum fault_type type, uint32_t fault_param)
{
    __disable_irq();

    // Wait for timer (but IRQs disabled!)
    while (!timer_expired) {
        // Infinite loop - timer interrupt can't run
    }
}

// Complex fault handling
void fault_detected(enum fault_type type, uint32_t fault_param)
{
    // Trying to be too clever
    analyze_system_state();
    calculate_checksums();
    compress_data();
    encrypt_fault_report();

    // All this complexity increases chance of failure
    // Keep it simple!
}
```

#### Course Reference
Lesson 2 (Background) - "Panic Mode Operation" section
Lesson 4 (Fault Handling) - Panic mode implementation in fault_detected()

---

### 4.3 Offline Analysis Tools

#### Definition
Python or other host-side tools that decode raw binary data from embedded system into human-readable format using source code analysis.

#### Why Important
Embedded systems have limited resources for formatting. Storing raw binary saves space. Offline tools can use powerful host CPU, access source code, provide rich formatting and analysis.

#### Pattern to Adopt
```c
// Embedded: Store compact binary data
void lwl_rec(uint8_t id, int32_t num_arg_bytes, ...)
{
    // Store just ID and raw bytes
    buffer[put++] = id;
    for (int i = 0; i < num_arg_bytes; i++) {
        buffer[put++] = va_arg(ap, uint32_t);
    }
    // No formatting, no string storage
}

void fault_detected(enum fault_type type, uint32_t fault_param)
{
    // Write binary fault data to flash
    flash_write(&fault_data, sizeof(fault_data));

    // Also write raw hex to console for easy capture
    console_dump_hex(&fault_data, sizeof(fault_data));
}

// Python tool: Format offline (log_format.py example)
def format_lwl_logs(source_dir, raw_log_file):
    # Search source code for LWL statements
    lwl_formats = find_lwl_statements(source_dir)

    # Parse raw hex data
    log_buffer = parse_hex_dump(raw_log_file)

    # Decode and format
    for record in log_buffer:
        lwl_id = record[0]
        params = record[1:]

        if lwl_id in lwl_formats:
            format_str = lwl_formats[lwl_id]
            # Format with Python's rich capabilities
            print(format_str % tuple(params))
        else:
            print(f"Unknown LWL ID: {lwl_id}")

# Usage
# $ python log_format.py ./source_code ./captured_logs.txt
# Output: Formatted logs with timestamps, function names, etc.

// Another example: Address-to-function mapping
def analyze_fault_data(source_dir, fault_file, elf_file):
    fault_data = parse_fault_dump(fault_file)

    # Use .elf/.map file to convert addresses to functions
    pc = fault_data['pc']
    func_name, line_num = addr_to_source(elf_file, pc)

    print(f"Fault occurred in {func_name} at line {line_num}")
    print(f"File: {get_file_for_function(func_name)}")

    # Show stack backtrace
    analyze_stack_trace(fault_data['stack_data'], elf_file)
```

#### Anti-Pattern to Avoid
```c
// Formatting on embedded system
void lwl_rec(uint8_t id, const char* fmt, ...)
{
    char buffer[256];  // Wastes RAM

    // Format with sprintf (slow, uses CPU)
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

    // Store formatted string
    strcpy(log_buffer[log_idx++], buffer);
    // Each log takes 256 bytes vs 10-20 bytes for binary
}

// Storing format strings in flash
const char* formats[] = {
    "Temperature: %d.%d C\n",
    "Humidity: %d.%d %%\n",
    // ... hundreds of format strings
    // Wastes precious flash space
};

// No offline tools - manual analysis
// User has to:
// 1. Copy hex dump from terminal
// 2. Manually look up LWL IDs in source code
// 3. Manually decode byte-packed parameters
// 4. Manually look up addresses in map file
// Time-consuming and error-prone!

// Incomplete tooling
def format_logs(raw_file):
    # Only handles one type of log
    # No source code analysis
    # No address-to-symbol conversion
    # User still needs to do manual work
    print(raw_file.read())  # Just prints hex
```

#### Course Reference
Lesson 3 (Lightweight Logging) - Python log_format.py tool
Lesson 4 (Fault Handling) - Formatting tool decodes fault data
"Offline formatting tools decode raw data using source code"

---

### 4.4 Build System Configuration

#### Definition
Linker scripts, startup code, and build configuration modified to support RAM features: reserved flash pages, no-init RAM sections, stack guard regions, MPU alignment.

#### Why Important
Production features require careful memory layout. Default IDE-generated configuration insufficient. Linker scripts control exactly where code and data reside.

#### Pattern to Adopt
```ld
/* Linker script modifications for RAM course features */

/* Constants */
MPU_MIN_BLOCK_SIZE = 32;
FAULT_DATA_PAGE_SIZE = 16K;

MEMORY
{
    /* Reserve flash page for fault data */
    ISR_VECTOR (rx) : ORIGIN = 0x08000000, LENGTH = 16K
    FAULT_DATA (r)  : ORIGIN = 0x08004000, LENGTH = 16K  /* Reserved */
    FLASH (rx)      : ORIGIN = 0x08008000, LENGTH = 480K /* Remaining */

    RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 96K
}

SECTIONS
{
    /* Normal sections... */

    /* No-init section for cross-reset data */
    .noinit.vars (NOLOAD) :
    {
        . = ALIGN(32);
        PROVIDE(_snoinit_vars = .);
        *(.noinit.vars)
        PROVIDE(_enoinit_vars = .);
    } >RAM

    /* Heap and stack with guard */
    .heap_stack :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        PROVIDE(_Heap_Begin = .);
        . = . + _Min_Heap_Size;
        PROVIDE(_Heap_Limit = .);

        /* Stack guard region */
        . = ALIGN(MPU_MIN_BLOCK_SIZE);  /* Must be aligned for MPU */
        PROVIDE(_sstart_guard = .);      /* Guard start */
        . = . + MPU_MIN_BLOCK_SIZE;      /* Allocate 32 bytes */
        PROVIDE(_send_guard = .);        /* Guard end */

        /* Stack (grows downward from _estack) */
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
        PROVIDE(_estack = .);           /* Initial stack pointer */
    } >RAM

    /* Fault data page - provides address */
    .fault_data_page :
    {
        PROVIDE(_fault_data_page = .);
        KEEP(*(.fault_data_page))
    } >FAULT_DATA
}

/* Export symbols for C code */
_sram_start = ORIGIN(RAM);
_sram_end = ORIGIN(RAM) + LENGTH(RAM);
_fault_data_flash_addr = ORIGIN(FAULT_DATA);
```

```c
// Access linker symbols in C
extern uint32_t _sstart_guard;  // Stack guard start
extern uint32_t _send_guard;    // Stack guard end
extern uint32_t _estack;        // Stack top
extern uint32_t _fault_data_flash_addr;  // Fault data location

void configure_stack_guard(void)
{
    ARM_MPU_SetRegion(
        0,  // Region number
        (uint32_t)&_sstart_guard,  // Start address
        ARM_MPU_REGION_SIZE_32B,   // 32 bytes
        ARM_MPU_ACCESS_RO,         // Read-only
        0);
}

// Startup file modifications (startup_stm32f401xe.s)
Default_Handler:
    MRS r0, MSP              // Save stack pointer to R0 (arg 1)
    LDR r1, =_estack         // Load initial stack value
    MSR MSP, r1              // Reset stack pointer
    B fault_exception_handler // Jump to C fault handler
```

#### Anti-Pattern to Avoid
```c
// Using default linker script
// No reserved flash page - fault data overwrites code!
// No no-init section - cross-reset counting doesn't work
// No stack guard - stack overflows undetected
// All RAM given to heap - stack too small

// Hard-coded addresses instead of linker symbols
#define STACK_GUARD_START 0x20018220  // Fragile!
#define STACK_END         0x20018000  // Changes if RAM size changes

// Not aligning for MPU
.heap_stack :
{
    _sstart_guard = .;  // No alignment!
    . = . + 32;
    // MPU requires 32-byte alignment - this will fail
} >RAM

// No startup file modifications
// Default handlers just infinite loop
// No fault_exception_handler call
// Can't collect fault data

// Mixing code and reserved sections
.text : {
    *(.text)
    *(.rodata)
    . = 0x4000;  // Try to reserve space
    // Doesn't work - code might overflow into it
} >FLASH
```

#### Course Reference
Lesson 4 (Fault Handling) - Linker script modifications for fault data page
Lesson 5 (Watchdogs) - No-init RAM section for cross-reset counter
Lesson 6 (Stack Overflow Protection) - Stack guard in linker script, startup file modifications

---

### 4.5 Separation of Concerns

#### Definition
Each module has single, well-defined responsibility. Fault handling in fault module, logging in LWL module, watchdogs in WDG module, etc. Clear interfaces between modules.

#### Why Important
Maintainability through modularity. Bugs isolated to responsible module. Can test/modify one module without affecting others. Can reuse modules in other projects.

#### Pattern to Adopt
```c
// Each module has specific responsibility

// lwl module: Logging only
int32_t lwl_rec(uint8_t id, int32_t num_arg_bytes, ...);
uint8_t* lwl_get_buffer(uint32_t* len);
// Does NOT handle faults, watchdogs, flash, etc.

// fault module: Fault handling only
void fault_detected(enum fault_type type, uint32_t fault_param);
uint32_t fault_get_rcc_csr(void);
// Does NOT do logging (calls LWL), flash writes (calls flash module)

// wdg module: Watchdog monitoring only
int32_t wdg_register(uint32_t wdg_id, uint32_t period_ms);
int32_t wdg_feed(uint32_t wdg_id);
// Does NOT handle faults (calls callback), timing (uses timer module)

// flash module: Flash operations only
int32_t flash_panic_erase_page(uint32_t page_num);
int32_t flash_panic_write(uint32_t addr, uint32_t len, const uint8_t* data);
// Does NOT decide what to store (fault module decides)

// Clear interfaces - modules don't know about each other's internals
void fault_detected(enum fault_type type, uint32_t fault_param)
{
    // Fault module uses LWL interface
    uint32_t lwl_len;
    uint8_t* lwl_buf = lwl_get_buffer(&lwl_len);  // Public API

    // Doesn't know how LWL stores data internally
    // Just gets buffer and length

    // Fault module uses flash interface
    flash_panic_write(addr, lwl_len, lwl_buf);  // Public API

    // Doesn't know flash implementation details
}

// Callback for cross-module communication
void wdg_init(void)
{
    // Watchdog module doesn't depend on fault module
    // Uses callback pattern for loose coupling
}

void fault_init(void)
{
    // Register to receive watchdog triggers
    wdg_register_triggered_cb(watchdog_triggered_callback);
}

static void watchdog_triggered_callback(uint32_t wdg_id)
{
    // Fault module handles watchdog triggers
    fault_detected(FAULT_TYPE_WDG, wdg_id);
}
```

#### Anti-Pattern to Avoid
```c
// God module - does everything
void system_module_run(void)
{
    // Logging
    log_message("Running");

    // Watchdog
    check_watchdogs();

    // Fault handling
    if (fault_detected) {
        save_fault_data();
    }

    // Flash management
    write_to_flash();

    // Sensor reading
    read_sensors();

    // Display update
    update_display();

    // Everything in one module - hard to maintain!
}

// Tight coupling - modules know each other's internals
void fault_detected(void)
{
    // Directly accessing other module's internals
    memcpy(fault_data, lwl_module.buffer, lwl_module.size);  // BAD!
    flash_module.write_ptr = flash_module.base_addr;         // BAD!
    wdg_module.enabled = false;                              // BAD!

    // Should use public APIs instead
}

// Circular dependencies
// fault.h includes wdg.h
// wdg.h includes fault.h
// Can't compile!

// Module doing too much
void logging_module_run(void)
{
    // Logging, but also...
    check_flash_space();       // Should be flash module's job
    monitor_system_health();   // Should be separate monitoring
    handle_faults();           // Should be fault module's job
}
```

#### Course Reference
Lesson 1 (Introduction) - "All new modules conform to established API"
Throughout course - Clean separation: lwl, fault, wdg, flash modules with clear responsibilities

---

### 4.6 Testing and Debugging Support

#### Definition
Every module provides console commands to trigger test scenarios, inject faults, display status, and verify operation. Built into production code for field debugging.

#### Why Important
"Ability to test fault handling is critical." Can't test rare conditions without injection. Console commands enable field debugging. Demonstrates confidence in code quality.

#### Pattern to Adopt
```c
// Fault module test commands
static int32_t cmd_fault_test(int32_t argc, const char** argv)
{
    if (argc < 2) {
        printc("Usage: fault test <test>\n");
        printc("Tests:\n");
        printc("  pointer  - Null pointer dereference\n");
        printc("  overflow - Stack overflow\n");
        printc("  divzero  - Divide by zero\n");
        return 0;
    }

    if (strcasecmp(argv[1], "pointer") == 0) {
        // Intentionally trigger fault
        uint32_t bad = 0xBAD;
        *(volatile uint32_t*)0xFFFFFFFF = bad;
    } else if (strcasecmp(argv[1], "overflow") == 0) {
        test_stack_overflow();  // Infinite recursion
    } else if (strcasecmp(argv[1], "divzero") == 0) {
        volatile int x = 5;
        volatile int y = 0;
        volatile int z = x / y;  // Trigger usage fault
        (void)z;
    }

    return 0;
}

// Watchdog module test commands
static int32_t cmd_wdg_test(int32_t argc, const char** argv)
{
    if (strcasecmp(argv[1], "no_feed_hdw") == 0) {
        // Stop feeding hardware watchdog
        stop_feeding_hdw = true;
        printc("Stopped feeding hardware watchdog\n");
        printc("System will reset in ~%lu seconds\n",
               HDW_WDG_TIMEOUT_MS / 1000);
        return 0;
    }

    return 0;
}

// Status commands for visibility
static int32_t cmd_wdg_status(int32_t argc, const char** argv)
{
    uint32_t now = tmr_get_ms();

    printc("Watchdog Status:\n");
    printc("ID  Period(ms)  LastFeed  Elapsed  Status\n");

    for (uint32_t i = 0; i < NUM_WATCHDOGS; i++) {
        if (wdg_state.watchdogs[i].period_ms > 0) {
            uint32_t elapsed = now - wdg_state.watchdogs[i].last_feed_time_ms;
            const char* status = (elapsed > wdg_state.watchdogs[i].period_ms)
                                 ? "EXPIRED" : "OK";

            printc("%2lu  %10lu  %8lu  %7lu  %s\n",
                   i,
                   wdg_state.watchdogs[i].period_ms,
                   wdg_state.watchdogs[i].last_feed_time_ms,
                   elapsed,
                   status);
        }
    }

    return 0;
}

// Stack usage visibility
static int32_t cmd_fault_status(int32_t argc, const char** argv)
{
    // Calculate stack usage
    uint32_t* p = (uint32_t*)&_sstart_guard;
    while (*p == FAULT_STACK_INIT_PATTERN && p < (uint32_t*)_estack) {
        p++;
    }
    uint32_t used = _estack - (uint32_t)p;
    uint32_t total = _estack - (uint32_t)&_sstart_guard;

    printc("Stack: %lu / %lu bytes used (%.1f%%)\n",
           used, total, 100.0 * used / total);

    return 0;
}
```

#### Anti-Pattern to Avoid
```c
// No test commands
// Can't trigger faults for testing
// Can't verify recovery mechanisms work
// No confidence in fault handling

// Test code only in debug builds
#ifdef DEBUG
static int32_t cmd_test(int32_t argc, const char** argv) { ... }
#endif
// Can't test in field when problems occur!

// No status visibility
// Can't see what system is doing
// Can't diagnose problems
// Black box operation

// Dangerous test commands
static int32_t cmd_erase_all_flash(void)
{
    flash_erase_all();  // No confirmation!
    // Accidentally typed command destroys system
}

// Should require confirmation for destructive operations
static int32_t cmd_erase_all_flash(int32_t argc, const char** argv)
{
    if (argc < 2 || strcasecmp(argv[1], "confirm") != 0) {
        printc("This will erase all flash! Type 'erase_all confirm' to proceed.\n");
        return 0;
    }

    printc("Erasing...\n");
    flash_erase_all();
    return 0;
}
```

#### Course Reference
Throughout course - Every module provides comprehensive test and status commands
Lesson 2 (Background) - Console commands called out as key maintainability feature

---

## 5. Quick Reference Checklist

Use this checklist when writing or reviewing code:

### Reliability Checklist
- [ ] All function parameters validated (NULL checks, range checks)
- [ ] All external inputs validated (sensors, communications, user input)
- [ ] Error codes defined and used consistently
- [ ] Errors checked and handled (not ignored)
- [ ] Critical data structures have integrity checks (magic numbers, checksums)
- [ ] Asserts used for development-time checking
- [ ] Fault detection mechanisms in place (exceptions, watchdogs, app-level checks)

### Availability Checklist
- [ ] Automatic recovery implemented (reset on fault)
- [ ] Watchdogs registered for all critical tasks
- [ ] Watchdogs fed only when work completes successfully (not unconditionally)
- [ ] Hardware watchdog as backup for software watchdog system
- [ ] Initialization watchdog with retry limit
- [ ] All blocking operations have timeouts
- [ ] Graceful degradation when components fail
- [ ] System can restart cleanly after reset

### Maintainability Checklist
- [ ] Module follows standard pattern (init, start, run)
- [ ] Configuration uses struct (not individual parameters)
- [ ] Lightweight logging used for activity recording
- [ ] Console commands provided (status, test)
- [ ] Functions documented (purpose, parameters, return)
- [ ] Non-obvious code has explanatory comments
- [ ] Magic numbers used for data structure validation
- [ ] Error messages include context (which module, what operation)
- [ ] Resource usage tracked and reported (stack, buffers, flash)

### Cross-Cutting Checklist
- [ ] Run functions are non-blocking (return quickly)
- [ ] Interrupts set flags, run functions do processing
- [ ] Critical sections minimized (IRQs disabled briefly)
- [ ] Critical sections properly paired (__enable_irq after __disable_irq)
- [ ] Panic mode used for fault handling (simple, minimal dependencies)
- [ ] Offline tools provided for analysis (log formatting, fault decoding)
- [ ] Build system configured correctly (linker script, startup code)
- [ ] Module has single, clear responsibility
- [ ] Module uses other modules via public APIs (not internals)
- [ ] Test commands provided for fault injection

### Before You Commit
- [ ] Code compiles without warnings
- [ ] All functions have error checking
- [ ] New code follows existing patterns
- [ ] Console commands added for new features
- [ ] Logging added for significant events
- [ ] Watchdog considerations addressed (feeds added if needed)
- [ ] Stack usage reasonable (no huge local variables)
- [ ] Flash usage reasonable (no large constant arrays)
- [ ] Tested normal operation
- [ ] Tested error cases
- [ ] Tested recovery from failures

---

## Conclusion: From University to Production

The practices in this guide represent the difference between code that works on your desk and code that works reliably in the field for years. Key mindset shifts:

**University Code**:
- Assumes correct inputs
- Doesn't check for errors
- No recovery from failures
- No visibility into operation
- Works once, throws away

**Production Code**:
- Validates all inputs defensively
- Checks and handles every error
- Detects and recovers from faults automatically
- Provides extensive debug/test capabilities
- Runs for years in harsh field conditions

The RAM course practices give you a proven architecture for production embedded systems. Start adopting these practices now, and your code will immediately become more robust, maintainable, and professional.

Remember the core principle: **Assume failures will occur. Detect them. Record information. Recover automatically.**

This is what separates professional embedded engineers from beginners.
