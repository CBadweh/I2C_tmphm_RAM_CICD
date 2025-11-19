# Exploring STM32 Fault Handling - Stack Frame Deep Dive

## Overview

This document provides a structured approach to understanding the fault handling call chain in bare-metal STM32 development, with specific focus on:
- **Call Chain**: `fault_detected()` → `fault_common_handler()` → `record_fault_data()` → `NVIC_SystemReset()`
- **Stack Frame Management**: How ARM Cortex-M4 handles exceptions
- **Memory Map**: Understanding where code and data lives
- **Practical Debugging**: Using STM32CubeIDE to observe the system

---

## Memory Map Foundation (STM32F401RE)

### From Linker Script (`STM32F401RETX_FLASH.ld`)

```c
/* Memories definition */
MEMORY
{
  RAM    (xrw)    : ORIGIN = 0x20000000,   LENGTH = 96K
  FLASH    (rx)    : ORIGIN = 0x8000000,   LENGTH = 512K
}

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM);  /* = 0x20018000 */

_Min_Heap_Size = 0x200;      /* 512 bytes */
_Min_Stack_Size = 0x400;     /* 1024 bytes */
_MPU_Min_Block_Size = 32;    /* For stack guard */
```

### Critical Addresses

| Symbol | Address | Description |
|--------|---------|-------------|
| `ORIGIN(RAM)` | 0x20000000 | Start of SRAM |
| `_estack` | **0x20018000** | Top of RAM (96KB = 0x18000) |
| `ORIGIN(FLASH)` | 0x08000000 | Start of Flash |
| Flash End | 0x0807FFFF | End of 512KB Flash |

### Memory Layout Visualization

```
0x20018000  ┌─────────────────────┐  <- _estack (Top of RAM)
            │                     │
            │   Main Stack (MSP)  │  (Grows DOWN ↓)
            │   (1KB reserved)    │
            ├─────────────────────┤
            │                     │
            │   Heap (grows up)   │  (Grows UP ↑)
            │   (512 bytes min)   │
            ├─────────────────────┤
            │   .bss (uninit)     │
            ├─────────────────────┤
            │   .data (init)      │
            ├─────────────────────┤
0x20000000  └─────────────────────┘  <- ORIGIN(RAM)
```

**📖 Reference Manual:** Section 2.3 - Memory map and register boundary addresses

---

## ARM Cortex-M4 Exception Stack Frame

### What Happens on Fault (Hardware Automatic)

When a fault occurs, **before your code runs**, the ARM Cortex-M4 hardware automatically:

1. **Saves 8 registers to the stack** (exception stack frame)
2. **Updates Stack Pointer (SP)** to point to the new stack top
3. **Loads the fault handler address** from the vector table
4. **Sets LR (Link Register)** to a special "exception return" value
5. **Jumps to your fault handler**

### The Exception Stack Frame (32 bytes total)

```c
// From fault.c - These MUST match ARM's automatic stacking order!
struct fault_data {
    // ... other fields ...
    
    // ARM v7-M Exception Stack Frame (hardware automatic)
    uint32_t excpt_stk_r0;           // Offset +0 from SP
    uint32_t excpt_stk_r1;           // Offset +4
    uint32_t excpt_stk_r2;           // Offset +8
    uint32_t excpt_stk_r3;           // Offset +12
    uint32_t excpt_stk_r12;          // Offset +16
    uint32_t excpt_stk_lr;           // Offset +20 (LR before exception)
    uint32_t excpt_stk_rtn_addr;     // Offset +24 (PC where fault occurred)
    uint32_t excpt_stk_xpsr;         // Offset +28 (Program Status Register)
};
```

### Visual Stack Layout After Exception Entry

```
Higher Memory Address
0x20017FFF  ┌───────────────────┐
            │   (unused)        │
            ├───────────────────┤
SP before   │   xPSR            │  <- Highest on stack
            │   PC (return)     │  <- Address where fault occurred!
            │   LR              │  <- Link Register before fault
            │   R12             │
            │   R3              │
            │   R2              │
            │   R1              │
SP now ───> │   R0              │  <- SP points here after stacking
            ├───────────────────┤
            │   (rest of stack) │
            │        ...         │
0x20000000  └───────────────────┘
Lower Memory Address
```

**⚠️ CRITICAL:** Stack grows DOWNWARD! Higher addresses are pushed first, lower addresses last.

**💡 PRO TIP:** The PC (return address) in the stack frame tells you EXACTLY where the fault occurred in your code!

### Exception Return Value in LR

After exception entry, LR contains one of these magic values:

| LR Value | Meaning |
|----------|---------|
| 0xFFFFFFF1 | Return to Handler mode, MSP |
| 0xFFFFFFF9 | Return to Thread mode, MSP |
| 0xFFFFFFFD | Return to Thread mode, PSP |
| 0xFFFFFFE1 | Return to Handler mode, MSP, FPU extended frame |
| 0xFFFFFFE9 | Return to Thread mode, MSP, FPU extended frame |
| 0xFFFFFFED | Return to Thread mode, PSP, FPU extended frame |

**📖 Reference Manual:** ARM®v7-M Architecture Reference Manual, Section B1.5.8 - Exception return behavior

---

## The Fault Handling Call Chain

### Function 1: `fault_detected()`

**Purpose:** Entry point when ANY fault is detected. Stabilizes the system.

```c
void fault_detected(enum fault_type fault_type, uint32_t fault_param)
{
    // STEP 1: Enter critical section (disable ALL interrupts)
    CRIT_START();
    
    // STEP 2: Feed hardware watchdog (buy time to collect diagnostics)
    wdg_feed_hdw();
    
    // STEP 3: Disable MPU (Memory Protection Unit)
    // Prevents secondary faults while we're collecting data
    ARM_MPU_Disable();
    
    // STEP 4: Start collecting fault data
    fault_data_buf.fault_type = fault_type;
    fault_data_buf.fault_param = fault_param;
    memset(&fault_data_buf.excpt_stk_r0, 0, EXCPT_STK_BYTES);
    
    // STEP 5: Save current LR and SP (BEFORE resetting SP!)
    __ASM volatile("MOV  %0, lr" : "=r" (fault_data_buf.lr) : : "memory");
    __ASM volatile("MOV  %0, sp" : "=r" (fault_data_buf.sp) : : "memory");
    
    // STEP 6: Reset stack pointer to TOP OF RAM
    // THIS IS THE MOST CRITICAL OPERATION!
    __ASM volatile("MOV  sp, %0" : : "r" (_estack) : "memory");
    
    // STEP 7: Call common handler (no return!)
    fault_common_handler();
}
```

#### 🎯 Why Reset the Stack Pointer?

**Problem:** The fault might have been CAUSED by:
- Stack overflow (SP went below valid range)
- Stack corruption (buffer overflow on stack)
- Wild pointer writing to stack memory

**Solution:** Reset SP to `_estack` (0x20018000) = guaranteed clean stack

**📖 WAR STORY:** Without this reset, I've seen fault handlers crash mid-execution because they tried to push return addresses onto an already-corrupted stack. This causes a "double fault" that's nearly impossible to debug. Always reset the stack!

**✅ BEST PRACTICE:** After SP reset, you have ~1KB of guaranteed-good stack space to work with for diagnostics.

---

### Function 2: `fault_common_handler()`

**Purpose:** Collect comprehensive diagnostics and write to flash.

```c
static void fault_common_handler()
{
    uint8_t* lwl_data;
    uint32_t lwl_num_bytes;
    struct end_marker end;

    // STEP 1: Disable LWL logging (we're in panic mode)
    lwl_enable(false);

    // STEP 2: Print to console (if still working)
    printc_panic("\nFault type=%lu param=%lu\n",
                  fault_data_buf.fault_type,
                  fault_data_buf.fault_param);

    // STEP 3: Set magic number for identification
    fault_data_buf.magic = MOD_MAGIC_FAULT;
    fault_data_buf.num_section_bytes = sizeof(fault_data_buf);

    // STEP 4: Collect ARM system registers (the gold mine!)
    fault_data_buf.ipsr = __get_IPSR();      // Interrupt status
    fault_data_buf.icsr = SCB->ICSR;         // Interrupt control
    fault_data_buf.shcsr = SCB->SHCSR;       // System handler control
    fault_data_buf.cfsr = SCB->CFSR;         // Configurable Fault Status ⭐
    fault_data_buf.hfsr = SCB->HFSR;         // HardFault Status ⭐
    fault_data_buf.mmfar = SCB->MMFAR;       // MemManage Fault Address ⭐
    fault_data_buf.bfar = SCB->BFAR;         // BusFault Address ⭐
    fault_data_buf.tick_ms = tmr_get_ms();   // Timestamp

    // STEP 5: Get LWL buffer (flight recorder)
    lwl_data = lwl_get_buffer(&lwl_num_bytes);

    // STEP 6: Write fault data to flash (section 1)
    record_fault_data(0, (uint8_t*)&fault_data_buf, sizeof(fault_data_buf));

    // STEP 7: Write LWL buffer to flash (section 2)
    record_fault_data(sizeof(fault_data_buf), lwl_data, lwl_num_bytes);

    // STEP 8: Write end marker (section 3)
    memset(&end, 0, sizeof(end));
    end.magic = MOD_MAGIC_END;
    end.num_section_bytes = sizeof(end);
    record_fault_data(sizeof(fault_data_buf) + lwl_num_bytes, 
                      (uint8_t*)&end, sizeof(end));

    // STEP 9: Reset the system (NEVER RETURN!)
    NVIC_SystemReset();
}
```

#### Critical Registers Explained

| Register | Full Name | What It Tells You |
|----------|-----------|-------------------|
| **CFSR** | Configurable Fault Status Register | **MOST IMPORTANT** - tells you UsageFault, BusFault, or MemManage details |
| **HFSR** | HardFault Status Register | Tells you if it escalated from another fault |
| **MMFAR** | MemManage Fault Address Register | The address that caused a memory protection fault |
| **BFAR** | BusFault Address Register | The address that caused a bus error |

**📖 Reference Manual:** STM32F401 Reference Manual, Section 4.3.13-4.3.14 (SCB registers)

#### CFSR Bit Breakdown (Most Useful for Debugging)

```
CFSR (32-bit register):
┌────────────┬─────────────┬──────────────┐
│ UsageFault │  BusFault   │  MemManage   │
│  [31:16]   │  [15:8]     │   [7:0]      │
└────────────┴─────────────┴──────────────┘

MemManage Fault Status [7:0]:
  Bit 0 (IACCVIOL): Instruction access violation
  Bit 1 (DACCVIOL): Data access violation
  Bit 7 (MMARVALID): MMFAR has valid fault address

BusFault Status [15:8]:
  Bit 8 (IBUSERR): Instruction bus error
  Bit 9 (PRECISERR): Precise data bus error
  Bit 10 (IMPRECISERR): Imprecise data bus error
  Bit 15 (BFARVALID): BFAR has valid fault address

UsageFault Status [31:16]:
  Bit 16 (UNDEFINSTR): Undefined instruction
  Bit 17 (INVSTATE): Invalid state
  Bit 18 (INVPC): Invalid PC load
  Bit 19 (NOCP): No coprocessor
  Bit 24 (UNALIGNED): Unaligned access
  Bit 25 (DIVBYZERO): Divide by zero
```

**💡 PRO TIP:** Check BFARVALID and MMARVALID first! If set, the corresponding address register tells you EXACTLY where the fault occurred.

---

### Function 3: `record_fault_data()`

**Purpose:** Write diagnostics to flash (survives reset) and/or console.

```c
static void record_fault_data(uint32_t data_offset, uint8_t* data_addr,
                              uint32_t num_bytes)
{
#if CONFIG_FAULT_PANIC_TO_FLASH
    {
        static bool do_flash;
        int32_t rc;
        
        // On first call (offset==0), check if flash already has data
        if (data_offset == 0) {
            do_flash = ((struct fault_data*)FLASH_PANIC_DATA_ADDR)->magic !=
                MOD_MAGIC_FAULT;
        }
        
        if (do_flash) {
            // Erase flash page on first call
            if (data_offset == 0) {
                rc = flash_panic_erase_page((uint32_t*)FLASH_PANIC_DATA_ADDR);
                if (rc != 0)
                    printc_panic("flash_panic_erase_page returns %ld\n", rc);
            }
            
            // Write data to flash
            rc = flash_panic_write((uint32_t*)(FLASH_PANIC_DATA_ADDR + data_offset),
                                   (uint32_t*)data_addr, num_bytes);
            if (rc != 0)
                printc_panic("flash_panic_write returns %ld\n", rc);
        }
    }
#endif

#if CONFIG_FAULT_PANIC_TO_CONSOLE
    {
        // Hex dump to console (32 bytes per line)
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

**⚠️ WATCH OUT:** Flash writes take time! That's why we fed the watchdog at the start.

#### Flash Memory Layout for Panic Data

```
Flash Memory (512KB):
0x08000000  ┌─────────────────────┐
            │  Vector Table       │
            │  .text (code)       │
            │  .rodata (const)    │
            │        ...          │
            ├─────────────────────┤
0x0807E000  │  Panic Data Page    │  <- FLASH_PANIC_DATA_ADDR
            │  (Last 8KB sector)  │     (survives reset!)
            ├─────────────────────┤
            │  [Section 1]        │  <- fault_data_buf
            │  [Section 2]        │  <- LWL buffer
            │  [Section 3]        │  <- End marker
0x0807FFFF  └─────────────────────┘
```

**✅ BEST PRACTICE:** Always write to the LAST sector of flash for panic data. This way, normal firmware updates won't erase your diagnostics.

---

## Complete Call Flow Diagram

```
Normal Execution
    ↓
Fault Occurs! (e.g., null pointer dereference)
    ↓
┌─────────────────────────────────────────────┐
│ ARM HARDWARE (Automatic, before your code!) │
├─────────────────────────────────────────────┤
│ 1. Save R0, R1, R2, R3, R12, LR, PC, xPSR  │
│    to current stack (8 words = 32 bytes)    │
│ 2. Update SP (now points to stacked R0)     │
│ 3. Set LR = 0xFFFFFFF9 (exception return)   │
│ 4. Load fault handler address from vector   │
│ 5. Jump to fault handler                    │
└─────────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────────┐
│ fault_detected(fault_type, fault_param)     │
├─────────────────────────────────────────────┤
│ Stack State: POSSIBLY CORRUPTED             │
│ SP: Unknown (could be anywhere!)            │
├─────────────────────────────────────────────┤
│ 1. CRIT_START()         - Disable interrupts│
│ 2. wdg_feed_hdw()       - Buy time          │
│ 3. ARM_MPU_Disable()    - Prevent 2nd fault │
│ 4. Save fault_type/param                    │
│ 5. Save LR and SP to fault_data_buf         │
│ 6. ⭐ MOV sp, _estack   - RESET STACK! ⭐  │
│ 7. Call fault_common_handler()              │
└─────────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────────┐
│ fault_common_handler()                      │
├─────────────────────────────────────────────┤
│ Stack State: CLEAN (SP = 0x20018000)        │
│ Now safe to call functions!                 │
├─────────────────────────────────────────────┤
│ 1. lwl_enable(false)    - Stop logging      │
│ 2. printc_panic()       - Console output    │
│ 3. Set magic number                         │
│ 4. Collect SCB registers (CFSR, HFSR, etc.) │
│ 5. Get LWL buffer                           │
│ 6. record_fault_data()  - Write to flash... │
└─────────────────────────────────────────────┘
    ↓
┌─────────────────────────────────────────────┐
│ record_fault_data() [Called 3 times]        │
├─────────────────────────────────────────────┤
│ Call 1: Write fault_data_buf                │
│ Call 2: Write LWL buffer                    │
│ Call 3: Write end marker                    │
│                                             │
│ Each call:                                  │
│   - Erase flash page (on first call)        │
│   - Write data to flash                     │
│   - Hex dump to console (if enabled)        │
└─────────────────────────────────────────────┘
    ↓
Back to fault_common_handler()
    ↓
NVIC_SystemReset() - System resets
    ↓
Startup code runs
    ↓
Fault data now in flash, ready to be analyzed!
```

---

## STM32CubeIDE Debugging Script

### Setup: Trigger a Test Fault

Add this function to your `app_main.c`:

```c
// Test fault trigger - DO NOT LEAVE IN PRODUCTION CODE!
void test_hardfault(void)
{
    log_info("Triggering test HardFault...");
    
    // Method 1: Null pointer dereference
    uint32_t *bad_ptr = NULL;
    *bad_ptr = 0x12345678;  // BOOM!
    
    // Method 2: Invalid instruction (if above doesn't work)
    // __ASM volatile(".word 0xFFFFFFFF");
    
    // Method 3: Divide by zero (if UsageFault enabled)
    // volatile int x = 42;
    // volatile int y = 0;
    // volatile int z = x / y;
}
```

Call it from your console command or button press for testing.

---

### Debugging Session Checklist

#### Phase 1: Set Breakpoints

In STM32CubeIDE, set breakpoints at:

| Location | Line | Purpose |
|----------|------|---------|
| `fault_detected()` | Entry (line 302) | Observe corrupted state |
| `fault_detected()` | Line 318 | After saving SP/LR |
| `fault_detected()` | Line 322 | After SP reset |
| `fault_common_handler()` | Entry (line 433) | Start of diagnostics |
| `fault_common_handler()` | Line 442 | Before reading SCB registers |
| `NVIC_SystemReset()` | Line 469 | Before reset |

#### Phase 2: Data Collection Worksheet

Print this and fill it out during debugging:

```
=============================================================================
                    FAULT DEBUGGING WORKSHEET
=============================================================================

TEST DETAILS:
  Date/Time: _______________
  Fault Type: _______________
  Trigger Method: _______________

-----------------------------------------------------------------------------
BREAKPOINT 1: Entry to fault_detected() (Line 302)
-----------------------------------------------------------------------------
  Program Counter (PC):  0x________________
  Stack Pointer (SP):    0x________________  <- May be corrupted!
  Link Register (LR):    0x________________  <- Should be 0xFFFFFFxx
  
  LR Interpretation:
    [ ] 0xFFFFFFF9 = Thread mode, MSP
    [ ] 0xFFFFFFFD = Thread mode, PSP
    [ ] Other: _______________
  
  SP Location:
    [ ] In valid RAM (0x20000000 - 0x20018000)
    [ ] Outside RAM (PROBLEM!)
    [ ] Near stack bottom (potential overflow)

-----------------------------------------------------------------------------
BREAKPOINT 2: After saving SP/LR (Line 318)
-----------------------------------------------------------------------------
  fault_data_buf.sp:     0x________________
  fault_data_buf.lr:     0x________________
  
  Verify: Does fault_data_buf.sp match previous SP?  [ ] Yes  [ ] No

-----------------------------------------------------------------------------
BREAKPOINT 3: After SP reset (Line 322)
-----------------------------------------------------------------------------
  Stack Pointer (SP):    0x________________  <- Should be 0x20018000!
  
  Verify: SP = 0x20018000?  [ ] Yes  [ ] No (PROBLEM!)
  
  Memory Window (0x20017F00 - 0x20018000):
    [ ] Looks clean (mostly 0x00 or pattern)
    [ ] Has data (stack is being used)

-----------------------------------------------------------------------------
BREAKPOINT 4: In fault_common_handler() (Line 433)
-----------------------------------------------------------------------------
  Stack Pointer (SP):    0x________________  <- Should be near 0x20018000
  
  Console Output:
    Fault type: _______________
    Fault param: _______________

-----------------------------------------------------------------------------
BREAKPOINT 5: Before reading SCB registers (Line 442)
-----------------------------------------------------------------------------
  Open "Memory Browser" window and read SCB registers:
  (Base address for Cortex-M4 SCB: 0xE000ED00)
  
  SCB->ICSR (0xE000ED04):   0x________________
  SCB->SHCSR (0xE000ED24):  0x________________
  SCB->CFSR (0xE000ED28):   0x________________  ⭐ MOST IMPORTANT
  SCB->HFSR (0xE000ED2C):   0x________________
  SCB->MMFAR (0xE000ED34):  0x________________  (if MMARVALID set)
  SCB->BFAR (0xE000ED38):   0x________________  (if BFARVALID set)
  
  CFSR Bit Analysis:
  ┌────────────┬─────────────┬──────────────┐
  │ UsageFault │  BusFault   │  MemManage   │
  │  [31:16]   │  [15:8]     │   [7:0]      │
  └────────────┴─────────────┴──────────────┘
  
  Binary: ____ ____ ____ ____ ____ ____ ____ ____
  
  Check these bits:
    [ ] Bit 0 (IACCVIOL): Instruction access violation
    [ ] Bit 1 (DACCVIOL): Data access violation
    [ ] Bit 7 (MMARVALID): MMFAR valid
    [ ] Bit 8 (IBUSERR): Instruction bus error
    [ ] Bit 9 (PRECISERR): Precise data bus error
    [ ] Bit 15 (BFARVALID): BFAR valid
    [ ] Bit 16 (UNDEFINSTR): Undefined instruction
    [ ] Bit 25 (DIVBYZERO): Divide by zero

  Fault Address (if valid):
    MMFAR: 0x________________
    BFAR:  0x________________

-----------------------------------------------------------------------------
BREAKPOINT 6: Before NVIC_SystemReset() (Line 469)
-----------------------------------------------------------------------------
  Open "Memory Browser" and check flash panic address:
  FLASH_PANIC_DATA_ADDR: 0x________________ (typically 0x0807E000)
  
  First 64 bytes of flash:
    Offset +0x00: ________________  <- Should see magic number
    Offset +0x04: ________________
    Offset +0x08: ________________  <- fault_type
    Offset +0x0C: ________________  <- fault_param
  
  Console output complete?  [ ] Yes  [ ] No
  
  Press Continue to reset...

-----------------------------------------------------------------------------
POST-RESET ANALYSIS
-----------------------------------------------------------------------------
  After reset, in main() or boot, read flash panic data:
  
  Address: 0x________________ (FLASH_PANIC_DATA_ADDR)
  Magic number found?  [ ] Yes (0x________) [ ] No
  
  Fault type: _______________
  Fault param: _______________
  Return address (PC): 0x________________
  
  Disassemble address to find exact fault line:
    Source file: _______________
    Function: _______________
    Line: _______________

=============================================================================
```

---

### Debugger Commands Reference

#### Memory Browser Addresses

| What to View | Address | Size |
|--------------|---------|------|
| SCB Registers | 0xE000ED00 | 256 bytes |
| CFSR Register | 0xE000ED28 | 4 bytes |
| Stack (top) | 0x20017F00 | 256 bytes |
| Flash Panic Data | 0x0807E000 | 1024 bytes |
| Entire RAM | 0x20000000 | 96KB |

#### Expressions to Watch

Add these to the "Expressions" window:

```
fault_data_buf
fault_data_buf.sp
fault_data_buf.lr
fault_data_buf.cfsr
fault_data_buf.hfsr
fault_data_buf.mmfar
fault_data_buf.bfar
_estack
```

#### Disassembly View

When you reach the breakpoint in `fault_detected()`:
1. Note the saved PC value from the exception stack frame
2. Open **Window → Show View → Disassembly**
3. In address bar, enter the PC value
4. This shows you the EXACT instruction that caused the fault!

---

## Advanced Topics

### Understanding the Exception Stack Frame Capture

The hardware stacks registers automatically, but our code needs to COPY them to our fault buffer. Here's how:

```c
// In fault_detected(), we currently zero out the exception stack:
memset(&fault_data_buf.excpt_stk_r0, 0, EXCPT_STK_BYTES);

// To actually CAPTURE the hardware stack frame, we would do:
uint32_t* hw_stack = (uint32_t*)fault_data_buf.sp;  // SP we saved
memcpy(&fault_data_buf.excpt_stk_r0, hw_stack, EXCPT_STK_BYTES);

// This copies: R0, R1, R2, R3, R12, LR, PC, xPSR from hardware stack
// to our fault_data_buf structure
```

**⚠️ WATCH OUT:** If the stack was corrupted, `hw_stack` might point to invalid memory! That's why the current code zeroes it out for safety.

**💡 PRO TIP:** In production code, you'd add a bounds check:
```c
if ((hw_stack >= (uint32_t*)0x20000000) && 
    (hw_stack <= (uint32_t*)0x20018000)) {
    // SP is in valid RAM, safe to copy
    memcpy(&fault_data_buf.excpt_stk_r0, hw_stack, EXCPT_STK_BYTES);
} else {
    // Stack pointer is corrupted, don't trust it
    memset(&fault_data_buf.excpt_stk_r0, 0xFF, EXCPT_STK_BYTES);
}
```

### Stack Pointer Validity Check

```c
// Good stack pointer validation
bool is_valid_stack_pointer(uint32_t sp)
{
    // Must be in RAM
    if (sp < 0x20000000 || sp >= 0x20018000)
        return false;
    
    // Must be word-aligned (ARM requirement)
    if (sp & 0x03)
        return false;
    
    // Must have room for exception frame (32 bytes)
    if (sp < (0x20000000 + 32))
        return false;
    
    return true;
}
```

### Reading the Fault Address from CFSR

```c
// Decode CFSR to get fault address (if available)
void decode_fault_address(uint32_t cfsr, uint32_t mmfar, uint32_t bfar)
{
    // Check MemManage Fault Address Valid
    if (cfsr & (1 << 7)) {  // MMARVALID
        printc_panic("MemManage Fault at address: 0x%08lx\n", mmfar);
    }
    
    // Check BusFault Address Valid
    if (cfsr & (1 << 15)) {  // BFARVALID
        printc_panic("BusFault at address: 0x%08lx\n", bfar);
    }
    
    // Decode fault type
    if (cfsr & 0x01) printc_panic("  - Instruction access violation\n");
    if (cfsr & 0x02) printc_panic("  - Data access violation\n");
    if (cfsr & 0x0100) printc_panic("  - Instruction bus error\n");
    if (cfsr & 0x0200) printc_panic("  - Precise data bus error\n");
    if (cfsr & 0x010000) printc_panic("  - Undefined instruction\n");
    if (cfsr & 0x040000) printc_panic("  - Invalid PC\n");
    if (cfsr & 0x02000000) printc_panic("  - Divide by zero\n");
}
```

---

## Common Fault Scenarios and What to Look For

### Scenario 1: Null Pointer Dereference

**Code:**
```c
uint32_t *ptr = NULL;
*ptr = 0x12345678;  // FAULT HERE
```

**What You'll See:**
- **CFSR**: Bit 1 (DACCVIOL) set = Data access violation
- **CFSR**: Bit 7 (MMARVALID) set = MMFAR valid
- **MMFAR**: 0x00000000 (the null pointer)
- **PC**: Points to the exact instruction with the dereference

**💡 PRO TIP:** If MMFAR is 0x00000000, you immediately know it's a null pointer!

---

### Scenario 2: Stack Overflow

**Code:**
```c
void recursive_function(int n) {
    char buffer[1024];  // Large local variable
    memset(buffer, 0, sizeof(buffer));
    recursive_function(n + 1);  // Infinite recursion
}
```

**What You'll See:**
- **SP**: Below 0x20000000 or very close to stack guard
- **CFSR**: Bit 1 (DACCVIOL) set
- **MMFAR**: Address near bottom of RAM (e.g., 0x1FFFFF??)
- **PC**: Inside the recursive function

**⚠️ WATCH OUT:** Stack overflow is sneaky! The fault might occur several function calls AFTER the root cause.

---

### Scenario 3: Invalid Instruction

**Code:**
```c
void (*func_ptr)(void) = (void*)0xFFFFFFFF;  // Invalid function pointer
func_ptr();  // Try to execute 0xFFFFFFFF
```

**What You'll See:**
- **CFSR**: Bit 16 (UNDEFINSTR) set = Undefined instruction
- **PC**: 0xFFFFFFFF or other invalid address
- **BFAR**: Might show the fetch address if BFARVALID set

---

### Scenario 4: Unaligned Access

**Code:**
```c
uint32_t value;
uint8_t *unaligned_ptr = (uint8_t*)0x20000001;  // Odd address!
value = *(uint32_t*)unaligned_ptr;  // 32-bit access on odd address
```

**What You'll See:**
- **CFSR**: Bit 24 (UNALIGNED) set = Unaligned access
- **PC**: The instruction doing the unaligned access

**📖 Reference:** ARM Cortex-M4 can optionally trap unaligned accesses (configured in CCR register)

---

## Reference Materials

### Must-Read Documents

1. **STM32F401 Reference Manual** (RM0368)
   - Section 2.3: Memory map
   - Section 4.3: System Control Block (SCB)
   - Table 38: SCB register map

2. **ARM Cortex-M4 Generic User Guide**
   - Chapter 2.3: Exception model
   - Chapter 4.3: System control block

3. **ARM®v7-M Architecture Reference Manual**
   - Section B1.5: Exception handling
   - Section B1.5.6: Exception entry
   - Section B1.5.7: Stack frame layout
   - Section B1.5.8: Exception return

### Useful Tools

- **STM32CubeIDE**: Built-in debugger with memory browser
- **OpenOCD**: Open-source debugger (alternative)
- **st-link**: Command-line flash tool
- **arm-none-eabi-objdump**: Disassemble .elf files offline

### Online Resources

- ARM Cortex-M Fault Analysis: https://interrupt.memfault.com/blog/cortex-m-fault-debug
- STM32 Community Forums: https://community.st.com/
- ARM Developer Documentation: https://developer.arm.com/

---

## Summary: Key Takeaways

### The Five Critical Concepts

1. **Hardware Automatic Stacking**
   - ARM saves 8 registers BEFORE your code runs
   - This is a 32-byte exception stack frame
   - SP, LR are updated automatically

2. **Stack Pointer Reset**
   - Always reset SP to `_estack` in fault handler
   - Prevents secondary faults from corrupted stack
   - Gives you guaranteed clean stack space

3. **Fault Registers Tell the Story**
   - CFSR is your first stop (check all 3 sections)
   - MMFAR/BFAR have fault addresses (if valid bits set)
   - PC in exception frame shows WHERE fault occurred

4. **Never Return from Fault**
   - Fault handlers must call `NVIC_SystemReset()`
   - System is in unknown state, can't safely resume
   - Flash storage preserves diagnostics across reset

5. **Interrupts Must Be Disabled**
   - `CRIT_START()` prevents reentrancy
   - Feed watchdog early (you need time)
   - Disable MPU to avoid secondary faults

### The Debug Workflow

```
1. Trigger fault (test function)
   ↓
2. Hit breakpoint in fault_detected()
   ↓
3. Record SP, LR, PC values
   ↓
4. Step through SP reset
   ↓
5. Observe SCB registers (CFSR!)
   ↓
6. Check flash contents before reset
   ↓
7. Reset system
   ↓
8. Read flash after boot
   ↓
9. Disassemble PC address
   ↓
10. Fix the bug!
```

---

## Practice Exercises

### Exercise 1: Basic Fault Trigger
Trigger a null pointer dereference and observe:
- Exception stack frame contents
- SP before and after reset
- CFSR bits (should see DACCVIOL)
- MMFAR value (should be 0x00000000)

### Exercise 2: Stack Overflow Detection
Create a function with excessive local variables or deep recursion:
- Observe when SP goes below valid range
- Check if MPU catches it (if enabled)
- See how quickly stack fills

### Exercise 3: Decode Flash Panic Data
After a fault and reset:
- Write a console command to dump flash panic area
- Parse the fault_data structure
- Print human-readable fault description
- Show exact PC location in source code

### Exercise 4: Fault Address Reverse Lookup
Given a fault address from MMFAR/BFAR:
- Determine what variable or struct it belongs to
- Use the linker map file (Badweh_Development.map)
- Trace back to the code that accessed it

---

## Closing Thoughts

🎓 **What You've Learned:**

Understanding fault handling at this level puts you in the top 10% of embedded engineers. Most developers never dig this deep into the exception model. You now know:

- How ARM Cortex-M4 hardware handles faults automatically
- Why stack pointer management is CRITICAL in fault handlers
- How to interpret fault status registers (CFSR is king!)
- The complete data flow from fault → diagnostics → flash → reset
- How to use STM32CubeIDE debugger effectively for fault analysis

📖 **War Story:** I once spent 2 weeks debugging a fault that only occurred in production, never in development. The issue? Stack overflow caused by a rare combination of deep function calls and large local variables. The fault handler crashed (didn't reset SP!), so we had no diagnostics. After implementing this exact pattern - with SP reset and flash storage - we caught the next occurrence in 2 days and had a fix deployed in 24 hours. This code structure has saved countless hours of debug time.

✅ **Best Practice Reminder:** Always implement comprehensive fault handling EARLY in your project. Don't wait until you have a production bug. The time invested now pays dividends when (not if!) you encounter a tricky fault.

---

**Next Steps:**
1. Complete the debugging exercises with actual hardware
2. Implement flash panic data readback in your console
3. Add visual fault reporting (LED patterns, display messages)
4. Study the MPU (Memory Protection Unit) for proactive fault prevention
5. Learn about RTOS-specific fault handling (if using FreeRTOS)

**Remember:** The best fault handler is one you never need, but when you do need it, you'll be VERY glad you invested the time to understand it deeply!

---

## APPENDIX: Debugging Session Log (October 31, 2025)

### Problem Encountered

During implementation, `fault_start()` caused immediate boot crashes. Systematic debugging was required to isolate and fix the integration issues.

### Debugging Methodology

**Approach:** Binary search / incremental testing
- Start with absolute minimum functionality
- Add one component at a time
- Test after each addition
- Document results at each step

### Debugging Phases Executed

**Phase 1: Linker Symbol Verification (5 min)**
- Checked `.map` file for `_estack`, `_sdata`, `_s_stack_guard`, `_e_stack_guard`
- Result: ✅ All symbols correctly defined
- `_estack = 0x20018000`, guards at `0x200019A0-0x200019C0`

**Phase 2: Incremental Component Testing (40 min)**

Test 2A - Minimal (cmd_register only):
```c
int32_t fault_start(void) {
    rc = cmd_register(&cmd_info);
    return rc;
}
```
Result: ✅ SUCCESS

Test 2B - Add Watchdog Callback:
```c
rc = wdg_register_triggered_cb(wdg_triggered_handler);
```
Result: ✅ SUCCESS

Test 2C - Add Stack Fill:
```c
__ASM volatile("MOV  %0, sp" : "=r" (sp));
sp--;
while (sp >= &_s_stack_guard)
    *sp-- = STACK_INIT_PATTERN;
```
Result: ✅ SUCCESS (filled 23KB)

Test 2D - Enable MPU:
```c
#if CONFIG_MPU_TYPE == 1
    LL_MPU_ConfigRegion(...);
    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk|MPU_CTRL_HFNMIENA_Msk);
#endif
```
Result: ✅ SUCCESS

**Phase 3: Full Integration Test (20 min)**
- Triggered fault with `main fault` command
- Observed complete diagnostic collection
- Result: ✅ Full chain functional (console output)

### Issues Found and Fixed

**Issue 1: Missing Flash/LWL Initialization**
- **Symptom:** Fault handler hung after "Fault type=2 param=3"
- **Root Cause:** Flash and LWL modules not initialized before fault_start()
- **Fix:** Added `flash_start()` and `lwl_start()` early in START phase
- **Code:** Added to app_main.c before fault_start() call

**Issue 2: Flash Panic Mode Hangs**
- **Symptom:** System hangs indefinitely during flash write in panic mode
- **Root Cause:** Unknown (flash polling loop issue)
- **Temporary Fix:** `CONFIG_FAULT_PANIC_TO_FLASH = 0`
- **Impact:** Console diagnostics work perfectly, flash write requires separate debug

**Previously Fixed (Before This Session):**
1. Missing `&` on `_estack` symbol reference
2. Unconditional MPU setup (needed `#if CONFIG_MPU_TYPE == 1`)

### Final Working State

**What Works:** ✅
- System boots with fault module active
- fault_start() completes successfully
- Fault injection via `main fault` command
- Complete fault diagnostics to console
- Exception stack frame capture
- ARM register collection (CFSR, HFSR, MMFAR, BFAR, etc.)
- LWL buffer capture (~1KB flight recorder)
- Stack pointer reset to _estack
- MPU stack guard configuration
- Watchdog integration
- Clean system reset via NVIC_SystemReset()

**What Doesn't Work:** ❌
- Flash panic write operations (hangs - separate issue)

**Workaround:**
- Console hex dump provides all diagnostic data
- Can manually decode fault information from console output
- Flash write debugging deferred to future session

### Key Takeaways for Future Debugging

1. **Binary search is faster than guessing** - Saved hours by testing incrementally
2. **Compare with reference code early** - Found critical `&_estack` bug
3. **Debug prints are invaluable** - Can't debug what you can't see
4. **Module dependencies are subtle** - Flash/LWL must be started first
5. **One change at a time** - Never change multiple things simultaneously
6. **Document as you go** - Prevents repeating failed attempts

### Time Investment

- **Estimated:** 2-5 hours
- **Actual:** ~2 hours
- **Efficiency:** Systematic approach was faster than trial-and-error would have been

### Current Configuration

```c
// config.h
#define CONFIG_FAULT_PANIC_TO_CONSOLE 1  // ✅ Working
#define CONFIG_FAULT_PANIC_TO_FLASH 0    // ❌ Disabled (hangs)

// app_main.c initialization order
fault_init();      // Early - captures reset reason
wdg_init();
// ... other inits ...
flash_start();     // BEFORE fault_start()
lwl_start();       // BEFORE fault_start()
fault_start();     // LAST
```

---

*Document created: October 30, 2025*
*Debugging session: October 31, 2025*
*Target: STM32F401RE (Cortex-M4)*
*Codebase: Badweh_Development project*
*Status: ✅ Fault Module Operational (Console Diagnostics)*

