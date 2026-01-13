/*******************************************************************************
 * fault_simple.c - Educational STM32F401xE Fault Handler
 *
 * PURPOSE:
 *   This single-file demonstration shows the complete fault handling flow for
 *   the "fault test ptr" command. It combines functionality from fault.c,
 *   flash.c, lwl.c, and other modules into one readable file for educational
 *   purposes.
 *
 * EXECUTION FLOW:
 *
 *   1. main() initializes SysTick and LWL buffer
 *   2. main() writes to invalid address 0xffffffff
 *   3. CPU triggers Hard Fault exception
 *   4. Default_Handler (in startup code) calls fault_exception_handler()
 *   5. fault_exception_handler() collects CPU context (registers, stack frame)
 *   6. fault_common_handler() writes data to flash:
 *      - Fault data (80 bytes) at 0x08004000
 *      - LWL buffer (1024 bytes) at 0x08004050
 *      - End marker (8 bytes) at 0x08004450
 *   7. NVIC_SystemReset() resets the system
 *   8. Fault data persists in flash for post-mortem debugging
 *
 * TARGET HARDWARE:
 *   STM32F401xE (ARM Cortex-M4, 512KB flash, 96KB RAM)
 *
 * FLASH MEMORY LAYOUT:
 *   Sector 1 (0x08004000 - 0x08007FFF, 16KB):
 *   ┌─────────────────────────────────────────┐
 *   │ 0x08004000: struct fault_data (80 bytes)│
 *   ├─────────────────────────────────────────┤
 *   │ 0x08004050: struct lwl_data (1024 bytes)│
 *   ├─────────────────────────────────────────┤
 *   │ 0x08004450: struct end_marker (8 bytes) │
 *   └─────────────────────────────────────────┘
 *   Total used: 1112 bytes of 16KB available
 *
 * COMPILATION:
 *   arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -DSTM32F401xE \
 *     -I<CMSIS_PATH>/Core/Include \
 *     -I<CMSIS_PATH>/Device/ST/STM32F4xx/Include \
 *     -O0 -g3 -c fault_simple.c -o fault_simple.o
 *
 *   Link with existing startup_stm32f401retx.s and linker script.
 *
 * EDUCATIONAL GOALS:
 *   - Understand ARM Cortex-M4 exception handling
 *   - Learn STM32F4 flash erase/write operations
 *   - See how fault data is preserved across resets
 *   - Understand circular buffer implementation
 *   - Learn about fault status registers (CFSR, HFSR, etc.)
 *
 ******************************************************************************/

#include <stdint.h>
#include <string.h>
#include "stm32f401xe.h"   // STM32F401xE device-specific definitions (MUST be first - defines __FPU_PRESENT, __NVIC_PRIO_BITS, IRQn_Type)
#include "core_cm4.h"      // CMSIS Cortex-M4 definitions (SCB, SysTick, NVIC)

/*******************************************************************************
 * HARDWARE CONSTANTS
 ******************************************************************************/

// Flash memory constants
#define FLASH_SECTOR1_ADDR  0x08004000   // Sector 1 base address (16KB)
#define FLASH_KEY1          0x45670123   // First unlock key
#define FLASH_KEY2          0xCDEF89AB   // Second unlock key

// Magic numbers for data identification
#define MAGIC_FAULT         0xdead0001   // Identifies fault data section
#define MAGIC_LWL           0xf00d0001   // Identifies LWL data section
#define MAGIC_END           0xc0da0001   // Marks end of fault data

// Buffer sizes
#define LWL_BUF_SIZE        1008         // LWL circular buffer size (bytes)

// SysTick configuration (16MHz HSI clock)
#define SYSTICK_RELOAD      15999        // For 1ms tick: 16MHz / 16000 = 1kHz

/*******************************************************************************
 * DATA STRUCTURES
 ******************************************************************************/

/*
 * Fault Data Structure (80 bytes, 8-byte aligned for STM32F401xE flash)
 *
 * This structure captures the complete CPU state at the time of the fault.
 * It includes the exception stack frame automatically saved by the ARM
 * Cortex-M4 hardware, plus additional ARM fault status registers.
 */
struct fault_data {
    uint32_t magic;                  // 0xdead0001 - identifies this section
    uint32_t num_section_bytes;      // sizeof(struct fault_data) = 80

    uint32_t fault_type;             // 1 = EXCEPTION
    uint32_t fault_param;            // IPSR value (exception number)

    // Exception stack frame (automatically saved by CPU on fault)
    // These 8 registers are pushed to the stack by hardware
    uint32_t excpt_stk_r0;           // R0 register
    uint32_t excpt_stk_r1;           // R1 register
    uint32_t excpt_stk_r2;           // R2 register
    uint32_t excpt_stk_r3;           // R3 register
    uint32_t excpt_stk_r12;          // R12 register
    uint32_t excpt_stk_lr;           // LR before exception
    uint32_t excpt_stk_rtn_addr;     // PC where fault occurred
    uint32_t excpt_stk_xpsr;         // PSR at time of fault

    uint32_t sp;                     // Stack pointer at fault
    uint32_t lr;                     // LR in handler (EXC_RETURN value)

    // ARM Cortex-M4 fault status registers
    // These registers provide detailed information about what caused the fault
    uint32_t ipsr;                   // Interrupt Program Status Register
    uint32_t icsr;                   // Interrupt Control State Register
    uint32_t shcsr;                  // System Handler Control State Register
    uint32_t cfsr;                   // Configurable Fault Status Register
    uint32_t hfsr;                   // Hard Fault Status Register
    uint32_t mmfar;                  // MemManage Fault Address Register
    uint32_t bfar;                   // Bus Fault Address Register
    uint32_t tick_ms;                // Timestamp in milliseconds
};  // Total: 20 fields × 4 bytes = 80 bytes

/*
 * LWL (Last Words Log) Data Structure (1024 bytes, 8-byte aligned)
 *
 * This structure implements a circular buffer for lightweight logging.
 * When the buffer fills, new entries overwrite the oldest ones, ensuring
 * we always have the most recent log data before a fault.
 */
struct lwl_data {
    uint32_t magic;                  // 0xf00d0001 - identifies this section
    uint32_t num_section_bytes;      // sizeof(struct lwl_data) = 1024
    uint32_t buf_size;               // 1008 bytes
    uint32_t put_idx;                // Current write position (wraps at 1008)
    uint8_t buf[LWL_BUF_SIZE];       // Circular buffer for log entries
};  // Total: 16 + 1008 = 1024 bytes

/*
 * End Marker Structure (8 bytes, 8-byte aligned)
 *
 * This marks the end of valid fault data in flash. Recovery software can
 * read the magic numbers sequentially to parse all fault data sections.
 */
struct end_marker {
    uint32_t magic;                  // 0xc0da0001 - marks end of data
    uint32_t num_section_bytes;      // sizeof(struct end_marker) = 8
};  // Total: 8 bytes

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

// Fault data buffer (populated during fault handling, written to flash)
struct fault_data fault_data_buf;

// LWL circular buffer (populated during normal operation, written to flash on fault)
struct lwl_data lwl_data;

// SysTick millisecond counter (incremented every 1ms by SysTick_Handler)
volatile uint32_t tick_ms_ctr = 0;

/*******************************************************************************
 * FUNCTION DECLARATIONS
 ******************************************************************************/

// Forward declarations (needed because C requires functions to be declared before use)
void fault_common_handler(void);

/*******************************************************************************
 * FLASH OPERATIONS
 *
 * STM32F401xE flash memory is organized in sectors:
 *   Sector 0: 0x08000000 (16KB) - Application code
 *   Sector 1: 0x08004000 (16KB) - Fault data storage ← We use this
 *   Sector 2: 0x08008000 (16KB)
 *   ...
 *
 * Flash operations must follow specific sequences:
 *   1. Unlock with two keys
 *   2. Erase entire sector before writing
 *   3. Write 8 bytes at a time (64-bit words)
 *   4. Wait for busy flag to clear between operations
 ******************************************************************************/

/*
 * flash_unlock() - Unlock flash for erase/write operations
 *
 * Why two keys?
 *   The two-key unlock sequence prevents accidental flash modifications.
 *   Both keys must be written in sequence to FLASH->KEYR register.
 *   The LOCK bit in FLASH->CR is automatically cleared when unlocked.
 */
void flash_unlock(void) {
    if (FLASH->CR & FLASH_CR_LOCK) {
        FLASH->KEYR = FLASH_KEY1;    // Write first key
        FLASH->KEYR = FLASH_KEY2;    // Write second key
    }
}

/*
 * flash_erase_sector1() - Erase flash sector 1 (0x08004000, 16KB)
 *
 * STM32F401xE sector erase sequence:
 *   1. Wait for any previous operation to complete (BSY = 0)
 *   2. Disable instruction cache (prevents reading stale data)
 *   3. Clear error flags from previous operations
 *   4. Set sector number (1) in SNB field and set SER bit
 *   5. Set STRT bit to begin erase
 *   6. Wait for erase to complete (BSY = 0)
 *   7. Clear SER bit
 *   8. Flush and re-enable instruction cache
 *
 * Why disable cache?
 *   The instruction cache might contain data from flash that we're erasing.
 *   We must disable it to prevent executing stale instructions.
 */
void flash_erase_sector1(void) {
    // Wait for any previous flash operation to complete
    while (FLASH->SR & FLASH_SR_BSY) {}

    // Disable instruction cache during flash operation
    FLASH->ACR &= ~FLASH_ACR_ICEN;

    // Clear error flags (write 1 to clear)
    // Flags: WRPERR, PGAERR, PGPERR, PGSERR, RDERR
    FLASH->SR |= 0xF3;

    // Set sector number (1) in SNB field (bits 3-6) and set sector erase bit
    FLASH->CR = (FLASH->CR & ~FLASH_CR_SNB) |
                (1 << FLASH_CR_SNB_Pos) |
                FLASH_CR_SER;

    // Start the erase operation
    FLASH->CR |= FLASH_CR_STRT;

    // Wait for erase to complete (blocking)
    // Erasing 16KB sector takes ~500ms typically
    while (FLASH->SR & FLASH_SR_BSY) {}

    // Clear sector erase bit
    FLASH->CR &= ~FLASH_CR_SER;

    // Flush instruction cache (reset cache contents)
    FLASH->ACR |= FLASH_ACR_ICRST;
    FLASH->ACR &= ~FLASH_ACR_ICRST;

    // Re-enable instruction cache
    FLASH->ACR |= FLASH_ACR_ICEN;
}

/*
 * flash_write_64bit() - Write 8 bytes (64 bits) to flash
 *
 * STM32F401xE flash write requirements:
 *   - Must write exactly 8 bytes at a time (two 32-bit words)
 *   - Flash address must be 8-byte aligned
 *   - Data address must be 4-byte aligned
 *   - PSIZE must be set to 2 (32-bit parallelism for 64-bit writes)
 *
 * Why 8-byte writes?
 *   STM32F4 flash memory cells are organized in 64-bit words. The hardware
 *   only supports writing 8 bytes at a time. Attempting smaller writes will
 *   cause a programming sequence error (PGSERR).
 *
 * Parameters:
 *   dest: Flash address (must be 8-byte aligned)
 *   src:  RAM address containing data (must be 4-byte aligned)
 */
void flash_write_64bit(uint32_t* dest, uint32_t* src) {
    // Wait for any previous flash operation to complete
    while (FLASH->SR & FLASH_SR_BSY) {}

    // Set programming bit and 32-bit parallelism (for 64-bit writes)
    // PSIZE = 2 means x32 parallelism (write two 32-bit words = 64 bits)
    FLASH->CR = (FLASH->CR & ~FLASH_CR_PSIZE) |
                (2 << FLASH_CR_PSIZE_Pos) |
                FLASH_CR_PG;

    // Write two 32-bit words (8 bytes total)
    // These two writes must be consecutive to form a valid 64-bit write
    *dest++ = *src++;    // Write first word
    *dest++ = *src++;    // Write second word

    // Wait for programming to complete
    // Programming 8 bytes takes ~16µs typically
    while (FLASH->SR & FLASH_SR_BSY) {}

    // Clear programming bit
    FLASH->CR &= ~FLASH_CR_PG;
}

/*
 * flash_write_buffer() - Write arbitrary buffer to flash
 *
 * This function writes a buffer to flash by calling flash_write_64bit()
 * repeatedly for each 8-byte chunk.
 *
 * Parameters:
 *   flash_addr: Destination in flash (must be 8-byte aligned)
 *   data:       Source data in RAM (must be 4-byte aligned)
 *   num_bytes:  Number of bytes to write (must be multiple of 8)
 */
void flash_write_buffer(uint32_t* flash_addr, uint32_t* data, uint32_t num_bytes) {
    // Write 8 bytes at a time
    for (; num_bytes > 0; num_bytes -= 8) {
        flash_write_64bit(flash_addr, data);
        flash_addr += 2;    // Advance by 8 bytes (2 words)
        data += 2;          // Advance by 8 bytes (2 words)
    }
}

/*******************************************************************************
 * LWL (LAST WORDS LOG) OPERATIONS
 *
 * The LWL module provides a lightweight circular buffer for logging events.
 * It's designed to capture the most recent system activity before a fault.
 *
 * Circular Buffer Logic:
 *   - Buffer size: 1008 bytes
 *   - When put_idx reaches 1008, it wraps back to 0
 *   - New entries overwrite the oldest entries
 *   - This ensures we always have the most recent 1008 bytes of logs
 ******************************************************************************/

/*
 * lwl_init() - Initialize the LWL circular buffer
 *
 * This sets up the LWL buffer metadata and clears the buffer contents.
 */
void lwl_init(void) {
    lwl_data.magic = MAGIC_LWL;              // 0xf00d0001
    lwl_data.num_section_bytes = sizeof(lwl_data);  // 1024
    lwl_data.buf_size = LWL_BUF_SIZE;        // 1008
    lwl_data.put_idx = 0;                    // Start at beginning
    memset(lwl_data.buf, 0, LWL_BUF_SIZE);   // Clear buffer
}

/*
 * lwl_record() - Record a single byte to the circular buffer
 *
 * In a full implementation, this would support variable-length entries with
 * arguments. For simplicity, this version just records a single ID byte.
 *
 * Circular buffer behavior:
 *   - idx = put_idx % 1008 calculates the actual buffer position
 *   - put_idx increments past 1008, but we use modulo to wrap
 *   - Example: put_idx=1010 → idx=2 (wrapped around)
 *
 * Parameters:
 *   id: Event identifier byte to record
 */
void lwl_record(uint8_t id) {
    uint32_t idx = lwl_data.put_idx % LWL_BUF_SIZE;
    lwl_data.buf[idx] = id;
    lwl_data.put_idx = (idx + 1) % LWL_BUF_SIZE;
}

/*******************************************************************************
 * SYSTICK TIMER
 *
 * The SysTick timer provides a 1ms periodic tick for timestamping.
 *
 * Clock configuration:
 *   - STM32F401xE powers up with 16MHz HSI (High Speed Internal) oscillator
 *   - SysTick uses the processor clock (no prescaler)
 *   - For 1ms tick: 16MHz / 16000 = 1kHz = 1ms period
 *
 * SysTick registers:
 *   - LOAD: Reload value (counts from this down to 0)
 *   - VAL:  Current value (cleared on write)
 *   - CTRL: Control register (enable, interrupt enable, clock source)
 ******************************************************************************/

/*
 * systick_init() - Initialize SysTick for 1ms tick
 *
 * CTRL register bits:
 *   bit 0: ENABLE     - Enable counter
 *   bit 1: TICKINT    - Enable interrupt
 *   bit 2: CLKSOURCE  - 1 = processor clock, 0 = external clock
 */
void systick_init(void) {
    SysTick->LOAD = SYSTICK_RELOAD;          // 16000 - 1 = 15999
    SysTick->VAL = 0;                        // Clear current value
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |  // Use processor clock
                    SysTick_CTRL_TICKINT_Msk |     // Enable interrupt
                    SysTick_CTRL_ENABLE_Msk;       // Enable SysTick
}

/*
 * SysTick_Handler() - SysTick interrupt handler (called every 1ms)
 *
 * This function is called automatically by hardware when SysTick counts to 0.
 * It's defined as a weak symbol in the startup code, so our definition here
 * overrides it.
 *
 * IMPORTANT: Keep this handler short and fast. It runs every 1ms.
 */
void SysTick_Handler(void) {
    tick_ms_ctr++;
}

/*******************************************************************************
 * FAULT HANDLERS
 *
 * ARM Cortex-M4 Exception Handling:
 *
 * When a fault occurs (like our bad pointer write to 0xffffffff), the CPU:
 *   1. Automatically saves R0-R3, R12, LR, PC, xPSR to the stack
 *   2. Sets LR to a special EXC_RETURN value
 *   3. Loads PC from the exception vector table
 *   4. Begins executing the exception handler
 *
 * Exception Stack Frame (32 bytes):
 *   SP+0:  R0  - Argument/result register
 *   SP+4:  R1  - Argument register
 *   SP+8:  R2  - Argument register
 *   SP+12: R3  - Argument register
 *   SP+16: R12 - Intra-procedure-call scratch register
 *   SP+20: LR  - Link register before exception
 *   SP+24: PC  - Program counter where fault occurred ← Shows what instruction caused the fault
 *   SP+28: xPSR - Program Status Register
 *
 * The startup code's Default_Handler passes SP to us as an argument so we
 * can copy this exception stack frame for debugging.
 ******************************************************************************/

/*
 * fault_exception_handler() - Exception handler called from startup code
 *
 * This function is called by Default_Handler in startup_stm32f401retx.s:
 *   Default_Handler:
 *       mov  r0, sp              ; Save current SP to R0 (argument)
 *       ldr  sp, =_estack        ; Reset SP to top of stack (safety)
 *       b    fault_exception_handler
 *
 * The SP reset is important because if the fault was caused by stack overflow,
 * continuing to use the faulted stack would cause another fault.
 *
 * Parameters:
 *   sp: Stack pointer at the time of exception (points to exception frame)
 *
 * EXC_RETURN value in LR:
 *   This special value tells the CPU how to return from exception:
 *     0xFFFFFFF1: Return to Handler mode, use MSP
 *     0xFFFFFFF9: Return to Thread mode, use MSP
 *     0xFFFFFFFD: Return to Thread mode, use PSP
 *   We save this for debugging (shows execution mode when fault occurred).
 */
void fault_exception_handler(uint32_t sp) {
    // Set fault type to EXCEPTION (value: 1)
    fault_data_buf.fault_type = 1;

    // Get exception number from IPSR
    // Common values:
    //   3 = HardFault
    //   4 = MemManage
    //   5 = BusFault
    //   6 = UsageFault
    fault_data_buf.fault_param = __get_IPSR();

    // Get LR register (contains EXC_RETURN value)
    __ASM volatile("MOV %0, lr" : "=r" (fault_data_buf.lr) : : "memory");

    // Store stack pointer
    fault_data_buf.sp = sp;

    // Copy exception stack frame if SP looks valid
    // Check: SP is 8-byte aligned (bottom 3 bits are 0)
    if ((sp & 0x7) == 0) {
        // Copy 32 bytes (8 registers × 4 bytes)
        memcpy(&fault_data_buf.excpt_stk_r0, (uint8_t*)sp, 32);
    } else {
        // SP is invalid, zero the exception frame
        memset(&fault_data_buf.excpt_stk_r0, 0, 32);
    }

    // Call common handler (will not return)
    fault_common_handler();
}

/*
 * fault_common_handler() - Common fault handling and flash write
 *
 * This function:
 *   1. Populates the fault_data_buf with CPU state
 *   2. Erases flash sector 1
 *   3. Writes fault data to flash
 *   4. Writes LWL buffer to flash
 *   5. Writes end marker to flash
 *   6. Resets the system
 *
 * IMPORTANT: This function will NOT return. The system will reset.
 *
 * Flash write layout:
 *   0x08004000 + 0    = 0x08004000: fault_data (80 bytes)
 *   0x08004000 + 80   = 0x08004050: lwl_data (1024 bytes)
 *   0x08004000 + 1104 = 0x08004450: end_marker (8 bytes)
 */
void fault_common_handler(void) {
    struct end_marker end;

    // Populate fault data buffer with ARM Cortex-M4 registers
    fault_data_buf.magic = MAGIC_FAULT;      // 0xdead0001
    fault_data_buf.num_section_bytes = sizeof(fault_data_buf);  // 80

    // Collect ARM Cortex-M4 fault status registers
    // These registers provide detailed information about the fault
    fault_data_buf.ipsr = __get_IPSR();      // Exception number
    fault_data_buf.icsr = SCB->ICSR;         // Pending interrupts, active exceptions
    fault_data_buf.shcsr = SCB->SHCSR;       // System handler enables and active flags
    fault_data_buf.cfsr = SCB->CFSR;         // Configurable Fault Status (MemManage, BusFault, UsageFault)
    fault_data_buf.hfsr = SCB->HFSR;         // Hard Fault Status (shows if fault escalated)
    fault_data_buf.mmfar = SCB->MMFAR;       // MemManage Fault Address (if valid)
    fault_data_buf.bfar = SCB->BFAR;         // Bus Fault Address (if valid) ← Should contain 0xffffffff
    fault_data_buf.tick_ms = tick_ms_ctr;    // Timestamp in milliseconds

    /*
     * CFSR Register Breakdown (shows specific fault cause):
     *   Bits 0-7:   MemManage Fault Status (MMFSR)
     *   Bits 8-15:  Bus Fault Status (BFSR) ← Our fault will set IMPRECISERR or PRECISERR
     *   Bits 16-31: Usage Fault Status (UFSR)
     *
     * For our 0xffffffff write, we expect BFSR to show a bus fault.
     */

    // Unlock flash for erase/write operations
    flash_unlock();

    // Erase sector 1 (this takes ~500ms)
    flash_erase_sector1();

    // Write fault data (80 bytes = 10 × 8-byte writes)
    flash_write_buffer((uint32_t*)0x08004000,
                      (uint32_t*)&fault_data_buf,
                      80);

    // Write LWL buffer (1024 bytes = 128 × 8-byte writes)
    flash_write_buffer((uint32_t*)0x08004050,
                      (uint32_t*)&lwl_data,
                      1024);

    // Write end marker (8 bytes = 1 × 8-byte write)
    end.magic = MAGIC_END;                   // 0xc0da0001
    end.num_section_bytes = sizeof(end);     // 8
    flash_write_buffer((uint32_t*)0x08004450,
                      (uint32_t*)&end,
                      8);

    // Reset the system
    // After reset, the fault data will persist in flash and can be read
    // by recovery software or a debugger.
    NVIC_SystemReset();

    // Should never reach here (system reset does not return)
    while(1);
}

/*******************************************************************************
 * MAIN FUNCTION
 *
 * This simulates the "fault test ptr" command by directly writing to an
 * invalid address. In the full system, this would be triggered by the
 * console command handler, but here we simplify by calling it directly.
 ******************************************************************************/

/*
 * main() - Entry point
 *
 * Execution sequence:
 *   1. Initialize SysTick for 1ms ticks
 *   2. Initialize LWL circular buffer
 *   3. Record some sample LWL entries
 *   4. Delay to accumulate some tick counts
 *   5. Trigger fault by writing to 0xffffffff
 *   6. [System faults, handler writes to flash, system resets]
 *
 * After reset, you can read fault data from flash at 0x08004000 using:
 *   - Debugger memory view
 *   - Custom recovery software
 *   - ST-LINK utility
 */
int main(void) {
    // Initialize SysTick timer for timestamps
    systick_init();

    // Initialize LWL buffer
    lwl_init();

    // Record some sample events (for demonstration)
    // In a real system, these would be logged during normal operation
    lwl_record(0x10);    // Example event ID 0x10
    lwl_record(0x20);    // Example event ID 0x20
    lwl_record(0x30);    // Example event ID 0x30

    // Delay loop to accumulate some tick counts
    // This allows SysTick_Handler to run and increment tick_ms_ctr
    // After this loop, tick_ms_ctr should be several milliseconds
    for (volatile int i = 0; i < 1000000; i++) {
        // Empty loop - just burning time
    }

    // TRIGGER FAULT: Write to invalid address
    // This simulates the "fault test ptr" command from fault.c:726
    //
    // Why does this cause a fault?
    //   Address 0xffffffff is not mapped to any valid memory region.
    //   The CPU will attempt to write to the system bus, but no peripheral
    //   will respond (bus error). This triggers a BusFault exception.
    //
    // What happens next?
    //   1. CPU saves R0-R3, R12, LR, PC, xPSR to stack
    //   2. CPU loads PC from BusFault vector (or HardFault if BusFault disabled)
    //   3. Execution jumps to Default_Handler in startup code
    //   4. Default_Handler branches to fault_exception_handler()
    //   5. Fault data is written to flash
    //   6. System resets
    *((uint32_t*)0xffffffff) = 0xbad;

    // Should never reach here (fault handler does not return)
    while(1);
}

/*******************************************************************************
 * END OF FILE
 ******************************************************************************/
