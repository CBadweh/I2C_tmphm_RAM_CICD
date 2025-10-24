# Summarize introduction to RAM course
_Exported on 10/23/2025 at 21:11:19 EDT from Cursor (1.7.54)_

---

**User**

Summarize @Reliability, Availability, and Maintainability Lesson 1. Introduction to the Course.txt   compredhensively. 

---

**Cursor**

# Comprehensive Summary: RAM Lesson 1 - Introduction to the Course

## Course Overview
This is a 7-video course (approximately 3 hours total) by Gene Schrader on **Reliability, Availability, and Maintainability (RAM)** theory and practice for STM32 embedded systems. RAM is particularly important for embedded systems and represents a key distinction between university demos and real-world products.

## Course Objectives

1. **Learn RAM Terminology** - Understanding the meaning and abbreviation (RAM) to communicate precisely with others and understand documentation

2. **Master RAM Techniques** - Each technique covered in a separate lesson:
   - **Lightweight Logging** - Circular buffer dumped to flash on faults (like a flight recorder)
   - **Fault Handling** - Processing different fault types in "panic mode" (core software)
   - **Watchdog Timers** - Monitoring software execution
   - **Stack Overflow Protection** - Memory management safeguards
   - **Audits and Asserts** - Discussion-only lesson (no implementation/demo)

3. **Field-Grade Implementations** - Complete, sound designs (not just demos) that conform to super loop architecture and provide debug/test capabilities

## Prerequisites

- **Previous Course**: Gene's base course on bare metal embedded STM32 (or similar MCU/embedded knowledge)
- **C Programming**: Beyond beginner level
- **Willingness**: To learn new concepts, technologies, and dig into reference manuals

This course is more advanced than the base course, covering exception handling, stack details, linker scripts, and system programming.

## Hardware & Software

**Hardware:**
- STM32 Nucleo board (STM32F401RE MCU)
- Adafruit temperature/humidity sensor (minor role)

**Software (all free):**
- STM32CubeIDE (for debugging during development)
- Python (for fault report decoding tool)
- Infrastructure code from previous courses

**Note**: You can watch without hardware to learn concepts, or use different hardware with modifications.

## Software Architecture

The course builds on a super loop architecture with modular design:

**New Modules** (outlined in red in the course diagram):
- Lightweight logging module
- Fault module (core of the course)
- Watchdog module  
- Flash module (for writing data)

**Module Categories:**
- Gray background = Hardware interface modules (using STM32 LL library or CMSIS)
- Yellow background = Hardware-independent modules using abstracted APIs
- Infrastructure modules = Pre-existing from base course (timers, console, etc.)

Code above the "purple line" = Custom code (in GitHub repo)  
Code below = Standard C libraries and IDE-generated code

## Additional Notes

- Corrections/clarifications posted in pinned YouTube comments and video notes
- **Two GitHub repos**:
  1. Source code repo (base course + enhancements, excluding IDE code)
  2. Course materials repo (documentation + STM32CubeIDE project zip with IDE code)
- Developed on Windows 10, but tools support Linux and macOS
- Software-oriented course, but hardware also plays a role in RAM and safety

## Key Insight
RAM techniques handle "very bad events you hope will never happen" - essential panic mode processing that Gene's 40 years of embedded experience shows does occur in real-world products.

---

**User**

Summarize @Reliability, Availability, and Maintainability Lesson 2. Background.txt  compredhensively. 


---

**Cursor**

# Comprehensive Summary: RAM Lesson 2 - Background

## Lesson Overview
This lesson covers the theoretical foundation of Reliability, Availability, and Maintainability (RAM), which is applied practically in subsequent lessons. It includes terminology definitions, fault handling principles, and maintainability support strategies.

## Lesson Outline
1. Facts of software life
2. RAM terminology
3. Introduction to fault handling
4. Maintainability support for non-fault scenarios
5. Discussion prompts/questions

---

## Facts of Software Life

**Reality of Software Development:**
- **Bugs are inevitable** in complex systems
- **New features → new bugs** (expected)
- **Bug fixes → new bugs** (the "whack-a-mole" problem)
- Products often ship with **known non-serious bugs**; unknown bugs are the real concern
- Near major releases, minor bugs may not be fixed to avoid introducing worse issues

**Defensive Programming Principles:**
- Weaknesses in defensive programming **are bugs themselves**
- External devices/systems will sometimes misbehave
- Users will not behave as expected
- **IETF RFC quote**: "Assume the network is filled with malevolent entities that will send packets designed to have the worst possible effect"
- If your code can't handle unexpected/wrong behavior from external sources or users, **you have a bug**

**Critical Practice:**
- **Analysis of potential failures** is essential for embedded systems (even non-safety-critical ones)
- Key questions: What can go wrong? How can I detect and react to these failures?

---

## Terminology Definitions

### 1. **Reliability**
- **Definition**: Ability of the system to function without failure
- **Measurement**: Failures per unit of time (MTBF - Mean Time Between Failures, e.g., every 5 years)
- **Application**: Works well for hardware that wears out; harder to apply to software
- **Reality**: Reliability plans based on bug-free code are **unrealistic**
- **Course approach**: Assume failures will happen; focus on **how to deal with them**
- **Technique example**: Protection against double-free in heap systems

### 2. **Availability**
- **Definition**: Ability of a system to function at any point in time
- **Measurement**: Percentage of time the system is up/functioning
- **Affected by**:
  - MTBF (fewer failures = higher availability)
  - MTTR (Mean Time To Repair - how quickly system recovers)
- **Course focus**: Detection of failures and **automatic repair** to reduce MTTR (e.g., automatic system reset)

### 3. **Maintainability**
- **Definition**: Ease with which a problem can be corrected

**Two perspectives:**

**A. Customer's Point of View:**
- Features/tools to diagnose and fix:
  - Configuration/installation errors
  - Hardware problems
  - Attached system problems
- Should have **explicit product requirements**
- Not the focus of this course

**B. Developer's Point of View:**
- Features/tools to diagnose and fix **software bugs**
- **Scenarios**:
  1. System has problems while running (accessible in lab)
  2. System experienced failure, auto-restarted, now running fine (more difficult)
- Usually **no written requirements** (internal to development team)
- **Note**: Code quality and development environment (readability, testing) are important but outside course scope

### 4. **Safety**
- **Definition**: Ability of a system to not harm people, environment, or assets
- **Not the focus** of this course (quickly becomes application-dependent)

### 5. **Resiliency & Robustness**
- **General meaning**: Ability to function in the face of adversity
- **Vague terms** - "adversity" can mean:
  - Unexpected behavior of external device/system
  - Software stuck in a loop
- **Relationship**: Can be viewed as components needed to achieve reliability and availability

### 6. **Non-Functional Requirements**
- **Definition**: Not *what* the system must do, but *how* it must do it
- **Includes**: Reliability, availability, maintainability, security, safety
- **Typically**: Customer's point of view
- **Can also include**: Developer's point of view (features/tools for debugging)

---

## Fault Handling - Key Topic

### **Three-Step Process:**

#### 1. **Detect the Fault**
- **Addresses**: Availability (must know when something is wrong)
- **Worst field events**: System messed up but doesn't know it
- **Techniques**: Watchdog timers, CPU exceptions, application checks

#### 2. **Collect Information**
- **Addresses**: Maintainability (helps developers diagnose/fix problems)
- **Information includes**:
  - Return address
  - CPU registers
  - Fault registers
  - Circular buffer logs

#### 3. **Perform Recovery Actions** (Policy-based)

**Policy A: Fail-Safe**
- **Addresses**: Safety
- System shuts down safely (e.g., all outputs off, stay idle)
- Hardware design often plays big role
- **No automatic restart** - requires higher-level action (e.g., person)
- **Example**: Robot controller in factory - if fault occurs mid-task, stays idle until people intervene
- **Used when**: Safety is most important

**Policy B: Automatic Recovery and Restart**
- **Addresses**: Availability
- Hardware/software reset of part or all system
- Attempts to automatically restart operation
- **Used when**: Availability is most important (communications, entertainment, consumer electronics)

---

## Fault Handling Design Principles

1. **Highest Quality Code**: Bugs in fault handling can leave system in "zombie state"
2. **Keep it Simple**: Easy to read and understand - not a place to be clever
3. **Full Hardware Reset Advantage**: Single recovery action (full reset) is simplest if acceptable
4. **Escalation Complexity**: Partial recovery (e.g., reinitialize I²C controller) requires escalation logic if it fails
5. **When to avoid full reset**: Only when absolutely necessary - worth starting small and escalating

### **Panic Mode Operation**

**When**: Overall system integrity is questionable (e.g., memory corruption possible)

**Goal**: Get system into basic sane state, operate simply, depend on minimal resources

**Actions:**
- **Disable interrupts** - all future operations in polling mode
- Use **special panic versions** of console/flash output functions (polling, blocking)
- For RTOS: Disabling interrupts should stop scheduling (verify this)
- **Validate/reset stack pointer** to initial value (like power-up)
- **Consider reinitializing peripherals** (e.g., UART) - not done in this course due to hardware init code organization
- **Hardware watchdog backup**: If fault handling gets stuck, watchdog resets system

---

## Maintainability Support (Non-Fault Scenarios)

**Techniques used in Gene's work and courses:**

### 1. **Console and Console Commands**
- Your "window into the system"
- Low cost with today's MCUs
- **Uses**:
  - Get internal module status
  - Control logging

### 2. **Multi-Level Logging (Per Module)**
- Walk up to system and turn on logging for specific module at specific level
- **Output devices**:
  - Console (particularly for small systems)
  - USB drive (convenient for lab/field - holds lots of data)

### 3. **Performance Measurements (PMs)**
- **Counters** of unusual events (e.g., bus errors, communication errors)
- **Flags** indicating unusual conditions (e.g., alarms)
- Often displayed/reset using console commands
- **Uses**:
  - Check long-running systems for issues
  - Debug communication interfaces (check framing errors, etc.)

---

## Discussion Questions & Answers

### Q1: Why is reliability sometimes best when a product enters maintenance-support-only phase?
**A**: Software updates are only for bug fixes (no new features), so fewer bugs are introduced. Overall bug count decreases over time, making the product very reliable.

### Q2: If a system must be 99.999% available (five 9's), how much downtime per year is allowed?
**A**: Calculation: 365 days × 24 hours × 60 minutes × 0.00001 = **5.25 minutes per year**
- Common expectation for communication network equipment
- Often achieved using redundant hardware that can be switched/replaced while running

### Q3: If a system runs at only 50% capacity, how should this be treated from an availability viewpoint?
**A**: 
- Only applies to systems with partial capacity concept (e.g., communication systems)
- Availability can be **pro-rated**: n minutes at 50% capacity = n/2 minutes of downtime
- Important for service level agreements
- Large distributed systems can update in stages piece-by-piece, potentially eliminating downtime

### Q4: With a communication device, if all links are down, should you treat it as a fault?
**A**: 
- **Normally no** - assume problem is external
- **But consider**: If system is doing nothing useful, might treat as fault after timeout (e.g., 15 minutes) and reset
- Real-world example: Communication processors resetting every 15 minutes when all links down (discovered in lab work)

---

## Key Takeaway
This lesson establishes the theoretical foundation showing that **failures are inevitable in complex systems**, and the focus should be on detection, information collection, and appropriate recovery actions based on whether safety or availability is the priority.

---

**User**

Summarize @Reliability, Availability, and Maintainability Lesson 3. Lightweight Logging.txt  compredhensively. include the related code

The source code is in @app_main.c @lwl.c @lwl.py 

don't make things up

---

**Cursor**

# Comprehensive Summary: RAM Lesson 3 - Lightweight Logging

## Overview
Lightweight logging (LWL) is a mechanism to record software activity using minimal CPU time and memory compared to traditional logging. It's designed to run continuously on field products without affecting operation, acting as a "flight recorder" for embedded systems.

---

## Key Objectives

1. **Enable always-on logging** in production systems without performance impact
2. **Capture pre-fault activity** to understand what happened before crashes/watchdog triggers
3. **Minimal runtime overhead** - no runtime formatting of log messages
4. **Compact storage** in circular RAM buffer

---

## Requirements

### **Usage Requirements**
- **Printf-like API** - familiar to C programmers
- **Simple to add** log statements to files
- **Usable in base-level and interrupt contexts** (bare metal requirement)

### **Low Overhead Requirements**
- **No runtime formatting** - integers not converted to text during logging
- **Format strings not stored** on the system (not in flash image)
- **Support integer-like types** only (enums, booleans, pointers) - strings and floats NOT required
- **User-specified byte lengths** - save space by storing only needed bytes (e.g., 1 byte instead of 4 for small values)

### **General Requirements**
- **No source file editing during build** - uses macros instead of preprocessing
- **Non-blocking** (super loop requirement)
- **Test and debug support** via console commands
- **Offline formatting tools** decode raw data using Python

---

## Design Architecture

### **Data Storage**
```c
struct lwl_data {
    uint32_t magic;
    uint32_t num_section_bytes;
    uint32_t buf_size;
    uint32_t put_idx;           // Points to next write location
    uint8_t buf[LWL_BUF_SIZE];  // Circular buffer (default 1008 bytes)
};
```

From `lwl.c` lines 66-73:

```66:73:ram-class-nucleo-f401re/modules/lwl/lwl.c
struct lwl_data
{
    uint32_t magic;
    uint32_t num_section_bytes;
    uint32_t buf_size;
    uint32_t put_idx;
    uint8_t buf[LWL_BUF_SIZE];
};
```

### **Record Format**
Each log record in the circular buffer consists of:
1. **LWL ID** (1 byte) - unique identifier for the log statement
2. **Argument bytes** (variable length) - parameter values stored in big-endian format

**Example**: `LWL("state=%d mask=0x%04x", 3, LWL_1(state), LWL_2(0xFF00))`
- Takes 4 bytes total: `[ID=20][state_value][mask_high_byte][mask_low_byte]`

---

## C-Level API and Usage

### **ID Management**
Each source file must define its ID range:

```c
#define LWL_BASE_ID 20
#define LWL_NUM 10
```

This file can use IDs 20-29. Compile-time errors occur if exceeded.

### **Logging Macro**
From `lwl.h` (referenced in transcript), the user calls:

```c
LWL("format string", total_arg_bytes, LWL_1(param1), LWL_2(param2), ...)
```

**Byte specification macros**:
- `LWL_1(x)` - Store 1 byte
- `LWL_2(x)` - Store 2 bytes  
- `LWL_3(x)` - Store 3 bytes
- `LWL_4(x)` - Store 4 bytes

### **Core Recording Function**

```162:189:ram-class-nucleo-f401re/modules/lwl/lwl.c
void lwl_rec(uint8_t id, int32_t num_arg_bytes, ...)
{
    CRIT_STATE_VAR;
    va_list ap;
    uint32_t put_idx;

    va_start(ap, num_arg_bytes);

    CRIT_BEGIN_NEST();
    if (lwl_off_cnt != 0 && --lwl_off_cnt == 0)
        _lwl_active = false;
    put_idx = lwl_data.put_idx % LWL_BUF_SIZE;
    lwl_data.put_idx = (put_idx + 1 + num_arg_bytes) % LWL_BUF_SIZE;

    // We could end the critical section here, since the put index has been
    // updated. But there is a chance that a fault occurs before all of the
    // argument bytes have been written, such that the buffer would contain
    // invalid data.
    //
    // For now we play it safe.

    lwl_data.buf[put_idx] = id;
    while (num_arg_bytes-- > 0) {
        put_idx = (put_idx + 1) % LWL_BUF_SIZE;
        lwl_data.buf[put_idx] = (uint8_t)va_arg(ap, unsigned);
    }
    CRIT_END_NEST();
}
```

**Key implementation details**:
- **Critical section** protects against interrupts/concurrent access
- **Modulo arithmetic** handles circular buffer wrapping
- **Put index** updated before writing data
- **Extended critical section** to prevent fault mid-write (lines 176-181 comment)

### **Enable/Disable Control**

```196:199:ram-class-nucleo-f401re/modules/lwl/lwl.c
void lwl_enable(bool on)
{
    _lwl_active = on;
}
```

### **Buffer Access for Fault Module**

```208:213:ram-class-nucleo-f401re/modules/lwl/lwl.c
uint8_t* lwl_get_buffer(uint32_t* len)
{
    prepare_data_for_output();
    *len = sizeof(lwl_data);
    return (uint8_t*)&lwl_data;
}
```

This allows the fault module to dump the buffer to flash on system crashes.

---

## Console Commands

From `lwl.c` lines 97-118:

```97:118:ram-class-nucleo-f401re/modules/lwl/lwl.c
static struct cmd_cmd_info cmds[] = {
    {
        .name = "status",
        .func = cmd_lwl_status,
        .help = "Status",
    },
    {
        .name = "test",
        .func = cmd_lwl_test,
        .help = "Test logging",
    },
    {
        .name = "enable",
        .func = cmd_lwl_enable,
        .help = "Recording on/off (1/0)",
    },
    {
        .name = "dump",
        .func = cmd_lwl_dump,
        .help = "Dump buffer",
    },
};
```

1. **`lwl status`** - Shows if logging is active and current put_idx
2. **`lwl test`** - Generates 4 test log entries then disables logging
3. **`lwl enable {0|1}`** - Turn logging on/off
4. **`lwl dump`** - Outputs raw hexadecimal buffer data

### **Test Command Implementation**

```255:264:ram-class-nucleo-f401re/modules/lwl/lwl.c
static int32_t cmd_lwl_test(int32_t argc, const char** argv)
{
    LWL("test 1", 0);
    LWL("test 2 %d", 1, LWL_1(10));
    LWL("test 3 %d %d", 3, LWL_1(10), LWL_2(1000));
    LWL("test 4 %d %d %d", 7, LWL_1(10), LWL_2(1000), LWL_4(100000));

    lwl_enable(false);
    return 0;
}
```

---

## Module Integration

From `app_main.c`, LWL is started early and enabled immediately:

```543:549:ram-class-nucleo-f401re/app1/app_main.c
#if CONFIG_FAULT_PRESENT
    wdg_start_init_hdw_wdg();
#endif

#if CONFIG_LWL_PRESENT
    lwl_enable(true);
#endif
```

The module is registered in the module list:

```373:379:ram-class-nucleo-f401re/app1/app_main.c
#if CONFIG_LWL_PRESENT
    {
        .name = "lwl",
        .instance = MOD_NO_INSTANCE,
        .ops.singleton.mod_start = (mod_start)lwl_start,
    },
#endif
```

---

## Python Formatting Tool (lwl.py)

The offline formatting tool decodes raw LWL data dumps:

### **Key Classes**

**`LwlMsg`** - Metadata for a single log message:

```13:32:ram-class-nucleo-f401re/modules/lwl/lwl.py
class LwlMsg:
    """
    This class represents the meta data for a single LWL messages.

    This data is extracted from source files, and is used to format the raw
    data.
    """

    def __init__(self, id, fmt, arg_bytes, file_path, line_num, lwl_statement):
        self.id = id
        self.fmt = fmt
        self.arg_bytes = arg_bytes
        self.file_path = file_path
        self.line_num = line_num
        self.lwl_statement = lwl_statement
        self.num_arg_bytes = 0
        for d in self.arg_bytes:
            self.num_arg_bytes += int(d)
        _log.debug('Create LwlMsg ID=%d arg_bytes=%s(%d) for %s:%d' %
                   (id, arg_bytes, self.num_arg_bytes, file_path, line_num))
```

**`LwlMsgSet`** - Collection of all log messages with error checking:

```34:71:ram-class-nucleo-f401re/modules/lwl/lwl.py
class LwlMsgSet:
    """
    This class represents the set of meta data for all LWL messages.

    This data is extracted from source files, and is used to format the raw
    data. Error checking is done to ensure consistency of the message set.
    """

    def __init__(self):
        self.lwl_msgs = {}
        self.max_msg_len = 0
    def add_lwl_msg(self, id, fmt, arg_bytes, lwl_statement, file_path,
                    line_num):
        if id in self.lwl_msgs:
            print('%s:%d Duplicate ID %d: %s, previously at %s:%d: %s' %
                  (file_path, line_num, id, lwl_statement,
                   self.lwl_msgs[id].file_path,
                   self.lwl_msgs[id].line_num, 
                   self.lwl_msgs[id].lwl_statement))
            return False
        else:
            self.lwl_msgs[id] = LwlMsg(id, fmt, arg_bytes,
                                       file_path, line_num, lwl_statement)
            msg_len = 1
            for l in arg_bytes:
                msg_len += int(l)
            if msg_len > self.max_msg_len:
                self.max_msg_len = msg_len
                _log.debug('New max msg len %d for %s:%d %s' %
                           (msg_len, file_path, line_num, lwl_statement))
        return True

    def get_metadata(self, id):
        try:
            return self.lwl_msgs[id]
        except KeyError:
            return None
```

### **Processing Steps**

1. **Parse source files** to extract LWL statements and format strings (lines 94-249)
2. **Find optimal start index** using heuristic to minimize invalid IDs (lines 291-357)
3. **Decode and print** formatted messages (lines 359-420)

### **Optimal Start Index Algorithm**

Because the put_idx might point into the middle of an overwritten message, the tool uses a brute-force approach:

```291:357:ram-class-nucleo-f401re/modules/lwl/lwl.py
def get_optimal_start_idx(data, put_idx):
    """
    Find the optimal starting index to decode log buffer.

    Parameters:
        data (bytearray)  : Log data buffer.
        put_idx (idx)     : The index of the next buffer byte to write log data.

    Return:
        The optimal starting index in [put_idx, put_idx + max_msg_len - 1]

    This is a brute-force solution where we start at the known "put idx", add an
    offset to it, and using that starting point, determine how invalid log
    message IDs are encountered

    The maximum offset is the one less than the length of the largest log
    message (ID plus argument bytes).

    We assume that the optimal starting point is the one that minimizes the
    number of invalid log IDs.  This is not guaranteed to be the correct offset,
    but likely is.

    """

    _log.debug('get_optimal_start_idx(put_idx=%d)' % put_idx)
    min_invalid_ids = len(data)
    optimal_start_idx = None
    num_data_bytes = len(data)
    for offset in range(lwl_msg_set.max_msg_len):
        start_idx = put_idx + offset
        if (start_idx >= num_data_bytes):
            start_idx -= num_data_bytes
        idx = start_idx
        first_get = True
        invalid_id_ctr = 0

        while (True):
            try:
                id, idx = get_data_bytes(data, idx, 1, put_idx, first_get)
            except EOFError:
                break

            first_get = False
            msg_meta = lwl_msg_set.get_metadata(id)
            if msg_meta == None:
                _log.debug('Invalid ID %d at idx %d' % (id, idx));
                invalid_id_ctr += 1
                continue

            _log.debug('Valid ID %d at idx %d' % (id, idx));

            # The ID is valid. Try to get the argument bytes.
            save_idx = idx
            try:
                _, idx = get_data_bytes(data, idx, msg_meta.num_arg_bytes,
                                        put_idx, first_get)
            except EOFError:
                _log.debug('Insufficient data for aguments')
                idx = save_idx

        if invalid_id_ctr < min_invalid_ids:
            min_invalid_ids = invalid_id_ctr
            optimal_start_idx = start_idx
            _log.debug('New optimal start_idx %d invalid_id_ctr=%d' %
                       (start_idx, invalid_id_ctr))

    return optimal_start_idx
```

---

## Demo Workflow (from transcript)

1. **Reset board** - Temperature sampling starts
2. **`lwl status`** - Shows logging active, put_idx incrementing
3. **`lwl test`** - Creates 4 test logs, disables logging
4. **`lwl status`** - Shows logging now off, put_idx frozen
5. **`lwl dump`** - Outputs raw hexadecimal data
6. **Save to file** (e.g., `raw1.txt`)
7. **Run Python tool**: `python log_format.py -d <source_dir> <raw_file>`
8. **View formatted output** - Shows uptime seconds, 100ms ticks, temperature measurements, and test logs

**Example output pattern**:
- Uptime seconds incrementing (31, 32, 33...)
- 100ms tick markers for timing reference
- Temperature/humidity measurements every 1 second
- Test logs at the end (last 4 entries before logging disabled)

---

## Discussion Questions & Answers

### **Q1: How improve LWL with build-time source file editing?**

**Advantages**:
- Users wouldn't need to manage `#define LWL_BASE_ID` and `LWL_NUM`
- Wouldn't need to specify total argument bytes (redundant information)
- Could optimize record creation for speed (load data to local buffer or directly inline)
- Special case optimization (e.g., zero-argument logs handled very efficiently)

**Tradeoff**: Complicates build process, less transparent

### **Q2: How implement real-time LWL viewer?**

**Solution** (Gene implemented this before):
- Write raw LWL data to UART/TTY driver (in addition to or instead of circular buffer)
- Laptop program connects to serial port
- Receives raw LWL data stream
- Decodes and prints to screen in real-time
- **Advantage**: Very compact data transmission (not formatted text)

### **Q3: Avoid filling buffer with same repeated message?**

**Problem**: Super loop logging fills entire buffer with one message type

**Solution** (seen in Linux logging):
- Keep last LWL ID in buffer header
- If new log matches last ID, discard it and increment counter
- When different ID arrives, insert special "discarded N records" message first
- Example output: "Previous log message occurred 1000 times"

### **Q4: Timestamp every log record - advantages/disadvantages?**

**Advantages**: Precise timing for every event

**Disadvantages**: 
- Wastes space for logs where timing isn't critical
- Uses buffer capacity that could hold more logs

**Compromise used in this course**:
- Occasional timestamp logs (uptime seconds every 1s, 100ms ticks)
- Gives sufficient time sense without wasting space
- Could use 2-byte millisecond timestamp if needed

---

## Key Design Principle

LWL achieves low overhead by **deferring all expensive operations** (formatting, string storage) to offline tools, storing only the absolute minimum in the circular buffer: a 1-byte ID plus argument bytes. This allows it to run continuously in production systems, providing critical diagnostic information when faults occur.

```c
struct lwl_data {
    uint32_t magic;
    uint32_t num_section_bytes;
    uint32_t buf_size;
    uint32_t put_idx;           // Points to next write location
    uint8_t buf[LWL_BUF_SIZE];  // Circular buffer (default 1008 bytes)
};
```

```c
struct lwl_data
{
    uint32_t magic;
    uint32_t num_section_bytes;
    uint32_t buf_size;
    uint32_t put_idx;
    uint8_t buf[LWL_BUF_SIZE];
};
```

```c
#define LWL_BASE_ID 20
#define LWL_NUM 10
```

```c
LWL("format string", total_arg_bytes, LWL_1(param1), LWL_2(param2), ...)
```

```c
void lwl_rec(uint8_t id, int32_t num_arg_bytes, ...)
{
    CRIT_STATE_VAR;
    va_list ap;
    uint32_t put_idx;

    va_start(ap, num_arg_bytes);

    CRIT_BEGIN_NEST();
    if (lwl_off_cnt != 0 && --lwl_off_cnt == 0)
        _lwl_active = false;
    put_idx = lwl_data.put_idx % LWL_BUF_SIZE;
    lwl_data.put_idx = (put_idx + 1 + num_arg_bytes) % LWL_BUF_SIZE;

    // We could end the critical section here, since the put index has been
    // updated. But there is a chance that a fault occurs before all of the
    // argument bytes have been written, such that the buffer would contain
    // invalid data.
    //
    // For now we play it safe.

    lwl_data.buf[put_idx] = id;
    while (num_arg_bytes-- > 0) {
        put_idx = (put_idx + 1) % LWL_BUF_SIZE;
        lwl_data.buf[put_idx] = (uint8_t)va_arg(ap, unsigned);
    }
    CRIT_END_NEST();
}
```

```c
void lwl_enable(bool on)
{
    _lwl_active = on;
}
```

```c
uint8_t* lwl_get_buffer(uint32_t* len)
{
    prepare_data_for_output();
    *len = sizeof(lwl_data);
    return (uint8_t*)&lwl_data;
}
```

```c
static struct cmd_cmd_info cmds[] = {
    {
        .name = "status",
        .func = cmd_lwl_status,
        .help = "Status",
    },
    {
        .name = "test",
        .func = cmd_lwl_test,
        .help = "Test logging",
    },
    {
        .name = "enable",
        .func = cmd_lwl_enable,
        .help = "Recording on/off (1/0)",
    },
    {
        .name = "dump",
        .func = cmd_lwl_dump,
        .help = "Dump buffer",
    },
};
```

```c
static int32_t cmd_lwl_test(int32_t argc, const char** argv)
{
    LWL("test 1", 0);
    LWL("test 2 %d", 1, LWL_1(10));
    LWL("test 3 %d %d", 3, LWL_1(10), LWL_2(1000));
    LWL("test 4 %d %d %d", 7, LWL_1(10), LWL_2(1000), LWL_4(100000));

    lwl_enable(false);
    return 0;
}
```

```c
#if CONFIG_FAULT_PRESENT
    wdg_start_init_hdw_wdg();
#endif

#if CONFIG_LWL_PRESENT
    lwl_enable(true);
#endif
```

```c
#if CONFIG_LWL_PRESENT
    {
        .name = "lwl",
        .instance = MOD_NO_INSTANCE,
        .ops.singleton.mod_start = (mod_start)lwl_start,
    },
#endif
```

```python
class LwlMsg:
    """
    This class represents the meta data for a single LWL messages.

    This data is extracted from source files, and is used to format the raw
    data.
    """

    def __init__(self, id, fmt, arg_bytes, file_path, line_num, lwl_statement):
        self.id = id
        self.fmt = fmt
        self.arg_bytes = arg_bytes
        self.file_path = file_path
        self.line_num = line_num
        self.lwl_statement = lwl_statement
        self.num_arg_bytes = 0
        for d in self.arg_bytes:
            self.num_arg_bytes += int(d)
        _log.debug('Create LwlMsg ID=%d arg_bytes=%s(%d) for %s:%d' %
                   (id, arg_bytes, self.num_arg_bytes, file_path, line_num))
```

```python
class LwlMsgSet:
    """
    This class represents the set of meta data for all LWL messages.

    This data is extracted from source files, and is used to format the raw
    data. Error checking is done to ensure consistency of the message set.
    """

    def __init__(self):
        self.lwl_msgs = {}
        self.max_msg_len = 0
    def add_lwl_msg(self, id, fmt, arg_bytes, lwl_statement, file_path,
                    line_num):
        if id in self.lwl_msgs:
            print('%s:%d Duplicate ID %d: %s, previously at %s:%d: %s' %
                  (file_path, line_num, id, lwl_statement,
                   self.lwl_msgs[id].file_path,
                   self.lwl_msgs[id].line_num, 
                   self.lwl_msgs[id].lwl_statement))
            return False
        else:
            self.lwl_msgs[id] = LwlMsg(id, fmt, arg_bytes,
                                       file_path, line_num, lwl_statement)
            msg_len = 1
            for l in arg_bytes:
                msg_len += int(l)
            if msg_len > self.max_msg_len:
                self.max_msg_len = msg_len
                _log.debug('New max msg len %d for %s:%d %s' %
                           (msg_len, file_path, line_num, lwl_statement))
        return True

    def get_metadata(self, id):
        try:
            return self.lwl_msgs[id]
        except KeyError:
            return None
```

```python
def get_optimal_start_idx(data, put_idx):
    """
    Find the optimal starting index to decode log buffer.

    Parameters:
        data (bytearray)  : Log data buffer.
        put_idx (idx)     : The index of the next buffer byte to write log data.

    Return:
        The optimal starting index in [put_idx, put_idx + max_msg_len - 1]

    This is a brute-force solution where we start at the known "put idx", add an
    offset to it, and using that starting point, determine how invalid log
    message IDs are encountered

    The maximum offset is the one less than the length of the largest log
    message (ID plus argument bytes).

    We assume that the optimal starting point is the one that minimizes the
    number of invalid log IDs.  This is not guaranteed to be the correct offset,
    but likely is.

    """

    _log.debug('get_optimal_start_idx(put_idx=%d)' % put_idx)
    min_invalid_ids = len(data)
    optimal_start_idx = None
    num_data_bytes = len(data)
    for offset in range(lwl_msg_set.max_msg_len):
        start_idx = put_idx + offset
        if (start_idx >= num_data_bytes):
            start_idx -= num_data_bytes
        idx = start_idx
        first_get = True
        invalid_id_ctr = 0

        while (True):
            try:
                id, idx = get_data_bytes(data, idx, 1, put_idx, first_get)
            except EOFError:
                break

            first_get = False
            msg_meta = lwl_msg_set.get_metadata(id)
            if msg_meta == None:
                _log.debug('Invalid ID %d at idx %d' % (id, idx));
                invalid_id_ctr += 1
                continue

            _log.debug('Valid ID %d at idx %d' % (id, idx));

            # The ID is valid. Try to get the argument bytes.
            save_idx = idx
            try:
                _, idx = get_data_bytes(data, idx, msg_meta.num_arg_bytes,
                                        put_idx, first_get)
            except EOFError:
                _log.debug('Insufficient data for aguments')
                idx = save_idx

        if invalid_id_ctr < min_invalid_ids:
            min_invalid_ids = invalid_id_ctr
            optimal_start_idx = start_idx
            _log.debug('New optimal start_idx %d invalid_id_ctr=%d' %
                       (start_idx, invalid_id_ctr))

    return optimal_start_idx
```

---

**User**

Summarize @Reliability, Availability, and Maintainability Lesson 4. Fault Handling.txt   compredhensively. include the related code

The source code is in @app_main.c @fault.c @logfmt.py 

don't make things up

---

**Cursor**

# Comprehensive Summary: RAM Lesson 4 - Fault Handling

## Overview
The fault module is the core software for this course, handling faults regardless of where or how they're detected. It operates in "panic mode" when the system is in a bad state, collects diagnostic data, writes it to flash/console, and resets the MCU. Stack overflow protection is implemented here but covered in a separate lesson.

---

## Requirements

### **Supported Fault Types**
1. **CPU-detected faults**:
   - Invalid pointer (doesn't point to valid memory)
   - Invalid memory operation (writing to read-only memory)
   - Stack overflow into read-only guard area
   - Integer divide by zero
   - Illegal instruction

2. **Other faults**:
   - Watchdog trigger
   - Application-detected faults (serious data structure corruption)

### **Fault Data Recording**
Upon fault detection, write to **flash AND console**:
- Fault type
- Program counter and stack pointer (if possible)
- Selected CPU registers
- ARM system registers with fault information
- **Lightweight log buffer** (critical for understanding pre-fault activity)

### **Flash Protection**
- If flash already contains fault data, **don't overwrite** (prevents flash wear in continuous reboot scenarios)
- Still write to console for current fault

### **Recovery Action**
- After writing fault data, **reset the MCU** (automatic recovery approach)
- Whether system returns to operation or waits depends on post-restart behavior

### **Formatting Tool**
- Provide offline Python tool to decode raw binary fault data
- Makes data human-readable

---

## MCU Reset Design

### **Reset Pin Behavior**
- MCU reset pin is **bidirectional** (input AND output) using open-drain hardware
- **Input**: External entity resets MCU (e.g., reset button)
- **Output**: MCU-initiated reset signals to hardware

### **Key Advantage**
MCU-initiated reset can **reset external hardware** - critical for:
- Ensuring outputs go to safe state (e.g., I/O expanders)
- Recovering stuck devices (sensors, peripherals)
- Real example from I²C course: device stuck, requires hardware reset

### **System Components**
- Reset button
- External devices (temp/humidity sensor, I/O expander) on reset line
- Voltage supervisor (monitors power, holds reset unless voltage is good)

---

## Panic Mode Operation

### **When Activated**
System integrity questionable (e.g., memory corruption possible) - can't trust normal operation.

### **Panic Mode Actions**
From transcript and code:

```493:532:ram-class-nucleo-f401re/modules/fault/fault.c
static void fault_common_handler()
{
    uint8_t* lwl_data;
    uint32_t lwl_num_bytes;
    struct end_marker end;

    lwl_enable(false);
    printc_panic("\nFault type=%lu param=%lu\n", fault_data_buf.fault_type,
                 fault_data_buf.fault_param);

    // Populate data buffer and then record it.
    fault_data_buf.magic = MOD_MAGIC_FAULT;
    fault_data_buf.num_section_bytes = sizeof(fault_data_buf);
    fault_data_buf.ipsr = __get_IPSR();
    fault_data_buf.icsr = SCB->ICSR;
    fault_data_buf.shcsr =  SCB->SHCSR;
    fault_data_buf.cfsr =  SCB->CFSR;
    fault_data_buf.hfsr =  SCB->HFSR;
    fault_data_buf.mmfar =  SCB->MMFAR;
    fault_data_buf.bfar =  SCB->BFAR;
    fault_data_buf.tick_ms = tmr_get_ms();

    // Record the MCU data.
    record_fault_data(0, (uint8_t*)&fault_data_buf, sizeof(fault_data_buf));

    // Record the LWL buffer.
    lwl_data = lwl_get_buffer(&lwl_num_bytes);
    record_fault_data(sizeof(fault_data_buf), lwl_data, lwl_num_bytes);

    // Record end marker.
    memset(&end, 0, sizeof(end));
    end.magic = MOD_MAGIC_END;
    end.num_section_bytes = sizeof(end);

    record_fault_data(sizeof(fault_data_buf) + lwl_num_bytes, (uint8_t*)&end,
                      sizeof(end));

    // Reset system - this function will not return.
    NVIC_SystemReset();
}
```

**Key principles**:
1. **Disable interrupts** immediately
2. **Reset stack pointer** to top of RAM (ensure valid stack)
3. **Use polling mode** for all hardware (UART, flash) - no interrupts/DMA
4. **Special panic versions** of console/flash functions (blocking, polling)
5. **Hardware watchdog backup** - if fault handling gets stuck, watchdog resets system
6. **Disable MPU** to avoid secondary faults

---

## Fault Data Format

### **Section-Based Organization**
Data organized in sections, each with:
- **Magic number** (identifies section type to formatting tool)
- **Section length**
- **Data**

### **Three Sections**
1. **Generic fault data** - fault type, registers, etc.
2. **LWL buffer** - lightweight log
3. **End marker** - indicates complete data set

### **Flash Alignment**
Section sizes must be multiple of minimum flash write size (8 or 16 bytes).

### **Data Structure**

```103:146:ram-class-nucleo-f401re/modules/fault/fault.c
struct fault_data
{
    uint32_t magic;                  //@fault_data,magic,4
    uint32_t num_section_bytes;      //@fault_data,num_section_bytes,4

    uint32_t fault_type;             //@fault_data,fault_type,4
    uint32_t fault_param;            //@fault_data,fault_param,4

    // The following fields must follow the exception stack format described
    // in the ARM v7-M Architecture Reference Manual.

    uint32_t excpt_stk_r0;           //@fault_data,excpt_stk_r0,4
    uint32_t excpt_stk_r1;           //@fault_data,excpt_stk_r1,4
    uint32_t excpt_stk_r2;           //@fault_data,excpt_stk_r2,4
    uint32_t excpt_stk_r3;           //@fault_data,excpt_stk_r3,4
    uint32_t excpt_stk_r12;          //@fault_data,excpt_stk_r12,4
    uint32_t excpt_stk_lr;           //@fault_data,excpt_stk_lr,4
    uint32_t excpt_stk_rtn_addr;     //@fault_data,excpt_stk_rtn_addr,4
    uint32_t excpt_stk_xpsr;         //@fault_data,excpt_stk_xpsr,4

    uint32_t sp;                     //@fault_data,sp,4
    uint32_t lr;                     //@fault_data,lr,4

    uint32_t ipsr;                   //@fault_data,ipsr,4
    uint32_t icsr;                   //@fault_data,icsr,4

    uint32_t shcsr;                  //@fault_data,shcsr,4
    uint32_t cfsr;                   //@fault_data,cfsr,4

    uint32_t hfsr;                   //@fault_data,hfsr,4
    uint32_t mmfar;                  //@fault_data,mmfar,4

    uint32_t bfar;                   //@fault_data,bfar,4
    uint32_t tick_ms;                //@fault_data,tick_ms,4

    #if CONFIG_FLASH_WRITE_BYTES == 16
    uint32_t pad1[2];                //@fault_data,pad1,8
    #endif
};
```

**Special comments** (`//@fault_data,...`) are parsed by formatting tool to understand structure.

---

## ARM Cortex-M4 Exception Processing

### **Exception Flow**
1. Fault/interrupt detected
2. Address of handler found in **vector table**
3. Critical state pushed onto **exception stack**
4. Initial handler (assembly) collects info
5. Secondary handler (C in fault module) continues processing

### **Exception Stack Layout**
When exception occurs, ARM automatically pushes to stack (aligned to 8-byte boundary):

```416:433:ram-class-nucleo-f401re/modules/fault/fault.c
 *               +------------------------+
 *      SP+28 -> |         xPSR           |
 *               +------------------------+
 *      SP+24 -> |    Return Address      |
 *               +------------------------+
 *      SP+20 -> |         LR (R14)       |
 *               +------------------------+
 *      SP+16 -> |           R12          |
 *               +------------------------+
 *      SP+12 -> |           R3           |
 *               +------------------------+
 *       SP+8 -> |           R2           |
 *               +------------------------+
 *       SP+4 -> |           R1           |
 *               +------------------------+
 * Current SP -> |           R0           |
 *               +------------------------+
```

This critical information is saved in fault data if stack pointer is valid.

### **Initial Exception Handler (Assembly)**
Modified default handler in startup code:
```assembly
Default_Handler:
    mov  r0, sp          ; Save original stack pointer to r0 (first arg)
    ldr  sp, =_estack    ; Reset stack pointer to known good value
    b    fault_exception_handler  ; Jump to C handler
```

**Key actions**:
1. Save original SP to r0 (becomes first C function parameter)
2. Reset SP to top of RAM
3. Branch (not call) to C handler

### **Secondary Handler (C)**

```449:472:ram-class-nucleo-f401re/modules/fault/fault.c
void fault_exception_handler(uint32_t sp)
{
    // Panic mode.
    CRIT_START();
    wdg_feed_hdw();

    // Disable MPU to avoid another fault (shouldn't be necessary)
    ARM_MPU_Disable();

    // Start to collect fault data.
    fault_data_buf.fault_type = FAULT_TYPE_EXCEPTION;
    fault_data_buf.fault_param = __get_IPSR();
    __ASM volatile("MOV  %0, lr" : "=r" (fault_data_buf.lr) : : "memory");
    fault_data_buf.sp = sp;

    if (((sp & 0x7) == 0) &&
        (sp >= (uint32_t)&_sdata) &&
        ((sp + EXCPT_STK_BYTES + 4) <= (uint32_t)&_estack)) {
        memcpy(&fault_data_buf.excpt_stk_r0, (uint8_t*)sp, EXCPT_STK_BYTES);
    } else {
        memset(&fault_data_buf.excpt_stk_r0, 0, EXCPT_STK_BYTES);
    }
    fault_common_handler();
}
```

**Validation**: Before copying exception stack, verify SP is:
- 8-byte aligned (`sp & 0x7 == 0`)
- Within RAM bounds
- Has sufficient space for exception data

---

## Flash Writing

### **Flash Constraints**
- Must **erase before writing** (not like RAM)
- Erase in chunks (pages/sections/blocks) - sizes may vary
- Write must be on even boundary (multiple of 8 or 16 bytes)
- Write size must be multiple of 8 or 16 bytes

### **Page Reservation Strategy**
For this MCU (STM32F401):
- **Not using last page** (too large, would waste space)
- Using **second page** after vector table (16KB, smaller)
- Vector table must be first page
- Requires linker script editing

### **Recording Fault Data**

```546:594:ram-class-nucleo-f401re/modules/fault/fault.c
static void record_fault_data(uint32_t data_offset, uint8_t* data_addr,
                              uint32_t num_bytes)
{
#if CONFIG_FAULT_PANIC_TO_FLASH
    {
        static bool do_flash;
        int32_t rc;
        
        if (data_offset == 0) {
            do_flash = ((struct fault_data*)FLASH_PANIC_DATA_ADDR)->magic !=
                MOD_MAGIC_FAULT;
        }
        if (do_flash) {
            if (data_offset == 0) {
                rc = flash_panic_erase_page((uint32_t*)FLASH_PANIC_DATA_ADDR);
                if (rc != 0)
                    printc_panic("flash_panic_erase_page returns %ld\n", rc);
            }
            rc = flash_panic_write((uint32_t*)(FLASH_PANIC_DATA_ADDR +
                                               data_offset),
                                   (uint32_t*)data_addr, num_bytes);
            if (rc != 0)
                printc_panic("flash_panic_write returns %ld\n", rc);
        }
    }
#endif

#if CONFIG_FAULT_PANIC_TO_CONSOLE
    {
        const int bytes_per_line = 32;
        uint32_t line_byte_ctr = 0;
        uint32_t idx;

        for (idx = 0; idx < num_bytes; idx++) {
            if (line_byte_ctr == 0)
                printc_panic("%08x: ", (unsigned int)data_offset);
            printc_panic("%02x", (unsigned)*data_addr++);
            data_offset++;
            if (++line_byte_ctr >= bytes_per_line) {
                printc_panic("\n");
                line_byte_ctr = 0;
            }
        }
        if (line_byte_ctr != 0)
            printc_panic("\n");
    }
#endif

}
```

**Flash logic**:
1. Check if magic number already present (existing fault data)
2. If no existing data, erase page on first write
3. Write data chunks
4. Always write to console (hexadecimal format)

---

## Fault Module API

From `fault.c` header section:

### **Fault Types**

```c
enum fault_type {
    FAULT_TYPE_WDG,        // Watchdog triggered
    FAULT_TYPE_EXCEPTION,  // CPU exception
};
```

### **Core Functions**

**1. Software-detected fault:**

```384:401:ram-class-nucleo-f401re/modules/fault/fault.c
void fault_detected(enum fault_type fault_type, uint32_t fault_param)
{
    // Panic mode.
    CRIT_START();
    wdg_feed_hdw();

    // Disable MPU to avoid another fault (shouldn't be necessary)
    ARM_MPU_Disable();

    // Start to collect fault data.
    fault_data_buf.fault_type = fault_type;
    fault_data_buf.fault_param = fault_param;
    memset(&fault_data_buf.excpt_stk_r0, 0, EXCPT_STK_BYTES);
    __ASM volatile("MOV  %0, lr" : "=r" (fault_data_buf.lr) : : "memory");
    __ASM volatile("MOV  %0, sp" : "=r" (fault_data_buf.sp) : : "memory");
    __ASM volatile("MOV  sp, %0" : : "r" (&_estack) : "memory");
    fault_common_handler();
}
```

Called by any code detecting a fault. Never returns.

**2. Exception handler** (shown earlier) - called from assembly initial handler.

**3. Watchdog callback:**

```603:606:ram-class-nucleo-f401re/modules/fault/fault.c
static void wdg_triggered_handler(uint32_t wdg_client_id)
{
    fault_detected(FAULT_TYPE_WDG, wdg_client_id);
}
```

### **Console Commands**

**`fault data [erase]`** - Print/erase fault data from flash:

```618:639:ram-class-nucleo-f401re/modules/fault/fault.c
static int32_t cmd_fault_data(int32_t argc, const char** argv)
{
    int32_t rc = 0;

    if (argc > 3 || (argc == 3 && strcasecmp(argv[2], "erase") != 0)) {
        printc("Invalid command arguments\n");
        return MOD_ERR_BAD_CMD;
    }

    if (argc == 3) {
        rc = flash_panic_erase_page((uint32_t*)FLASH_PANIC_DATA_ADDR);
        if (rc != 0)
            printc("Flash erase fails\n");
    } else {
        uint32_t num_bytes;
        lwl_get_buffer(&num_bytes);
        num_bytes += sizeof(fault_data_buf);
        num_bytes += sizeof(struct end_marker);
        console_data_print((uint8_t*)FLASH_PANIC_DATA_ADDR, num_bytes);
    }
    return rc;
}
```

**`fault status`** - Shows stack usage and reset cause.

**`fault test [op]`** - Test fault scenarios:

```702:732:ram-class-nucleo-f401re/modules/fault/fault.c
static int32_t cmd_fault_test(int32_t argc, const char** argv)
{
    int32_t num_args;
    struct cmd_arg_val arg_vals[2];

    // Handle help case.
    if (argc == 2) {
        printc("Test operations and param(s) are as follows:\n"
               "  Report fault: usage: fault test report <type> <param>\n"
               "  Stack overflow: usage: fault test stack\n"
               "  Bad pointer: usage: fault test ptr\n"
            );
        return 0;
    }

    if (strcasecmp(argv[2], "report") == 0) {
        num_args = cmd_parse_args(argc-3, argv+3, "ui", arg_vals);
        if (num_args != 2) {
            return MOD_ERR_BAD_CMD;
        }
        fault_detected(arg_vals[0].val.u, arg_vals[1].val.i);
    } else if (strcasecmp(argv[2], "stack") == 0) {
        test_overflow_stack();
    } else if (strcasecmp(argv[2], "ptr") == 0) {
        *((uint32_t*)0xffffffff) = 0xbad;
    } else {
        printc("Invalid test '%s'\n", argv[2]);
        return MOD_ERR_BAD_CMD;
    }
    return 0;
}
```

**Bad pointer test** (line 726): Writes to 0xffffffff - guaranteed fault.

---

## Module Integration

From `app_main.c`:

```356:363:ram-class-nucleo-f401re/app1/app_main.c
#if CONFIG_FAULT_PRESENT
    {
        .name = "fault",
        .instance = MOD_NO_INSTANCE,
        .ops.singleton.mod_init = (mod_init)fault_init,
        .ops.singleton.mod_start = (mod_start)fault_start,
    },
#endif
```

Fault module initialized and started during system startup.

---

## Python Formatting Tool (logfmt.py)

More sophisticated than LWL-only tool - handles multiple section types.

### **Section Types**

```44:64:ram-class-nucleo-f401re/modules/fault/logfmt.py
class Data:
    """
    This class represents the fault data being processed.
    """

    # Fault data section tokens, taken from module.h. These are mapped to
    # to section IDs.

    MOD_MAGIC_FAULT = 0xdead0001
    MOD_MAGIC_LWL = 0xf00d0001
    MOD_MAGIC_TRAILER = 0xc0da0001                  

    SECTION_TYPE_FAULT = 0
    SECTION_TYPE_LWL = 1
    SECTION_TYPE_TRAILER = 2

    magic_to_fault_type = {
        MOD_MAGIC_FAULT : SECTION_TYPE_FAULT,
        MOD_MAGIC_LWL : SECTION_TYPE_LWL,
        MOD_MAGIC_TRAILER : SECTION_TYPE_TRAILER,
        }
```

### **Processing Flow**

```125:184:ram-class-nucleo-f401re/modules/fault/logfmt.py
    def process_data(self):
        """
        Process fault data.

        The fault data consists of one of more sections, which have this format:
        - magic number (4 bytes)
        - number of section data bytes, starting with the magic number (4 bytes)
        - section data bytes

        This function finds each section, determines the section type based on
        the magic number, and calls other functions to "pretty print" the data.

        If corruption is found, we print a warning and continue on if possible.
        In this way, we still might be able to print some useful data. However,
        you should look at the warning, as you might be able to fix it and get
        more data.
        """

        _log.debug('process_data()')
        idx = 0
        section_type = 0
        section_len = 0

        while idx < self.data_len:
            if idx + 8 > self.data_len:
                print('ERROR: Insufficient bytes for a section header '
                      'idx=%d data_len=%d' % (idx, self.data_len))
                return False

            # Get magic of next segment and determine data type. We determine
            # endianess by trial and error.

            magic = self.get_data(idx, 4)
            if magic not in self.magic_to_fault_type:
                g_swab = not g_swab
                magic = self.get_data(idx, 4)
                if magic not in self.magic_to_fault_type:
                    print('ERROR: Can not determine section type at idx %d' %
                          (idx))
                    return False
            section_type = self.magic_to_fault_type[magic]
            section_len = self.get_data(idx + 4, 4)
            _log.debug('Got section magic=0x%08x type=%d len=%d',
                       magic, section_type, section_len)
            if idx + section_len > self.data_len:
                print('ERROR: Insufficient bytes for a section header '
                      'idx=%d section_len=%d data_len=%d' %
                      (idx, section_len, self.data_len))
                return False
            if section_type == self.SECTION_TYPE_FAULT:
                g_fault_data.pretty_print(idx, section_len)
            elif section_type == self.SECTION_TYPE_LWL:
                lwl_printer.pretty_print(idx, section_len)
            elif section_type == self.SECTION_TYPE_TRAILER:
                print('=' * 80)
                print("End of fault data")
                print('=' * 80)
            idx += section_len

        return True
```

### **Source Parsing**
Tool parses source files to extract:
1. **Fault field metadata** from special comments (`//@fault_data,name,size`)
2. **LWL statements** (reuses logic from lwl.py)

```679:864:ram-class-nucleo-f401re/modules/fault/logfmt.py
    def parse_source_file(self, file_path):
        """
        Search through a source file (normally a .c or .h), finding meta data.
        There are lwl statements, and fault data descriptions.

        lwl statements:

            Each file using lwl needs to have a statement like this before any
            lwl statement:

                #define LWL_BASE_ID 10
                #define LWL_NUM 5

            Example statements:
                LWL("Trace fmt %d", 2, LWL_2(abc));
                LWL("Simple trace", 0)

            The message defintions are checked for syntax and consistency, and
            errors reported.

            Multi-line LWL statements complicate this task.  But we make
            simplifying assumptions to help:
            - All non-final lines end with a comma (optionally followed by
              whitespace)
            - The final line ends with ");" (optionally followed by whitespace)

        fault data descriptions:

            A snippet:

                struct fault_data
                {
                    uint32_t magic;              //@fault_data,magic,4
                    uint32_t num_section_bytes;  //@fault_data,num_section_bytes,4

                    uint32_t fault_type;         //@fault_data,fault_type,4
                    uint32_t fault_param;        //@fault_data,fault_param,4
                    uint32_t return_addr;        //@fault_data,return_addr,4

                    :
                    :

            We assume fault data defintions are always on a single line.

        """

        _log.debug('parse_source_file(file_path=%s)', file_path)

        error_count = 0
        lwl_base_id = None
        fault_field_offset = 0

        parse_fault_data_pat = re.compile('//@fault_data,([^,]+),(\d+)')

        lwl_base_id_pat = re.compile('^\s*#define\s+LWL_BASE_ID\s+(\d+)')
        lwl_num_pat = re.compile('^\s*#define\s+LWL_NUM\s+(\d+)')
        detect_lwl_pat = re.compile('^\s*LWL\(')
        parse_fmt_pat = re.compile('\s*LWL\("([^"]*)"')
        parse_zero_pat = re.compile('\s*,\s*0')
        parse_arg_string_pat = re.compile('\s*,\s*"([^"]*)"')
        parse_num_arg_bytes_pat = re.compile('\s*,\s*([0-9]+)')
        parse_arg_pat = re.compile(',\s*LWL_([\d])\(')
        with open(file_path) as f:
            line_num = 0
            lwl_id_offset = -1
            lwl_statement = None
            for line in f:
                line_num += 1
                line = line.strip()
                if not line:
                    continue
                #_log.debug('line=[%s]', line)

                # Start with fault data descriptions.
                m = re.search(parse_fault_data_pat, line)
                if m:
                    name = m.group(1)
                    num_bytes = int(m.group(2))
                    g_fault_data.add_fault_field(name, fault_field_offset,
                                                 num_bytes)
                    fault_field_offset += num_bytes
                    continue

                # Now for lwl statements.
                m = re.match(lwl_base_id_pat, line)
                if m:
                    lwl_base_id = int(m.group(1))
                    lwl_line_num = line_num
                    _log.debug('%s:%d LWL_BASE_ID=%d',
                               file_path, line_num, lwl_base_id)
                    if lwl_base_id == 0:
                        print('ERROR: %s:%d: Invalid LWL base ID %d' %
                              (file_path, line_num, lwl_base_id))
                if lwl_statement:
                   # Check for acceptable line ending.
                   if (line[-1] != ',') and (line[-2:] != ');'):
                        print('ERROR: %s:%d: Invalid LWL continuation line: %s'
                              % (file_path, line_num, line))
                        error_count += 1
                        lwl_statement = None
                   else:
                        lwl_statement += line
                else:
                    # Check for start of statement.
                    if re.match(detect_lwl_pat, line):
                        # Check for acceptable line ending.
                        if (line[-2:] != ');') and (line[-1] != ','):
                            print('ERROR: %s:%d: Invalid LWL line ending: %s' %
                                  (file_path, line_num, line))
                            error_count += 1
                        else:
                            lwl_statement = line
                if (not lwl_statement) or (lwl_statement[-2:] != ');'):
                    continue

                # Got a complete statement. First get the format.
                _log.debug('Got statement: %s', lwl_statement)
                lwl_id_offset += 1
                m = re.match(parse_fmt_pat, lwl_statement)
                if not m:
                    print('ERROR: %s:%d Cannot parse LWL fmt in %s' %
                          (file_path, line_num, lwl_statement))
                    error_count += 1
                    lwl_statement = None
                    continue

                lwl_fmt = m.group(1)
                lwl_remain = lwl_statement[m.end():]
                _log.debug('Got fmt: %s, remain: %s', lwl_fmt, lwl_remain)

                # Get num_arg_bytes
                m = re.match(parse_num_arg_bytes_pat, lwl_remain)
                if m:
                    lwl_num_arg_bytes = int(m.group(1))
                else:
                    print('ERROR: %s:%d Cannot parse LWL num arg bytes '
                          'parameter in %s' %(file_path, line_num,
                                              lwl_statement))
                    error_count += 1
                    lwl_statement = None
                    continue
                lwl_remain = lwl_remain[m.end():]
                _log.debug('Got num_arg_bytes: %d, remain: %s',
                           lwl_num_arg_bytes, lwl_remain)

                # Get the arg lengths
                lwl_arg_lengths = ''
                num_arg_bytes_check = 0
                while True:
                    m = re.search(parse_arg_pat, lwl_remain)
                    if m:
                        lwl_arg_lengths += m.group(1)
                        num_arg_bytes_check += int(m.group(1))
                        lwl_remain = lwl_remain[m.end():]
                        _log.debug('Got arg length: %s, remain: %s',
                                   m.group(1), lwl_remain)
                    else:
                        _log.debug('Out of args')
                        break

                if num_arg_bytes_check != lwl_num_arg_bytes:
                    print('ERROR: %s:%d Inconsistent num arg bytes '
                          '(%d vs %d) in %s' %(file_path, line_num,
                                               num_arg_bytes_check,
                                               lwl_num_arg_bytes,
                                               lwl_statement))

                _log.debug('[%s] [%s] [%d] [%s]', lwl_id_offset, lwl_fmt,
                           lwl_num_arg_bytes, lwl_arg_lengths)
                if lwl_base_id is None:
                    print('ERROR: %s:%d No #define LWL_BASE_ID present' %
                          (file_path, line_num))
                    error_count += 1
                    lwl_statement = None
                    continue
                
                if not g_lwl_msg_set.add_lwl_msg(lwl_base_id +
                                                 int(lwl_id_offset),
                                                 lwl_fmt, lwl_arg_lengths,
                                                 lwl_statement, file_path,
                                                 lwl_line_num):
                    error_count += 1

                lwl_statement = None
                continue
        return error_count
```

---

## Demo Workflow

From transcript (lines 273-316):

1. **Reset system** (black button)
2. **Stop logging** (Ctrl-L)
3. **Run test**: `fault test ptr` - writes to 0xffffffff
4. **System resets automatically**
5. **Fault data dumped to console** (hexadecimal)
6. **Copy data to file** (e.g., `raw.txt`)
7. **Run formatting tool**: 
   ```bash
   python logfmt.py -d <source_dir> -f raw.txt
   ```
8. **Analyze decoded output**:
   - Fault type: EXCEPTION (value 3 = hard fault)
   - Register R2 contained 0xBAD (value being written)
   - Return address: 0x800c974 (location in fault test code)
   - System registers (CFSR, HFSR, etc.)
   - LWL trace showing pre-fault activity
   - Uptime: 16 seconds when fault occurred

### **Flash Management**
- **`fault data`** - Dumps fault data from flash
- **`fault data erase`** - Clears flash to capture next fault
- After erase, flash shows all 0xFF (erased state)

---

## Discussion Questions & Answers

### **Q1: How store multiple fault data dumps in flash?**

**A**: If reserved page can hold multiple instances:
- Write fault data sequentially (one after another)
- More complex erase logic
- Could reserve multiple pages for extensive fault history
- For this MCU: 16KB page could store multiple dumps

### **Q2: Alternative to stack backtrace (since ARM makes it difficult)?**

**A**: Search backward in stack for values that are:
- Valid flash addresses
- Potentially return addresses
- Save first ~10 such values in fault data
- Offline tool converts addresses to function names/line numbers
- User manually works out backtrace from this information

### **Q3: If you can save entire RAM image, how analyze it?**

**A**: Equivalent to Linux core dump:
- Linux: Process crash → core dump file (RAM image)
- GDB allows offline analysis at source level (variables, etc.)
- **Same can be done for MCUs** - Gene implemented this on large embedded system
- **"Invaluable"** for crash analysis

### **Q4: Is resetting stack pointer to top of RAM too drastic? Alternatives?**

**Current approach**: Simple and safe, but potentially wasteful.

**Alternatives**:
1. **Check current SP** - if sufficient stack remaining, leave it alone
2. **Reserve dedicated RAM** for fault handling stack - completely avoid touching normal stack (requires spare RAM)

---

## Key Design Principles

1. **Panic mode simplicity** - primitive operations, don't worry about efficiency
2. **Highest quality fault handling code** - bugs here leave "zombie state" (worst case)
3. **Hardware watchdog backup** - safety net if fault handling fails
4. **Flash wear protection** - don't overwrite existing fault data in continuous reboot
5. **Comprehensive data collection** - LWL buffer is critical for understanding pre-fault context
6. **Offline formatting** - raw binary in embedded system, human-readable on development machine

```c
static void fault_common_handler()
{
    uint8_t* lwl_data;
    uint32_t lwl_num_bytes;
    struct end_marker end;

    lwl_enable(false);
    printc_panic("\nFault type=%lu param=%lu\n", fault_data_buf.fault_type,
                 fault_data_buf.fault_param);

    // Populate data buffer and then record it.
    fault_data_buf.magic = MOD_MAGIC_FAULT;
    fault_data_buf.num_section_bytes = sizeof(fault_data_buf);
    fault_data_buf.ipsr = __get_IPSR();
    fault_data_buf.icsr = SCB->ICSR;
    fault_data_buf.shcsr =  SCB->SHCSR;
    fault_data_buf.cfsr =  SCB->CFSR;
    fault_data_buf.hfsr =  SCB->HFSR;
    fault_data_buf.mmfar =  SCB->MMFAR;
    fault_data_buf.bfar =  SCB->BFAR;
    fault_data_buf.tick_ms = tmr_get_ms();

    // Record the MCU data.
    record_fault_data(0, (uint8_t*)&fault_data_buf, sizeof(fault_data_buf));

    // Record the LWL buffer.
    lwl_data = lwl_get_buffer(&lwl_num_bytes);
    record_fault_data(sizeof(fault_data_buf), lwl_data, lwl_num_bytes);

    // Record end marker.
    memset(&end, 0, sizeof(end));
    end.magic = MOD_MAGIC_END;
    end.num_section_bytes = sizeof(end);

    record_fault_data(sizeof(fault_data_buf) + lwl_num_bytes, (uint8_t*)&end,
                      sizeof(end));

    // Reset system - this function will not return.
    NVIC_SystemReset();
}
```

```c
struct fault_data
{
    uint32_t magic;                  //@fault_data,magic,4
    uint32_t num_section_bytes;      //@fault_data,num_section_bytes,4

    uint32_t fault_type;             //@fault_data,fault_type,4
    uint32_t fault_param;            //@fault_data,fault_param,4

    // The following fields must follow the exception stack format described
    // in the ARM v7-M Architecture Reference Manual.

    uint32_t excpt_stk_r0;           //@fault_data,excpt_stk_r0,4
    uint32_t excpt_stk_r1;           //@fault_data,excpt_stk_r1,4
    uint32_t excpt_stk_r2;           //@fault_data,excpt_stk_r2,4
    uint32_t excpt_stk_r3;           //@fault_data,excpt_stk_r3,4
    uint32_t excpt_stk_r12;          //@fault_data,excpt_stk_r12,4
    uint32_t excpt_stk_lr;           //@fault_data,excpt_stk_lr,4
    uint32_t excpt_stk_rtn_addr;     //@fault_data,excpt_stk_rtn_addr,4
    uint32_t excpt_stk_xpsr;         //@fault_data,excpt_stk_xpsr,4

    uint32_t sp;                     //@fault_data,sp,4
    uint32_t lr;                     //@fault_data,lr,4

    uint32_t ipsr;                   //@fault_data,ipsr,4
    uint32_t icsr;                   //@fault_data,icsr,4

    uint32_t shcsr;                  //@fault_data,shcsr,4
    uint32_t cfsr;                   //@fault_data,cfsr,4

    uint32_t hfsr;                   //@fault_data,hfsr,4
    uint32_t mmfar;                  //@fault_data,mmfar,4

    uint32_t bfar;                   //@fault_data,bfar,4
    uint32_t tick_ms;                //@fault_data,tick_ms,4

    #if CONFIG_FLASH_WRITE_BYTES == 16
    uint32_t pad1[2];                //@fault_data,pad1,8
    #endif
};
```

```c
 *               +------------------------+
 *      SP+28 -> |         xPSR           |
 *               +------------------------+
 *      SP+24 -> |    Return Address      |
 *               +------------------------+
 *      SP+20 -> |         LR (R14)       |
 *               +------------------------+
 *      SP+16 -> |           R12          |
 *               +------------------------+
 *      SP+12 -> |           R3           |
 *               +------------------------+
 *       SP+8 -> |           R2           |
 *               +------------------------+
 *       SP+4 -> |           R1           |
 *               +------------------------+
 * Current SP -> |           R0           |
 *               +------------------------+
```

```plaintext
Default_Handler:
    mov  r0, sp          ; Save original stack pointer to r0 (first arg)
    ldr  sp, =_estack    ; Reset stack pointer to known good value
    b    fault_exception_handler  ; Jump to C handler
```

```c
void fault_exception_handler(uint32_t sp)
{
    // Panic mode.
    CRIT_START();
    wdg_feed_hdw();

    // Disable MPU to avoid another fault (shouldn't be necessary)
    ARM_MPU_Disable();

    // Start to collect fault data.
    fault_data_buf.fault_type = FAULT_TYPE_EXCEPTION;
    fault_data_buf.fault_param = __get_IPSR();
    __ASM volatile("MOV  %0, lr" : "=r" (fault_data_buf.lr) : : "memory");
    fault_data_buf.sp = sp;

    if (((sp & 0x7) == 0) &&
        (sp >= (uint32_t)&_sdata) &&
        ((sp + EXCPT_STK_BYTES + 4) <= (uint32_t)&_estack)) {
        memcpy(&fault_data_buf.excpt_stk_r0, (uint8_t*)sp, EXCPT_STK_BYTES);
    } else {
        memset(&fault_data_buf.excpt_stk_r0, 0, EXCPT_STK_BYTES);
    }
    fault_common_handler();
}
```

```c
static void record_fault_data(uint32_t data_offset, uint8_t* data_addr,
                              uint32_t num_bytes)
{
#if CONFIG_FAULT_PANIC_TO_FLASH
    {
        static bool do_flash;
        int32_t rc;
        
        if (data_offset == 0) {
            do_flash = ((struct fault_data*)FLASH_PANIC_DATA_ADDR)->magic !=
                MOD_MAGIC_FAULT;
        }
        if (do_flash) {
            if (data_offset == 0) {
                rc = flash_panic_erase_page((uint32_t*)FLASH_PANIC_DATA_ADDR);
                if (rc != 0)
                    printc_panic("flash_panic_erase_page returns %ld\n", rc);
            }
            rc = flash_panic_write((uint32_t*)(FLASH_PANIC_DATA_ADDR +
                                               data_offset),
                                   (uint32_t*)data_addr, num_bytes);
            if (rc != 0)
                printc_panic("flash_panic_write returns %ld\n", rc);
        }
    }
#endif

#if CONFIG_FAULT_PANIC_TO_CONSOLE
    {
        const int bytes_per_line = 32;
        uint32_t line_byte_ctr = 0;
        uint32_t idx;

        for (idx = 0; idx < num_bytes; idx++) {
            if (line_byte_ctr == 0)
                printc_panic("%08x: ", (unsigned int)data_offset);
            printc_panic("%02x", (unsigned)*data_addr++);
            data_offset++;
            if (++line_byte_ctr >= bytes_per_line) {
                printc_panic("\n");
                line_byte_ctr = 0;
            }
        }
        if (line_byte_ctr != 0)
            printc_panic("\n");
    }
#endif

}
```

```c
enum fault_type {
    FAULT_TYPE_WDG,        // Watchdog triggered
    FAULT_TYPE_EXCEPTION,  // CPU exception
};
```

```c
void fault_detected(enum fault_type fault_type, uint32_t fault_param)
{
    // Panic mode.
    CRIT_START();
    wdg_feed_hdw();

    // Disable MPU to avoid another fault (shouldn't be necessary)
    ARM_MPU_Disable();

    // Start to collect fault data.
    fault_data_buf.fault_type = fault_type;
    fault_data_buf.fault_param = fault_param;
    memset(&fault_data_buf.excpt_stk_r0, 0, EXCPT_STK_BYTES);
    __ASM volatile("MOV  %0, lr" : "=r" (fault_data_buf.lr) : : "memory");
    __ASM volatile("MOV  %0, sp" : "=r" (fault_data_buf.sp) : : "memory");
    __ASM volatile("MOV  sp, %0" : : "r" (&_estack) : "memory");
    fault_common_handler();
}
```

```c
static void wdg_triggered_handler(uint32_t wdg_client_id)
{
    fault_detected(FAULT_TYPE_WDG, wdg_client_id);
}
```

```c
static int32_t cmd_fault_data(int32_t argc, const char** argv)
{
    int32_t rc = 0;

    if (argc > 3 || (argc == 3 && strcasecmp(argv[2], "erase") != 0)) {
        printc("Invalid command arguments\n");
        return MOD_ERR_BAD_CMD;
    }

    if (argc == 3) {
        rc = flash_panic_erase_page((uint32_t*)FLASH_PANIC_DATA_ADDR);
        if (rc != 0)
            printc("Flash erase fails\n");
    } else {
        uint32_t num_bytes;
        lwl_get_buffer(&num_bytes);
        num_bytes += sizeof(fault_data_buf);
        num_bytes += sizeof(struct end_marker);
        console_data_print((uint8_t*)FLASH_PANIC_DATA_ADDR, num_bytes);
    }
    return rc;
}
```

```c
static int32_t cmd_fault_test(int32_t argc, const char** argv)
{
    int32_t num_args;
    struct cmd_arg_val arg_vals[2];

    // Handle help case.
    if (argc == 2) {
        printc("Test operations and param(s) are as follows:\n"
               "  Report fault: usage: fault test report <type> <param>\n"
               "  Stack overflow: usage: fault test stack\n"
               "  Bad pointer: usage: fault test ptr\n"
            );
        return 0;
    }

    if (strcasecmp(argv[2], "report") == 0) {
        num_args = cmd_parse_args(argc-3, argv+3, "ui", arg_vals);
        if (num_args != 2) {
            return MOD_ERR_BAD_CMD;
        }
        fault_detected(arg_vals[0].val.u, arg_vals[1].val.i);
    } else if (strcasecmp(argv[2], "stack") == 0) {
        test_overflow_stack();
    } else if (strcasecmp(argv[2], "ptr") == 0) {
        *((uint32_t*)0xffffffff) = 0xbad;
    } else {
        printc("Invalid test '%s'\n", argv[2]);
        return MOD_ERR_BAD_CMD;
    }
    return 0;
}
```

```c
#if CONFIG_FAULT_PRESENT
    {
        .name = "fault",
        .instance = MOD_NO_INSTANCE,
        .ops.singleton.mod_init = (mod_init)fault_init,
        .ops.singleton.mod_start = (mod_start)fault_start,
    },
#endif
```

```python
class Data:
    """
    This class represents the fault data being processed.
    """

    # Fault data section tokens, taken from module.h. These are mapped to
    # to section IDs.

    MOD_MAGIC_FAULT = 0xdead0001
    MOD_MAGIC_LWL = 0xf00d0001
    MOD_MAGIC_TRAILER = 0xc0da0001                  

    SECTION_TYPE_FAULT = 0
    SECTION_TYPE_LWL = 1
    SECTION_TYPE_TRAILER = 2

    magic_to_fault_type = {
        MOD_MAGIC_FAULT : SECTION_TYPE_FAULT,
        MOD_MAGIC_LWL : SECTION_TYPE_LWL,
        MOD_MAGIC_TRAILER : SECTION_TYPE_TRAILER,
        }
```

```python
    def process_data(self):
        """
        Process fault data.

        The fault data consists of one of more sections, which have this format:
        - magic number (4 bytes)
        - number of section data bytes, starting with the magic number (4 bytes)
        - section data bytes

        This function finds each section, determines the section type based on
        the magic number, and calls other functions to "pretty print" the data.

        If corruption is found, we print a warning and continue on if possible.
        In this way, we still might be able to print some useful data. However,
        you should look at the warning, as you might be able to fix it and get
        more data.
        """

        _log.debug('process_data()')
        idx = 0
        section_type = 0
        section_len = 0

        while idx < self.data_len:
            if idx + 8 > self.data_len:
                print('ERROR: Insufficient bytes for a section header '
                      'idx=%d data_len=%d' % (idx, self.data_len))
                return False

            # Get magic of next segment and determine data type. We determine
            # endianess by trial and error.

            magic = self.get_data(idx, 4)
            if magic not in self.magic_to_fault_type:
                g_swab = not g_swab
                magic = self.get_data(idx, 4)
                if magic not in self.magic_to_fault_type:
                    print('ERROR: Can not determine section type at idx %d' %
                          (idx))
                    return False
            section_type = self.magic_to_fault_type[magic]
            section_len = self.get_data(idx + 4, 4)
            _log.debug('Got section magic=0x%08x type=%d len=%d',
                       magic, section_type, section_len)
            if idx + section_len > self.data_len:
                print('ERROR: Insufficient bytes for a section header '
                      'idx=%d section_len=%d data_len=%d' %
                      (idx, section_len, self.data_len))
                return False
            if section_type == self.SECTION_TYPE_FAULT:
                g_fault_data.pretty_print(idx, section_len)
            elif section_type == self.SECTION_TYPE_LWL:
                lwl_printer.pretty_print(idx, section_len)
            elif section_type == self.SECTION_TYPE_TRAILER:
                print('=' * 80)
                print("End of fault data")
                print('=' * 80)
            idx += section_len

        return True
```

```python
    def parse_source_file(self, file_path):
        """
        Search through a source file (normally a .c or .h), finding meta data.
        There are lwl statements, and fault data descriptions.

        lwl statements:

            Each file using lwl needs to have a statement like this before any
            lwl statement:

                #define LWL_BASE_ID 10
                #define LWL_NUM 5

            Example statements:
                LWL("Trace fmt %d", 2, LWL_2(abc));
                LWL("Simple trace", 0)

            The message defintions are checked for syntax and consistency, and
            errors reported.

            Multi-line LWL statements complicate this task.  But we make
            simplifying assumptions to help:
            - All non-final lines end with a comma (optionally followed by
              whitespace)
            - The final line ends with ");" (optionally followed by whitespace)

        fault data descriptions:

            A snippet:

                struct fault_data
                {
                    uint32_t magic;              //@fault_data,magic,4
                    uint32_t num_section_bytes;  //@fault_data,num_section_bytes,4

                    uint32_t fault_type;         //@fault_data,fault_type,4
                    uint32_t fault_param;        //@fault_data,fault_param,4
                    uint32_t return_addr;        //@fault_data,return_addr,4

                    :
                    :

            We assume fault data defintions are always on a single line.

        """

        _log.debug('parse_source_file(file_path=%s)', file_path)

        error_count = 0
        lwl_base_id = None
        fault_field_offset = 0

        parse_fault_data_pat = re.compile('//@fault_data,([^,]+),(\d+)')

        lwl_base_id_pat = re.compile('^\s*#define\s+LWL_BASE_ID\s+(\d+)')
        lwl_num_pat = re.compile('^\s*#define\s+LWL_NUM\s+(\d+)')
        detect_lwl_pat = re.compile('^\s*LWL\(')
        parse_fmt_pat = re.compile('\s*LWL\("([^"]*)"')
        parse_zero_pat = re.compile('\s*,\s*0')
        parse_arg_string_pat = re.compile('\s*,\s*"([^"]*)"')
        parse_num_arg_bytes_pat = re.compile('\s*,\s*([0-9]+)')
        parse_arg_pat = re.compile(',\s*LWL_([\d])\(')
        with open(file_path) as f:
            line_num = 0
            lwl_id_offset = -1
            lwl_statement = None
            for line in f:
                line_num += 1
                line = line.strip()
                if not line:
                    continue
                #_log.debug('line=[%s]', line)

                # Start with fault data descriptions.
                m = re.search(parse_fault_data_pat, line)
                if m:
                    name = m.group(1)
                    num_bytes = int(m.group(2))
                    g_fault_data.add_fault_field(name, fault_field_offset,
                                                 num_bytes)
                    fault_field_offset += num_bytes
                    continue

                # Now for lwl statements.
                m = re.match(lwl_base_id_pat, line)
                if m:
                    lwl_base_id = int(m.group(1))
                    lwl_line_num = line_num
                    _log.debug('%s:%d LWL_BASE_ID=%d',
                               file_path, line_num, lwl_base_id)
                    if lwl_base_id == 0:
                        print('ERROR: %s:%d: Invalid LWL base ID %d' %
                              (file_path, line_num, lwl_base_id))
                if lwl_statement:
                   # Check for acceptable line ending.
                   if (line[-1] != ',') and (line[-2:] != ');'):
                        print('ERROR: %s:%d: Invalid LWL continuation line: %s'
                              % (file_path, line_num, line))
                        error_count += 1
                        lwl_statement = None
                   else:
                        lwl_statement += line
                else:
                    # Check for start of statement.
                    if re.match(detect_lwl_pat, line):
                        # Check for acceptable line ending.
                        if (line[-2:] != ');') and (line[-1] != ','):
                            print('ERROR: %s:%d: Invalid LWL line ending: %s' %
                                  (file_path, line_num, line))
                            error_count += 1
                        else:
                            lwl_statement = line
                if (not lwl_statement) or (lwl_statement[-2:] != ');'):
                    continue

                # Got a complete statement. First get the format.
                _log.debug('Got statement: %s', lwl_statement)
                lwl_id_offset += 1
                m = re.match(parse_fmt_pat, lwl_statement)
                if not m:
                    print('ERROR: %s:%d Cannot parse LWL fmt in %s' %
                          (file_path, line_num, lwl_statement))
                    error_count += 1
                    lwl_statement = None
                    continue

                lwl_fmt = m.group(1)
                lwl_remain = lwl_statement[m.end():]
                _log.debug('Got fmt: %s, remain: %s', lwl_fmt, lwl_remain)

                # Get num_arg_bytes
                m = re.match(parse_num_arg_bytes_pat, lwl_remain)
                if m:
                    lwl_num_arg_bytes = int(m.group(1))
                else:
                    print('ERROR: %s:%d Cannot parse LWL num arg bytes '
                          'parameter in %s' %(file_path, line_num,
                                              lwl_statement))
                    error_count += 1
                    lwl_statement = None
                    continue
                lwl_remain = lwl_remain[m.end():]
                _log.debug('Got num_arg_bytes: %d, remain: %s',
                           lwl_num_arg_bytes, lwl_remain)

                # Get the arg lengths
                lwl_arg_lengths = ''
                num_arg_bytes_check = 0
                while True:
                    m = re.search(parse_arg_pat, lwl_remain)
                    if m:
                        lwl_arg_lengths += m.group(1)
                        num_arg_bytes_check += int(m.group(1))
                        lwl_remain = lwl_remain[m.end():]
                        _log.debug('Got arg length: %s, remain: %s',
                                   m.group(1), lwl_remain)
                    else:
                        _log.debug('Out of args')
                        break

                if num_arg_bytes_check != lwl_num_arg_bytes:
                    print('ERROR: %s:%d Inconsistent num arg bytes '
                          '(%d vs %d) in %s' %(file_path, line_num,
                                               num_arg_bytes_check,
                                               lwl_num_arg_bytes,
                                               lwl_statement))

                _log.debug('[%s] [%s] [%d] [%s]', lwl_id_offset, lwl_fmt,
                           lwl_num_arg_bytes, lwl_arg_lengths)
                if lwl_base_id is None:
                    print('ERROR: %s:%d No #define LWL_BASE_ID present' %
                          (file_path, line_num))
                    error_count += 1
                    lwl_statement = None
                    continue
                
                if not g_lwl_msg_set.add_lwl_msg(lwl_base_id +
                                                 int(lwl_id_offset),
                                                 lwl_fmt, lwl_arg_lengths,
                                                 lwl_statement, file_path,
                                                 lwl_line_num):
                    error_count += 1

                lwl_statement = None
                continue
        return error_count
```

```shellscript
   python logfmt.py -d <source_dir> -f raw.txt
```

---

**User**

Summarize @Reliability, Availability, and Maintainability Lesson 5. Watchdogs.txt  compredhensively. include the related snippet code

The source code is in @app_main.c @wdg.c 

don't make things up

---

**Cursor**

# Comprehensive Summary: RAM Lesson 5 - Watchdogs

## Overview
Watchdogs are a key tool to ensure embedded applications are healthy. If the application isn't healthy, a watchdog triggers recovery action - important for both **safety** and **availability**. The watchdog module provides multiple software-based watchdogs backed by a single hardware watchdog.

---

## Introduction to Watchdogs

### **What is a Watchdog?**
A software or hardware mechanism to detect faults in a software system - generally detecting when something isn't being done (e.g., stuck code).

### **How Watchdogs Work**
- Software must **periodically feed** the watchdog to signal it's alive and well
- If too long without feeding → **watchdog triggers** (something is wrong)

### **Software vs Hardware Watchdogs**

**Software-based watchdog trigger:**
- Can take software-based actions
- Save state information about the system
- Controlled shutdown
- **This course**: Implements information saving

**Hardware-based watchdog trigger:**
- **Immediate hardware reset** of MCU ("boom reset")
- Some have **early warning interrupt** capability (interrupt before reset)
- STM32F401RE (used in course) does **NOT** have early warning feature

---

## Watchdog Features

### **Clock Types**
Trade-off between clock **accuracy** and **reliability** (hardware issue, beyond course scope)

### **Timeout Types**

**1. Simple timeout** (used in this course):
- Just feed before time expires

**2. Windowed timeout**:
- Must feed **within specified time window**
- Can't feed too early OR too late

### **Initialization vs Steady State**
- Initialization very different from steady state
- System can get stuck **before infrastructure is running**
- **Special handling** for initialization watchdog (covered later)

---

## Watchdog Usage Strategy (Gene's Experience)

### **Key Principle**
> **Watchdogs ensure critical work is being DONE, not simply that code is being EXECUTED**

### **Multiple Watchdogs Approach**
- Have **many software-based watchdogs** (MCUs typically support only 1-2 hardware watchdogs)
- Each watchdog ensures **specific critical work** is performed

**Examples**:
- Watchdog for reading inputs every 100ms → feed only on **successful** read
- Watchdog for control loop → feed only after **successfully created and written output**

### **Hierarchy of Checkers**
- Main loop watchdog is okay, but **not sufficient**
  - Main loop could run fine, but product not doing its job (bad data state)
- **Hardware watchdog** ensures software watchdogs are processed correctly
- **"Checks the checkers"** in a hierarchy

---

## Requirements

### **Core Requirements**
1. **Multiple independent software watchdogs** for monitoring different system parts
2. **Runtime registration** - no code dependencies between watchdog module and users
3. **Single callback registration** - one module (fault module) gets notified of triggers
4. **Hardware watchdog as backup** - only when software watchdogs not working

### **Initialization Watchdog**
- **Single simple watchdog** for initialization (limited time to finish)
- **After N consecutive failed initializations** → don't enable watchdog (let system try without time limit)
- N can be set to infinity for continuous retry

### **Standard Module Requirements**
- Super loop compatible (non-blocking)
- Standard API
- Logging and console commands for test/debug

---

## Design: How the Watchdog Module Works

### **Conceptual Model**

**Normal operation:**
1. Application software (green ovals) performs critical work
2. Each has associated software watchdog
3. Application feeds watchdog on **successful completion**
4. Software watchdogs checked **periodically** (e.g., every 100ms)
5. If all software watchdogs OK → **hardware watchdog is fed**

**Normal watchdog fault:**
1. Application has problem → **stops feeding its watchdog**
2. Detected during periodic check
3. Watchdog module **notifies fault module**
4. Fault module saves information and **resets MCU**
5. System hopefully recovers

**Failure scenario 1 - Periodic checking stops:**
1. Periodic checking of software watchdogs stops
2. Never determine "all watchdogs OK"
3. **Stop feeding hardware watchdog**
4. Hardware watchdog **resets MCU**

**Failure scenario 2 - Fault module gets stuck:**
1. Software watchdog triggers
2. Fault module notified but **gets stuck** processing
3. No one feeding hardware watchdog (stopped scanning)
4. Hardware watchdog **resets MCU**

**Result**: System of checks with simple, reliable design.

---

## Initialization Watchdog Design

### **Challenge: Counting Across Resets**
Need to count failed initializations **across MCU resets** - normal variables are re-initialized after reset (in `.data` and `.bss` sections).

### **Solution: No-Init RAM Block**
Special memory block in RAM that **doesn't get initialized** after reset.

**Implementation:**
1. Add `.no.init.vars` section to linker script
2. Place consecutive failed initialization counter there
3. Also includes **magic number** and **check value** for validation

**Counter reset conditions:**
- Successful initialization
- Invalid data (e.g., after power cycle - all RAM lost)
- Previous reset **NOT** due to hardware watchdog (e.g., button press)

**Alternative considered**: Store in flash (easier in some ways, but writing to flash has issues)

### **Linker Script Modification**

From transcript (lines 120-140), Gene added to linker script around line 163:

```ld
/* No-init variables for watchdog module */
.no.init.vars (NOLOAD) :
{
  . = ALIGN(4);
  *(.no.init.vars)
  . = ALIGN(4);
} >RAM
```

**Key points**:
- Defines new section of memory
- Variables in `.no.init.vars` section go here
- Located in RAM, right after `.bss` section, before heap
- **Not touched by startup code** after reset

---

## Watchdog Module API

From `wdg.c`:

### **Data Structures**

**Software watchdog:**
```58:62:ram-class-nucleo-f401re/modules/wdg/wdg.c
struct soft_wdg
{
    uint32_t period_ms;
    uint32_t last_feed_time_ms;
};
```

**Module state:**
```64:68:ram-class-nucleo-f401re/modules/wdg/wdg.c
struct wdg_state
{
    struct soft_wdg soft_wdgs[CONFIG_WDG_NUM_WDGS];
    wdg_triggered_cb triggered_cb;
};
```

**No-init variables:**
```70:74:ram-class-nucleo-f401re/modules/wdg/wdg.c
struct wdg_no_init_vars {
    uint32_t magic;
    uint32_t consec_failed_init_ctr;
    uint32_t check;
};
```

**Declaration with special attribute:**
```121:121:ram-class-nucleo-f401re/modules/wdg/wdg.c
struct wdg_no_init_vars no_init_vars __attribute__((section (".no.init.vars")));
```

**Key**: `__attribute__((section (".no.init.vars")))` tells GCC compiler to place this variable in the special section defined in linker script.

### **Core Functions**

**1. Module start - sets up periodic timer:**
```155:174:ram-class-nucleo-f401re/modules/wdg/wdg.c
int32_t wdg_start(void)
{
    int32_t rc;

    rc = cmd_register(&cmd_info);
    if (rc < 0) {
        log_error("wdg_start: cmd error %d\n", rc);
        goto exit;
    }

    rc = tmr_inst_get_cb(CONFIG_WDG_RUN_CHECK_MS,
                         wdg_tmr_cb, 0, TMR_CNTX_BASE_LEVEL);
    if (rc < 0) {
        log_error("wdg_start: tmr error %d\n", rc);
        goto exit;
    }

exit:
    return rc;
}
```

Timer runs every `CONFIG_WDG_RUN_CHECK_MS` (10ms) to check software watchdogs.

**2. Client registration:**
```184:196:ram-class-nucleo-f401re/modules/wdg/wdg.c
int32_t wdg_register(uint32_t wdg_id, uint32_t period_ms)
{
    struct soft_wdg* soft_wdg;

    if (wdg_id >= CONFIG_WDG_NUM_WDGS)
        return MOD_ERR_ARG;

    soft_wdg = &state.soft_wdgs[wdg_id];
    soft_wdg->last_feed_time_ms = tmr_get_ms();
    soft_wdg->period_ms = period_ms;

    return 0;
}
```

Client passes static ID (from `config.h`) and period.

**3. Feed watchdog:**
```205:211:ram-class-nucleo-f401re/modules/wdg/wdg.c
int32_t wdg_feed(uint32_t wdg_id)
{
    if (wdg_id >= CONFIG_WDG_NUM_WDGS)
        return MOD_ERR_ARG;
    state.soft_wdgs[wdg_id].last_feed_time_ms = tmr_get_ms();
    return 0;
}
```

Simply updates timestamp.

**4. Register trigger callback (for fault module):**
```218:222:ram-class-nucleo-f401re/modules/wdg/wdg.c
int32_t wdg_register_triggered_cb(wdg_triggered_cb triggered_cb)
{
    state.triggered_cb = triggered_cb;
    return 0;
}
```

**5. Start initialization hardware watchdog:**
```234:248:ram-class-nucleo-f401re/modules/wdg/wdg.c
void wdg_start_init_hdw_wdg(void)
{
    validate_no_init_vars();

    if ((fault_get_rcc_csr() & RCC_CSR_IWDGRSTF_Msk) == 0) {
        // Reset was not due to IWDG, so we reset the counter.
        no_init_vars.consec_failed_init_ctr = 0;
    }
    if (no_init_vars.consec_failed_init_ctr < CONFIG_WDG_MAX_INIT_FAILS ||
        CONFIG_WDG_MAX_INIT_FAILS == 0) {
        wdg_start_hdw_wdg(CONFIG_WDG_INIT_TIMEOUT_MS);
    }
    no_init_vars.consec_failed_init_ctr++;
    update_no_init_vars();
}
```

**Logic**:
- Validate no-init vars
- If reset **NOT** due to watchdog → reset counter
- If under max failures (or max=0 for infinite) → start hardware watchdog
- **Increment counter** (assume failure; will be reset on success)

**6. Successful initialization:**
```256:261:ram-class-nucleo-f401re/modules/wdg/wdg.c
void wdg_init_successful(void)
{
    validate_no_init_vars();
    no_init_vars.consec_failed_init_ctr = 0;
    update_no_init_vars();
}
```

Called when initialization completes - resets counter.

**7. Start hardware watchdog:**
```284:327:ram-class-nucleo-f401re/modules/wdg/wdg.c
int32_t wdg_start_hdw_wdg(uint32_t timeout_ms)
{
    int32_t ctr;

    #define SANITY_CTR_LIMIT 1000000
    #define LSI_FREQ_HZ 32000
    #define WDG_PRESCALE 64
    #define WDG_PRESCALE_SETTING LL_IWDG_PRESCALER_64
    #define WDG_CLK_FREQ_HZ (LSI_FREQ_HZ/WDG_PRESCALE)
    #define WDG_MAX_RL 0xfff
    #define MS_PER_SEC 1000
    #define WDG_MS_TO_RL(ms) \
        (((ms) * WDG_CLK_FREQ_HZ + MS_PER_SEC/2)/MS_PER_SEC - 1)

    _Static_assert(CONFIG_WDG_HARD_TIMEOUT_MS <=
                   ((WDG_MAX_RL + 1) * 1000) / WDG_CLK_FREQ_HZ,
                   "Watchdog timeout too large");

    ctr = WDG_MS_TO_RL(timeout_ms);
    if (ctr < 0)
        ctr = 0;
    else if (ctr > WDG_MAX_RL)
        return MOD_ERR_ARG;

    LL_IWDG_Enable(IWDG);
    LL_IWDG_EnableWriteAccess(IWDG);
    LL_IWDG_SetPrescaler(IWDG, WDG_PRESCALE_SETTING); 
    LL_IWDG_SetReloadCounter(IWDG, ctr);
    for (ctr = 0; ctr < SANITY_CTR_LIMIT; ctr++) {
        if (LL_IWDG_IsReady(IWDG))
            break;
    }
    if (ctr >= SANITY_CTR_LIMIT)
        return MOD_ERR_PERIPH;

    // Stop the watchdog counter when the debugger stops the MCU.
    #ifdef DBGMCU_APB1FZR1_DBG_IWDG_STOP_Msk
        DBGMCU->APB1FZR1 |= DBGMCU_APB1FZR1_DBG_IWDG_STOP_Msk;
    #elif defined DBGMCU_APB1_FZ_DBG_IWDG_STOP_Msk
        DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_IWDG_STOP_Msk;
    #endif

    return 0;
}
```

**Key details**:
- LSI clock: 32 kHz
- Prescale: 64 → 2ms resolution
- Maximum timeout: 8192ms
- **Lines 320-324**: **Critical for debugging** - stops watchdog when debugger breaks
  - Without this, breakpoint → watchdog expires → MCU resets (hard to debug!)
  - Gene "found this out the hard way"

**8. Feed hardware watchdog:**
```332:335:ram-class-nucleo-f401re/modules/wdg/wdg.c
void wdg_feed_hdw(void)
{
    LL_IWDG_ReloadCounter(IWDG);
}
```

---

## Timer Callback - The Heart of the System

```358:397:ram-class-nucleo-f401re/modules/wdg/wdg.c
static enum tmr_cb_action wdg_tmr_cb(int32_t tmr_id,
                                     uint32_t user_data)
{
    int32_t idx;
    struct soft_wdg* soft_wdg;
    bool wdg_triggered = false;

    if (test_cmd_disable_wdg) {
        wdg_feed_hdw();
        goto exit;
    }

    for (idx = 0, soft_wdg = &state.soft_wdgs[0];
         idx < CONFIG_WDG_NUM_WDGS;
         idx++, soft_wdg++)
    {
        if (soft_wdg->period_ms != 0) {

            // We have to careful with race conditions, especially for
            // watchdogs fed from interrupt handlers.

            uint32_t last_feed_time_ms = soft_wdg->last_feed_time_ms;
            if (tmr_get_ms() - last_feed_time_ms > soft_wdg->period_ms) {
                wdg_triggered = true;
                if (state.triggered_cb != NULL) {
                    // This function will normally not return.
                    state.triggered_cb(idx);
                }
            }
        }
    }

    if (!wdg_triggered) {
        if (!test_cmd_fail_hard_wdg)
            wdg_feed_hdw();
    }

exit:
    return TMR_CB_RESTART;
}
```

**How it works**:
1. Called every 10ms (periodic timer)
2. **Check all software watchdogs** (loop through array)
3. If any triggered → **call fault module callback** (normally doesn't return)
4. **Only if no watchdog triggered** → feed hardware watchdog (line 392)
5. Return `TMR_CB_RESTART` to keep running periodically

**Critical insight**: If this callback stops running:
- Hardware watchdog never fed
- MCU resets
- **Monitors timer service AND super loop** (BASE_LEVEL timers called from super loop)

---

## No-Init Variables Validation

```402:421:ram-class-nucleo-f401re/modules/wdg/wdg.c
static void validate_no_init_vars(void)
{
    static const uint32_t num_u32_to_check = 
        sizeof(struct wdg_no_init_vars)/sizeof(uint32_t) - 1;
    uint32_t idx;
    uint32_t new_check = 0xBAADCEED; // Seed.

    for (idx = 0; idx < num_u32_to_check; idx++) {
        new_check = ((new_check << 1) | (new_check >> 31)) ^
            ((uint32_t*)&no_init_vars)[idx];
    }

    if (no_init_vars.magic != WDG_NO_INIT_VARS_MAGIC ||
        no_init_vars.check != new_check)
    {
        memset(&no_init_vars, 0, sizeof(no_init_vars));
        no_init_vars.magic = WDG_NO_INIT_VARS_MAGIC;
        no_init_vars.check = new_check;
    }
}
```

**Checksum algorithm**: Rotating XOR to detect corruption or invalid data (e.g., after power cycle).

---

## Module Integration

From `app_main.c`:

```381:388:ram-class-nucleo-f401re/app1/app_main.c
#if CONFIG_WDG_PRESENT
    {
        .name = "wdg",
        .instance = MOD_NO_INSTANCE,
        .ops.singleton.mod_init = (mod_init)wdg_init,
        .ops.singleton.mod_start = (mod_start)wdg_start,
    },
#endif
```

**Early in initialization:**
```543:545:ram-class-nucleo-f401re/app1/app_main.c
#if CONFIG_FAULT_PRESENT
    wdg_start_init_hdw_wdg();
#endif
```

**After successful initialization:**
```634:641:ram-class-nucleo-f401re/app1/app_main.c
#if CONFIG_FAULT_PRESENT
    wdg_init_successful();
    rc = wdg_start_hdw_wdg(CONFIG_WDG_HARD_TIMEOUT_MS);
    if (rc < 0) {
        log_error("main: wdg_start_hdw_wdg error %d\n", rc);
        INC_SAT_U16(cnts_u16[CNT_START_ERR]);
    }
#endif
```

---

## Demo Walkthrough

### **Setup**
- Temperature/humidity module (from I²C course) uses watchdog
- Watchdog triggers if problems getting measurements
- Fault module handles watchdog trigger notifications

### **Demo Steps**

**1. Normal operation:**
- System logs temp/humidity samples
- `wdg status` command shows:
  - Software watchdogs registered
  - Last feed time incrementing
  - Elapsed time since last feed

**2. Trigger software watchdog:**
- Use `i2c test` command to reserve I²C bus
- Temp/humidity module can't access bus
- **Stops feeding its watchdog**
- Few seconds later: Fault module output appears
  - `Fault type=1` (watchdog)
  - `watchdog id=0`
- System **resets** (from fault module)
- After reset: Bus released, temp/humidity recovers

**3. Trigger hardware watchdog:**
- `wdg test fail-hdw` command
- Sets flag to **prevent feeding hardware watchdog**
- System continues taking measurements
- Hardware watchdog expires → **direct MCU reset**
- **Fault module NOT involved** (no chance)

### **Timeout Values**
Demo uses **small values** (5 seconds) to keep demo short.

**Real systems**: Use **much larger values** than you think you need - don't want unnecessary resets!

---

## Console Commands

**`wdg status`:**
```451:468:ram-class-nucleo-f401re/modules/wdg/wdg.c
static int32_t cmd_wdg_status(int32_t argc, const char** argv)
{
    uint32_t id;

    printc("Current time: %10lu\nWatchdog %s.\n",
           tmr_get_ms(),
           test_cmd_disable_wdg ? "disabled" : "enabled");
    printc("consec_failed_init_ctr=%lu\n", no_init_vars.consec_failed_init_ctr);

    printc("\nID  PERIOD LAST_FEED  ELAPSED\n"
             "--- ------ ---------- -------\n");
    for (id = 0; id < ARRAY_SIZE(state.soft_wdgs); id++) {
        struct soft_wdg* c = &state.soft_wdgs[id];
        printc("%3lu %6lu %10lu %7ld\n", id, c->period_ms, c->last_feed_time_ms,
               tmr_get_ms() - c->last_feed_time_ms);
    }
     return 0;
}
```

Shows current time, all watchdog states, consecutive failed init counter.

**`wdg test`** - Various test operations:
- `fail-hdw` - Prevent feeding hardware watchdog
- `disable` - Disable watchdog checking
- `enable` - Re-enable watchdog
- `init-fails N` - Set consecutive failed initialization counter

---

## Discussion Questions & Answers

### **Q1: How enhance module for client that needs to be busy and can't feed watchdog temporarily?**

**Options**:

**A. Increase timeout on the fly:**
- API to change timeout period temporarily
- Client restores original value when done
- **Downside**: Must predict how much time needed (difficult)

**B. Stop/start checking API (Gene thinks he did this):**
- API to tell watchdog module: "stop checking this watchdog"
- Another API: "start checking again"

**C. Use timeout handler (related to next question):**
- Client intercepts trigger event

### **Q2: How allow client to handle trigger event before system reset?**

**Solution**: Add `handler` parameter to registration API:
- Client specifies callback function for when watchdog triggers
- When watchdog triggers:
  1. Watchdog module calls client's handler first
  2. Based on return value, either:
     - **Ignore** the trigger (client handled it)
     - **Pass to fault module** as normal

Client can say: "Never mind, I took care of it"

---

## Summary (from transcript)

1. **Watchdogs are common** way to ensure system performs its function
2. **Hardware-based watchdogs** typically reset MCU directly
3. Sometimes want **actions before reset** (save state information)
4. **Software-based watchdogs** trigger before hardware watchdog (under normal conditions)
5. Some **hardware watchdogs support early warning interrupt** (alternative approach)

---

## Key Design Principles

1. **Multiple watchdogs** for different critical functions
2. **Hierarchy of checkers** - hardware checks software watchdogs
3. **Work-based monitoring** - ensure critical work is DONE, not just code executing
4. **Graceful degradation** - consecutive init failure limit prevents infinite boot loops
5. **No-init RAM** - clever linker trick to preserve data across resets
6. **Debugger-friendly** - stop watchdog when debugger breaks

```plaintext
/* No-init variables for watchdog module */
.no.init.vars (NOLOAD) :
{
  . = ALIGN(4);
  *(.no.init.vars)
  . = ALIGN(4);
} >RAM
```

```c
struct soft_wdg
{
    uint32_t period_ms;
    uint32_t last_feed_time_ms;
};
```

```c
struct wdg_state
{
    struct soft_wdg soft_wdgs[CONFIG_WDG_NUM_WDGS];
    wdg_triggered_cb triggered_cb;
};
```

```c
struct wdg_no_init_vars {
    uint32_t magic;
    uint32_t consec_failed_init_ctr;
    uint32_t check;
};
```

```c
struct wdg_no_init_vars no_init_vars __attribute__((section (".no.init.vars")));
```

```c
int32_t wdg_start(void)
{
    int32_t rc;

    rc = cmd_register(&cmd_info);
    if (rc < 0) {
        log_error("wdg_start: cmd error %d\n", rc);
        goto exit;
    }

    rc = tmr_inst_get_cb(CONFIG_WDG_RUN_CHECK_MS,
                         wdg_tmr_cb, 0, TMR_CNTX_BASE_LEVEL);
    if (rc < 0) {
        log_error("wdg_start: tmr error %d\n", rc);
        goto exit;
    }

exit:
    return rc;
}
```

```c
int32_t wdg_register(uint32_t wdg_id, uint32_t period_ms)
{
    struct soft_wdg* soft_wdg;

    if (wdg_id >= CONFIG_WDG_NUM_WDGS)
        return MOD_ERR_ARG;

    soft_wdg = &state.soft_wdgs[wdg_id];
    soft_wdg->last_feed_time_ms = tmr_get_ms();
    soft_wdg->period_ms = period_ms;

    return 0;
}
```

```c
int32_t wdg_feed(uint32_t wdg_id)
{
    if (wdg_id >= CONFIG_WDG_NUM_WDGS)
        return MOD_ERR_ARG;
    state.soft_wdgs[wdg_id].last_feed_time_ms = tmr_get_ms();
    return 0;
}
```

```c
int32_t wdg_register_triggered_cb(wdg_triggered_cb triggered_cb)
{
    state.triggered_cb = triggered_cb;
    return 0;
}
```

```c
void wdg_start_init_hdw_wdg(void)
{
    validate_no_init_vars();

    if ((fault_get_rcc_csr() & RCC_CSR_IWDGRSTF_Msk) == 0) {
        // Reset was not due to IWDG, so we reset the counter.
        no_init_vars.consec_failed_init_ctr = 0;
    }
    if (no_init_vars.consec_failed_init_ctr < CONFIG_WDG_MAX_INIT_FAILS ||
        CONFIG_WDG_MAX_INIT_FAILS == 0) {
        wdg_start_hdw_wdg(CONFIG_WDG_INIT_TIMEOUT_MS);
    }
    no_init_vars.consec_failed_init_ctr++;
    update_no_init_vars();
}
```

```c
void wdg_init_successful(void)
{
    validate_no_init_vars();
    no_init_vars.consec_failed_init_ctr = 0;
    update_no_init_vars();
}
```

```c
int32_t wdg_start_hdw_wdg(uint32_t timeout_ms)
{
    int32_t ctr;

    #define SANITY_CTR_LIMIT 1000000
    #define LSI_FREQ_HZ 32000
    #define WDG_PRESCALE 64
    #define WDG_PRESCALE_SETTING LL_IWDG_PRESCALER_64
    #define WDG_CLK_FREQ_HZ (LSI_FREQ_HZ/WDG_PRESCALE)
    #define WDG_MAX_RL 0xfff
    #define MS_PER_SEC 1000
    #define WDG_MS_TO_RL(ms) \
        (((ms) * WDG_CLK_FREQ_HZ + MS_PER_SEC/2)/MS_PER_SEC - 1)

    _Static_assert(CONFIG_WDG_HARD_TIMEOUT_MS <=
                   ((WDG_MAX_RL + 1) * 1000) / WDG_CLK_FREQ_HZ,
                   "Watchdog timeout too large");

    ctr = WDG_MS_TO_RL(timeout_ms);
    if (ctr < 0)
        ctr = 0;
    else if (ctr > WDG_MAX_RL)
        return MOD_ERR_ARG;

    LL_IWDG_Enable(IWDG);
    LL_IWDG_EnableWriteAccess(IWDG);
    LL_IWDG_SetPrescaler(IWDG, WDG_PRESCALE_SETTING); 
    LL_IWDG_SetReloadCounter(IWDG, ctr);
    for (ctr = 0; ctr < SANITY_CTR_LIMIT; ctr++) {
        if (LL_IWDG_IsReady(IWDG))
            break;
    }
    if (ctr >= SANITY_CTR_LIMIT)
        return MOD_ERR_PERIPH;

    // Stop the watchdog counter when the debugger stops the MCU.
    #ifdef DBGMCU_APB1FZR1_DBG_IWDG_STOP_Msk
        DBGMCU->APB1FZR1 |= DBGMCU_APB1FZR1_DBG_IWDG_STOP_Msk;
    #elif defined DBGMCU_APB1_FZ_DBG_IWDG_STOP_Msk
        DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_IWDG_STOP_Msk;
    #endif

    return 0;
}
```

```c
void wdg_feed_hdw(void)
{
    LL_IWDG_ReloadCounter(IWDG);
}
```

```c
static enum tmr_cb_action wdg_tmr_cb(int32_t tmr_id,
                                     uint32_t user_data)
{
    int32_t idx;
    struct soft_wdg* soft_wdg;
    bool wdg_triggered = false;

    if (test_cmd_disable_wdg) {
        wdg_feed_hdw();
        goto exit;
    }

    for (idx = 0, soft_wdg = &state.soft_wdgs[0];
         idx < CONFIG_WDG_NUM_WDGS;
         idx++, soft_wdg++)
    {
        if (soft_wdg->period_ms != 0) {

            // We have to careful with race conditions, especially for
            // watchdogs fed from interrupt handlers.

            uint32_t last_feed_time_ms = soft_wdg->last_feed_time_ms;
            if (tmr_get_ms() - last_feed_time_ms > soft_wdg->period_ms) {
                wdg_triggered = true;
                if (state.triggered_cb != NULL) {
                    // This function will normally not return.
                    state.triggered_cb(idx);
                }
            }
        }
    }

    if (!wdg_triggered) {
        if (!test_cmd_fail_hard_wdg)
            wdg_feed_hdw();
    }

exit:
    return TMR_CB_RESTART;
}
```

```c
static void validate_no_init_vars(void)
{
    static const uint32_t num_u32_to_check = 
        sizeof(struct wdg_no_init_vars)/sizeof(uint32_t) - 1;
    uint32_t idx;
    uint32_t new_check = 0xBAADCEED; // Seed.

    for (idx = 0; idx < num_u32_to_check; idx++) {
        new_check = ((new_check << 1) | (new_check >> 31)) ^
            ((uint32_t*)&no_init_vars)[idx];
    }

    if (no_init_vars.magic != WDG_NO_INIT_VARS_MAGIC ||
        no_init_vars.check != new_check)
    {
        memset(&no_init_vars, 0, sizeof(no_init_vars));
        no_init_vars.magic = WDG_NO_INIT_VARS_MAGIC;
        no_init_vars.check = new_check;
    }
}
```

```c
#if CONFIG_WDG_PRESENT
    {
        .name = "wdg",
        .instance = MOD_NO_INSTANCE,
        .ops.singleton.mod_init = (mod_init)wdg_init,
        .ops.singleton.mod_start = (mod_start)wdg_start,
    },
#endif
```

```c
#if CONFIG_FAULT_PRESENT
    wdg_start_init_hdw_wdg();
#endif
```

```c
#if CONFIG_FAULT_PRESENT
    wdg_init_successful();
    rc = wdg_start_hdw_wdg(CONFIG_WDG_HARD_TIMEOUT_MS);
    if (rc < 0) {
        log_error("main: wdg_start_hdw_wdg error %d\n", rc);
        INC_SAT_U16(cnts_u16[CNT_START_ERR]);
    }
#endif
```

```c
static int32_t cmd_wdg_status(int32_t argc, const char** argv)
{
    uint32_t id;

    printc("Current time: %10lu\nWatchdog %s.\n",
           tmr_get_ms(),
           test_cmd_disable_wdg ? "disabled" : "enabled");
    printc("consec_failed_init_ctr=%lu\n", no_init_vars.consec_failed_init_ctr);

    printc("\nID  PERIOD LAST_FEED  ELAPSED\n"
             "--- ------ ---------- -------\n");
    for (id = 0; id < ARRAY_SIZE(state.soft_wdgs); id++) {
        struct soft_wdg* c = &state.soft_wdgs[id];
        printc("%3lu %6lu %10lu %7ld\n", id, c->period_ms, c->last_feed_time_ms,
               tmr_get_ms() - c->last_feed_time_ms);
    }
     return 0;
}
```

---

**User**

Summarize @Reliability, Availability, and Maintainability Lesson 6. Stack Overflow Protection.txt   compredhensively. include the related snippet code

The source code is in @app_main.c @lwl.c @fault.c 

don't make things up

---

**Cursor**

# Comprehensive Summary: RAM Lesson 6 - Stack Overflow Protection

## Overview
Stack overflow is a **nasty problem** - when it occurs, the system often just starts acting strange with random effects. The goal is to prevent it, but if it does occur, take action. This lesson covers two main techniques: **MPU-based stack guard** and **high water mark detection**.

---

## What is Stack Overflow?

### **The Stack**
RAM region used by functions and interrupt handlers for:
- **Return addresses**
- **Saving registers**
- **Local variables** (particularly)
- **Argument passing** (sometimes; CPU registers used when possible)

### **How Stack Works**
- Stack pointer **initialized to highest RAM address**
- As functions call and interrupts nest, **stack grows downward** (toward lower addresses)
- Stack grows and shrinks as software runs

### **Stack Overflow Problem**
- Stack extends into other memory areas (global variables, heap)
- **CPU doesn't know** it's exceeding stack area
- Memory gets **modified at random**
- System starts doing **weird things**
- **Very confusing to debug** - developers spend considerable time before realizing stack overflow

### **The Challenge**
**Hard to estimate maximum stack size** because:
- Must understand code very well
- Need to know stack usage of each function
- Extremely difficult to predict worst-case

**Tools exist** (compiler knows function stack usage and call chains) but have **limitations**.

---

## Techniques for Addressing Stack Overflow

### **1. MPU-Based Stack Guard**
For MCUs with Memory Protection Unit (MPU), create a **guard region** that limits stack growth.

### **2. High Water Mark Detection**
Fill stack with special pattern during initialization, then determine maximum usage by finding where pattern is overwritten.

---

## Memory Protection Unit (MPU)

### **What is MPU?**
- **NOT an MMU** (Memory Management Unit for virtual memory/Linux)
- Hardware device within MCU that places **restrictions on specific memory regions**
- Controls whether memory can be read or written

### **MPU Characteristics**
- **Limited number of regions** (8 for this MCU)
- To set up region, specify:
  - Starting address
  - Size or ending address
  - Access restrictions (e.g., read-only)

### **Stack Guard with MPU**
- Create **small read-only region** just beyond stack area
- For STM32F401: **Minimum region size = 32 bytes**
- 32 bytes wasted memory (acceptable tradeoff)

### **What Happens on Overflow?**
1. Processor tries to write to stack guard region
2. **MemManage fault occurs**
3. Exception handler invoked
4. Software typically does **reset**

**Benefits:**
- **Safety**: Reset prevents dangerous behavior from memory corruption
- **Availability**: Maximizes time system runs properly

**Best Practice:**
- Catch during testing before field deployment
- **Size stack bigger than needed** (safety margin)
- Give all spare RAM to stack if available

---

## RAM Memory Map

### **Initial Map** (IDE-generated linker script)
```
High Address (0x20018000) ← Stack pointer init
│
├─ STACK (all unused RAM)
│
├─ Heap (minimum size from settings)
│
├─ .bss (uninitialized globals/statics)
│
├─ .data (initialized globals/statics)
│
Low Address (0x20000000)
```

### **Modified Map** (with stack guard)
```
High Address (0x20018000) ← Stack pointer init
│
├─ STACK (reduced by 32 bytes)
│
├─ STACK GUARD (32 bytes, read-only) ← Danger zone!
│
├─ Heap
│
├─ .bss
│
├─ .data
│
Low Address (0x20000000)
```

---

## Linker Script Modifications

### **Define MPU Block Size Constant**
```ld
_min_mpu_region_size = 32;
```

### **Allocate Stack Guard**
From transcript (lines 101-124):

```ld
. = ALIGN(4);
PROVIDE(end = .);
. = . + _Min_Heap_Size;
. = ALIGN(_min_mpu_region_size);  /* NEW: Align to 32-byte boundary */
_s_stack_guard = .;                 /* NEW: Start of guard region */
. = . + _min_mpu_region_size;      /* NEW: Allocate 32 bytes */
_e_stack_guard = .;                 /* NEW: End of guard region */
. = . + _Min_Stack_Size;
. = ALIGN(8);
PROVIDE(_estack = .);
```

**Key additions**:
1. Align to 32-byte boundary (MPU requirement)
2. Define `_s_stack_guard` (start address)
3. Allocate 32 bytes for guard
4. Define `_e_stack_guard` (end address)
5. These symbols accessible from C code

---

## Implementation in fault.c

### **Stack Pattern Constant**

```64:64:ram-class-nucleo-f401re/modules/fault/fault.c
#define STACK_INIT_PATTERN 0xcafebadd
```

Memorable pattern (reads as "cafe bad") - unlikely to occur by chance, recognizable in hex dump.

### **Fill Stack with Pattern**

From `fault_start()` function:

```270:280:ram-class-nucleo-f401re/modules/fault/fault.c
    // Fill stack with a pattern so we can detect high water mark. We also fill
    // the stack guard region (which hasn't been set up yet) just for
    // completeness even though it isn't necessary.
    //
    // The stack pointer points to the last word written, so we first decrement
    // it to get to the next word to write.

    __ASM volatile("MOV  %0, sp" : "=r" (sp) : : "memory");
    sp--;
    while (sp >= &_s_stack_guard)
        *sp-- = STACK_INIT_PATTERN;
```

**How it works**:
1. Use **inline assembly** to get current stack pointer (GCC extension)
2. Decrement SP (ARM decrements before writing)
3. Fill from current SP down to stack guard start
4. Also fills guard region (before MPU setup)

### **Set Up MPU Stack Guard**

For MPU Type 1 (STM32F401):

```282:318:ram-class-nucleo-f401re/modules/fault/fault.c
#if CONFIG_MPU_TYPE == 1

    // Set up stack guard region.
    //
    // - Region = 0, the region number we are using.
    // - SubRegionDisable = 0, meaning all subregions are enabled (really
    //   doesn't matter).
    // - Address = _s_stack_guard, the base address of the guard region.
    // - Attributes:
    //   + LL_MPU_REGION_SIZE_32B, meaning 32 byte region.
    //   + LL_MPU_REGION_PRIV_RO_URO, meaning read-only access (priv/unpriv).
    //   + LL_MPU_TEX_LEVEL0, meaning strongly ordered (really doesn't matter).
    //   + LL_MPU_INSTRUCTION_ACCESS_DISABLE, meaning an instruction fetch 
    //     never allowed.
    //   + LL_MPU_ACCESS_SHAREABLE, meaning sharable (but "don't care" since
    //     TEX is set to 000).
    //   + LL_MPU_ACCESS_NOT_CACHEABLE, meaning not cachable (really doesn't
    //     matter).
    //   + LL_MPU_ACCESS_NOT_BUFFERABLE, meaning not bufferable (really doesn't
    //     matter).

    LL_MPU_ConfigRegion(0, 0, (uint32_t)(&_s_stack_guard),
                        LL_MPU_REGION_SIZE_32B |
                        LL_MPU_REGION_PRIV_RO_URO |
                        LL_MPU_TEX_LEVEL0 |
                        LL_MPU_INSTRUCTION_ACCESS_DISABLE |
                        LL_MPU_ACCESS_SHAREABLE |
                        LL_MPU_ACCESS_NOT_CACHEABLE |
                        LL_MPU_ACCESS_NOT_BUFFERABLE);

    // Now enable the MPU.
    // - PRIVDEFENA = 1, meaning the default memory map is used if there is no
    //   MPU region.
    // - HFNMIENA = 1, meaning MPU is used for even high priority exception
    //   handlers.

    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk|MPU_CTRL_HFNMIENA_Msk);
```

**Key settings**:
- **Region 0** (MCU supports 8 regions: 0-7)
- **Base address**: `_s_stack_guard` (from linker script)
- **Size**: 32 bytes
- **Permissions**: **Read-only** (LL_MPU_REGION_PRIV_RO_URO)
- Enable MPU with default memory map

### **Determine Stack Usage (High Water Mark)**

From `cmd_fault_status()`:

```670:679:ram-class-nucleo-f401re/modules/fault/fault.c
    printc("Stack: 0x%08lx -> 0x%08lx (%lu bytes)\n",
           (uint32_t)&_estack,
           (uint32_t)&_e_stack_guard,
           (uint32_t)((&_estack - &_e_stack_guard) * sizeof(uint32_t)));
    sp = &_e_stack_guard;
    while (sp < &_estack && *sp == STACK_INIT_PATTERN)
        sp++;
    printc("Stack usage: 0x%08lx -> 0x%08lx (%lu bytes)\n",
           (uint32_t)&_estack, (uint32_t)sp,
           (uint32_t)((&_estack - sp) * sizeof(uint32_t)));
```

**Algorithm**:
1. Start at lowest stack address (`_e_stack_guard`)
2. Search upward until finding memory **not** containing `STACK_INIT_PATTERN`
3. That location is the **high water mark**
4. Calculate bytes used: `_estack - high_water_mark`

**Why it works**: Stack grows/shrinks but doesn't restore pattern when shrinking - leaves permanent "mark".

### **Stack Overflow Test**

```734:739:ram-class-nucleo-f401re/modules/fault/fault.c
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winfinite-recursion"
static void test_overflow_stack(void){
    test_overflow_stack();
}
#pragma GCC diagnostic pop
```

**Infinite recursion** - each call uses more stack, quickly causes overflow.

**Pragmas**: Suppress compiler warning about infinite recursion (intentional for testing).

Triggered by: `fault test stack` console command.

---

## High Water Mark Visualization

```
Initial State (after init):
┌─────────────────┐ ← 0x20018000 (_estack)
│ PATTERN PATTERN │
│ PATTERN PATTERN │  Stack filled with
│ PATTERN PATTERN │  0xcafebadd
│ PATTERN PATTERN │
└─────────────────┘ ← _e_stack_guard

After Running:
┌─────────────────┐ ← 0x20018000 (_estack)
│  Real Data      │
│  Real Data      │  Used stack
│  Real Data      │  (overwrites pattern)
├─────────────────┤ ← High water mark
│ PATTERN PATTERN │
│ PATTERN PATTERN │  Unused stack
│ PATTERN PATTERN │  (pattern intact)
└─────────────────┘ ← _e_stack_guard
```

---

## Demo Walkthrough

### **1. Check Initial Stack Usage**
```
> fault status
Stack usage: ... 864 bytes
```

### **2. Execute Stack-Heavy Commands**
```
> ttys status
> fault status
Stack usage: ... 940 bytes   (increased!)

> dio status port a
> fault status  
Stack usage: ... 1012 bytes  (increased more!)

> ttys status
> fault status
Stack usage: ... 1012 bytes  (no change - not high water)
```

**Use case**: Run system for long periods, check high water mark, add safety margin.

### **3. Trigger Stack Overflow**
```
> fault test stack
```

**Results**:
- System immediately resets
- Fault module output before reset:
  - `Fault type=2` (exception)
  - `Fault param=4` (MemManage fault)

### **4. Decode Fault Data**

**Map file shows**:
- Stack guard start: `0x20001820`
- Stack guard end: `0x20001840`

**Decoded fault data**:
- **Fault type**: Exception
- **Fault param**: 4 (MemManage)
- **MMFAR** (MemManage Fault Address Register): `0x20001838` ← **Inside guard region!**
- **Stack pointer at fault**: `0x20001820` ← **Right at guard edge!**
- **Exception stack data**: All contains `0xcafebadd` (pattern) - stack wasn't working!

**Confirms**: Stack overflow detected by MPU guard.

---

## Discussion Questions & Answers

### **Q1: Can the stack guard miss stack overflow? (i.e., not work)**

**A: Yes, possible scenario:**

**Situation**:
- Function has **block of local variables ≥ 32 bytes**
- These variables are **never written**
- When function called, locals **aligned/overlap with stack guard**
- No writes attempted to guard region (holding unused locals)
- **No fault occurs** - program "jumped over" guard region

**Likelihood**: Probably unlikely, but:
- Used as **intentional attack vector** in Linux
- Larger guard region makes less likely (but still possible)

**Mitigation**: GCC `-fstack-check` option
- Probes stack to ensure writability
- Prevents jumping over guard regions
- Not sure if works for small 32-byte guard

### **Q2: Could you keep running after stack overflow instead of resetting?**

**A: Yes, possible but very difficult**

**Challenge**: System was doing something when it hit guard - how to recover?

**Option 1: Reset stack and jump to main**
```c
// Reset SP to initial value
sp = _estack;
// Jump back to main
goto main;  // Conceptually
```

**Interesting**: Global/static variables **NOT reset** to init values
- Code might be smart enough to realize what happened
- Try to pick up where it left off

**Real-world example**: **Apollo lunar lander**
- Went through resets like this during landing
- Just picked up and kept going
- "Really interesting story"

**Option 2: setjmp/longjmp**
- `setjmp()` somewhere in code - fills data structure
- Later call `longjmp()` with that structure
- Magically returns to where `setjmp()` was
- "Go back in time"
- Functions are "dodgy" - not considered safe
- Questionable practicality

---

## Key Design Principles

1. **Multiple layers of protection**: MPU guard + high water mark detection
2. **Fail fast and safe**: Reset on overflow prevents dangerous corruption
3. **Measurable during development**: High water mark provides concrete data
4. **Safety margins**: Size stack bigger than needed
5. **Memorable patterns**: `0xcafebadd` recognizable in hex dumps
6. **Debugger-friendly**: MPU setup includes detailed comments
7. **Hardware-specific but portable**: Code supports multiple MPU types via CONFIG

---

## Summary

Stack overflow is **difficult to predict** but **catastrophic when it occurs**. Two complementary techniques address this:

1. **MPU Stack Guard** (proactive): Prevents overflow from corrupting other memory, triggers immediate fault
2. **High Water Mark** (diagnostic): Measures actual stack usage during development/testing

Together they provide **safety** (prevent dangerous corruption), **availability** (automatic reset and recovery), and **maintainability** (diagnostic information for developers).

```plaintext
High Address (0x20018000) ← Stack pointer init
│
├─ STACK (all unused RAM)
│
├─ Heap (minimum size from settings)
│
├─ .bss (uninitialized globals/statics)
│
├─ .data (initialized globals/statics)
│
Low Address (0x20000000)
```

```plaintext
High Address (0x20018000) ← Stack pointer init
│
├─ STACK (reduced by 32 bytes)
│
├─ STACK GUARD (32 bytes, read-only) ← Danger zone!
│
├─ Heap
│
├─ .bss
│
├─ .data
│
Low Address (0x20000000)
```

```plaintext
_min_mpu_region_size = 32;
```

```plaintext
. = ALIGN(4);
PROVIDE(end = .);
. = . + _Min_Heap_Size;
. = ALIGN(_min_mpu_region_size);  /* NEW: Align to 32-byte boundary */
_s_stack_guard = .;                 /* NEW: Start of guard region */
. = . + _min_mpu_region_size;      /* NEW: Allocate 32 bytes */
_e_stack_guard = .;                 /* NEW: End of guard region */
. = . + _Min_Stack_Size;
. = ALIGN(8);
PROVIDE(_estack = .);
```

```c
#define STACK_INIT_PATTERN 0xcafebadd
```

```c
    // Fill stack with a pattern so we can detect high water mark. We also fill
    // the stack guard region (which hasn't been set up yet) just for
    // completeness even though it isn't necessary.
    //
    // The stack pointer points to the last word written, so we first decrement
    // it to get to the next word to write.

    __ASM volatile("MOV  %0, sp" : "=r" (sp) : : "memory");
    sp--;
    while (sp >= &_s_stack_guard)
        *sp-- = STACK_INIT_PATTERN;
```

```c
#if CONFIG_MPU_TYPE == 1

    // Set up stack guard region.
    //
    // - Region = 0, the region number we are using.
    // - SubRegionDisable = 0, meaning all subregions are enabled (really
    //   doesn't matter).
    // - Address = _s_stack_guard, the base address of the guard region.
    // - Attributes:
    //   + LL_MPU_REGION_SIZE_32B, meaning 32 byte region.
    //   + LL_MPU_REGION_PRIV_RO_URO, meaning read-only access (priv/unpriv).
    //   + LL_MPU_TEX_LEVEL0, meaning strongly ordered (really doesn't matter).
    //   + LL_MPU_INSTRUCTION_ACCESS_DISABLE, meaning an instruction fetch 
    //     never allowed.
    //   + LL_MPU_ACCESS_SHAREABLE, meaning sharable (but "don't care" since
    //     TEX is set to 000).
    //   + LL_MPU_ACCESS_NOT_CACHEABLE, meaning not cachable (really doesn't
    //     matter).
    //   + LL_MPU_ACCESS_NOT_BUFFERABLE, meaning not bufferable (really doesn't
    //     matter).

    LL_MPU_ConfigRegion(0, 0, (uint32_t)(&_s_stack_guard),
                        LL_MPU_REGION_SIZE_32B |
                        LL_MPU_REGION_PRIV_RO_URO |
                        LL_MPU_TEX_LEVEL0 |
                        LL_MPU_INSTRUCTION_ACCESS_DISABLE |
                        LL_MPU_ACCESS_SHAREABLE |
                        LL_MPU_ACCESS_NOT_CACHEABLE |
                        LL_MPU_ACCESS_NOT_BUFFERABLE);

    // Now enable the MPU.
    // - PRIVDEFENA = 1, meaning the default memory map is used if there is no
    //   MPU region.
    // - HFNMIENA = 1, meaning MPU is used for even high priority exception
    //   handlers.

    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk|MPU_CTRL_HFNMIENA_Msk);
```

```c
    printc("Stack: 0x%08lx -> 0x%08lx (%lu bytes)\n",
           (uint32_t)&_estack,
           (uint32_t)&_e_stack_guard,
           (uint32_t)((&_estack - &_e_stack_guard) * sizeof(uint32_t)));
    sp = &_e_stack_guard;
    while (sp < &_estack && *sp == STACK_INIT_PATTERN)
        sp++;
    printc("Stack usage: 0x%08lx -> 0x%08lx (%lu bytes)\n",
           (uint32_t)&_estack, (uint32_t)sp,
           (uint32_t)((&_estack - sp) * sizeof(uint32_t)));
```

```c
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winfinite-recursion"
static void test_overflow_stack(void){
    test_overflow_stack();
}
#pragma GCC diagnostic pop
```

```plaintext
Initial State (after init):
┌─────────────────┐ ← 0x20018000 (_estack)
│ PATTERN PATTERN │
│ PATTERN PATTERN │  Stack filled with
│ PATTERN PATTERN │  0xcafebadd
│ PATTERN PATTERN │
└─────────────────┘ ← _e_stack_guard

After Running:
┌─────────────────┐ ← 0x20018000 (_estack)
│  Real Data      │
│  Real Data      │  Used stack
│  Real Data      │  (overwrites pattern)
├─────────────────┤ ← High water mark
│ PATTERN PATTERN │
│ PATTERN PATTERN │  Unused stack
│ PATTERN PATTERN │  (pattern intact)
└─────────────────┘ ← _e_stack_guard
```

```plaintext
> fault status
Stack usage: ... 864 bytes
```

```plaintext
> ttys status
> fault status
Stack usage: ... 940 bytes   (increased!)

> dio status port a
> fault status  
Stack usage: ... 1012 bytes  (increased more!)

> ttys status
> fault status
Stack usage: ... 1012 bytes  (no change - not high water)
```

```plaintext
> fault test stack
```

```c
// Reset SP to initial value
sp = _estack;
// Jump back to main
goto main;  // Conceptually
```

---

**User**

Summarize @Reliability, Availability, and Maintainability Lesson 7. Asserts and Audits.txt   compredhensively. include the related code

The source code is in @app_main.c @lwl.c @fault.c 


---

**Cursor**

# Comprehensive Summary: RAM Lesson 7 - Asserts and Audits

## Overview
This is a **short, discussion-oriented lesson** covering two additional RAM techniques: **Asserts** and **Audits**. Unlike previous lessons, **this lesson has NO implementation or demo** - it's purely to make developers aware of these tools for potential use in their work.

---

## ASSERTS

### **What is an Assert?**
A software statement declaring a **condition that should be true**.

**Example**: 
```c
assert(pointer_var != NULL);
```

### **Traditional Behavior (Unix Origin)**
Failed assert → **Process aborts** (drastic action)

### **Custom Assert Macros (Common in Projects)**

Multiple assert types with different failure actions:

#### **1. Log-Only Assert**
```c
// If fails: Log error (file name + line number), continue execution
```

#### **2. Return-Error Assert**
```c
// If fails: Generate function return with error code
// Let caller deal with failure
```

#### **3. Fatal Assert**
```c
// If fails: Declare fatal fault (comparable to Linux abort)
```

---

## Example Assert Macro for This Course

**Hypothetical implementation** (not actually in codebase):

```c
#define ASSERT_FATAL(condition) \
    do { \
        if (!(condition)) { \
            fault_detected(FAULT_TYPE_ASSERT, \
                          (FILE_ID << 16) | __LINE__); \
        } \
    } while(0)
```

**How it works**:
- If `condition` is **false** → call `fault_detected()`
- New fault type: `FAULT_TYPE_ASSERT`
- Parameter: `FILE_ID` (shifted 16 bits) + `__LINE__` (predefined macro)
- `fault_detected()` **never returns** → complete fault report generated
- System resets after collecting diagnostic data

---

## Assert Usage Patterns

### **Common Use Cases**
- **Very common**: Beginning of functions to validate arguments
- **Varies by project/developer**: Some never use, some use extensively
- **Development/Testing**: Universally agreed to be useful
  - Fast and easy to write
  - Provide **exact location** where problem spotted

### **Example - Argument Validation**
```c
void process_data(void* data, uint32_t size) {
    ASSERT_FATAL(data != NULL);
    ASSERT_FATAL(size > 0);
    ASSERT_FATAL(size <= MAX_BUFFER_SIZE);
    
    // Function implementation...
}
```

---

## Production/Release Builds - The Debate

### **Traditional Approach**
- **Asserts disabled** in production/release builds
- Macros "nulled out" - no code generated
- **Rationale**: 
  - Assert code served its purpose (found bugs during development)
  - Save time and space by compiling them out

### **Alternative View: Keep Asserts Active**
Many opinions on this - **depends on product and application**.

---

## When to Keep Asserts in Production

### **Safety-Critical Applications**

**Argument FOR keeping asserts**:
- Anything out of the ordinary → reason to enter **fail-safe state**
- Minor symptom might indicate **more serious problem**
- If not seen in testing but appears in field → concerning

**Example: Robotics (Gene's experience)**
- Little tolerance for anything out of ordinary
- Pretty much anything wrong → **shut down**
- Asserts fully justified

---

## When to Disable Asserts in Production

### **Non-Safety-Critical Applications**

**Arguments AGAINST keeping asserts**:
- Need **more sophisticated error handling** instead
- Assert is **primitive tool** - "black and white"
- Want to **avoid unnecessary recovery actions** (hurts availability)
- Affects product perception

### **Real-World Example: Communication Product**

**Gene's experience** with complex communication system:
- Used **lots of asserts** (part of development culture)
- Went to field with **asserts enabled** (unclear if conscious decision)
- Field conditions **much more complex** than test labs
- Experienced **unnecessary recovery actions/resets** due to asserts
- **Problem**: Asserts needed to be **replaced with better error handling**
- **Result**: "Some pain experienced along the way"

**Lesson**: Field conditions can trigger asserts that wouldn't occur in testing, causing unnecessary disruptions.

### **Performance-Oriented Products**

**Another consideration**:
- Asserts use **CPU resources**
- Do **no productive work**
- If need "every bit of CPU" → factor to consider

---

## Summary: Asserts

> **Asserts are a great tool, but for production builds, you need to think carefully about:**
> - Application type (safety-critical vs. not)
> - Error handling sophistication needed
> - Performance requirements
> - Field vs. lab environment differences

---

# AUDITS

### **What are Audits?**
Software that **checks system integrity** - proactive health monitoring.

---

## What Audits Check

### **1. Critical Data Structures and Memory**
- **Valid values** in variables
- **Pointers** are valid (not NULL, in valid range)
- **Indices** within bounds
- Not exhaustive - focus on "big problem" scenarios

### **2. Configuration Data Integrity**
- Compare **working memory** configuration vs. **configuration database**
- Mismatch → problem detected

### **3. Resource Checks**
- **Heap** usage/integrity
- **Work queues** status
- **Orphan nodes** in tree structures
- File system space (if applicable)

### **4. File System Space** (Critical!)

**Example**: `/var` in Linux

**Real-world problem**: Systems run fine for months, then:
- File system fills up
- System starts behaving **erratically**
- Very common issue

**NASA Mars Rover "Spirit" Example** (early 2000s):
- Problem after landing on Mars
- **During Earth→Mars trip**: Background work slowly filled file system
- Never ran system for that duration in testing → problem unknown
- **Fortunately**: Fixed remotely from Earth
- **Lesson**: Long-running processes can slowly consume resources

---

## Purpose of Audits

**Find problems that:**
1. Might **not be visible** immediately
2. Would result in **incorrect operation**
3. May cause **stability problems** in near future

---

## When Audits Run

### **Scheduled Execution**
- Every N minutes/hours
- During **periods of low processing**

### **Types of Audits**

#### **1. Detecting Audits**
- Find problem
- Report it
- Take action based on severity

#### **2. Correcting Audits** (Auto-correcting)
- **Detect AND fix** problem on the fly

**Examples of auto-correction**:
- **Clean up file system** (delete old logs, temp files)
- **Correct configuration settings** in working memory to match database
- **Repair data structures** (remove orphan nodes)

**Warning**: Be careful with correcting audits!
- Not always simple to just "change things in memory"
- Need to think through consequences
- Could make things worse if not done properly

---

## Actions on Failed Audit

Depends on severity:

### **1. Minor Issues**
- **Log error/warning** and continue
- Especially for **correcting audits** (already fixed)

### **2. Moderate Issues**
- **Set alarm** (if system has alarm concept)
- Alert operator to check what's happening

### **3. Serious Issues**
- System in **big trouble**
- **Declare fault** → automatic recovery (possibly full reset)

**Caution**: Don't make audits "trigger happy"
- Running automatically
- Don't want **constant resetting**
- Be very cautious about what triggers full reset

---

## Integration with Fault Module

**In this course's software architecture**:

```c
// Audit detects serious problem
if (critical_problem_detected) {
    fault_detected(FAULT_TYPE_AUDIT, audit_error_code);
    // Never returns - fault module handles recovery
}
```

Similar pattern to assert example - invoke fault module API.

---

## Key Differences: Asserts vs. Audits

| Aspect | Asserts | Audits |
|--------|---------|--------|
| **When** | During code execution (inline) | Scheduled/periodic |
| **What** | Single condition at specific point | System-wide health check |
| **Scope** | Local (function arguments, preconditions) | Global (resources, data integrity) |
| **Frequency** | Every time code path executes | Periodic (minutes/hours) |
| **Purpose** | Catch programming errors | Detect system degradation |
| **Fix** | Cannot fix (just detect) | Can be correcting |

---

## Examples of What Each Catches

### **Asserts Catch:**
- NULL pointer passed to function
- Invalid function argument
- Precondition violation
- Programming errors

### **Audits Catch:**
- File system filling up over time
- Memory leaks accumulating
- Configuration drift
- Resource exhaustion
- Slow system degradation

---

## Implementation Status

**Important Note**: Unlike previous lessons, **NO implementation or demo for this lesson**.

This is a **conceptual discussion** to make developers aware of these tools. The code examples shown (assert macro, audit pseudocode) are **illustrative/hypothetical**, not actual implementations in the codebase.

**From transcript** (lines 260-265):
> "This is the end of the lesson and it is the last lesson in the course on reliability availability and maintainability. As I said at the start, I find this a fun topic and I hope you found it useful. Thanks for watching."

---

## Practical Takeaways

### **For Asserts:**
1. **Great for development** - everyone agrees
2. **Production decision** - requires careful thought:
   - Safety-critical? → Keep them
   - Complex field conditions? → Might want better error handling
   - Performance-critical? → Consider overhead
3. **Don't rely on asserts** as primary error handling in production
4. **Be aware** of field vs. lab environment differences

### **For Audits:**
1. **Proactive health monitoring** - catch problems before catastrophic failure
2. **Long-running systems** especially benefit
3. **File systems** are common problem area
4. **Correcting audits** can maintain system health automatically
5. **Be conservative** with recovery actions - avoid trigger-happy resets

---

## Summary

Asserts and audits are **complementary tools** for system reliability:

- **Asserts**: Fast, local, programming error detection
- **Audits**: Slow, global, system health monitoring

Both contribute to **maintainability** by detecting problems early. The key is using them appropriately:
- Asserts for development and (carefully) in production
- Audits for proactive monitoring of long-running systems

**Final note from Gene**: "I find this a fun topic and I hope you found it useful."

```c
assert(pointer_var != NULL);
```

```c
// If fails: Log error (file name + line number), continue execution
```

```c
// If fails: Generate function return with error code
// Let caller deal with failure
```

```c
// If fails: Declare fatal fault (comparable to Linux abort)
```

```c
#define ASSERT_FATAL(condition) \
    do { \
        if (!(condition)) { \
            fault_detected(FAULT_TYPE_ASSERT, \
                          (FILE_ID << 16) | __LINE__); \
        } \
    } while(0)
```

```c
void process_data(void* data, uint32_t size) {
    ASSERT_FATAL(data != NULL);
    ASSERT_FATAL(size > 0);
    ASSERT_FATAL(size <= MAX_BUFFER_SIZE);
    
    // Function implementation...
}
```

```c
// Audit detects serious problem
if (critical_problem_detected) {
    fault_detected(FAULT_TYPE_AUDIT, audit_error_code);
    // Never returns - fault module handles recovery
}
```

