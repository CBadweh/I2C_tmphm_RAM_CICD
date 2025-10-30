# Most Basic Timer Interrupt Code Reference

## Core Data Structures

```c
enum tmr_state {
    TMR_UNUSED,   // Not allocated
    TMR_STOPPED,  // Allocated but not running
    TMR_RUNNING,  // Active and counting
    TMR_EXPIRED   // Timeout occurred
};

struct tmr_inst_info {
    uint32_t period_ms;         // Timer period
    uint32_t start_time;        // When timer started (from tick_ms_ctr)
    enum tmr_state state;       // Current state
    tmr_cb_func cb_func;        // Callback function (optional)
    uint32_t cb_user_data;      // User data for callback
};

static uint32_t tick_ms_ctr;                    // Global millisecond counter
static struct tmr_inst_info tmrs[TMR_NUM_INST]; // Timer pool (5 instances)

// Callback return values
enum tmr_cb_action {
    TMR_CB_NONE,     // One-shot timer (don't restart)
    TMR_CB_RESTART   // Periodic timer (restart automatically)
};

typedef enum tmr_cb_action (*tmr_cb_func)(int32_t tmr_id, uint32_t user_data);
```

## Essential Pattern Implementation

### Initialization
```c
int32_t tmr_init(struct tmr_cfg* cfg) {
    memset(&tmrs, 0, sizeof(tmrs));  // Clear all timer instances
    LL_SYSTICK_EnableIT();            // Enable SysTick interrupt
    return 0;
}

int32_t tmr_start(void) {
    return cmd_register(&cmd_info);   // Register console commands
}
```

### SysTick Interrupt (Hardware - 1ms tick)
```c
// Called by hardware interrupt every 1ms
void tmr_SysTick_Handler(void) {
    tick_ms_ctr++;  // Increment global millisecond counter
}

// In stm32f4xx_it.c - actual interrupt handler
void SysTick_Handler(void) {
    tmr_SysTick_Handler();  // Call timer module handler
}
```

### Timer Processing (Software - called from main loop)
```c
/* Check all timers for expiration and execute callbacks
 * MUST be called from super loop to process timer events
 */
int32_t tmr_run(void) {
    static uint32_t last_ms = 0;
    uint32_t now_ms = tmr_get_ms();
    
    if (now_ms == last_ms) return 0;  // Fast exit if no time change
    last_ms = now_ms;
    
    for (int idx = 0; idx < TMR_NUM_INST; idx++) {
        struct tmr_inst_info* ti = &tmrs[idx];
        
        if (ti->state == TMR_RUNNING) {
            // Check if timer expired: elapsed >= period
            if (now_ms - ti->start_time >= ti->period_ms) {
                ti->state = TMR_EXPIRED;
                
                if (ti->cb_func != NULL) {
                    enum tmr_cb_action result = ti->cb_func(idx, ti->cb_user_data);
                    
                    if (result == TMR_CB_RESTART) {
                        ti->state = TMR_RUNNING;
                        ti->start_time += ti->period_ms;  // Precise periodic timing
                    }
                }
            }
        }
    }
    return 0;
}
```

## Helper Functions

### Get Current Time
```c
uint32_t tmr_get_ms(void) {
    return tick_ms_ctr;  // Returns milliseconds since boot (rolls over ~49.7 days)
}
```

### Allocate Timer
```c
int32_t tmr_inst_get(uint32_t ms) {
    for (uint32_t idx = 0; idx < TMR_NUM_INST; idx++) {
        if (tmrs[idx].state == TMR_UNUSED) {
            tmrs[idx].period_ms = ms;
            tmrs[idx].state = (ms == 0) ? TMR_STOPPED : TMR_RUNNING;
            tmrs[idx].start_time = (ms == 0) ? 0 : tmr_get_ms();
            tmrs[idx].cb_func = NULL;
            return idx;  // Return timer ID
        }
    }
    return MOD_ERR_RESOURCE;  // Out of timers
}

int32_t tmr_inst_get_cb(uint32_t ms, tmr_cb_func cb_func, uint32_t cb_user_data) {
    int32_t tmr_id = tmr_inst_get(ms);
    if (tmr_id >= 0) {
        tmrs[tmr_id].cb_func = cb_func;
        tmrs[tmr_id].cb_user_data = cb_user_data;
    }
    return tmr_id;
}
```

### Control Timer
```c
int32_t tmr_inst_start(int32_t tmr_id, uint32_t ms) {
    if (tmr_id >= 0 && tmr_id < TMR_NUM_INST) {
        struct tmr_inst_info* ti = &tmrs[tmr_id];
        ti->period_ms = ms;
        ti->state = (ms == 0) ? TMR_STOPPED : TMR_RUNNING;
        ti->start_time = tmr_get_ms();
        return 0;
    }
    return MOD_ERR_ARG;
}

int32_t tmr_inst_release(int32_t tmr_id) {
    if (tmr_id >= 0 && tmr_id < TMR_NUM_INST) {
        tmrs[tmr_id].state = TMR_UNUSED;  // Free timer for reuse
        return 0;
    }
    return MOD_ERR_ARG;
}
```

## Integration Example

### app_main.c Setup
```c
void app_main(void) {
    // INIT Phase
    tmr_init(NULL);      // Initialize timer module
    
    // START Phase
    tmr_start();         // Register console commands
    
    // Super Loop
    while (1) {
        tmr_run();       // Process timers (MUST call every loop iteration!)
        // ... other modules ...
    }
}
```

### Client Usage - Periodic Callback (e.g., Watchdog)
```c
// Callback function
static enum tmr_cb_action wdg_tmr_cb(int32_t tmr_id, uint32_t user_data) {
    check_all_watchdogs();  // Do periodic work
    return TMR_CB_RESTART;  // Keep running
}

// In wdg_start()
int32_t wdg_start(void) {
    int32_t tmr_id = tmr_inst_get_cb(CONFIG_WDG_RUN_CHECK_MS, wdg_tmr_cb, 0);
    // Timer now fires every CONFIG_WDG_RUN_CHECK_MS (e.g., 10ms)
    return tmr_id;
}
```

### Client Usage - One-Shot Timer (e.g., Delay)
```c
// Start 500ms one-shot timer
int32_t tmr_id = tmr_inst_get(500);

// In main loop: poll for expiration
if (tmr_inst_is_expired(tmr_id)) {
    do_delayed_action();
    tmr_inst_release(tmr_id);  // Free timer
}
```

## Dependencies

### Hardware Configuration (main.c)
```c
void SystemClock_Config(void) {
    // ... clock setup ...
    LL_Init1msTick(84000000);        // Configure SysTick for 1ms at 84MHz
    LL_SetSystemCoreClock(84000000); // Set system clock to 84MHz
}
```

### STM32 Low-Level Drivers
- `stm32f4xx_ll_cortex.h` - For `LL_SYSTICK_EnableIT()`
- SysTick hardware peripheral - ARM Cortex-M core timer

## Key Concept: Two-Layer Architecture

```
Hardware Layer (1ms interrupt):
    SysTick_Handler() → tmr_SysTick_Handler() → tick_ms_ctr++

Software Layer (main loop):
    tmr_run() → Check timers → Execute callbacks
```

## Timing Precision

```c
// Precise periodic timing example:
// Timer period = 10ms
// Start: tick_ms_ctr = 100
// Callback at: 110, 120, 130, 140... (no drift!)

if (result == TMR_CB_RESTART) {
    ti->start_time += ti->period_ms;  // Add period, don't reset to "now"
}
```

**Critical Notes:**
1. **Must call `tmr_run()` in super loop** - callbacks don't run in interrupt context
2. **SysTick fires every 1ms** - configured by `LL_Init1msTick()`
3. **Timer pool is fixed size** - 5 instances, allocate wisely
4. **Rollover safe** - unsigned arithmetic handles 32-bit rollover (~49.7 days)
5. **Callback context** - runs in main loop, not interrupt (safe for most operations)

