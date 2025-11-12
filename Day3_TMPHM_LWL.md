# Day 3: TMPHM Module + Lightweight Logging Integration

## Summary of Accomplishment

✅ **Build Status:** Success (43.6 KB, zero errors, one non-critical warning)  
✅ **TMPHM Module:** Complete with console commands and full error handling  
✅ **LWL Instrumentation:** Comprehensive logging in both I2C and TMPHM  
✅ **App Integration:** Mode selection between TMPHM and button test  
✅ **Operating Mode:** TMPHM automatic sampling enabled by default

---

## 1. Introduction - What Was Accomplished

On Day 3, we built upon the I2C driver foundation (Day 2) to create a complete sensor integration system. Three major components were implemented:

### The TMPHM Module (Temperature/Humidity Module)
A sensor-specific driver that abstracts away all the details of communicating with the SHT31-D temperature and humidity sensor. This module:
- Samples sensor data automatically every 1 second
- Manages the entire measurement cycle via a 5-state machine
- Validates data integrity using CRC-8 checksums
- Converts raw sensor values to human-readable units
- Provides console commands for debugging and testing

### Lightweight Logging (LWL)
A production-grade diagnostic system that captures system behavior with minimal overhead:
- Records state transitions in both I2C and TMPHM modules
- Stores events in a compact circular buffer (no expensive string formatting)
- Acts as a "flight recorder" for post-mortem debugging
- Essential for maintaining embedded systems in the field

### Application Integration
Updated `app_main.c` to support two mutually exclusive operating modes:
- **MODE A (Default):** TMPHM runs automatically in the background
- **MODE B:** Manual I2C button testing (Day 2 functionality)

---

## 2. TMPHM Module Explained

### Purpose: Sensor Abstraction Layer

The TMPHM module sits between the generic I2C driver and your application. It knows the specific protocol for talking to the SHT31-D sensor so your application doesn't have to.

**✅ BEST PRACTICE:** Always separate generic bus drivers (I2C, SPI) from sensor-specific drivers (TMPHM, GPS, IMU). This creates clean, testable layers.

```
Application Layer:  "Give me temperature"
      ↓
TMPHM Module:      "I know how to talk to SHT31-D"
      ↓
I2C Driver:        "I know how to use the I2C bus"
      ↓
Hardware:          STM32 I2C3 peripheral
```

### The 5-State Machine

The TMPHM module uses a non-blocking state machine that progresses through these stages every 1 second:

```c
enum states {
    STATE_IDLE,               // Waiting for timer to trigger
    STATE_RESERVE_I2C,        // Get exclusive bus access
    STATE_WRITE_MEAS_CMD,     // Sending 0x2c 0x06 to sensor
    STATE_WAIT_MEAS,          // Waiting 17ms for sensor to measure
    STATE_READ_MEAS_VALUE     // Reading 6 bytes from sensor
};
```

#### State Transition Flow:

```
IDLE
  ↓ (1 sec timer fires)
RESERVE_I2C
  ↓ (reserve succeeds)
WRITE_MEAS_CMD (sends 0x2c 0x06)
  ↓ (I2C write completes)
WAIT_MEAS (waits 17ms)
  ↓ (time expires)
READ_MEAS_VALUE (reads 6 bytes)
  ↓ (read completes, CRC validates)
IDLE (stores data, waits for next cycle)
```

**💡 PRO TIP:** Non-blocking state machines are fundamental to embedded systems. They allow multiple modules to run concurrently in a super loop without blocking each other.

#### Key State: RESERVE_I2C

```c
case STATE_RESERVE_I2C:
    LWL("TMPHM: Attempting I2C reserve", 0);
    
    rc = i2c_reserve(st.cfg.i2c_instance_id);
    if (rc == 0) {
        // Success! Prepare measurement command
        memcpy(st.msg_bfr, sensor_i2c_cmd, sizeof(sensor_i2c_cmd));
        rc = i2c_write(st.cfg.i2c_instance_id, st.cfg.i2c_addr, st.msg_bfr, 2);
        if (rc == 0) {
            LWL("TMPHM: Write started", 0);
            st.state = STATE_WRITE_MEAS_CMD;
        } else {
            // Write failed - clean up and return to IDLE
            LWL("TMPHM: Write init failed rc=%d", 4, LWL_4(rc));
            INC_SAT_U16(cnts_u16[CNT_WRITE_INIT_FAIL]);
            i2c_release(st.cfg.i2c_instance_id);
            st.state = STATE_IDLE;
        }
    } else {
        // Reserve failed - stay in this state and try again
        LWL("TMPHM: Reserve failed rc=%d", 4, LWL_4(rc));
        INC_SAT_U16(cnts_u16[CNT_RESERVE_FAIL]);
    }
    break;
```

**⚠️ WATCH OUT:** If reserve fails, we stay in RESERVE state and try again next time through the super loop. This is the correct behavior when multiple modules share the same bus.

### CRC-8 Validation - Why It's Critical

The SHT31-D sensor sends 6 bytes for each measurement:
```
[Temp MSB] [Temp LSB] [Temp CRC] [Hum MSB] [Hum LSB] [Hum CRC]
```

The CRC-8 checksum detects:
- **Bit flips** due to electrical noise on the I2C bus
- **Bus glitches** during data transmission
- **Sensor malfunction** (corrupted internal data)

```c
if (crc8(&msg[0], 2) != msg[2] || crc8(&msg[3], 2) != msg[5]) {
    // CRC validation failed
    LWL("TMPHM: CRC error", 0);
    INC_SAT_U16(cnts_u16[CNT_CRC_FAIL]);
} else {
    // CRC valid - safe to use this data
    // ... convert and store ...
}
```

**🎯 PITFALL:** Never trust sensor data without validation. In production systems, a single bad reading can trigger cascading failures. Always check CRC/parity when available.

### Integer Math for Fixed-Point Conversion

Embedded systems often avoid floating-point math due to:
- **Speed:** Floating-point is slow on microcontrollers without an FPU
- **Code size:** Floating-point libraries are large
- **Determinism:** Integer math has predictable execution time

The SHT31-D datasheet specifies these conversion formulas:
```
Temperature (°C) = -45 + 175 × (raw_value / 65535)
Humidity (%)     = 100 × (raw_value / 65535)
```

We convert to fixed-point (value × 10) using integer-only math:

```c
const uint32_t divisor = 65535;

// Combine MSB and LSB
int32_t temp = (msg[0] << 8) + msg[1];
uint32_t hum = (msg[3] << 8) + msg[4];

// Apply conversion formulas (from datasheet)
temp = -450 + (1750 * temp + divisor/2) / divisor;
hum = (1000 * hum + divisor/2) / divisor;

// Results are in 0.1° and 0.1% units
// Example: temp=235 means 23.5°C
```

**💡 PRO TIP:** Adding `divisor/2` before dividing implements rounding. This is essential for maintaining accuracy in fixed-point arithmetic.

### Console Commands Added

```bash
# View current state and last measurement
tmphm status

# Get last measurement with age
tmphm test lastmeas 0
# Output: Temp=23.5 C Hum=45.2 % age=123 ms

# Test CRC-8 algorithm (for learning/debugging)
tmphm test crc8 0xBE 0xEF
# Output: crc8: 0x92 (matches datasheet example)
```

---

## 3. LWL (Lightweight Logging) Explained

### Why LWL is Essential for Embedded Systems

Traditional logging with `printf()` is problematic in embedded systems:

| Approach | Runtime Cost | When Useful |
|----------|--------------|-------------|
| `printf("temp=%d\n", temp)` | **Very High** (string formatting, UART transmission) | Development only |
| `LWL("temp=%d", 2, LWL_2(temp))` | **Very Low** (just stores numbers) | Production systems |

**📖 WAR STORY:** In a production system, a critical bug only occurred after hours of operation. Traditional logging was too slow and changed timing enough to mask the bug. LWL captured pre-fault activity in its circular buffer without affecting timing, revealing the root cause.

### How LWL Works

LWL stores events as compact binary records:

```c
LWL("TMPHM: Got good measurement temp=%d hum=%d", 4, LWL_2(temp), LWL_2(hum));
```

This stores:
- **Event ID:** Auto-generated by `__COUNTER__` macro (unique per LWL call)
- **Timestamp:** Automatically added by LWL module
- **Data:** Just the raw numbers (4 bytes total)

Total overhead: ~8-12 bytes per event (vs. ~40+ bytes for equivalent printf)

### Circular Buffer Concept

```
 [oldest] → [event] → [event] → [event] → [newest]
     ↑                                        ↓
     └────────── wraps around ────────────────┘
```

When the buffer fills up, new events overwrite the oldest ones. This means you always have the most recent N events in memory, which is perfect for debugging intermittent issues.

### LWL Instrumentation Added

#### I2C Module - State Transitions

```c
case STATE_MSTR_WR_GEN_START:
    LWL("I2C: WR_GEN_START", 0);
    if (sr1 & LL_I2C_SR1_SB) {
        LWL("I2C: START sent, sending addr=0x%02x", 1, LWL_1(st->dest_addr));
        st->i2c_reg_base->DR = st->dest_addr << 1;
        st->state = STATE_MSTR_WR_SENDING_ADDR;
    }
    break;
```

This creates a trail of breadcrumbs showing:
- Which state the I2C driver was in
- What address it was trying to communicate with
- Exact sequence of events leading to success or failure

#### TMPHM Module - Measurement Cycle

```c
LWL("TMPHM: Attempting I2C reserve", 0);
rc = i2c_reserve(st.cfg.i2c_instance_id);
if (rc == 0) {
    LWL("TMPHM: Write started", 0);
    // ...
} else {
    LWL("TMPHM: Reserve failed rc=%d", 4, LWL_4(rc));
    INC_SAT_U16(cnts_u16[CNT_RESERVE_FAIL]);
}
```

### Decoding LWL Logs

After capturing LWL data (via console command `lwl dump` or from flash after a crash), you decode it with a Python script:

```bash
python lwl.py lwl_dump.txt
```

Output shows:
- Timestamp of each event
- Human-readable format string
- Decoded parameter values
- Sequence of events leading to the problem

---

## 4. Code Changes Summary

### `tmphm.c` - Complete Sensor Driver

**Added:**
- Console command infrastructure (`cmd_tmphm_status`, `cmd_tmphm_test`)
- Performance counters (reserve_fail, write_fail, crc_fail, etc.)
- Comprehensive LWL instrumentation at every state transition
- Full error handling with performance counter increments
- Conditional watchdog support (`#ifdef CONFIG_WDG_PRESENT`)

**Key Functions:**
```c
tmphm_get_def_cfg()   // Get default configuration
tmphm_init()          // Initialize state structure
tmphm_start()         // Register commands, start timer
tmphm_run()           // State machine (call in super loop)
tmphm_get_last_meas() // Query latest measurement
```

### `i2c.c` - Enhanced Logging

**Added:**
- LWL calls at entry to each interrupt handler state
- LWL for critical events (address ACK, data transfer complete, etc.)
- Comprehensive event trail for debugging

**Example:**
```c
case STATE_MSTR_WR_SENDING_ADDR:
    LWL("I2C: WR_SENDING_ADDR", 0);
    if (sr1 & LL_I2C_SR1_ADDR) {
        LWL("I2C: Addr ACKed, starting data transfer", 0);
        // ... rest of code ...
    }
    break;
```

### `app_main.c` - Mode Selection

**Added:**
- Operating mode selection (`#define ENABLE_MODE_A_TMPHM 1`)
- Clear documentation comments explaining MODE A vs MODE B
- Conditional compilation for TMPHM initialization, start, and run
- Instructions for switching between modes

**Structure:**
```c
// INIT phase
#if ENABLE_MODE_A_TMPHM
    tmphm_get_def_cfg(TMPHM_INSTANCE_1, &tmphm_cfg);
    tmphm_init(TMPHM_INSTANCE_1, &tmphm_cfg);
#endif

// START phase
#if ENABLE_MODE_A_TMPHM
    tmphm_start(TMPHM_INSTANCE_1);
#endif

// SUPER LOOP
#if ENABLE_MODE_A_TMPHM
    tmphm_run(TMPHM_INSTANCE_1);
#else
    // Button test code
#endif
```

---

## 5. Operating Modes Explained

### MODE A: TMPHM Automatic Sensor Sampling (Default)

**Enabled by:** `#define ENABLE_MODE_A_TMPHM 1` in `app_main.c`

**Behavior:**
- TMPHM module starts automatically on boot
- Samples temperature/humidity every 1 second in the background
- Uses timer-driven state machine (completely non-blocking)
- Console commands available for querying data

**Use this mode for:**
- Production operation
- Learning how background tasks work
- Testing sensor integration

**Console commands:**
```bash
tmphm status              # View state and last measurement
tmphm test lastmeas 0     # Get latest temp/humidity
i2c status                # Check I2C driver state
lwl dump                  # View lightweight log buffer
```

### MODE B: Manual I2C Button Test

**Enabled by:** `#define ENABLE_MODE_A_TMPHM 0` in `app_main.c`

**Behavior:**
- TMPHM module NOT started
- Press USER button to trigger I2C test sequence
- Tests I2C driver functionality directly
- Good for debugging I2C communication issues

**Use this mode for:**
- Debugging I2C hardware issues
- Understanding I2C protocol details
- Day 2 review

### Why Mutually Exclusive?

Both modes use the same I2C bus (I2C_INSTANCE_3). Running them simultaneously would require:
- Bus arbitration logic
- Priority management
- Deadlock prevention

For learning purposes, we keep it simple by running one mode at a time.

### Switching Modes

1. Open `Badweh_Development/app1/app_main.c`
2. Find line: `#define ENABLE_MODE_A_TMPHM 1`
3. Change to `0` for MODE B, or `1` for MODE A
4. Rebuild: `ci-cd-tools/build.bat`
5. Flash will occur automatically

---

## 6. Testing Instructions

### MODE A Testing (Current Configuration)

**1. Connect to serial console:**
- Baud rate: 115200
- COM port: Check Device Manager

**2. Observe boot sequence:**
```
========================================
  DAY 3: TMPHM Module Integration
========================================

MODE: TMPHM Automatic Sensor Sampling
      (Background operation, 1 sec cycle)
      Query with: tmphm test lastmeas 0

[INIT] Initializing modules...
  - TMPHM (Temp/Humidity) initialized

[START] Starting modules...
  - LWL (Lightweight Logging) started
  - TMPHM started (1-sec sampling)

[READY] Entering super loop...
TMPHM running in background.
Console commands available:
  - tmphm status
  - tmphm test lastmeas 0
  - i2c status
```

**3. Query current measurement:**
```bash
tmphm test lastmeas 0
```

**Expected output:**
```
Temp=23.5 C Hum=45.2 % age=234 ms
```

**4. Check module status:**
```bash
tmphm status
```

**Expected output:**
```
         Got  Last Last Meas Meas
ID State Meas Temp Hum  Age  Time
-- ----- ---- ---- ---- ---- ----
 0     0    1  235  452  567   17
```

Interpretation:
- State=0 (IDLE, waiting for next cycle)
- Got Meas=1 (have valid data)
- Temp=235 (23.5°C)
- Hum=452 (45.2%)
- Age=567ms (measurement is 567ms old)
- Meas Time=17ms (sensor takes 17ms to measure)

**5. View LWL logs:**
```bash
lwl dump
```

You'll see raw hex data. To decode:
1. Copy output to a file
2. Run Python decoder (if available)

### MODE B Testing (Button Test)

**1. Switch to MODE B** (see "Switching Modes" above)

**2. Connect to console**

**3. Press USER button (blue button on Nucleo board)**

**Expected output:**
```
>> Button pressed - Starting I2C auto test...
RESERVED
WRITE
READ
RELEASE - Test SUCCESS!
>> I2C auto test completed
```

### Troubleshooting

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No temperature readings | Sensor not connected | Check I2C3 wiring (SCL=PA8, SDA=PC9) |
| CRC errors in logs | Loose wire | Check connections, add pull-up resistors |
| Timer overrun errors | Sensor too slow | Increase `meas_time_ms` in config |
| Reserve failures | I2C bus stuck | Power cycle board, check for bus conflicts |

---

## 7. Why This Matters - Design Principles

### Separation of Concerns

```
┌─────────────────────────┐
│   Application Layer     │ ← Doesn't know about I2C or sensors
├─────────────────────────┤
│   TMPHM Module          │ ← Knows SHT31-D protocol, not I2C details
├─────────────────────────┤
│   I2C Driver            │ ← Knows I2C protocol, not sensors
├─────────────────────────┤
│   Hardware (STM32 I2C3) │
└─────────────────────────┘
```

**✅ BEST PRACTICE:** Each layer has a single responsibility. This makes code:
- **Testable:** Can test I2C driver without a sensor
- **Reusable:** Can use I2C driver with different sensors
- **Maintainable:** Changes to sensor don't affect I2C driver

### Testability Through Console Commands

Every module provides console commands for:
- **Status queries:** See current state without debugger
- **Manual testing:** Trigger operations on demand
- **Diagnostics:** Check performance counters

This is essential for:
- **Development:** Debug without JTAG
- **Production:** Diagnose field issues remotely
- **Manufacturing:** Automated test scripts

### Maintainability Through LWL

LWL provides:
- **Post-mortem debugging:** Analyze crashes after they occur
- **Performance analysis:** Track timing of operations
- **Field diagnostics:** Capture data from deployed systems

**💡 PRO TIP:** In production systems, LWL data is often written to non-volatile memory (flash, EEPROM) on crash. After reboot, this data is uploaded to a server for analysis.

### Non-Blocking Architecture

The super loop architecture allows:
- Multiple modules running concurrently
- Predictable timing (no RTOS overhead)
- Easy to understand flow

```c
while (1) {
    console_run();   // Handle user input
    tmr_run();       // Update timers
    tmphm_run();     // Progress state machine
    // Each function returns quickly (non-blocking)
}
```

**⚠️ WATCH OUT:** Never use blocking delays (`HAL_Delay()`) in super loop code. They prevent other modules from running.

---

## Build Results

**Final Binary:**
- Text (code): 43,640 bytes
- Data (initialized globals): 760 bytes
- BSS (uninitialized globals): 6,728 bytes
- **Total:** 51,128 bytes (< 10% of 512KB flash)

**Build Status:** ✅ Success
- Zero errors
- One non-critical warning (LWL internal redefinition)
- Successfully flashed to target board

---

## Next Steps (Day 4 and Beyond)

Day 3 has given you a production-quality sensor integration system. Future topics might include:

**Day 4: Watchdog and Fault Handling**
- Software watchdog for detecting stuck tasks
- Fault logging to non-volatile memory
- Graceful degradation strategies

**Day 5: Power Management**
- Low-power sleep modes
- Sensor power control
- Battery-efficient sampling strategies

**Day 6: Data Filtering and Calibration**
- Moving average filters
- Outlier rejection
- Sensor calibration coefficients

---

## Key Takeaways

1. **Layered Architecture:** Generic drivers (I2C) separate from sensor-specific drivers (TMPHM) separate from application logic

2. **Non-Blocking Design:** State machines allow concurrent operation without RTOS complexity

3. **Error Handling:** Check every return code, increment performance counters, handle failures gracefully

4. **Validation:** Always validate sensor data (CRC checks) before trusting it

5. **Diagnostics:** Console commands + LWL logging make systems maintainable

6. **Integer Math:** Fixed-point arithmetic avoids floating-point overhead

7. **Resource Sharing:** Reserve/release pattern prevents bus conflicts

8. **Testability:** Console commands enable testing without debugger

---

**Congratulations!** You now have a complete, professional-grade sensor integration system with production-quality diagnostics. This architecture scales from hobby projects to safety-critical industrial systems.

