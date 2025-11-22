// Temporary stub functions for Day 3 build (FAULT feature disabled)
// These will be replaced with actual implementations in Day 4

#include <stdint.h>
#include <stdbool.h>

// Watchdog stubs (Day 4 feature)
void wdg_register(uint32_t wdg_id, uint32_t timeout_ms)
{
    // Stub: watchdog not active in Day 3
    (void)wdg_id;
    (void)timeout_ms;
}

void wdg_feed(uint32_t wdg_id)
{
    // Stub: watchdog not active in Day 3
    (void)wdg_id;
}

// Fault handler stub (Day 4 feature)
void fault_exception_handler(uint32_t* sp)
{
    // Stub: fault handling not active in Day 3
    // In Day 4, this will implement panic mode
    (void)sp;
    while (1) {
        // Infinite loop for safety
    }
}
