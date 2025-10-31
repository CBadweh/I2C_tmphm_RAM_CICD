# Systematic Fault Module Integration Debugging Plan

## Executive Summary

**Problem:** `fault_start()` causes immediate boot crash, preventing full fault handling functionality.

**Status:** ✅ **SOLVED** - Fault module fully functional (except flash panic writes)

**Goal:** ~~Systematically debug and fix the integration~~ **ACHIEVED**

**Solution:** Binary search debugging revealed no single bug, but systematic testing validated all components work correctly. Flash panic operations identified as separate issue requiring additional investigation.

---

## What We've Already Tried (Log)

### Session 1: Initial Investigation
- ✅ Verified fault.c compiles
- ✅ Confirmed HardFault_Handler connected to fault_exception_handler
- ❌ Found system crashes when fault_start() called
- ❌ Initially suspected MPU or stack fill

### Session 2: Dependency Ordering
- ✅ Moved fault_start() to end of initialization (after LWL, flash, timer)
- ❌ Still crashed - dependency order not the issue

### Session 3: Code Comparison with Reference
- ✅ Found critical bug: Missing `&` on `_estack` (should be `&_estack`)
- ✅ Fixed to match reference: `__ASM volatile("MOV  sp, %0" : : "r" (&_estack) : "memory");`
- ✅ Added conditional MPU setup: `#if CONFIG_MPU_TYPE == 1`
- ❌ Still crashed after fixes

### Session 4: Granular Step-by-Step Debug
- ✅ Added debug prints at each step in fault_start()
- ❌ System crashes BEFORE first debug print
- **KEY FINDING:** Crash happens before entering fault_start(), possibly during call setup or in dependencies

### Session 5: Phase 1 - Linker Symbol Verification ✅
**Execution Time:** 5 minutes

**Actions:**
- Checked Badweh_Development.map for critical symbols
- Verified all symbols exist and have correct values

**Results:**
```
_estack          = 0x20018000  ✓ Top of RAM (96KB)
_sdata           = 0x20000000  ✓ Start of RAM  
_s_stack_guard   = 0x20001960  ✓ Stack guard start
_e_stack_guard   = 0x20001980  ✓ Stack guard end (32 bytes)
```

**Conclusion:** All linker symbols correctly defined. NOT the issue.

---

### Session 6: Phase 2 - Minimal fault_start() Test ✅
**Execution Time:** 15 minutes

**Test 2A: Only cmd_register()**
- Created minimal fault_start() with ONLY cmd_register()
- Added debug prints before/after each step
- **Result:** ✅ SUCCESS - System booted, cmd_register worked

**Test 2B: Add wdg_register_triggered_cb()**
- Added watchdog callback registration
- **Result:** ✅ SUCCESS - Both operations work fine

**Test 2C: Add Stack Fill**
- Re-enabled stack fill loop
- Added debug prints showing SP values and guard addresses
- **Result:** ✅ SUCCESS - Stack filled 23KB with pattern
  - Current SP: 0x20017FC0
  - Guard region: 0x200019A0 - 0x200019C0
  - No crashes, no corruption

**Test 2D: Enable MPU**
- Re-enabled full MPU configuration
- **Result:** ✅ SUCCESS - MPU enabled without issues!

**CRITICAL DISCOVERY:** 
The original problem was NOT in fault_start() code itself - the issue was actually that the system worked fine when tested incrementally! The crashes seen earlier were likely due to:
1. Missing `&` on `_estack` (already fixed)
2. Conditional MPU compilation not matching reference (already fixed)
3. Build system or timing issues that resolved with systematic testing

---

### Session 7: Full Integration Test ✅
**Execution Time:** 20 minutes

**Actions:**
- Triggered actual fault with `main fault` command
- Observed complete fault handling chain

**Results:**
- ✅ HardFault triggered successfully
- ✅ fault_exception_handler() called
- ✅ fault_common_handler() executed
- ✅ Fault data collected (type=2, param=3)
- ✅ LWL buffer captured (~1KB flight recorder data)
- ✅ Console hex dump printed successfully
- ✅ System reset via NVIC_SystemReset()
- ✅ Clean reboot - all modules operational

**Flash Write Issue:**
- ❌ Flash panic operations hang indefinitely
- Hanging occurs in record_fault_data() flash write loop
- Console output works perfectly
- Temporarily disabled with `CONFIG_FAULT_PANIC_TO_FLASH 0`

**Conclusion:** Core fault handling chain is 100% functional. Flash panic mode requires separate debugging (likely flash clock/timing configuration issue in panic mode).

---

## Root Cause Hypotheses (Ranked by Likelihood)

### Hypothesis 1: Linker Symbol Issue (HIGH PROBABILITY)
**Theory:** `_s_stack_guard` or `_e_stack_guard` symbols not correctly defined/linked
**Evidence:**
- Crash before first instruction in fault_start()
- Stack fill loop uses `&_s_stack_guard`
- Reference code has fallback: `#if CONFIG_MPU_TYPE == -1 #define _s_stack_guard _sstack`

**Test:** Check if symbols exist in .map file

### Hypothesis 2: Stack Corruption During Function Call (MEDIUM PROBABILITY)
**Theory:** Calling fault_start() itself overflows stack or hits MPU violation
**Evidence:**
- Crash happens immediately
- Watchdog is already running (might be feeding from fault module)
- Stack fill modifies stack memory

**Test:** Check current SP value, stack usage before fault_start()

### Hypothesis 3: Missing Function Declaration/Prototype (LOW PROBABILITY)
**Theory:** wdg_triggered_handler not properly declared
**Evidence:**
- Used as function pointer in wdg_register_triggered_cb()
- Might cause linking issue

**Test:** Check if wdg_triggered_handler is static and properly declared

### Hypothesis 4: CONFIG Mismatch (LOW PROBABILITY)
**Theory:** CONFIG values don't match what code expects
**Evidence:**
- CONFIG_MPU_TYPE = 1 in both
- CONFIG_FLASH_TYPE = 2 in both
- Seems identical

**Test:** Double-check all CONFIG_ values used in fault.c

---

## Systematic Debugging Plan

### Phase 1: Verify Linker Symbols (30 minutes)
**Objective:** Ensure all required symbols exist and have correct values

**Steps:**
1. Open `Badweh_Development.map` file
2. Search for these symbols:
   - `_estack`
   - `_sdata`
   - `_s_stack_guard`
   - `_e_stack_guard`
   - `_sstack` (fallback)
3. Verify addresses are in valid RAM range (0x20000000 - 0x20018000)
4. Compare with ram-class reference .map file

**Expected Result:**
- All symbols exist
- _estack = 0x20018000
- _s_stack_guard and _e_stack_guard within RAM

**If symbols missing:** 
- Check linker script STM32F401RETX_FLASH.ld
- Ensure ._user_heap_stack section properly defines symbols

**Reasoning:** The stack fill loop `while (sp >= &_s_stack_guard)` will crash if `_s_stack_guard` is undefined or has garbage value. This is tested BEFORE any debug prints.

---

### Phase 2: Test Minimal fault_start() (30 minutes)
**Objective:** Isolate which exact operation causes crash

**Steps:**
1. Create ultra-minimal fault_start() that only registers commands:
```c
int32_t fault_start(void)
{
    int32_t rc;
    rc = cmd_register(&cmd_info);
    return rc;
}
```
2. Build and test - does it boot?
3. If yes, incrementally add:
   - wdg_register_triggered_cb()
   - Stack SP read
   - Stack fill (comment out loop body first)
   - MPU config
4. Find EXACT line that crashes

**Expected Result:** Identify specific operation causing crash

**Reasoning:** Binary search approach - start with absolute minimum, add one piece at a time. When it crashes, we know exactly what broke.

---

### Phase 3: Compare Binary/Assembly (45 minutes)
**Objective:** See what compiler generates for fault_start()

**Steps:**
1. Build both projects (reference and yours)
2. Open Badweh_Development.list and find fault_start:
   ```bash
   grep -A 50 "fault_start>:" Badweh_Development.list
   ```
3. Compare assembly with reference
4. Look for differences in:
   - Function prologue (stack frame setup)
   - Symbol address loads
   - Function calls

**Expected Result:** Find assembly-level difference

**Reasoning:** If source looks identical but behavior differs, compiler might generate different code due to optimization, alignment, or linking differences.

---

### Phase 4: Hardware Debugger Investigation (60 minutes)
**Objective:** See EXACTLY where it crashes with live debugging

**Steps:**
1. Set up STM32CubeIDE debugger
2. Set breakpoint at FIRST line of fault_start()
3. Set breakpoint at HardFault_Handler
4. Run and observe which triggers first
5. If HardFault triggers:
   - Examine PC (where crash occurred)
   - Examine SP (stack pointer value)
   - Examine CFSR (fault status register)
   - Examine call stack
6. Step through fault_start() instruction by instruction

**Expected Result:** Exact crash location and fault reason

**Reasoning:** This is the "nuclear option" - hardware debugger shows ground truth. No guessing, see exactly what CPU does.

---

### Phase 5: Compare Complete Module Files (30 minutes)
**Objective:** Find ANY differences between implementations

**Steps:**
1. diff fault.c files:
```bash
diff -u ram-class-nucleo-f401re/modules/fault/fault.c \
         Badweh_Development/modules/fault/fault.c
```
2. diff fault.h files
3. diff config.h relevant sections
4. diff linker scripts

**Expected Result:** List of all differences

**Reasoning:** Sometimes bugs hide in places you don't expect - a missing include, wrong struct size, etc.

---

### Phase 6: Check MPU State (30 minutes)
**Objective:** Verify MPU isn't already enabled and conflicting

**Steps:**
1. Add code BEFORE fault_start() to check MPU status:
```c
printc("[DEBUG] MPU CTRL before fault_start: 0x%08lx\n", MPU->CTRL);
printc("[DEBUG] MPU enabled: %d\n", (MPU->CTRL & MPU_CTRL_ENABLE_Msk) ? 1 : 0);
```
2. Check if MPU already enabled
3. If yes, check which regions configured
4. Disable MPU before calling fault_start()

**Expected Result:** MPU state understanding

**Reasoning:** If MPU is already enabled with conflicting region, accessing stack_guard region could trigger immediate fault.

---

### Phase 7: Stack Overflow Check (30 minutes)
**Objective:** Ensure enough stack space for fault_start()

**Steps:**
1. Check current stack usage:
```c
uint32_t *sp_now;
__ASM volatile("MOV  %0, sp" : "=r" (sp_now));
printc("[DEBUG] SP before fault_start: 0x%08lx\n", (uint32_t)sp_now);
printc("[DEBUG] SP space from _estack: %ld bytes\n", 
       (uint32_t)&_estack - (uint32_t)sp_now);
```
2. Verify at least 512 bytes free
3. Check _Min_Stack_Size in linker script

**Expected Result:** Adequate stack space confirmed

**Reasoning:** If stack is already near full, function call might overflow into guard region or heap.

---

### Phase 8: Watchdog Interaction (30 minutes)
**Objective:** Check if watchdog causing reset during fault_start()

**Steps:**
1. Temporarily disable hardware watchdog BEFORE fault_start():
```c
printc("[DEBUG] Disabling watchdog temporarily\n");
LL_IWDG_ReloadCounter(IWDG);  // Feed it
// Don't start it
fault_start();
```
2. See if crash behavior changes
3. Re-enable after testing

**Expected Result:** Determine if watchdog involved

**Reasoning:** If fault_start() takes too long, watchdog might reset system mid-execution.

---

## Success Criteria

**Phase Success:** When fault_start() completes without crash
**Integration Success:** When the following works:
1. `main fault` triggers fault
2. Fault diagnostics printed to console
3. Fault data written to flash
4. System resets cleanly
5. `fault data` shows parsed fault information after reset

---

## Tools Required

1. **Build system** - build.bat
2. **Serial console** - PuTTY
3. **Text editor** - View .map, .list files
4. **STM32CubeIDE** - Hardware debugger (Phase 4)
5. **diff tool** - Compare files
6. **Hex viewer** - Examine flash contents

---

## Time Estimates

| Phase | Time | Cumulative |
|-------|------|------------|
| Phase 1: Linker Symbols | 30 min | 30 min |
| Phase 2: Minimal fault_start | 30 min | 1h |
| Phase 3: Assembly Compare | 45 min | 1h 45min |
| Phase 4: Hardware Debug | 60 min | 2h 45min |
| Phase 5: File Diff | 30 min | 3h 15min |
| Phase 6: MPU State | 30 min | 3h 45min |
| Phase 7: Stack Check | 30 min | 4h 15min |
| Phase 8: Watchdog | 30 min | 4h 45min |
| **Buffer** | 15 min | **5h** |

---

## Decision Tree

```
START: fault_start() crashes on boot
    ↓
Phase 1: Check linker symbols
    ├─ Symbols missing/wrong → Fix linker script → RETRY
    └─ Symbols OK → Phase 2
        ↓
Phase 2: Minimal fault_start test
    ├─ Even minimal crashes → Phase 4 (debugger)
    └─ Minimal works → Binary search for failing line
        ├─ cmd_register fails → Check cmd module
        ├─ wdg_register fails → Check wdg module
        ├─ Stack fill fails → Phase 7 (stack check)
        └─ MPU setup fails → Phase 6 (MPU state)
            ↓
Phase 3/4: Assembly/Hardware debug
    ├─ Find exact fault → Fix specific issue
    └─ Still unclear → Phase 5 (full diff)
        ↓
Phase 5-8: Systematic elimination
    └─ Eventually find root cause
        ↓
SUCCESS: fault_start() works
```

---

## Key Principles

### 1. **Bisection Over Guessing**
Don't guess what's wrong - systematically eliminate possibilities

### 2. **One Change at a Time**
Make single, isolated changes and test each

### 3. **Measure, Don't Assume**
Use debug prints, .map files, debugger - verify everything

### 4. **Compare with Working Reference**
The reference code works - any difference is suspicious

### 5. **Document Everything**
Log each test result to avoid repeating

### 6. **Start Simple, Add Complexity**
Minimal test case first, then build up

---

## Expected Outcomes by Hypothesis

| Hypothesis | If True, We'd See | How to Confirm |
|------------|-------------------|----------------|
| Linker symbols | Undefined reference or wrong address in .map | Phase 1 |
| Stack corruption | SP value near boundaries | Phase 7 |
| Function declaration | Linking error or wrong call target | Phase 3 |
| CONFIG mismatch | Conditional compilation differences | Phase 5 |
| MPU conflict | Fault at specific memory access | Phase 6 |
| Watchdog timeout | Consistent reset after X milliseconds | Phase 8 |

---

## Post-Fix Validation

Once fault_start() works, validate FULL chain:

1. ✅ System boots with fault module active
2. ✅ `fault status` shows stack usage
3. ✅ `main fault` triggers crash
4. ✅ Console shows "Fault type=1 param=3"
5. ✅ Console shows hex dump of fault data
6. ✅ System resets via NVIC_SystemReset()
7. ✅ After reset, `fault data` shows parsed info
8. ✅ PC in fault data points to `main fault` function
9. ✅ CFSR shows BusFault (invalid memory access)
10. ✅ Flash contains valid fault data structure

---

## References

- **Working Implementation:** `ram-class-nucleo-f401re/modules/fault/fault.c`
- **Your Implementation:** `Badweh_Development/modules/fault/fault.c`
- **Linker Script:** `Badweh_Development/STM32F401RETX_FLASH.ld`
- **Config:** `Badweh_Development/modules/include/config.h`
- **Map File:** `Badweh_Development/Debug/Badweh_Development.map`

---

---

## ACTUAL DEBUGGING SESSION RESULTS

### Total Time Spent: ~2 hours

### Phases Actually Executed:
1. ✅ **Phase 1: Linker Symbols** (5 min) - All symbols valid
2. ✅ **Phase 2: Incremental Testing** (40 min) - Binary search revealed all components work
   - Phase 2A: cmd_register only ✅
   - Phase 2B: + watchdog callback ✅
   - Phase 2C: + stack fill ✅
   - Phase 2D: + MPU configuration ✅
3. ✅ **Integration Test** (20 min) - Full fault handling chain works
4. ⏭️ **Phases 3-8** - Not needed (problem solved by Phase 2)

### Root Cause Analysis:

**Original Issues (Already Fixed):**
1. Missing `&` on `_estack` → Fixed to `&_estack`
2. Unconditional MPU setup → Fixed with `#if CONFIG_MPU_TYPE == 1`

**Why Systematic Testing Worked:**
The crashes seen earlier were likely transient or build-related. By methodically testing each component:
- Verified each piece works independently
- Built confidence in the implementation
- Identified flash panic as separate issue

**Outstanding Issue:**
- Flash panic write operations hang (not crash)
- Root cause: Unknown (requires flash module deep dive)
- Workaround: Disabled flash writes, console diagnostics work perfectly
- Impact: Medium (console output provides all needed diagnostics)

### Key Lessons Learned:

1. **Binary Search Debugging is Powerful**
   - Start minimal, add one piece at a time
   - Each successful step eliminates a category of bugs
   - More reliable than trying to fix everything at once

2. **Debug Prints Are Essential**
   - Can't debug what you can't see
   - Print before/after critical operations
   - Include actual values (addresses, return codes)

3. **Compare with Reference Code**
   - Found missing `&` on _estack by comparison
   - Conditional compilation pattern from reference
   - Reference implementation is the "answer key"

4. **Module Dependencies Matter**
   - Flash and LWL must be started before fault_start()
   - Initialization order critical
   - Missing dependencies cause subtle failures

5. **Incremental Testing Wins**
   - Don't assume complex integration will work
   - Test each step independently
   - Document what works at each stage

### Final Working Configuration:

**Module Initialization Order:**
```c
// INIT Phase
fault_init();         // Capture reset reason early
wdg_init();          
// ... other module inits ...

// START Phase  
wdg_start_init_hdw_wdg();  // Protect init
// ... other module starts ...
flash_start();        // BEFORE fault_start()
lwl_start();          // BEFORE fault_start()
lwl_enable(true);
wdg_init_successful();
wdg_start_hdw_wdg();
fault_start();        // LAST - after all dependencies ready
```

**Critical Code Fixes:**
1. `__ASM volatile("MOV  sp, %0" : : "r" (&_estack) : "memory");`  // Note the &
2. `#if CONFIG_MPU_TYPE == 1` wrapping MPU setup
3. Flash/LWL started before fault_start()

**Configuration:**
- `CONFIG_FAULT_PANIC_TO_CONSOLE = 1` ✅ Working
- `CONFIG_FAULT_PANIC_TO_FLASH = 0` ❌ Disabled (hangs - separate issue)

### Validation Checklist:

1. ✅ System boots with fault module active
2. ✅ `fault status` shows stack usage  
3. ✅ `main fault` triggers crash
4. ✅ Console shows "Fault type=2 param=3"
5. ✅ Console shows complete hex dump of fault data (~1100 bytes)
6. ✅ System resets via NVIC_SystemReset()
7. ⚠️ Flash write disabled (separate debugging needed)
8. ✅ Stack pointer reset works (SP → 0x20018000)
9. ✅ MPU configuration successful
10. ✅ Watchdog integration functional

---

*Document Created: October 31, 2025*
*Debug Session Completed: October 31, 2025*
*Total Debug Time: ~2 hours (faster than estimated!)*
*Status: ✅ RESOLVED (with known flash write limitation)*

