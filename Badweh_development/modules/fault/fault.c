/*

 */

#include <stdint.h>
#include <string.h>

#include "config.h"
#include CONFIG_STM32_LL_CORTEX_HDR

#include "cmd.h"
#include "console.h"
#include "flash.h"
#include "log.h"
#include "lwl.h"
#include "module.h"
#include "tmr.h"
#include "wdg.h"

#include "fault.h"

////////////////////////////////////////////////////////////////////////////////
// Common macros
////////////////////////////////////////////////////////////////////////////////

#define STACK_INIT_PATTERN 0xcafebadd
#define STACK_GUARD_BLOCK_SIZE 32

// Address of last page of flash.
#if CONFIG_FLASH_TYPE == 1

#define FLASH_PANIC_DATA_ADDR ((uint8_t*)(CONFIG_FLASH_BASE_ADDR +  \
                                          CONFIG_FLASH_SIZE -       \
                                          CONFIG_FLASH_PAGE_SIZE))

#elif CONFIG_FLASH_TYPE == 2

#define FLASH_PANIC_DATA_ADDR ((uint8_t*)CONFIG_FAULT_FLASH_PANIC_ADDR)

#elif CONFIG_FLASH_TYPE == 3

#define FLASH_PANIC_DATA_ADDR ((uint8_t*)(CONFIG_FLASH_BASE_ADDR +  \
                                          CONFIG_FLASH_PAGE_SIZE))

#elif CONFIG_FLASH_TYPE == 4

#define FLASH_PANIC_DATA_ADDR ((uint8_t*)(CONFIG_FLASH_BASE_ADDR +  \
                                          CONFIG_FLASH_SIZE -       \
                                          CONFIG_FLASH_PAGE_SIZE))

#endif

#if CONFIG_MPU_TYPE == -1
    #define _s_stack_guard _sstack
    #define _e_stack_guard _sstack
#endif

////////////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////////////

// Data collected after a fault. For writing to flash, this structure needs
// to be a multiple of the flash write size (e.g. 8 or 16 bytes).

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

#define EXCPT_STK_BYTES (8*4)

_Static_assert((sizeof(struct fault_data) % CONFIG_FLASH_WRITE_BYTES) == 0,
               "Invalid struct fault_data");

struct end_marker {
    uint32_t magic;
    uint32_t num_section_bytes;

    #if CONFIG_FLASH_WRITE_BYTES == 16
        uint32_t pad1[2];
    #endif

};

_Static_assert((sizeof(struct end_marker) % CONFIG_FLASH_WRITE_BYTES) == 0,
               "Invalid struct end_marker");

////////////////////////////////////////////////////////////////////////////////
// Private (static) function declarations
////////////////////////////////////////////////////////////////////////////////

static void fault_common_handler(void);
static void record_fault_data(uint32_t data_offset, uint8_t* addr,
                              uint32_t num_bytes);
static void wdg_triggered_handler(uint32_t wdg_client_id);
static int32_t cmd_fault_data(int32_t argc, const char** argv);
static int32_t cmd_fault_status(int32_t argc, const char** argv);
static int32_t cmd_fault_test(int32_t argc, const char** argv);
static void test_overflow_stack(void);

////////////////////////////////////////////////////////////////////////////////
// Private (static) variables
////////////////////////////////////////////////////////////////////////////////

static struct fault_data fault_data_buf;

static int32_t log_level = LOG_DEFAULT;  

// Data structure with console command info.
static struct cmd_cmd_info cmds[] = {
    {
        .name = "data",
        .func = cmd_fault_data,
        .help = "Print/erase fault data, usage: fault data [erase]",
    },
    {
        .name = "status",
        .func = cmd_fault_status,
        .help = "Get module status, usage: fault status",
    },
    {
        .name = "test",
        .func = cmd_fault_test,
        .help = "Run test, usage: fault test [<op> [<arg>]] (enter no op for help)",
    }
};

// Data structure passed to cmd module for console interaction.
static struct cmd_client_info cmd_info = {
    .name = "fault",
    .num_cmds = ARRAY_SIZE(cmds),
    .cmds = cmds,
    .log_level_ptr = &log_level,
};

static uint32_t rcc_csr;
static bool got_rcc_csr = false;

////////////////////////////////////////////////////////////////////////////////
// Public (global) variables and externs
////////////////////////////////////////////////////////////////////////////////

// These symbols are defined in the linker script.
extern uint32_t _sdata;
extern uint32_t _estack;
extern uint32_t _s_stack_guard;
extern uint32_t _e_stack_guard;

////////////////////////////////////////////////////////////////////////////////
// Public (global) functions
////////////////////////////////////////////////////////////////////////////////

/*
 * @brief Initialize fault module instance.
 *
 * @param[in] cfg The fault configuration. (FUTURE)
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 *
 */
int32_t fault_init(struct fault_cfg* cfg)
{
    // TODO: Follow fault_handling_watchdog_progressive_reconstruction.md
    // Phase 2, Function 1: fault_init()
    // Question: What does fault_get_rcc_csr do?
    // Hint: Captures reset reason (watchdog, power-on, etc.)
    
	fault_get_rcc_csr(); // Capture reset reason NOW
    
    return 0;
}

/*
 * @brief Start fault module instance.
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 *
 * This function starts the fault singleton module, to enter normal operation.
 */
int32_t fault_start(void)
{
    int32_t rc;
    uint32_t* sp;

    rc = cmd_register(&cmd_info);
    if (rc < 0) {
        log_error("fault_start: cmd_register error %d\n", rc);
        return rc;
    }

    rc = wdg_register_triggered_cb(wdg_triggered_handler);
    if (rc != 0) {
        log_error("fault_start: wdg_register_triggered_cb returns %ld\n", rc);
        return rc;
    }

    // Fill stack with a pattern so we can detect high water mark.
    __ASM volatile("MOV  %0, sp" : "=r" (sp) : : "memory");
    sp--;
    while (sp >= &_s_stack_guard)
        *sp-- = STACK_INIT_PATTERN;

#if 0  // TEMPORARILY DISABLED MPU FOR DEBUGGING - Testing if MPU causes register corruption

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

#endif

    return 0;
}

/*
 * @brief Fault detected by software.
 *
 * @note This function will not return.
 *
 * This function is called by software that detects a fault, so if we get here,
 * the stack must be working to some extent. However, for safety we reset the
 * stack pointer (after saving the original value) before calling another
 * function.  We also save the current value of lr, which is often just after
 * the call to this function.
 */
void fault_detected(enum fault_type fault_type, uint32_t fault_param)
{
    // TODO: Follow fault_handling_watchdog_progressive_reconstruction.md
    // Phase 2, Function 3: fault_detected() - THE HEART

    // Question 1: Enter critical section (disable interrupts)
    // TODO: ???;
	CRIT_START();


    // Question 2: Feed hardware watchdog (buy time to collect data)
    wdg_feed_hdw();

    // Question 3: Disable MPU to avoid secondary faults)
    // TODO: ???();
    ARM_MPU_Disable();

    // Question 4: Start collecting fault data
     fault_data_buf.fault_type = fault_type;
     fault_data_buf.fault_param = fault_param;
     memset(&fault_data_buf.excpt_stk_r0, 0, EXCPT_STK_BYTES);

    // Question 5: Save current LR and SP registers (before resetting SP)
     __ASM volatile("MOV  %0, lr" : "=r" (fault_data_buf.lr) : : "memory");
     __ASM volatile("MOV  %0, sp" : "=r" (fault_data_buf.sp) : : "memory");

    // Question 6: Reset stack pointer to top of RAM
     __ASM volatile("MOV  sp, %0" : : "r" (&_estack) : "memory");  // CRITICAL: Use & to get address!

    // Question 7: Call common handler (collects more data, writes to flash, resets)
     fault_common_handler();
    
    (void)fault_type;    // Suppress unused parameter warning
    (void)fault_param;   // Suppress unused parameter warning
}

/*
 * @brief Unexpected exception treated as fault.
 *
 * @param[in] orig_sp The original sp register before it was reset.
 *
 * @note This function will not return.
 *
 * This function is jumped to (not called) by the initial exception handler. The
 * initial handler will have reset the stack pointer to ensure it is usable, and
 * passes the original stack pointer value to this handler.
 *
 * The exception stack is described in the ARM v7-M Architecture Reference
 * Manual. The relavent part is shown below:
 *
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
 *
 * This is useful information to collect, but we must ensure that that SP value
 * is valid, and that the memory is points to is valid RAM.
 *
 * Code in default handler modified to this:
 *
 *     Default_Handler:
 *         mov  r0, sp
 *	       b    fault_exception_handler
 *
 * These lines can be commented out.
 *
 *     Infinite_Loop:
 *         b    Infinite_Loop
 */
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

uint32_t fault_get_rcc_csr(void)
{
    if (!got_rcc_csr) {
        got_rcc_csr = true;
        rcc_csr = RCC->CSR;
        RCC->CSR |= RCC_CSR_RMVF_Msk;
    }
    return rcc_csr;
}

////////////////////////////////////////////////////////////////////////////////
// Private (static) functions
////////////////////////////////////////////////////////////////////////////////

/*
 * @brief Common fault handling.
 *
 * @note This function will not return.
 */
static void fault_common_handler()
{
    // TODO: Follow fault_handling_watchdog_progressive_reconstruction.md
    // Phase 2, Function 4: fault_common_handler()
    uint8_t* lwl_data;
    uint32_t lwl_num_bytes;
    struct end_marker end;

    // Question 1: Disable LWL (we're panicking, don't log anymore)
	lwl_enable(false);

    // Question 2: Print fault info to console
     printc_panic("\nFault type=%lu param=%lu\n",
                   fault_data_buf.fault_type,
                   fault_data_buf.fault_param);

    // Question 3: Set magic number (identifies this data structure)
     fault_data_buf.magic = MOD_MAGIC_FAULT;
     fault_data_buf.num_section_bytes = sizeof(fault_data_buf);

    // Question 4: Collect ARM system registers (detailed diagnostics)
    fault_data_buf.ipsr = __get_IPSR();
    fault_data_buf.icsr = SCB->ICSR;
    fault_data_buf.shcsr = SCB->SHCSR;
    fault_data_buf.cfsr = SCB->CFSR;
    fault_data_buf.hfsr = SCB->HFSR;
    fault_data_buf.mmfar = SCB->MMFAR;
    fault_data_buf.bfar = SCB->BFAR;
    fault_data_buf.tick_ms = tmr_get_ms();  // Hint: Get current time

    // Question 5: Get LWL buffer (flight recorder data)
    lwl_data = lwl_get_buffer(&lwl_num_bytes);

    // Question 6: Record fault data to flash (if enabled)
    record_fault_data(0, (uint8_t*)&fault_data_buf, sizeof(fault_data_buf));

    // Question 7: Record LWL buffer after fault data
    record_fault_data(sizeof(fault_data_buf), lwl_data, lwl_num_bytes);

    // Record end marker
    memset(&end, 0, sizeof(end));
    end.magic = MOD_MAGIC_END;
    end.num_section_bytes = sizeof(end);
    record_fault_data(sizeof(fault_data_buf) + lwl_num_bytes, (uint8_t*)&end,
                      sizeof(end));

    
    // Temporary: Reset system immediately (so function doesn't return)
    NVIC_SystemReset();
}

/*
 * @brief Record fault data.
 *
 * @param[in] data_offset The logical offset of this chunk of data.
 * @param[in] data_addr The location of the data. Must be on 8 byte boundary
 *                      if writing to flash.
 * @param[in] num_bytes The number of bytes of data. Must ba multiple of 8
 *                      if writing to flash.
 *
 * @note As we are in a panic, we tend to just ignore return codes and keep
 *       going.
 */
static void record_fault_data(uint32_t data_offset, uint8_t* data_addr,
                              uint32_t num_bytes)
{
#if CONFIG_FAULT_PANIC_TO_FLASH
    {
        static bool do_flash;
        int32_t rc;
        
        if (data_offset == 0) {
            printc_panic("[FLASH] Checking existing data...\n");
            do_flash = ((struct fault_data*)FLASH_PANIC_DATA_ADDR)->magic !=
                MOD_MAGIC_FAULT;
            printc_panic("[FLASH] do_flash=%d (addr=0x%08lx, magic=0x%08lx)\n", 
                        do_flash, (uint32_t)FLASH_PANIC_DATA_ADDR,
                        ((struct fault_data*)FLASH_PANIC_DATA_ADDR)->magic);
        }
        if (do_flash) {
            if (data_offset == 0) {
                printc_panic("[FLASH] About to ERASE page at 0x%08lx\n", 
                            (uint32_t)FLASH_PANIC_DATA_ADDR);
                rc = flash_panic_erase_page((uint32_t*)FLASH_PANIC_DATA_ADDR);
                printc_panic("[FLASH] Erase returned %ld\n", rc);
                if (rc != 0)
                    printc_panic("flash_panic_erase_page returns %ld\n", rc);
            }
            printc_panic("[FLASH] About to WRITE %lu bytes at offset %lu\n", 
                        num_bytes, data_offset);
            rc = flash_panic_write((uint32_t*)(FLASH_PANIC_DATA_ADDR +
                                               data_offset),
                                   (uint32_t*)data_addr, num_bytes);
            printc_panic("[FLASH] Write returned %ld\n", rc);
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

/*
 * @brief Callback from watchdog module in case of a trigger.
 *
 * @param[in] wdg_client_id The watchdog client that triggered.
 *
 * @note This function will not return.
 */
static void wdg_triggered_handler(uint32_t wdg_client_id)
{
    fault_detected(FAULT_TYPE_WDG, wdg_client_id);
}

/*
 * @brief Console command function for "fault data".
 *
 * @param[in] argc Number of arguments, including "fault"
 * @param[in] argv Argument values, including "fault"
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 *
 * Command usage: fault data [erase]
 */
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

/*
 * @brief Console command function for "fault status".
 *
 * @param[in] argc Number of arguments, including "fault"
 * @param[in] argv Argument values, including "fault"
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 *
 * Command usage: fault status
 */
static int32_t cmd_fault_status(int32_t argc, const char** argv)
{
    const static struct
    {
        const char* bit_name;
        uint32_t csr_bit_mask;
    } reset_info[] = 
      {
          { "LPWR", RCC_CSR_LPWRRSTF_Msk},
          { "WWDG", RCC_CSR_WWDGRSTF_Msk},
          { "IWDG", RCC_CSR_IWDGRSTF_Msk},
          { "SFT", RCC_CSR_SFTRSTF_Msk},
          { "POR", RCC_CSR_PORRSTF_Msk},
          { "PIN", RCC_CSR_PINRSTF_Msk},
          { "BOR", RCC_CSR_BORRSTF_Msk},
      };
    uint32_t* sp;
    uint32_t idx;

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
    printc("CSR: Poweron=0x%08lx Current=0x%08lx\n", rcc_csr,
           RCC->CSR);
    for (idx = 0; idx < ARRAY_SIZE(reset_info); idx++) {
        if (rcc_csr & reset_info[idx].csr_bit_mask) {
            printc("     %s reset bit set in CSR at power on.\n",
                   reset_info[idx].bit_name);
        }
    }

    return 0;
}

/*
 * @brief Console command function for "fault test".
 *
 * @param[in] argc Number of arguments, including "fault"
 * @param[in] argv Argument values, including "fault"
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 *
 * Command usage: fault test [<op> [<arg>]]
 */
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
        printc("Invalid test '\%s'\n", argv[2]);
        return MOD_ERR_BAD_CMD;
    }
    return 0;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winfinite-recursion"
static void test_overflow_stack(void){
    test_overflow_stack();
}
#pragma GCC diagnostic pop
