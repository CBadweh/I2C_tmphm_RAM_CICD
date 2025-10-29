# Fault Handling and Watchdog Protection - Progressive Reconstruction Guide

## Day 4: Fault Handling and Watchdog Protection

**Target Time:** 4 hours (2.5 hours morning + 1.5 hours afternoon)  
**Difficulty:** Advanced  
**Prerequisites:** I2C driver, TMPHM module, LWL logging (Days 1-3 completed)

---

## Overview

On Day 4, you'll build two critical reliability modules:

1. **Fault Module** - Handles crashes gracefully, collects diagnostic data, and performs safe recovery
2. **Watchdog Module** - Prevents system hangs and ensures critical work completes on time

**Why Essential:**
- Embedded systems crash in the field (environmental issues, bugs, hardware faults)
- You can't debug a deployed system with a debugger
- Watchdogs are the "last line of defense" against hangs and stuck loops
- Diagnostic data collection is your only clue about what went wrong

**Key Concepts:**
- Panic mode operation (minimal, safe state)
- Exception handling (ARM Cortex-M fault exceptions)
- Software vs hardware watchdogs
- Non-init RAM (surviving resets)
- MPU-based stack protection

---

## Essential Patterns Used in These Modules

Review these patterns before starting:

### **Pattern 1: State Machine Skeleton** (from Progressive_Reconstruction.md)
You'll use this for the watchdogs' periodic check mechanism.

### **Pattern 3: Periodic Timer with Callback** (from Progressive_Reconstruction.md)
Watchdog module uses a timer to periodically check software watchdogs.

### **Pattern 4: Error Handling with Resource Cleanup** (from Progressive_Reconstruction.md)
Panic mode must be extremely careful about resource management.

### **Pattern 9: Module Initialization Sequence** (from Progressive_Reconstruction.md)
Both modules follow standard init/start pattern.

### **NEW Pattern: No-Init Variables** (Watchdog-specific)
Variables that survive resets using special linker section:

```c
// In linker script: Add section after .bss
.no.init.vars (NOLOAD) :
{
  . = ALIGN(4);
  *(.no.init.vars)
  . = ALIGN(4);
} >RAM

// In code:
struct wdg_no_init_vars {
    uint32_t magic;
    uint32_t consec_failed_init_ctr;
    uint32_t check;
} no_init_vars __attribute__((section (".no.init.vars")));
```

**Key:** Variables in this section are NOT initialized by startup code, preserving values across resets!

---

## Phase 0: Big Picture First (1 hour) - CRITICAL!

### Study Activities

**1. Read entire fault.c (30 min)**
- Don't try to memorize
- Focus on understanding the panic mode flow
- Note how diagnostic data is collected
- Understand the relationship between fault types and handlers

**2. Read entire wdg.c (20 min)**
- Understand the software watchdog array concept
- Note the timer callback checking all watchdogs
- See how hardware watchdog feeds the software
- Note the no-init variables for initialization tracking

**3. Draw Architecture Diagrams (10 min)**

**Fault Module Flow:**
```
[Fault Detected] → [Enter Panic Mode]
                           ↓
                    [Disable Interrupts]
                           ↓
                    [Reset Stack Pointer]
                           ↓
                    [Disable MPU]
                           ↓
                    [Collect Diagnostic Data]
                           ↓
                    [Write to Flash]
                           ↓
                    [Write to Console]
                           ↓
                    [Reset MCU]
```

**Watchdog Hierarchy:**
```
[Application Software]
        ↓ feeds
[Software Watchdogs] (multiple, one per critical task)
        ↓ if all OK
[Hardware Watchdog Timer]
        ↓ if timeout
[Fault Module] → [Panic Mode]
```

### Deliverable
You can explain:
- "What happens when a fault occurs? (panic mode → collect data → reset)"
- "How do software and hardware watchdogs work together? (software checks work, hardware checks software)"
- "Why do we need no-init variables? (track initialization failures across resets)"

---

## Phase 1: Watchdog Module (1.5 hours)

**Why start with watchdog?** It's simpler, and the fault module depends on it.

---

### Function 1: wdg_init() - Why Essential?

**Purpose:** Initialize the watchdog module state

**Why Essential:**
- Clears state structure (standard pattern)
- No dependencies on other modules at this stage

**What You Need to Fill In:**

```c
int32_t wdg_init(struct wdg_cfg* cfg)
{
    // Question: What's the first thing to do with state?
    // Hint: Look at Pattern 9 - Module Initialization
    // Think: Clean slate for all state
    
    memset(???, 0, sizeof(???));
    
    return 0;
}
```

**Guiding Questions:**

1. **Which structure needs clearing?** `state` or `cfg`?
   - Hint: `state` holds module state, `cfg` is just config passed in
   - Why: Need to clear runtime state, not configuration

**Build Checkpoint:**
- [ ] Does it compile?
- [ ] Can you explain what memset does here?
- [ ] When ready: Move to Function 2

---

### Function 2: wdg_start() - Why Essential?

**Purpose:** Register console commands and start the periodic watchdog checker timer

**Why Essential:**
- Without the timer, watchdogs are never checked
- Console commands allow testing and debugging
- Timer callback is the heart of watchdog checking

**What You Need to Fill In:**

```c
int32_t wdg_start(void)
{
    int32_t rc;
    
    // Question 1: Register console commands
    // Hint: Look at previous modules (i2c, tmphm) for pattern
    // Think: cmd_register() with cmd_info
    rc = ???(&cmd_info);
    if (rc < 0) {
        log_error("wdg_start: cmd error %d\n", rc);
        goto exit;
    }
    
    // Question 2: Get a periodic timer
    // Hint: Pattern 3 - Periodic Timer with Callback
    // Think: Timer fires every CONFIG_WDG_RUN_CHECK_MS, calls wdg_tmr_cb
    rc = tmr_inst_get_cb(???, wdg_tmr_cb, 0, TMR_CNTX_BASE_LEVEL);
    if (rc < 0) {
        log_error("wdg_start: tmr error %d\n", rc);
        goto exit;
    }

exit:
    return rc;
}
```

**Guiding Questions:**

1. **What function registers commands?** Look at cmd.h for the API
   - Why: All modules follow this pattern for console interface

2. **Which function gets a periodic timer?** Look at Pattern 3
   - Why: Watchdogs must be checked periodically (every 10ms typically)

3. **What is CONFIG_WDG_RUN_CHECK_MS?** Hint: Check config.h
   - Why: Determines how often we check if watchdogs have timed out

**Build Checkpoint:**
- [ ] Does it compile?
- [ ] Can you explain why we need the timer?
- [ ] When ready: Move to Function 3

---

### Function 3: wdg_register() - Why Essential?

**Purpose:** Register a client's software watchdog with a timeout period

**Why Essential:**
- Allows any module to request watchdog protection
- Simple runtime registration (no hard dependencies)
- Sets initial feed time to "now" (so client has full timeout period)

**What You Need to Fill In:**

```c
int32_t wdg_register(uint32_t wdg_id, uint32_t period_ms)
{
    struct soft_wdg* soft_wdg;
    
    // Question 1: Validate watchdog ID
    // Hint: Can't exceed CONFIG_WDG_NUM_WDGS
    // Think: Array bounds check
    if (wdg_id >= ???)
        return MOD_ERR_ARG;
    
    // Question 2: Get pointer to this watchdog's state
    // Hint: Array access using wdg_id
    soft_wdg = &state.soft_wdgs[???];
    
    // Question 3: Initialize the watchdog
    // Hint: Need to set two fields: period and last_feed_time
    // Think: period is the timeout, last_feed_time starts at "now"
    soft_wdg->last_feed_time_ms = ???;
    soft_wdg->period_ms = ???;
    
    return 0;
}
```

**Guiding Questions:**

1. **How do you get the current time in milliseconds?** Look at tmr.h
   - Why: Need to record when registration happened (now = feed time)

2. **What do the two fields mean?**
   - `period_ms`: How long client has to feed before timeout
   - `last_feed_time_ms`: When was it last fed

3. **Why start last_feed_time at "now"?**
   - Hint: Client gets full timeout period from registration
   - Why: Fair to client - not penalized for being slow to register

**Build Checkpoint:**
- [ ] Does it compile?
- [ ] Can you explain what happens when TMPHM calls `wdg_register(WDG_TMPHM, 5000)`?
- [ ] When ready: Move to Function 4

---

### Function 4: wdg_feed() - Why Essential?

**Purpose:** Client feeds its watchdog to reset the timeout

**Why Essential:**
- Called by clients when critical work completes successfully
- Only successful operations feed the watchdog
- Prevents timeout if work completes in time

**What You Need to Fill In:**

```c
int32_t wdg_feed(uint32_t wdg_id)
{
    // Question 1: Validate watchdog ID (same as register)
    // Think: Can't feed non-existent watchdog
    if (wdg_id >= ???)
        return MOD_ERR_ARG;
    
    // Question 2: Update last feed time
    // Hint: Record that this watchdog was just fed
    // Think: Same function as in register for getting current time
    state.soft_wdgs[wdg_id].last_feed_time_ms = ???;
    
    return 0;
}
```

**Guiding Questions:**

1. **What does "feeding" a watchdog mean?** 
   - Reset the timer by updating last_feed_time

2. **Why do clients call this?**
   - To prove critical work completed within timeout

3. **When does a module feed its watchdog?**
   - After successful completion of critical work
   - NOT on every loop iteration (only on success!)

**🎯 PITFALL:** Feeding on every loop iteration defeats the purpose!
- Bad: Feed every loop even if work didn't complete
- Good: Feed only when work actually succeeds

**Build Checkpoint:**
- [ ] Does it compile?
- [ ] Can you explain the difference between feeding on every loop vs on success?
- [ ] When ready: Move to Function 5 (the timer callback - the heart!)

---

### Function 5: wdg_tmr_cb() - THE HEART - Why Essential?

**Purpose:** Timer callback that checks all software watchdogs for timeout

**Why Essential:**
- This is where timeout detection happens
- Checks if any watchdog needs feeding
- Feeds hardware watchdog if all software watchdogs OK

**What You Need to Fill In:**

```c
static enum tmr_cb_action wdg_tmr_cb(int32_t tmr_id, uint32_t user_data)
{
    uint32_t idx;
    bool wdg_triggered = false;
    uint32_t now_ms;
    
    // Question 1: Get current time (for comparison)
    // Hint: Same function used before
    now_ms = ???;
    
    // Question 2: Check each software watchdog
    // Hint: Loop through state.soft_wdgs array
    // Think: For each watchdog, check if (now - last_feed) > period
    for (idx = 0; idx < CONFIG_WDG_NUM_WDGS; idx++) {
        // Question 2a: Calculate how long since last feed
        // Think: now_ms - last_feed_time
        if (now_ms - state.soft_wdgs[idx].last_feed_time_ms > 
            state.soft_wdgs[idx].period_ms) {
            // Question 2b: Watchdog timed out - notify
            // Hint: Call the registered callback
            // Think: state.triggered_cb is the function pointer
            wdg_triggered = true;
            if (state.triggered_cb != NULL) {
                state.triggered_cb(???);
            }
            break;  // Only trigger once per check cycle
        }
    }
    
    // Question 3: Feed hardware watchdog if no software watchdogs triggered
    // Hint: Only if all software watchdogs are OK
    // Think: If no timeout, hardware is being fed properly
    if (???) {
        ???();
    }
    
    return TMR_CB_RESTART;  // Keep firing periodically
}
```

**Guiding Questions:**

1. **Why break after triggering once?**
   - Hint: Only need to know "something went wrong"
   - Why: Fault handler will be called, system will reset

2. **Why feed hardware watchdog only if no software watchdogs triggered?**
   - Hint: Software watchdogs failed, don't reset yet
   - Why: Want fault handler to run first, collect data, then reset

3. **What does the callback do?**
   - Hint: Goes to fault module
   - Why: Fault module handles the panic and system reset

4. **Why TMR_CB_RESTART?**
   - Hint: Pattern 3 - keep timer running indefinitely
   - Why: Must keep checking watchdogs continuously

**🎯 PITFALL:** Forgetting to break after triggering
- Result: Multiple callbacks fired, confusing diagnostics
- Fix: Break immediately after triggering

**Build Checkpoint:**
- [ ] Does it compile?
- [ ] Can you trace through: "What if TMPHM doesn't feed for 5 seconds?"
- [ ] When ready: Move to Function 6 (hardware watchdog)

---

### Function 6: wdg_start_hdw_wdg() - Why Essential?

**Purpose:** Start the hardware watchdog timer (independent of CPU)

**Why Essential:**
- Hardware watchdog runs even if CPU is stuck
- Protects against software watchdog module bugs
- Cannot be disabled by software bugs

**⚠️ WARNING:** Hardware watchdog configuration is complex. This section shows simplified version.

**What You Need to Fill In:**

```c
int32_t wdg_start_hdw_wdg(uint32_t timeout_ms)
{
    int32_t ctr;
    
    // Hardware-specific macros (given in reference)
    #define SANITY_CTR_LIMIT 1000000
    #define LSI_FREQ_HZ 32000
    #define WDG_PRESCALE 64
    #define WDG_PRESCALE_SETTING LL_IWDG_PRESCALER_64
    #define WDG_CLK_FREQ_HZ (LSI_FREQ_HZ/WDG_PRESCALE)
    #define WDG_MAX_RL 0xfff
    #define MS_PER_SEC 1000
    #define WDG_MS_TO_RL(ms) \
        (((ms) * WDG_CLK_FREQ_HZ + MS_PER_SEC/2)/MS_PER_SEC - 1)
    
    // Question 1: Convert timeout to hardware reload value
    // Hint: Hardware counts differently than milliseconds
    // Think: Use the macro to convert
    ctr = ???(timeout_ms);
    if (ctr < 0)
        ctr = 0;
    else if (ctr > WDG_MAX_RL)
        return MOD_ERR_ARG;
    
    // Question 2: Enable hardware watchdog
    // Hint: Look at LL_IWDG functions
    LL_IWDG_Enable(???);
    LL_IWDG_EnableWriteAccess(???);
    LL_IWDG_SetPrescaler(???, ???);
    LL_IWDG_SetReloadCounter(???, ctr);
    
    // Wait for hardware to be ready (poll)
    for (ctr = 0; ctr < SANITY_CTR_LIMIT; ctr++) {
        if (LL_IWDG_IsReady(???))
            break;
    }
    if (ctr >= SANITY_CTR_LIMIT)
        return MOD_ERR_PERIPH;
    
    return 0;
}
```

**Guiding Questions:**

1. **What is IWDG?** 
   - Independent WatchDog (hardware peripheral)
   - Hint: Look at your STM32 reference manual

2. **Why enable write access?**
   - Hardware watchdog is write-protected for safety
   - Must unlock before configuring

3. **What does SetPrescaler do?**
   - Divides clock to get desired resolution
   - 64x prescaler gives 2ms resolution from 32kHz clock

4. **Why wait for IsReady?**
   - Hardware registers take time to update
   - Don't proceed until ready

**🎯 PITFALL:** Hardware watchdog never resets if not fed!
- Critical: Must call wdg_feed_hdw() periodically
- Otherwise: System will reset after timeout_ms

**💡 PRO TIP:** This is hardware-specific. The reference code abstracts the complexity. Focus on understanding the concept, not memorizing register names.

**Build Checkpoint:**
- [ ] Does it compile? (may need to include CONFIG_STM32_LL_IWDG_HDR)
- [ ] Can you explain why hardware watchdog is needed in addition to software?
- [ ] When ready: Move to Fault Module

---

## Phase 2: Fault Module (2 hours)

**Now we build the fault handling system that responds to watchdog triggers.**

---

### Function 1: fault_init() - Why Essential?

**Purpose:** Initialize fault module (minimal for now)

**Why Essential:**
- Standard module pattern
- Captures RCC reset reason register (diagnostics)

**What You Need to Fill In:**

```c
int32_t fault_init(struct fault_cfg* cfg)
{
    // Question: What does fault_get_rcc_csr do?
    // Hint: Captures reset reason (watchdog, power-on, etc.)
    // Think: Call it to get diagnostics about how system started
    ???();
    
    return 0;
}
```

**Guiding Questions:**

1. **What is RCC_CSR?**
   - Reset and Clock Control - Control/Status Register
   - Contains flags for what caused last reset

2. **Why capture it in init?**
   - Register is cleared by hardware after reading
   - Must read early before it's lost

**Build Checkpoint:**
- [ ] Does it compile?
- [ ] When ready: Move to Function 2

---

### Function 2: fault_start() - Why Essential?

**Purpose:** Register console commands, register with watchdog module, initialize MPU stack guard

**Why Essential:**
- Registers callback for watchdog triggers
- Sets up stack protection (preview of Day 5)
- Initializes stack pattern for high water mark detection

**What You Need to Fill In:**

```c
int32_t fault_start(void)
{
    int32_t rc;
    uint32_t* sp;
    
    // Question 1: Register console commands
    // Hint: Same pattern as wdg_start
    rc = ???(&cmd_info);
    if (rc < 0) {
        log_error("fault_start: cmd_register error %d\n", rc);
        return rc;
    }
    
    // Question 2: Register callback with watchdog module
    // Hint: Look at wdg.h - wdg_register_triggered_cb
    // Think: Tell watchdog to call us when it triggers
    rc = ???(wdg_triggered_handler);
    if (rc != 0) {
        log_error("fault_start: wdg_register_triggered_cb returns %ld\n", rc);
        return rc;
    }
    
    // Question 3: Fill stack with pattern (for high water mark detection)
    // Hint: Get current stack pointer, fill downwards
    // Think: Write STACK_INIT_PATTERN to all unused stack space
    __ASM volatile("MOV  %0, sp" : "=r" (sp) : : "memory");
    sp--;  // First decrement
    while (sp >= &_s_stack_guard)
        ??? = STACK_INIT_PATTERN;
    
    // [MPU setup code - simplified, you can copy from reference]
    // [Skip MPU for now, add on Day 5]
    
    return 0;
}
```

**Guiding Questions:**

1. **Why fill stack with a pattern?**
   - Detect how much stack was actually used
   - Day 5: High water mark detection

2. **What is STACK_INIT_PATTERN?**
   - `0xcafebadd` - recognizable value
   - Any value works, this one is memorable

3. **Why decrement sp before loop?**
   - Stack pointer points to last word
   - Need to fill the next available word

4. **Why call wdg_register_triggered_cb?**
   - Connect watchdog to fault module
   - When watchdog triggers, fault module handles it

**Build Checkpoint:**
- [ ] Does it compile?
- [ ] Can you explain the watchdog-fault connection?
- [ ] When ready: Move to Function 3 (panic mode!)

---

### Function 3: fault_detected() - THE HEART - Why Essential?

**Purpose:** Software-detected fault entry point (called by modules or watchdog)

**Why Essential:**
- Main entry into panic mode
- Must be extremely safe (system may be corrupted)
- Collects diagnostic data before reset

**What You Need to Fill In:**

```c
void fault_detected(enum fault_type fault_type, uint32_t fault_param)
{
    // Question 1: Enter critical section (disable interrupts)
    // Hint: Look at Critical section macros
    // Think: CRIT_START() or __disable_irq()
    ???;
    
    // Question 2: Feed hardware watchdog (buy time to collect data)
    // Hint: Look at wdg.h
    ???();
    
    // Question 3: Disable MPU to avoid secondary faults
    // Hint: ARM_MPU_Disable()
    ???();
    
    // Question 4: Start collecting fault data
    // Hint: Populate fault_data_buf fields
    fault_data_buf.fault_type = ???;
    fault_data_buf.fault_param = ???;
    memset(&fault_data_buf.excpt_stk_r0, 0, EXCPT_STK_BYTES);
    
    // Question 5: Save current LR and SP registers (before resetting SP)
    // Hint: Use inline assembly
    __ASM volatile("MOV  %0, lr" : "=r" (fault_data_buf.lr) : : "memory");
    __ASM volatile("MOV  %0, sp" : "=r" (fault_data_buf.sp) : : "memory");
    
    // Question 6: Reset stack pointer to top of RAM
    // Hint: Reset to _estack (defined in linker script)
    __ASM volatile("MOV  sp, %0" : : "r" (???) : "memory");
    
    // Question 7: Call common handler (collects more data, writes to flash, resets)
    ???();
}
```

**Guiding Questions:**

1. **Why disable interrupts immediately?**
   - Prevent corrupt interrupts from interfering
   - System is in bad state, don't trust anything

2. **Why feed hardware watchdog?**
   - Give time to collect diagnostic data
   - Without feeding, HW watchdog will reset before data saved

3. **Why disable MPU?**
   - Avoid triggering another fault
   - System is already panicking, don't make it worse

4. **Why save LR and SP before resetting SP?**
   - LR/SP are important for debugging (where did fault occur?)
   - Can't read them after moving SP

5. **Why reset stack pointer to top?**
   - Old stack may be corrupted
   - New stack at _estack is guaranteed valid RAM

6. **Why call fault_common_handler()?**
   - Separates panic entry from panic work
   - Common handler does the actual data collection

**🎯 PITFALL:** Forgetting to feed hardware watchdog!
- Result: System resets before diagnostics saved
- Fix: Feed HW watchdog in panic mode

**🎯 PITFALL:** Saving SP/LR after moving SP!
- Result: Corrupt values in diagnostic data
- Fix: Save before moving

**Build Checkpoint:**
- [ ] Does it compile?
- [ ] Can you explain why we reset the stack pointer?
- [ ] When ready: Move to Function 4 (the final piece!)

---

### Function 4: fault_common_handler() - Why Essential?

**Purpose:** Collect diagnostic data, write to flash/console, reset system

**Why Essential:**
- Does the actual panic work
- Writes diagnostics to flash (survives reset!)
- Writes to console (for debugging)
- Performs system reset

**⚠️ NOTE:** This is complex. We'll build a simplified version focusing on understanding.

**What You Need to Fill In:**

```c
static void fault_common_handler(void)
{
    uint8_t* lwl_data;
    uint32_t lwl_num_bytes;
    
    // Question 1: Disable LWL (we're panicking, don't log anymore)
    // Hint: Look at lwl.h
    ???(false);
    
    // Question 2: Print fault info to console
    // Hint: printc_panic() (console in panic mode)
    printc_panic("\nFault type=%lu param=%lu\n", 
                 fault_data_buf.fault_type,
                 fault_data_buf.fault_param);
    
    // Question 3: Set magic number (identifies this data structure)
    // Hint: MOD_MAGIC_FAULT
    fault_data_buf.magic = ???;
    fault_data_buf.num_section_bytes = sizeof(fault_data_buf);
    
    // Question 4: Collect ARM system registers (detailed diagnostics)
    // Hint: Provided in reference code
    fault_data_buf.ipsr = __get_IPSR();
    fault_data_buf.icsr = SCB->ICSR;
    fault_data_buf.shcsr = SCB->SHCSR;
    fault_data_buf.cfsr = SCB->CFSR;
    fault_data_buf.hfsr = SCB->HFSR;
    fault_data_buf.mmfar = SCB->MMFAR;
    fault_data_buf.bfar = SCB->BFAR;
    fault_data_buf.tick_ms = ???;  // Hint: Get current time
    
    // Question 5: Get LWL buffer (flight recorder data)
    // Hint: lwl_get_buffer() returns pointer and size
    lwl_data = ???(&lwl_num_bytes);
    
    // Question 6: Record fault data to flash (if enabled)
    // Hint: record_fault_data() - offset, address, size
    record_fault_data(0, (uint8_t*)&fault_data_buf, sizeof(fault_data_buf));
    
    // Question 7: Record LWL buffer after fault data
    record_fault_data(sizeof(fault_data_buf), lwl_data, lwl_num_bytes);
    
    // Question 8: Reset system (function never returns!)
    // Hint: NVIC_SystemReset() from CMSIS
    ???();
}
```

**Guiding Questions:**

1. **Why disable LWL?**
   - System is panicking, don't create more logs
   - Preserve existing LWL buffer

2. **What is the magic number?**
   - Identifies data structure type
   - Reading code can check if valid fault data

3. **Why collect all those ARM registers?**
   - Different registers contain different fault information
   - CFSR = Configurable Fault Status Register (why fault occurred)
   - IPSR = Exception number (which exception)

4. **Why get LWL buffer?**
   - Flight recorder shows what happened before fault
   - Critical for debugging

5. **What does record_fault_data do?**
   - Writes data to flash (if enabled)
   - Writes data to console (for debugging)

6. **Why reset at the end?**
   - Recover from fault by starting fresh
   - NVIC_SystemReset() is MCU reset

**🎯 PITFALL:** Not feeding HW watchdog during panic!
- Flash erase/write takes time (seconds)
- HW watchdog must be fed or system resets mid-write

**💡 PRO TIP:** In production systems, you might feed HW watchdog in a tight loop during flash writes.

**Build Checkpoint:**
- [ ] Does it compile?
- [ ] Can you explain the panic mode flow?
- [ ] When ready: Integration testing!

---

## Phase 3: Integration & Testing (30 min)

### Test Plan

**1. Basic Watchdog Test:**
```
Console:
> wdg status
Expected: Show all watchdogs (should be empty/not registered)

> wdg test register 0 5000
> wdg status
Expected: WDG 0 registered with 5000ms timeout

[Wait 6 seconds without feeding]
Expected: System resets
```

**2. TMPHM Integration:**
```c
// In app_main.c, after tmphm_start():
wdg_register(WDG_TMPHM, 5000);

// In tmphm module, after successful measurement:
wdg_feed(WDG_TMPHM);
```

**Expected:** If TMPHM stops working, system resets after 5 seconds.

**3. Fault Data Test:**
```
Console after reset from watchdog:
> fault data
Expected: Shows fault data from flash (type=WDG, param=0)
```

**4. Manual Fault Test:**
```
Console:
> fault test overflow
Expected: Stack overflow, system resets

> fault test hardfault
Expected: Hard fault, system resets
```

### Troubleshooting Guide

| Symptom | Likely Cause | Check |
|---------|--------------|-------|
| Watchdog never triggers | Timer not started | Check wdg_start() called |
| System resets immediately | Hardware watchdog too short | Check wdg_start_hdw_wdg timeout |
| Fault data not saved | Flash not enabled | Check CONFIG_FAULT_PANIC_TO_FLASH |
| Can't read fault data | Not in flash | Check last reset reason |

---

## Completion Checklist

- [ ] Watchdog module implemented (init, start, register, feed, timer callback)
- [ ] Hardware watchdog starts and feeds correctly
- [ ] Fault module implemented (init, start, detected, common_handler)
- [ ] Panic mode collects and writes diagnostic data
- [ ] System resets properly on watchdog timeout
- [ ] Fault data persists in flash across resets
- [ ] TMPHM integration: watchdog registered and fed on success
- [ ] Code compiles with 0 warnings
- [ ] Console commands work (wdg status, fault data)
- [ ] Compared to reference code, understand all differences

---

## Key Takeaways

### Watchdog Module:
✅ **Multiple software watchdogs** monitor different critical tasks  
✅ **Hardware watchdog** monitors software watchdog module  
✅ **Feed only on success** - proves work completed, not just code running  
✅ **No-init variables** track initialization failures across resets  

### Fault Module:
✅ **Panic mode** is minimal, safe state (no interrupts, new stack, polling)  
✅ **Collect diagnostic data** before reset (ARM registers, LWL buffer)  
✅ **Write to flash AND console** for debugging  
✅ **Reset is recovery** - start fresh from fault  

### The Flow:
```
Application → Software Watchdog → Hardware Watchdog
                                       ↓ (timeout)
                              Fault Module (panic)
                                       ↓ (collect data)
                              Flash + Console
                                       ↓ (reset)
                              Fresh Start
```

---

## Next Steps: Day 5 Preview

On Day 5, you'll add:
- **MPU-based stack guard** (prevent stack overflow into other memory)
- **High water mark detection** (measure actual stack usage)
- **CI/CD foundation** (automate builds and tests)

The fault module already sets up the stack pattern for high water mark detection!

---

**Total Time:** ~4 hours  
**Feedback Cycles:** 6-8 tight loops  
**Learning Quality:** MAXIMUM

**You've built production-grade reliability features!** 🎯

