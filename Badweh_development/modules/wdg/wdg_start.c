/*
 * @brief Implementation of wdg module (STUDENT STARTING VERSION)
 *
 * This module provides a watchdog service. It supports a configurable number
 * of software-based watchdogs. A hardware-based watchdog is used to verify the
 * software-based watchdogs are operating correctly.
 *
 * Student starting code - fill in the blanks marked with ??? or "Question"
 *
 */

#include <stdint.h>
#include <string.h> 

#include "config.h"
#include CONFIG_STM32_LL_IWDG_HDR

#include "cmd.h"
#include "console.h"
#include "fault.h"
#include "log.h"
#include "module.h"
#include "tmr.h"
#include "wdg.h"

////////////////////////////////////////////////////////////////////////////////
// Common macros
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////////////

// Software watchdog structure
struct soft_wdg
{
    uint32_t period_ms;            // Timeout period in milliseconds
    uint32_t last_feed_time_ms;    // When was this watchdog last fed?
};

// Module state
struct wdg_state
{
    struct soft_wdg soft_wdgs[CONFIG_WDG_NUM_WDGS];  // Array of software watchdogs
    wdg_triggered_cb triggered_cb;                    // Callback when watchdog triggers
};

// No-init variables (survive resets)
struct wdg_no_init_vars {
    uint32_t magic;                       // Magic number for validation
    uint32_t consec_failed_init_ctr;      // Count of consecutive failed initializations
    uint32_t check;                       // For validating structure integrity
};

#define WDG_NO_INIT_VARS_MAGIC 0xdeaddead

////////////////////////////////////////////////////////////////////////////////
// Private (static) function declarations
////////////////////////////////////////////////////////////////////////////////

static enum tmr_cb_action wdg_tmr_cb(int32_t tmr_id, uint32_t user_data);
static void validate_no_init_vars(void);
static void update_no_init_vars(void);
static int32_t cmd_wdg_status(int32_t argc, const char** argv);
static int32_t cmd_wdg_test(int32_t argc, const char** argv);

////////////////////////////////////////////////////////////////////////////////
// Private (static) variables
////////////////////////////////////////////////////////////////////////////////

// Module state (global but internal to this module)
struct wdg_state state;

static int32_t log_level = LOG_DEFAULT;

// Console command definitions
static struct cmd_cmd_info cmds[] = {
    {
        .name = "status",
        .func = cmd_wdg_status,
        .help = "Get module status, usage: wdg status",
    },
    {
        .name = "test",
        .func = cmd_wdg_test,
        .help = "Run test, usage: wdg test [<op> [<arg>]] (enter no op for help)",
    }
};

// Data structure passed to cmd module for console interaction
static struct cmd_client_info cmd_info = {
    .name = "wdg",
    .num_cmds = ARRAY_SIZE(cmds),
    .cmds = cmds,
    .log_level_ptr = &log_level,
};

// No-init variables (in special linker section)
struct wdg_no_init_vars no_init_vars __attribute__((section (".no.init.vars")));

////////////////////////////////////////////////////////////////////////////////
// Public (global) functions
////////////////////////////////////////////////////////////////////////////////

/*
 * @brief Initialize wdg instance.
 *
 * @param[in] cfg The wdg configuration. (FUTURE)
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 */
int32_t wdg_init(struct wdg_cfg* cfg)
{
    // TODO: Clear state structure
    // Question: What structure needs to be zeroed?
    // Hint: Look at Pattern 9 - Module Initialization
    // Think: Clean slate for module state
    
    memset(???, 0, sizeof(???));
    
    return 0;
}

/*
 * @brief Start wdg instance.
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 */
int32_t wdg_start(void)
{
    int32_t rc;

    // TODO: Register console commands
    // Question: What function registers commands?
    rc = ???(&cmd_info);
    if (rc < 0) {
        log_error("wdg_start: cmd error %d\n", rc);
        goto exit;
    }

    // TODO: Get periodic timer
    // Question: Which function gets a periodic timer?
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

/*
 * @brief Client registration.
 *
 * @param[in] wdg_id The software-based watchdog ID.
 * @param[in] period_ms The watchdog timeout period.
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 */
int32_t wdg_register(uint32_t wdg_id, uint32_t period_ms)
{
    struct soft_wdg* soft_wdg;

    // TODO: Validate watchdog ID
    // Question: Can't exceed what limit?
    if (wdg_id >= ???)
        return MOD_ERR_ARG;

    // TODO: Get pointer to this watchdog's state
    soft_wdg = &state.soft_wdgs[???];

    // TODO: Initialize the watchdog
    // Question: What two fields need to be set?
    soft_wdg->last_feed_time_ms = ???;  // Hint: Get current time
    soft_wdg->period_ms = ???;          // Hint: Parameter passed in

    return 0;
}

/*
 * @brief Feed a software-based watchdog.
 *
 * @param[in] wdg_id The sofware-based watchdog ID.
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 */
int32_t wdg_feed(uint32_t wdg_id)
{
    // TODO: Validate watchdog ID
    if (wdg_id >= ???)
        return MOD_ERR_ARG;
    
    // TODO: Update last feed time
    // Question: What needs to be updated?
    state.soft_wdgs[wdg_id].last_feed_time_ms = ???;
    
    return 0;
}

/*
 * @brief Register to receive a callback when any watchdog triggers.
 *
 * @return 0 for success, else a "MOD_ERR" value. See code for details.
 */
int32_t wdg_register_triggered_cb(wdg_triggered_cb triggered_cb)
{
    // TODO: Save the callback function pointer
    state.triggered_cb = ???;
    
    return 0;
}

/*
 * @brief Feed the hardware-based watchdog.
 */
void wdg_feed_hdw(void)
{
    // TODO: Feed hardware watchdog
    // Hint: Look at LL_IWDG_ReloadCounter
    ???(IWDG);
}

////////////////////////////////////////////////////////////////////////////////
// Private (static) functions
////////////////////////////////////////////////////////////////////////////////

/*
 * @brief Timer callback used to check software-based watchdogs.
 *
 * @param[in] tmr_id The timer ID (not used).
 * @param[in] user_data  User data for this timer (not used).
 *
 * @return TMR_CB_RESTART so that it is periodically called.
 */
static enum tmr_cb_action wdg_tmr_cb(int32_t tmr_id, uint32_t user_data)
{
    uint32_t idx;
    bool wdg_triggered = false;
    uint32_t now_ms;

    // TODO: Get current time (for comparison)
    now_ms = ???;

    // TODO: Check each software watchdog
    // Question: Loop through all watchdogs
    for (idx = 0; idx < CONFIG_WDG_NUM_WDGS; idx++) {
        // TODO: Calculate how long since last feed
        // Question: Timeout = now - last_feed > period?
        if (now_ms - state.soft_wdgs[idx].last_feed_time_ms > 
            state.soft_wdgs[idx].period_ms) {
            // TODO: Watchdog timed out - notify
            wdg_triggered = true;
            if (state.triggered_cb != NULL) {
                state.triggered_cb(???);  // Hint: Which watchdog ID?
            }
            break;  // Only trigger once per check cycle
        }
    }

    // TODO: Feed hardware watchdog if no software watchdogs triggered
    if (???) {
        ???();
    }

    return TMR_CB_RESTART;  // Keep firing periodically
}

/*
 * @brief Validate no-init variables structure integrity.
 */
static void validate_no_init_vars(void)
{
    // TODO: Check if no_init_vars are valid
    // This is advanced - you can skip for now and copy from reference
    if (no_init_vars.magic != WDG_NO_INIT_VARS_MAGIC) {
        // First time - initialize
        no_init_vars.magic = WDG_NO_INIT_VARS_MAGIC;
        no_init_vars.consec_failed_init_ctr = 0;
        no_init_vars.check = ~WDG_NO_INIT_VARS_MAGIC;
    }
}

/*
 * @brief Update no-init variables with validation check.
 */
static void update_no_init_vars(void)
{
    // TODO: Update check field
    // This is advanced - you can skip for now
    no_init_vars.check = ~no_init_vars.magic;
}

/*
 * @brief Console command function for "wdg status".
 */
static int32_t cmd_wdg_status(int32_t argc, const char** argv)
{
    uint32_t idx;
    
    // TODO: Print status of all watchdogs
    // Question: How to format the output?
    printc("wdg module status:\n");
    for (idx = 0; idx < CONFIG_WDG_NUM_WDGS; idx++) {
        if (state.soft_wdgs[idx].period_ms != 0) {
            printc("  wdg[%lu]: period=%lu ms\n", 
                   idx, state.soft_wdgs[idx].period_ms);
        }
    }
    
    return 0;
}

/*
 * @brief Console command function for "wdg test".
 */
static int32_t cmd_wdg_test(int32_t argc, const char** argv)
{
    // TODO: Implement test commands
    // This is for testing - you can add later
    printc("wdg test not implemented yet\n");
    return 0;
}

