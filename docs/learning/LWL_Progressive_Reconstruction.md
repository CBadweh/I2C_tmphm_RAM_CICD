# LWL (Lightweight Logging) Progressive Reconstruction Guide
## Day 3 Afternoon Session - Black Box Flight Recorder for Embedded Systems

**Prerequisites:** Completed TMPHM module, understanding of circular buffers  
**Estimated Time:** 3-4 hours  
**Complexity:** Medium (circular buffer, macros, variadic functions)

---

## **Overview**

**What LWL Does:**
Lightweight Logging (LWL) is a "black box flight recorder" that captures system activity in a compact circular buffer with minimal runtime overhead. Unlike `printf()`, it stores raw data (IDs + parameters) without string formatting, allowing post-mortem analysis after crashes.

**Key Concepts:**
- Circular buffer (wrap-around storage)
- Compile-time macro magic (`__COUNTER__`)
- Variadic functions (`...` parameters)
- Critical sections (interrupt safety)
- Binary data storage (decode offline)

**Why It's Critical:**
When your system crashes in the field, LWL tells you **what happened in the last 5 seconds** without slowing down normal operation or using printf strings that eat RAM.

---

## **Essential Patterns Used in LWL Module**

This module uses the following code patterns. Review these before starting:

### **Pattern 9: Module Initialization Sequence**
```c
// Phase 1: Get default configuration (app_main.c)
struct module_cfg cfg;
module_get_def_cfg(INSTANCE_1, &cfg);

// Phase 2: Initialize (app_main.c)
module_init(INSTANCE_1, &cfg);

// Phase 3: Start (app_main.c)  
module_start(INSTANCE_1);

// In super loop: Run (app_main.c)
while (1) {
    module_run(INSTANCE_1);
}
```

### **Pattern 17: Boolean Flags for State Tracking**
```c
struct module_state {
    bool initialized;   // Has init() been called?
    bool reserved;      // Resource in use?
    bool have_data;     // Data available?
    bool error_occurred; // Error condition?
};

// Using flags
if (!st.initialized) {
    return MOD_ERR_STATE;  // Not ready
}

if (st.reserved) {
    return MOD_ERR_RESOURCE;  // Already in use
}
```

### **Pattern 12: Optional Parameters (NULL checks)**
```c
int32_t get_measurement(instance_id, meas, age_ms)
{
    // Required parameter - MUST check
    if (meas == NULL)
        return MOD_ERR_ARG;
    
    // Return required data
    *meas = st.last_meas;
    
    // Optional parameter - check before using
    if (age_ms != NULL) {
        *age_ms = tmr_get_ms() - st.last_meas_ms;
    }
    
    return 0;
}
```

### **Pattern 11: Fail-First Error Handling**
```c
int32_t process_data(data, len)
{
    // Check errors first (fail-fast)
    if (data == NULL)
        return MOD_ERR_ARG;
    
    if (len == 0)
        return MOD_ERR_ARG;
    
    if (!st.initialized)
        return MOD_ERR_STATE;
    
    // All checks passed - do the work
    do_actual_work(data, len);
    return 0;
}
```

### **Pattern 13: Configuration Structure Usage**
```c
// Configuration structure (what user sets)
struct module_cfg {
    uint32_t setting1;  // User-configurable values
    uint32_t setting2;
    uint8_t mode;
};

// State structure (runtime data)
struct module_state {
    struct module_cfg cfg;  // Copy of config (for later use)
    
    // Runtime state variables
    enum states state;
    uint32_t timestamp;
    uint8_t buffer[10];
    // ... more runtime data
};

// In init: Save config for later
int32_t module_init(instance_id, cfg)
{
    st.cfg = *cfg;  // Save it - you'll need these values later!
}
```

**When you see these patterns in the guide, you'll know the structure to use!**

---

## **Phase 0: Big Picture (45 min)**

### **Study Activities:**

1. **Read entire lwl.c** (20 min)
   - Focus on `lwl_rec()` - the recording function
   - Notice the circular buffer logic
   - See how macros simplify usage

2. **Understand the data flow** (15 min)
   ```
   Your Code: LWL("state=%d", 3, LWL_1(state))
                    ↓
   Macro expands to: lwl_rec(ID, 3, state_as_bytes)
                    ↓
   lwl_rec() writes: [ID][byte1][byte2][byte3] to circular buffer
                    ↓
   Later: Python script reads buffer and formats: "state=5"
   ```

3. **Draw circular buffer diagram** (10 min)
   ```
   Buffer[1008 bytes]:
   [0] [1] [2] ... [1006] [1007] [0] [1] ... (wraps around)
    ↑                              ↑
   Old data                    put_idx (next write position)
   ```

### **Deliverable:**
Can you explain: "Why use LWL instead of printf?" and "How does circular buffer prevent overflow?"

---

## **Phase 1: Understanding Before Building (30 min)**

### **Core Concept: The Flight Recorder**

**Traditional Logging (printf):**
```c
printf("I2C state=%d addr=0x%02x\n", state, addr);
```
- ❌ Slow (string formatting)
- ❌ Uses RAM (format strings)
- ❌ Can't log from interrupts
- ❌ Stops working if system crashes

**Lightweight Logging:**
```c
LWL("I2C state addr", 3, LWL_1(state), LWL_2(addr));
```
- ✅ Fast (just copy bytes)
- ✅ Minimal RAM (no strings)
- ✅ Works from interrupts
- ✅ **Survives crashes** (buffer stays in RAM)

**The Trade-Off:**
- Need offline tool (Python) to decode
- Less human-readable in raw form
- **But captures what printf can't!**

---

## **Phase 2: Function-by-Function Implementation**

### **Function 1: lwl_start() - Why Essential?**

**Purpose:**
Initialize the LWL module and register console commands.

**Why It's Essential:**
- Sets up the circular buffer structure
- Registers console commands (`lwl dump`, `lwl enable`)
- One-time setup before use

**Using Pattern 9 (Module Initialization Sequence):**

```c
int32_t lwl_start(void)
{
    // Question 1: What should you initialize in lwl_data?
    // Hint: Look at struct lwl_data - what needs setup?
    // Think: Pattern 9 says initialize state structure
    lwl_data.magic = ???;           // Magic number for validation
    lwl_data.put_idx = ???;         // Where to write next?
    lwl_data.buf_size = ???;        // Size of buffer
    
    // Question 2: How do you register console commands?
    // Hint: Same pattern as I2C and TMPHM modules
    // Think: Pattern 9 says register with system
    int32_t rc = ???(&cmd_info);
    if (rc < 0)
        return rc;
    
    return 0;
}
```

**Guiding Questions:**

1. **Why initialize put_idx to 0?**
   - It's the write position in circular buffer
   - Start at beginning

2. **What is magic number for?**
   - Validates data structure when reading from flash
   - Python script checks this first

3. **Why register console commands?**
   - `lwl dump` to view buffer
   - `lwl enable` to turn on/off
   - Debugging tools

**Critical Thinking:**
- "What happens if you forget to initialize put_idx?" (Random position!)
- "Why store buf_size in the structure?" (Python needs to know buffer boundaries)

**Test Your Understanding:**
*"If buffer is 1008 bytes and put_idx is 1007, where does next write go?"* (Answer: 0, it wraps!)

**Checkpoint:**
- [ ] Code compiles
- [ ] Can explain what each field does
- [ ] Ready for: `lwl_enable()`

---

### **Function 2: lwl_enable() - Why Essential?**

**Purpose:**
Turn logging on/off at runtime.

**Why It's Essential:**
- Control overhead (disable when not needed)
- Enable only during critical sections
- Start disabled, enable after init complete

**Using Pattern 17 (Boolean Flags for State Tracking):**

```c
void lwl_enable(bool on)
{
    // Question: What variable controls whether logging happens?
    // Hint: Look at lwl.h - there's an extern bool
    // Think: Pattern 17 shows boolean flags for on/off state
    ??? = on;
}
```

**Guiding Questions:**

1. **Why make it enable/disable?**
   - Overhead is tiny but not zero
   - During boot, may not want to log
   - In production, might enable only on error

2. **Why is `_lwl_active` global?**
   - LWL() macro checks it (needs fast access)
   - If it were local, macro couldn't see it

**Critical Thinking:**
- "What if you log before calling lwl_start()?" (Uninitialized buffer!)
- "Can you enable from interrupt?" (Yes! Just setting bool)

**Test Your Understanding:**
*"If _lwl_active is false, does LWL() macro do anything?"* (No - early exit)

**Checkpoint:**
- [ ] Understand on/off mechanism
- [ ] Know why it's a global variable
- [ ] Ready for: `lwl_rec()` (the complex one!)

---

### **Function 3: lwl_rec() - THE HEART (Most Complex)**

**Purpose:**
Record a log entry in the circular buffer.

**Why It's Essential:**
- **This is THE flight recorder function**
- Writes ID + data bytes to buffer
- Handles wrap-around (circular)
- Must be interrupt-safe (critical section)

**IMPORTANT:** This function is COMPLEX. We'll build it in pieces!

---

#### **Sub-Part A: Understanding Variadic Functions (15 min)**

**What are "..." parameters?**

```c
void lwl_rec(uint8_t id, int32_t num_arg_bytes, ...)
//                                               ^^^
//                                          Variable number of args!
```

**How to access them:**
```c
va_list ap;                    // Declare argument list
va_start(ap, num_arg_bytes);   // Initialize (last named param)

uint8_t byte1 = va_arg(ap, unsigned);  // Get next arg
uint8_t byte2 = va_arg(ap, unsigned);  // Get next arg

va_end(ap);                    // Clean up
```

**Example:**
```c
lwl_rec(20, 3, 0x44, 0x2C, 0x06);
//      ^   ^   ^^^^^^^^^^^^^^^^
//      ID  3   Three bytes to store
```

Inside function:
```c
byte1 = va_arg(ap, unsigned);  // Gets 0x44
byte2 = va_arg(ap, unsigned);  // Gets 0x2C
byte3 = va_arg(ap, unsigned);  // Gets 0x06
```

---

#### **Sub-Part B: Understanding Circular Buffer (15 min)**

**Concept:**
```
Buffer[1008]:
[0][1][2][3][4][5]...[1006][1007]
 ↑                               ↑
Start                           End
                                 
When put_idx reaches 1007:
Next write goes to [0] (wraps around)
Old data gets overwritten (FIFO)
```

**The Formula:**
```c
put_idx = put_idx % BUF_SIZE;  // Modulo wraps it!
// Example:
// 1007 % 1008 = 1007
// 1008 % 1008 = 0 (wrapped!)
// 1009 % 1008 = 1 (wrapped!)
```

---

#### **Sub-Part C: Build lwl_rec() Step-by-Step**

**Using Pattern 11 (Fail-First Error Handling) + Pattern 17 (Boolean Flags):**

```c
void lwl_rec(uint8_t id, int32_t num_arg_bytes, ...)
{
    CRIT_STATE_VAR;  // For critical section (interrupt disable)
    va_list ap;
    uint32_t put_idx;

    // Step 1: Initialize variadic argument access
    // Question: What function starts variadic arg processing?
    // Hint: va_start() - what's the second parameter?
    ???(ap, ???);

    // Step 2: Enter critical section (disable interrupts)
    // Why? Multiple contexts might call lwl_rec() simultaneously
    CRIT_BEGIN_NEST();
    
    // Step 3: Get current write position
    // Question: How do you get position in circular buffer?
    // Hint: Use modulo to wrap around
    put_idx = lwl_data.put_idx % ???;  // What's the buffer size?
    
    // Step 4: Update put_idx for NEXT write
    // Question: Where should next write go?
    // Hint: Current position + 1 (for ID) + num_arg_bytes
    // Then wrap with modulo
    lwl_data.put_idx = (put_idx + ??? + ???) % ???;
    
    // Step 5: Write the ID byte
    lwl_data.buf[put_idx] = ???;  // What do you write?
    
    // Step 6: Write argument bytes (loop)
    // Question: How many times do you loop?
    while (num_arg_bytes-- > 0) {
        // Increment position (with wrap)
        put_idx = (put_idx + 1) % ???;
        
        // Question: How do you get next argument byte?
        // Hint: va_arg(ap, type) - what type?
        lwl_data.buf[put_idx] = (uint8_t)va_arg(ap, ???);
    }
    
    // Step 7: Exit critical section
    CRIT_END_NEST();
}
```

**Guiding Questions:**

1. **Why critical section?**
   - Main code calls lwl_rec()
   - Interrupt calls lwl_rec()
   - **Both modify put_idx** → race condition!
   - Critical section = only one at a time

2. **Why update put_idx BEFORE writing data?**
   - If crash during write, next entry won't overwrite partial data
   - Defensive programming

3. **Why modulo everywhere?**
   - Automatic wrap-around
   - `1008 % 1008 = 0` (magic!)
   - No need for if-statements

**Critical Thinking:**

- "What if num_arg_bytes is 0?" (Just write ID, no data)
- "What happens to old data when buffer fills?" (Overwritten - FIFO)
- "Why cast to unsigned in va_arg?" (C varargs promote to int)

**Common Mistake:**

🎯 **PITFALL:** Forgetting `va_start()` before accessing arguments!
- Results in garbage data
- Hard to debug (looks like valid but wrong numbers)

**Checkpoint:**
- [ ] Understand variadic functions
- [ ] Understand circular buffer wrap
- [ ] Understand critical sections
- [ ] Code compiles
- [ ] Ready for: Using LWL in other modules

---

### **Function 4: lwl_dump() - Why Essential?**

**Purpose:**
Print entire buffer contents to console (for debugging).

**Why It's Essential:**
- See what was logged
- Debug state transitions
- Verify LWL is working

**This function is COMPLEX (console commands). For learning, we'll SKIP implementing it.**

**Instead, use the existing implementation!**

**Why:**
- It's just debug infrastructure
- The core concept is `lwl_rec()`, not `lwl_dump()`
- Production uses Python script, not console dump anyway

---

## **Phase 3: Integration - Using LWL in Your Modules**

### **This is WHERE THE VALUE IS!**

**Goal:** Add LWL logging to I2C and TMPHM modules you already built.

---

### **Part A: Adding LWL to I2C Driver (1 hour)**

**Study First (15 min):**
- Look at reference `i2c.c` in `/tmphm` folder
- Find LWL calls (search for "LWL(")
- See where they're placed (state transitions, errors)

**Build (30 min):**

**Using Pattern 9 (Module Initialization) + Pattern 17 (Boolean Flags):**

**Step 1: Add LWL header and IDs**

```c
// At top of i2c.c, after other includes:
#include "lwl.h"

// After defines:
// Question: What ID range should I2C module use?
// Hint: TMPHM uses 30-39, LWL uses 1-4, pick a range
// Think: Pattern 9 says each module needs unique identifiers
#define LWL_BASE_ID ???  // Maybe 20?
#define LWL_NUM 10       // 10 log IDs available
```

**Step 2: Add logging at key points**

**In interrupt handler, when START condition sent:**
```c
case STATE_MSTR_WR_GEN_START:
    if (sr1 & LL_I2C_SR1_SB) {
        // Question: What should you log here?
        // Hint: State change, destination address, data length
        LWL("WR_START", 3, LWL_1(st->dest_addr), 
                          LWL_2(st->msg_len), 
                          LWL_3(state));
        
        st->i2c_reg_base->DR = st->dest_addr << 1;
        st->state = STATE_MSTR_WR_SENDING_ADDR;
    }
    break;
```

**On errors:**
```c
if (sr1 & I2C_SR1_AF) {
    LWL("ACK_FAIL", 2, LWL_1(st->dest_addr), LWL_2(error_code));
    i2c_error = I2C_ERR_ACK_FAIL;
}
```

**Strategic Logging Points (Add 5-6 of these):**
- START condition generated
- Address ACKed/NACKed
- Data byte sent/received
- STOP condition
- Any error

**Test (15 min):**
- Build and flash
- Run I2C operation
- Use `lwl dump` command
- See your log entries!

---

### **Part B: Adding LWL to TMPHM Module (45 min)**

**Study (10 min):**
Look at reference `tmphm.c` - where are LWL calls?

**Build (25 min):**

**Using Pattern 9 (Module Initialization) + Pattern 17 (Boolean Flags):**

**Step 1: Add header and IDs**
```c
#include "lwl.h"

// Question: What ID range for TMPHM?
// Hint: I2C uses 20-29, so maybe 30-39?
// Think: Pattern 9 says each module needs unique range
#define LWL_BASE_ID ???
#define LWL_NUM 10
```

**Step 2: Log state transitions**

```c
case STATE_RESERVE_I2C:
    LWL("TMPHM_RESV", 2, LWL_1(st.cfg.i2c_instance_id), 
                        LWL_2(st.state));
    rc = i2c_reserve(st.cfg.i2c_instance_id);
    // ...
```

**Step 3: Log measurements**

```c
// After successful CRC validation
LWL("TMPHM_MEAS", 3, LWL_1(temp), LWL_2(hum), 
                    LWL_3(st.last_meas_ms));
```

**Strategic Logging Points:**
- State transitions (IDLE→RESERVE→WRITE→WAIT→READ)
- I2C reserve success/failure
- Write/read start
- CRC pass/fail
- Final measurement

**Test (10 min):**
- Clear buffer: `lwl clear` (if you implement it)
- Wait 5 seconds
- Dump buffer: `lwl dump`
- See 5 measurement cycles captured!

---

## **Phase 4: Understanding the Macro Magic (30 min)**

### **How LWL() Macro Works:**

**What You Write:**
```c
LWL("temp=%d", 3, LWL_1(temp))
```

**What Compiler Sees:**
```c
// Step 1: __COUNTER__ expands to unique number per file
LWL_CNT(0, "temp=%d", 3, LWL_1(temp))

// Step 2: LWL_1() expands bytes
LWL_CNT(0, "temp=%d", 3, (uint32_t)(temp))

// Step 3: LWL_CNT() expands to function call
if (_lwl_active)
    lwl_rec(LWL_BASE_ID + 0, 3, (uint32_t)(temp));
```

**The Magic:**
- `__COUNTER__` auto-increments: 0, 1, 2, 3... per file
- `LWL_BASE_ID + counter` creates unique IDs
- Compile-time string ("temp=%d") used by Python, not stored!

---

### **Understanding LWL_1, LWL_2, LWL_3, LWL_4:**

**These split multi-byte values:**

```c
// LWL_1: Store 1 byte (least significant)
LWL_1(0x44) → 0x44

// LWL_2: Store 2 bytes (MSB first, then LSB)
LWL_2(0x1234) → 0x12, 0x34

// LWL_4: Store 4 bytes (full 32-bit value)
LWL_4(0x12345678) → 0x12, 0x34, 0x56, 0x78
```

**Match num_arg_bytes to macro:**
```c
LWL("state", 1, LWL_1(state));      // 1 byte
LWL("addr", 2, LWL_2(address));     // 2 bytes
LWL("time", 4, LWL_4(timestamp));   // 4 bytes

// Multiple values:
LWL("state addr", 3, LWL_1(state), LWL_2(addr));  // 1+2 = 3 bytes
```

**Critical:**
🎯 **PITFALL:** Mismatch between num_arg_bytes and actual bytes!
```c
LWL("bad", 3, LWL_4(value));  // ❌ Says 3, provides 4!
LWL("good", 4, LWL_4(value)); // ✅ Match!
```

---

## **Phase 5: Integration Testing (1 hour)**

### **Enable LWL in app_main.c:**

```c
// In init section (after tmr_init):
result = lwl_start();
if (result < 0) {
    log_error("lwl_start error %d\n", result);
}

// After all modules started:
lwl_enable(true);  // Turn on recording
```

---

### **End-to-End Test:**

**Test 1: Basic Recording**
```
1. Build and flash
2. Connect serial console
3. Type: lwl enable 1
4. Type: lwl status (should show on=1)
5. Wait 5 seconds (system logs activity)
6. Type: lwl dump
7. Expected: See log entries from I2C and TMPHM!
```

**Test 2: Verify State Transitions**
```
1. lwl clear (if implemented, else reset board)
2. Press USER button (triggers I2C test)
3. lwl dump
4. Expected: See I2C state transitions in order
```

**Test 3: Crash Survival**
```
1. lwl clear
2. Wait 5 seconds
3. Force crash (if fault module exists)
4. After reboot: lwl dump
5. Expected: See activity before crash!
```

---

### **Troubleshooting Guide:**

| Problem | Cause | Fix |
|---------|-------|-----|
| lwl dump shows nothing | Not enabled | Call `lwl_enable(true)` |
| Compile error: LWL redefined | Multiple LWL_BASE_ID | Each module needs unique base |
| Wrong data in dump | num_arg_bytes mismatch | Count bytes carefully |
| Crash when logging | No lwl_start() called | Initialize before enabling |
| Buffer always empty | _lwl_active false | Check lwl_enable(true) called |

---

## **Completion Checklist**

### **Core Understanding:**
- [ ] Understand circular buffer concept
- [ ] Understand variadic functions
- [ ] Understand why LWL beats printf
- [ ] Know when to use LWL vs log_info()

### **Implementation:**
- [ ] lwl_start() implemented
- [ ] lwl_enable() implemented  
- [ ] lwl_rec() implemented (or using existing)
- [ ] Code compiles with 0 warnings

### **Integration:**
- [ ] LWL added to I2C driver (5-10 points)
- [ ] LWL added to TMPHM module (5-8 points)
- [ ] LWL enabled in app_main.c
- [ ] Can use `lwl dump` command

### **Testing:**
- [ ] lwl dump shows entries
- [ ] Can see I2C state transitions
- [ ] Can see TMPHM measurement cycles
- [ ] Understand buffer wraps correctly

---

## **Next Steps After Completion**

### **Production Enhancements:**
1. Add `lwl_clear()` command (reset buffer)
2. Add timestamp to each entry
3. Integrate with fault module (dump on crash)
4. Python decoder script (format pretty output)

### **Advanced Usage:**
1. Log from interrupt context
2. Conditional logging (error path only)
3. Performance impact measurement
4. Flash persistence (survive power loss)

---

## **Key Learning Points**

### **Why LWL is "Lightweight":**

**Traditional Logging:**
```c
printf("I2C write addr=0x%02x len=%d\n", addr, len);
```
Cost:
- 38 bytes for format string (RAM)
- String formatting (CPU cycles)
- Buffer management (complexity)

**LWL:**
```c
LWL("write", 3, LWL_1(addr), LWL_2(len));
```
Cost:
- 0 bytes RAM (string at compile-time only!)
- 3 bytes in buffer (just raw data)
- ~10 CPU instructions (trivial)

**Trade-off:** Need Python to decode, but **90% less overhead!**

---

### **When to Use What:**

| Situation | Use | Why |
|-----------|-----|-----|
| Debug message to user | `log_info()` | Human-readable |
| Error to console | `log_error()` | Immediate feedback |
| State transitions | `LWL()` | Capture flow |
| Before crash | `LWL()` | Survives reboot |
| In interrupt | `LWL()` | Fast enough |
| Performance path | `LWL()` | Minimal overhead |

---

## **Real-World War Story**

**📖 From Gene's Experience:**

**The Problem:**
Customer reported intermittent sensor failures in the field. Happened every few days, couldn't reproduce in lab.

**Without LWL:**
- No idea what happened before crash
- Spent 3 weeks trying to reproduce
- Added printf - system too slow, changed timing, couldn't reproduce
- **Frustration level: Maximum**

**With LWL:**
- Next crash in field
- Retrieved LWL buffer from RAM
- Saw sequence:
  1. I2C transaction started
  2. Sensor didn't ACK
  3. Driver returned error
  4. TMPHM retried
  5. Still failed
  6. Watchdog timeout → reset
- **Root cause found in 10 minutes:** Sensor brownout from voltage sag
- **Fix:** Added capacitor to sensor power rail

**LWL saved 3 weeks of debugging!**

---

## **Understanding Check Questions**

After completing this module, you should answer:

1. **"Why is LWL a circular buffer, not linear?"**
   - Limited RAM - can't grow forever
   - Want most recent data, not oldest
   - Automatic FIFO with wrap-around

2. **"Why store ID + bytes, not formatted strings?"**
   - Strings eat RAM (every format string stored)
   - Formatting is slow
   - Binary is compact and fast

3. **"When would you use LWL vs printf?"**
   - LWL: State machines, performance paths, interrupts, flight recorder
   - printf: User messages, debug during development

4. **"What's the purpose of critical sections?"**
   - Prevent race conditions
   - Main code and interrupts both log
   - put_idx must be atomic

5. **"How does Python decode the binary data?"**
   - Reads ID → looks up format string in source
   - Reads bytes → formats according to string
   - Example: ID=20, format="state=%d", bytes=[5] → "state=5"

---

## **Simplified Learning Version**

**For pure learning (not production), you can simplify lwl_rec():**

### **Ultra-Simple Version (No Variadic, No Critical Section):**

```c
void lwl_rec_simple(uint8_t id, uint8_t arg1, uint8_t arg2)
{
    uint32_t put_idx = lwl_data.put_idx % LWL_BUF_SIZE;
    
    lwl_data.buf[put_idx] = id;
    put_idx = (put_idx + 1) % LWL_BUF_SIZE;
    
    lwl_data.buf[put_idx] = arg1;
    put_idx = (put_idx + 1) % LWL_BUF_SIZE;
    
    lwl_data.buf[put_idx] = arg2;
    
    lwl_data.put_idx = (lwl_data.put_idx + 3) % LWL_BUF_SIZE;
}

// Usage:
lwl_rec_simple(20, state, address);  // Fixed 3 bytes
```

**When to use this:**
- Learning circular buffer concept
- Not ready for variadic functions
- Want to see core logic clearly

**When to graduate to full version:**
- Need variable number of arguments
- Need interrupt safety
- Moving to production

---

## **Build Strategy Recommendation**

### **Option A: Use Existing LWL Module (RECOMMENDED)**

**Why:**
- LWL is infrastructure (like printf)
- The VALUE is in **using it**, not implementing it
- Complex topics (variadic, critical sections) can be learned separately

**What to do:**
1. Copy lwl.c and lwl.h from reference
2. Understand the **usage** (LWL macro)
3. Focus on **integration** (adding to your modules)
4. Read source to understand circular buffer

**Time:** 2 hours (mostly integration)

---

### **Option B: Build Simplified Version (FOR DEEP LEARNING)**

**Why:**
- Learn circular buffer implementation
- Understand macro expansion
- Practice variadic functions

**What to do:**
1. Build simplified version first (no variadic)
2. Test it works
3. Upgrade to variadic version
4. Add critical sections
5. Compare to reference

**Time:** 4 hours (implementation + integration)

---

## **My Recommendation:**

**For Day 3 (TMPHM focus):** Use Option A
- LWL is a tool, not the lesson
- Focus: Adding LWL to TMPHM/I2C
- Save implementation study for later

**For Future Deep Dive:** Come back and do Option B
- Worthy of its own learning session
- Teaches advanced C concepts
- But not critical for TMPHM lesson

---

## **Completion Criteria**

**You're done when:**

1. ✅ LWL module compiles and links
2. ✅ Can enable/disable logging
3. ✅ Added 5+ LWL calls to I2C driver
4. ✅ Added 5+ LWL calls to TMPHM module
5. ✅ `lwl dump` shows captured activity
6. ✅ Can explain: "What's the difference between LWL and printf?"
7. ✅ Understand circular buffer wrap-around

---

## **Integration Example (Complete)**

**Using Pattern 9 (Module Initialization Sequence):**

### **app_main.c:**
```c
#include "lwl.h"

// In initialization (Pattern 9: Phase 3 - Start):
result = lwl_start();
if (result < 0) {
    log_error("lwl_start error %d\n", result);
}

// After all modules started:
lwl_enable(true);  // Pattern 17: Boolean flag control
printc("LWL enabled - flight recorder active!\n");
```

### **i2c.c:**
```c
#include "lwl.h"

// Pattern 9: Unique module identifier
#define LWL_BASE_ID 20
#define LWL_NUM 10

// In interrupt handler:
case STATE_MSTR_WR_SENDING_ADDR:
    if (sr1 & LL_I2C_SR1_ADDR) {
        LWL("I2C_ADDR_ACK", 2, LWL_1(st->dest_addr), LWL_2(st->msg_len));
        // ... rest of code
    }
```

### **tmphm.c:**
```c
#include "lwl.h"

// Pattern 9: Unique module identifier (different from I2C)
#define LWL_BASE_ID 30
#define LWL_NUM 10

// In tmphm_run():
case STATE_READ_MEAS_VALUE:
    if (rc == 0 && crc_valid) {
        LWL("TMPHM_OK", 3, LWL_1(temp), LWL_2(hum), LWL_3(state));
        // ... store measurement
    }
```

**Result:** Complete flight recorder across all modules! ✈️

---

## **Time Estimate Breakdown**

| Phase | Activity | Time |
|-------|----------|------|
| 0 | Big picture study | 45 min |
| 1 | Understand concepts | 30 min |
| 2 | lwl_start() and lwl_enable() | 30 min |
| 3 | lwl_rec() study (or skip) | 45 min |
| 4 | Add LWL to I2C | 1 hour |
| 5 | Add LWL to TMPHM | 45 min |
| 6 | Integration testing | 30 min |
| **Total** | **Option A** | **~4 hours** |
| **Total** | **Option B** | **~5-6 hours** |

---

## **Success Story Check**

**After completing, you should be able to:**

1. ✅ Explain LWL to a colleague in 2 minutes
2. ✅ Add LWL to a new module in 15 minutes
3. ✅ Debug using LWL buffer after system crash
4. ✅ Choose appropriate log points (state changes, errors)
5. ✅ Understand the performance benefit vs printf
6. ✅ Read lwl_dump output and trace execution
7. ✅ Appreciate why this is called a "flight recorder"

**If you can do all 7 → You've mastered LWL!** 🎯

---

**Day 3 is complete when TMPHM samples every 1 second AND LWL captures the activity in its flight recorder buffer!** 🚀

