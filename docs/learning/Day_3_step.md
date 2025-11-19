# Day 3 Action Plan: TMPHM Module + Lightweight Logging

## **Overview**

Build the Temperature/Humidity Module (TMPHM) and integrate Lightweight Logging (LWL) for production-quality debugging capabilities.

**Estimated Time:** ~2 hours (faster than lesson plan's 4 hours because TMPHM is already complete!)

---

## **GOOD NEWS: Your TMPHM Module is Already Complete! ✅**

Your `tmphm.c` is identical to the reference code - you've got:
- ✓ 5-state machine (IDLE → RESERVE → WRITE → WAIT → READ)
- ✓ CRC-8 validation
- ✓ Integer math for temp/humidity conversion
- ✓ Timer-driven background sampling
- ✓ Console test commands

---

## **Step 1: Enable TMPHM in `app_main.c`** (10 min)

You need to **uncomment** these lines:

### In `app_main.c` around line 344-348:
```c
result = tmphm_start(TMPHM_INSTANCE_1);
if (result < 0) {
    log_error("tmphm_start 1 error %d\n", result);
    INC_SAT_U16(cnts_u16[CNT_START_ERR]);
}
```

### In `app_main.c` around line 380-382:
```c
result = tmphm_run(TMPHM_INSTANCE_1);
if (result < 0)
    INC_SAT_U16(cnts_u16[CNT_RUN_ERR]);
```

### Test it:
1. Build and flash
2. Console should show temp/humidity readings every 1 second
3. Try: `tmphm test lastmeas 0` to query current reading

---

## **Step 2: Check for LWL Module** (5 min)

Verify you need to copy LWL from the reference project.

**Status:** LWL module not present in `Badweh_Development/modules/`

---

## **Step 3: Copy LWL Module from Reference** (15 min)

### 3a. Copy files from reference to your project:

**From:** `ram-class-nucleo-f401re/modules/lwl/` → **To:** `Badweh_Development/modules/lwl/`
- Copy `lwl.c`

**From:** `ram-class-nucleo-f401re/modules/include/` → **To:** `Badweh_Development/modules/include/`
- Copy `lwl.h`

### 3b. Add to build system:
Your Makefile should auto-detect new `.c` files in `modules/lwl/`, but verify by building.

### 3c. Enable LWL in `app_main.c`:

**Add to includes (around line 40):**
```c
#include "lwl.h"
```

**Add to initialization section (around line 240):**
```c
result = lwl_init(NULL);
if (result < 0) {
    log_error("lwl_init error %d\n", result);
    INC_SAT_U16(cnts_u16[CNT_INIT_ERR]);
}
```

**Add to start section (around line 330):**
```c
result = lwl_start();
if (result < 0) {
    log_error("lwl_start error %d\n", result);
    INC_SAT_U16(cnts_u16[CNT_START_ERR]);
}

// Enable LWL recording
lwl_enable(true);
```

---

## **Step 4: Add LWL to I2C Driver** (20 min)

### In `Badweh_Development/modules/i2c/i2c.c`:

### 4a. Add LWL header:
```c
#include "lwl.h"
```

### 4b. Define LWL IDs (near top, around line 35):
```c
// LWL (Lightweight Logging) configuration
#define LWL_BASE_ID 20  // I2C module uses IDs 20-29
#define LWL_NUM 10

#define LWL_I2C_STATE 0
#define LWL_I2C_SR1   1
#define LWL_I2C_ERROR 2
```

### 4c. Add LWL calls at key transitions:

**In `i2c_interrupt()`, at state changes:**
```c
case STATE_MSTR_WR_GEN_START:
    if (sr1 & LL_I2C_SR1_SB) {
        LWL("WR_START", 3, LWL_1(st->dest_addr), LWL_2(st->msg_len), LWL_3(LWL_I2C_STATE));
        st->i2c_reg_base->DR = st->dest_addr << 1;
        st->state = STATE_MSTR_WR_SENDING_ADDR;
    }
    break;
```

### 4d. Log errors:
```c
if (sr1 & I2C_SR1_AF) {
    LWL("ACK_FAIL", 2, LWL_1(st->dest_addr), LWL_2(LWL_I2C_ERROR));
    i2c_error = I2C_ERR_ACK_FAIL;
}
```

**Strategic LWL Points in I2C Driver:**
- START condition generated
- Address sent (with ACK/NACK)
- Data byte sent/received
- STOP condition
- Any error condition

---

## **Step 5: Add LWL to TMPHM Module** (15 min)

### In `Badweh_Development/modules/tmphm/tmphm.c`:

### 5a. Add header:
```c
#include "lwl.h"
```

### 5b. Define LWL IDs (around line 60):
```c
#define LWL_BASE_ID 30  // TMPHM module uses IDs 30-39
#define LWL_NUM 10
```

### 5c. Log state transitions in `tmphm_run()`:
```c
case STATE_RESERVE_I2C:
    LWL("TMPHM_RESV", 2, LWL_1(st->cfg.i2c_instance_id), LWL_2(st->state));
    rc = i2c_reserve(st->cfg.i2c_instance_id);
    // ... existing code ...
```

### 5d. Log measurements:
```c
// After successful CRC check
LWL("TMPHM_MEAS", 3, LWL_1(temp), LWL_2(hum), LWL_3(st->last_meas_ms));
st->last_meas.temp_deg_c_x10 = temp;
// ... existing code ...
```

**Strategic LWL Points in TMPHM:**
- State transitions (IDLE → RESERVE → WRITE → WAIT → READ)
- I2C reserve success/failure
- Write/Read operation start
- CRC validation (pass/fail)
- Final measurement values

---

## **Step 6: Test Everything** (30 min)

### 6a. Build and flash
```bash
cd Badweh_Development/ci-cd-tools
./build.bat
```

### 6b. Watch console output:
You should see periodic temp/humidity readings

### 6c. Test LWL commands:
```
lwl                    // Show help
lwl dump               // View flight recorder data
lwl clear              // Clear buffer
```

### 6d. Test TMPHM commands:
```
tmphm status           // Module state
tmphm test lastmeas 0  // Get reading
```

### 6e. Verify LWL captures activity:
1. Run `lwl clear`
2. Wait 5 seconds (TMPHM takes ~5 samples)
3. Run `lwl dump`
4. You should see I2C and TMPHM state transitions!

---

## **Step 7: Understanding the Data Flow** (15 min)

### Trace one complete measurement cycle:

1. **Timer fires** (every 1000ms) → `tmr_callback()` sets state to `RESERVE_I2C`
2. **Super loop** calls `tmphm_run()` → tries `i2c_reserve()`
3. **If successful** → calls `i2c_write()` with command `0x2c 0x06`
4. **I2C interrupt** handles the write transaction → state machine advances
5. **TMPHM waits** 15ms for sensor measurement
6. **TMPHM calls** `i2c_read()` for 6 bytes
7. **I2C interrupt** reads data → TMPHM validates CRC → converts to temp/humidity
8. **Cycle repeats** every 1 second

**LWL captures each transition** - this is your "black box flight recorder"!

---

## **📊 Day 3 Completion Checklist**

- [ ] TMPHM enabled in `app_main.c` (start + run)
- [ ] LWL module copied and integrated
- [ ] LWL initialized and enabled in `app_main.c`
- [ ] LWL logging added to I2C driver (5-10 strategic points)
- [ ] LWL logging added to TMPHM module (state transitions)
- [ ] Console shows periodic temp/humidity readings
- [ ] `lwl dump` shows captured activity
- [ ] `tmphm test lastmeas 0` works
- [ ] Build compiles with zero warnings
- [ ] Git commit: `"Day 3: Enable TMPHM + integrate LWL flight recorder"`

---

## **💡 PRO TIP: Why LWL Matters**

When your system crashes in the field, you want to know **what happened in the last 5 seconds**. LWL gives you that without:
- Slowing down the system (no `printf`)
- Filling up memory (circular buffer)
- Requiring a debugger (data survives reset in RAM)

It's like an airplane's black box - records just enough to reconstruct the sequence of events before a crash.

---

## **⏱️ Time Breakdown**

| Step | Task | Time |
|------|------|------|
| 1 | Enable TMPHM | 10 min |
| 2-3 | Copy and setup LWL | 20 min |
| 4-5 | Add LWL logging | 35 min |
| 6-7 | Testing and understanding | 45 min |
| **Total** | | **~2 hours** |

---

## **🎯 Key Learning Points**

### Understanding the TMPHM State Machine:

```
STATE_IDLE            → Waiting for timer to trigger
   ↓
STATE_RESERVE_I2C     → Acquire exclusive I2C bus access
   ↓
STATE_WRITE_MEAS_CMD  → Send 0x2c 0x06 to sensor (start measurement)
   ↓
STATE_WAIT_MEAS       → Wait 15ms for sensor to complete measurement
   ↓
STATE_READ_MEAS_VALUE → Read 6 bytes (temp MSB/LSB/CRC, hum MSB/LSB/CRC)
   ↓
[Validate CRC, convert to degC×10 and %×10]
   ↓
Back to STATE_IDLE    → Release I2C bus, wait for next timer trigger
```

### Data Format from SHT31-D Sensor:

**6 bytes received:**
```
Byte 0: Temperature MSB
Byte 1: Temperature LSB
Byte 2: Temperature CRC-8
Byte 3: Humidity MSB
Byte 4: Humidity LSB
Byte 5: Humidity CRC-8
```

### Integer Math Conversion (avoiding floating point):

**Temperature:** -45°C to +130°C, stored as degC × 10
```c
temp = -450 + (1750 * raw_temp + 32767) / 65535;
```

**Humidity:** 0% to 100%, stored as % × 10
```c
hum = (1000 * raw_hum + 32767) / 65535;
```

### CRC-8 Validation:

**Parameters from SHT31-D datasheet:**
- Polynomial: 0x31 (x^8 + x^5 + x^4 + 1)
- Initial value: 0xFF
- Final XOR: 0x00 (none)
- Example: 0xBEEF → 0x92

---

## **✅ BEST PRACTICES Demonstrated**

1. **Non-blocking Architecture:** TMPHM uses state machine in `run()` function - never blocks the super loop
2. **Resource Management:** Always reserve I2C before use, release after done
3. **Data Validation:** CRC check on sensor data before trusting it
4. **Integer Math:** Avoids floating point overhead on Cortex-M4
5. **Lightweight Logging:** Flight recorder without performance penalty
6. **Timer-Driven Sampling:** Periodic measurements without blocking
7. **Error Counting:** Performance metrics track failures

---

## **🎯 PITFALLS TO AVOID**

1. **Don't forget to release I2C** - Even on error paths! (Reference code does this correctly)
2. **Don't skip CRC validation** - Sensor data can be corrupted, especially with long cables
3. **Don't use `printf` in interrupt context** - Use LWL instead
4. **Don't block in `tmphm_run()`** - State machine must return quickly every iteration

---

## **📖 WAR STORY: Why LWL is Critical**

From Gene's experience: A system in the field would crash randomly every few days. Without LWL, engineers spent weeks trying to reproduce the issue. With LWL, they recovered the flight recorder data after the next crash and immediately saw:

1. I2C transaction to sensor started
2. Sensor didn't respond (ACK failure)
3. I2C driver correctly returned error
4. TMPHM incremented error counter
5. **BUT** - another module tried to use I2C without checking if it was released
6. Conflict led to watchdog timeout → system reset

**Fix:** Add assert that I2C is not reserved before allowing reserve operation. **5 minutes to fix after weeks of debugging.**

LWL = Your time machine for embedded debugging! 🕐

---

## **Next Steps After Day 3**

Once Day 3 is complete, you'll have:
- ✓ Working I2C driver (Day 2)
- ✓ Sensor integration with CRC validation (Day 3)
- ✓ Flight recorder for debugging (Day 3)

**Day 4 Preview:** Fault handling module and watchdog protection - learning how to survive crashes gracefully!

---

**Ready to start? Begin with Step 1!** 🚀

