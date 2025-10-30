# Most Basic Watchdog Code Reference

## Core Data Structures

```c
struct soft_wdg {
    uint32_t period_ms;              // Timeout period
    uint32_t last_feed_time_ms;      // When last fed
};

struct wdg_state {
    struct soft_wdg soft_wdgs[CONFIG_WDG_NUM_WDGS];
    wdg_triggered_cb triggered_cb;   // Callback when watchdog triggers
};

static struct wdg_state state;       // Global state
```

## Essential Functions

### 1. Initialization
```c
int32_t wdg_init(struct wdg_cfg* cfg) {
    memset(&state, 0, sizeof(state));
    return 0;
}

int32_t wdg_start(void) {
    int32_t rc;
    rc = cmd_register(&cmd_info);                               // Register console commands
    rc = tmr_inst_get_cb(CONFIG_WDG_RUN_CHECK_MS, wdg_tmr_cb, 0); // Start periodic checker
    return rc;
}
```

### 2. Client Registration & Feeding
```c
int32_t wdg_register(uint32_t wdg_id, uint32_t period_ms) {
    if (wdg_id >= CONFIG_WDG_NUM_WDGS) return MOD_ERR_ARG;
    state.soft_wdgs[wdg_id].last_feed_time_ms = tmr_get_ms();  // Start at "now"
    state.soft_wdgs[wdg_id].period_ms = period_ms;
    return 0;
}

int32_t wdg_feed(uint32_t wdg_id) {
    if (wdg_id >= CONFIG_WDG_NUM_WDGS) return MOD_ERR_ARG;
    state.soft_wdgs[wdg_id].last_feed_time_ms = tmr_get_ms();  // Update feed time
    return 0;
}
```

### 3. Hardware Watchdog
```c
int32_t wdg_start_hdw_wdg(uint32_t timeout_ms) {
    #define LSI_FREQ_HZ 32000
    #define WDG_PRESCALE 64
    #define WDG_CLK_FREQ_HZ (LSI_FREQ_HZ/WDG_PRESCALE)
    #define WDG_MAX_RL 0xfff
    #define WDG_MS_TO_RL(ms) (((ms) * WDG_CLK_FREQ_HZ + 500)/1000 - 1)
    
    int32_t ctr = WDG_MS_TO_RL(timeout_ms);
    if (ctr < 0 || ctr > WDG_MAX_RL) return MOD_ERR_ARG;
    
    LL_IWDG_Enable(IWDG);
    LL_IWDG_EnableWriteAccess(IWDG);
    LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_64);
    LL_IWDG_SetReloadCounter(IWDG, ctr);
    while (!LL_IWDG_IsReady(IWDG)) { }  // Wait for ready
    return 0;
}

void wdg_feed_hdw(void) {
    LL_IWDG_ReloadCounter(IWDG);  // Feed hardware watchdog
}
```

### 4. Timer Callback (The Heart)
```c
static enum tmr_cb_action wdg_tmr_cb(int32_t tmr_id, uint32_t user_data) {
    uint32_t now_ms = tmr_get_ms();
    
    // Check all software watchdogs for timeout
    for (uint32_t idx = 0; idx < CONFIG_WDG_NUM_WDGS; idx++) {
        if (now_ms - state.soft_wdgs[idx].last_feed_time_ms > state.soft_wdgs[idx].period_ms) {
            if (state.triggered_cb != NULL) {
                state.triggered_cb(idx);  // Call fault handler
            }
            break;
        }
    }
    
    // Feed hardware watchdog if all software watchdogs OK
    wdg_feed_hdw();
    
    return TMR_CB_RESTART;
}
```

## app_main.c Integration

```c
// INIT Phase
fault_init(NULL);
wdg_init(NULL);

// START Phase (early - protect initialization)
wdg_start_init_hdw_wdg();  // Start init watchdog
fault_start();
wdg_start();

// After successful initialization
wdg_init_successful();  // Reset failure counter
wdg_start_hdw_wdg(CONFIG_WDG_HARD_TIMEOUT_MS);  // Start runtime watchdog
```

## Client Module Usage (TMPHM Example)

```c
// In tmphm_start():
wdg_register(CONFIG_TMPHM_WDG_ID, CONFIG_TMPHM_WDG_MS);  // Register with timeout

// In tmphm_run() after successful measurement:
wdg_feed(CONFIG_TMPHM_WDG_ID);  // Feed on success only!
```

## Required Timer Functions

```c
// Get current time in milliseconds (SysTick increments every 1ms)
uint32_t tmr_get_ms(void) {
    return tick_ms_ctr;  // Global counter incremented by SysTick
}

// Get periodic timer with callback
int32_t tmr_inst_get_cb(uint32_t ms, tmr_cb_func cb_func, uint32_t cb_user_data) {
    int32_t tmr_id = tmr_inst_get(ms);  // Allocate timer instance
    if (tmr_id >= 0) {
        tmrs[tmr_id].cb_func = cb_func;          // Set callback function
        tmrs[tmr_id].cb_user_data = cb_user_data; // Set callback data
    }
    return tmr_id;
}

// SysTick handler (called every 1ms) - increments global counter
void tmr_SysTick_Handler(void) {
    tick_ms_ctr++;
}
```

## Key Concept: Hierarchy

```
Application → feeds → Software Watchdogs → OK → Hardware Watchdog
                                      ↓ timeout ↓ timeout
                                    Fault Module → Reset
```

**Critical:** Feed ONLY on success, not every loop iteration!

