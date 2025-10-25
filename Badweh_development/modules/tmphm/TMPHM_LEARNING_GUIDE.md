# TMPHM Module Learning Guide

## What This Document Contains

1. **What Was Removed and Why** - Understanding non-essential code
2. **What Was Kept and Why** - Understanding critical path
3. **Step-by-Step Development Guide** - How to build this from scratch

---

## Part 1: What Was Removed and Why

### ❌ Removed: Console Commands (180 lines)

**What:**
- `cmd_tmphm_status()` - Shows state machine status
- `cmd_tmphm_test()` - Manual test commands
- All the `struct cmd_cmd_info` and `struct cmd_client_info` setup

**Why Removed:**
- **Debugging tool only** - Not needed for core functionality
- **Learn Later** - Once you understand the state machine, adding console commands is straightforward
- **Cognitive Load** - Adds 180 lines that don't help you understand how the sensor works

**When You Need It:**
After you understand the state machine, add console commands for debugging. The pattern is identical to I2C driver's console commands.

---

### ❌ Removed: Performance Counters (50 lines)

**What:**
```c
enum tmphm_u16_pms {
    CNT_RESERVE_FAIL,
    CNT_WRITE_INIT_FAIL,
    CNT_WRITE_OP_FAIL,
    // ... etc
};
```

**Why Removed:**
- **Nice to have, not need to have** - System works fine without them
- **Production feature** - Important for field debugging, but not for learning
- **Adds complexity** - Every error path has `INC_SAT_U16(cnts_u16[CNT_xxx])`

**When You Need It:**
In production code, these counters are VERY useful for diagnosing field issues. Add them after you understand the basic flow.

---

### ❌ Removed: Multiple Instance Support (30 lines)

**What:**
- Arrays of states: `tmphm_states[TMPHM_NUM_INSTANCES]`
- Instance ID checking in every function
- Loop through all instances in status display

**Why Removed:**
- **You have one sensor** - KISS principle (Keep It Simple, Stupid)
- **Easy to scale** - Once you understand single instance, making it an array is trivial
- **Focus on logic** - The state machine is what matters

**When You Need It:**
If you need multiple sensors, just convert `struct tmphm_state st;` to `struct tmphm_state st[NUM_INSTANCES];`

---

### ❌ Removed: Complex Error Messages (15 lines)

**What:**
- Detailed `log_error()` calls with formatted strings
- ChatGPT debug comments

**Why Removed:**
- **Noise** - Makes the code harder to read
- **Learn later** - Add detailed logging after you understand the flow

**Kept:**
- Simple `log_info()` for successful measurements
- Simple `log_error()` for critical errors (CRC, timer overrun)

---

## Part 2: What Was Kept and Why

### ✅ Kept: 5-State State Machine

```c
enum states {
    STATE_IDLE,              // Waiting for timer
    STATE_RESERVE_I2C,       // Get I2C bus
    STATE_WRITE_MEAS_CMD,    // Send command
    STATE_WAIT_MEAS,         // Wait 15ms
    STATE_READ_MEAS_VALUE    // Read result
};
```

**Why Kept:**
- **This IS the driver** - Everything else supports this
- **Non-blocking pattern** - Learn how to do async in super loop
- **Clear flow** - Each state does ONE thing

**Critical Concept:**
The state machine advances **one step per `tmphm_run()` call**. This is non-blocking - the super loop keeps spinning.

---

### ✅ Kept: `tmphm_run()` - The Heart

**Why Kept:**
- **Where the magic happens** - This is what you MUST understand
- **Shows I2C interaction** - How to use the I2C driver you built on Day 2
- **Error handling pattern** - What to do when things fail

**Key Pattern to Learn:**
```c
case STATE_WRITE_MEAS_CMD:
    rc = i2c_get_op_status(st.cfg.i2c_instance_id);  // Poll status
    if (rc != MOD_ERR_OP_IN_PROG) {                  // Done?
        if (rc == 0) {                                // Success?
            // Move to next state
        } else {
            // Clean up and go back to IDLE
        }
    }
    // Still in progress? Do nothing, come back next loop
    break;
```

---

### ✅ Kept: Timer Callback

```c
static enum tmr_cb_action tmr_callback(int32_t tmr_id, uint32_t user_data)
{
    if (st.state == STATE_IDLE)
        st.state = STATE_RESERVE_I2C;
    else
        log_error("Timer overrun\n");
    return TMR_CB_RESTART;
}
```

**Why Kept:**
- **Shows async trigger** - Timer kicks off the cycle
- **Overrun detection** - What if previous measurement not done?
- **Simple but powerful** - Just changes state, that's all!

**Critical Concept:**
The timer doesn't DO the measurement - it just sets `state = RESERVE_I2C`. The super loop calling `tmphm_run()` does the actual work.

---

### ✅ Kept: CRC-8 Function

**Why Kept:**
- **Data validation is critical** - Sensor data can be corrupted
- **Learn the algorithm** - CRC is used everywhere in embedded
- **Matches datasheet** - Shows how to implement from specifications

**Critical Concept:**
CRC is a mathematical checksum. The sensor calculates it and sends it with the data. You calculate it on the received data. If they match, data is valid.

---

### ✅ Kept: Data Conversion (Integer Math)

```c
temp = -450 + (1750 * temp + divisor/2) / divisor;
hum = (1000 * hum + divisor/2) / divisor;
```

**Why Kept:**
- **Avoids floating point** - Faster on Cortex-M4
- **Shows fixed-point math** - Store degC × 10 instead of float
- **Rounding technique** - The `+ divisor/2` before division

**Critical Concept:**
Instead of storing `23.5°C` as a float, store `235` (degC × 10). All math stays integer!

---

## Part 3: Step-by-Step Development Guide

### **Prerequisites**
- Day 2 I2C driver working
- SHT31-D datasheet (Table 8, Table 4)
- Understanding of state machines

---

### **Step 1: Design the State Machine** (15 min)

**What to do:**
Draw the state flow on paper or whiteboard:

```
[IDLE] --timer--> [RESERVE_I2C] --reserve success--> [WRITE_MEAS_CMD]
                       |                                    |
                   reserve fail                       write complete
                       |                                    |
                      [IDLE]                          [WAIT_MEAS]
                                                            |
                                                    15ms elapsed
                                                            |
                                                    [READ_MEAS_VALUE]
                                                            |
                                                      read complete
                                                            |
                                                   validate CRC, convert
                                                            |
                                                         [IDLE]
```

**Questions to answer:**
- What happens if I2C reserve fails? (Stay in RESERVE state, try next loop)
- What happens if write fails? (Release I2C, go to IDLE)
- What happens if CRC fails? (Log error, still release I2C and go to IDLE)
- What if timer fires before previous measurement done? (Log overrun error)

---

### **Step 2: Define Data Structures** (10 min)

**Create the state enum:**
```c
enum states {
    STATE_IDLE,
    STATE_RESERVE_I2C,
    STATE_WRITE_MEAS_CMD,
    STATE_WAIT_MEAS,
    STATE_READ_MEAS_VALUE
};
```

**Create the state structure:**
```c
struct tmphm_state {
    struct tmphm_cfg cfg;              // Config (I2C instance, address, timings)
    struct tmphm_meas last_meas;       // Last measurement
    int32_t tmr_id;                    // Timer ID
    uint32_t i2c_op_start_ms;          // For timing the 15ms wait
    uint32_t last_meas_ms;             // When was last measurement?
    uint8_t msg_bfr[6];                // 6 bytes from sensor
    bool got_meas;                     // Have we gotten at least one?
    enum states state;                 // Current state
};
```

**Why each field:**
- `cfg` - Need to know which I2C, what address, timings
- `last_meas` - Store result for `tmphm_get_last_meas()` API
- `tmr_id` - Need this to start/stop timer
- `i2c_op_start_ms` - For the 15ms wait in STATE_WAIT_MEAS
- `last_meas_ms` - To report age of measurement
- `msg_bfr[6]` - Storage for I2C read data
- `got_meas` - Don't return uninitialized data
- `state` - Current state machine state

---

### **Step 3: Implement Init and Start** (15 min)

**Init - Just save config:**
```c
int32_t tmphm_init(enum tmphm_instance_id instance_id, struct tmphm_cfg* cfg)
{
    memset(&st, 0, sizeof(st));
    st.cfg = *cfg;
    return 0;
}
```

**Start - Get timer:**
```c
int32_t tmphm_start(enum tmphm_instance_id instance_id)
{
    st.tmr_id = tmr_inst_get_cb(st.cfg.sample_time_ms, tmr_callback, 0);
    if (st.tmr_id < 0)
        return MOD_ERR_RESOURCE;
    return 0;
}
```

**Timer callback - Kick off cycle:**
```c
static enum tmr_cb_action tmr_callback(int32_t tmr_id, uint32_t user_data)
{
    if (st.state == STATE_IDLE)
        st.state = STATE_RESERVE_I2C;
    else
        log_error("Timer overrun\n");
    return TMR_CB_RESTART;
}
```

---

### **Step 4: Implement STATE_IDLE** (5 min)

**Easiest state:**
```c
case STATE_IDLE:
    // Nothing to do - timer callback will move us to RESERVE_I2C
    break;
```

**Why:**
Just waiting for the timer to trigger. The `break` is important - don't fall through!

---

### **Step 5: Implement STATE_RESERVE_I2C** (20 min)

**What to do:**
```c
case STATE_RESERVE_I2C:
    rc = i2c_reserve(st.cfg.i2c_instance_id);
    if (rc == 0) {
        // Got the bus! Send measurement command
        memcpy(st.msg_bfr, sensor_i2c_cmd, sizeof(sensor_i2c_cmd));
        rc = i2c_write(st.cfg.i2c_instance_id, st.cfg.i2c_addr, 
                      st.msg_bfr, sizeof(sensor_i2c_cmd));
        if (rc == 0) {
            st.state = STATE_WRITE_MEAS_CMD;
        } else {
            i2c_release(st.cfg.i2c_instance_id);
            st.state = STATE_IDLE;
        }
    }
    // If reserve failed, stay in this state and try again
    break;
```

**Key points:**
- Reserve might fail (someone else using I2C) - just try again next loop
- If reserve succeeds but write fails - MUST release I2C!
- `sensor_i2c_cmd` is `{0x2c, 0x06}` from datasheet Table 8

---

### **Step 6: Implement STATE_WRITE_MEAS_CMD** (15 min)

**What to do:**
```c
case STATE_WRITE_MEAS_CMD:
    rc = i2c_get_op_status(st.cfg.i2c_instance_id);
    if (rc != MOD_ERR_OP_IN_PROG) {
        if (rc == 0) {
            st.i2c_op_start_ms = tmr_get_ms();
            st.state = STATE_WAIT_MEAS;
        } else {
            i2c_release(st.cfg.i2c_instance_id);
            st.state = STATE_IDLE;
        }
    }
    break;
```

**Key points:**
- Poll `i2c_get_op_status()` - non-blocking!
- `MOD_ERR_OP_IN_PROG` means "still working" - do nothing, come back next loop
- If success, record timestamp and move to WAIT state
- If failure, release and abort

---

### **Step 7: Implement STATE_WAIT_MEAS** (15 min)

**What to do:**
```c
case STATE_WAIT_MEAS:
    if (tmr_get_ms() - st.i2c_op_start_ms >= st.cfg.meas_time_ms) {
        rc = i2c_read(st.cfg.i2c_instance_id, st.cfg.i2c_addr,
                     st.msg_bfr, I2C_MSG_BFR_LEN);
        if (rc == 0) {
            st.state = STATE_READ_MEAS_VALUE;
        } else {
            i2c_release(st.cfg.i2c_instance_id);
            st.state = STATE_IDLE;
        }
    }
    break;
```

**Key points:**
- Wait 15ms (from datasheet Table 4) for sensor to measure
- Compute elapsed time: `current_time - start_time`
- Only when 15ms elapsed, start the read
- If read start fails, release and abort

---

### **Step 8: Implement STATE_READ_MEAS_VALUE** (30 min)

**What to do:**
```c
case STATE_READ_MEAS_VALUE:
    rc = i2c_get_op_status(st.cfg.i2c_instance_id);
    if (rc != MOD_ERR_OP_IN_PROG) {
        if (rc == 0) {
            uint8_t* msg = st.msg_bfr;
            
            // Validate CRC
            if (crc8(&msg[0], 2) == msg[2] &&
                crc8(&msg[3], 2) == msg[5]) {
                
                // Convert temperature
                const uint32_t divisor = 65535;
                int32_t temp = (msg[0] << 8) + msg[1];
                uint32_t hum = (msg[3] << 8) + msg[4];
                temp = -450 + (1750 * temp + divisor/2) / divisor;
                hum = (1000 * hum + divisor/2) / divisor;
                
                // Store result
                st.last_meas.temp_deg_c_x10 = temp;
                st.last_meas.rh_percent_x10 = hum;
                st.last_meas_ms = tmr_get_ms();
                st.got_meas = true;
                
                log_info("temp=%ld degC*10 hum=%d %%*10\n", temp, hum);
            } else {
                log_error("CRC error\n");
            }
        }
        // Always release and go back to IDLE
        i2c_release(st.cfg.i2c_instance_id);
        st.state = STATE_IDLE;
    }
    break;
```

**Key points:**
- Poll status until read complete
- Validate CRC before trusting data
- Convert using datasheet formulas (see Step 9)
- ALWAYS release I2C, even on error

---

### **Step 9: Implement CRC-8** (20 min)

**From datasheet page 14:**
- Polynomial: 0x31
- Initial value: 0xFF
- No final XOR

```c
static uint8_t crc8(const uint8_t *data, int len)
{
    const uint8_t polynomial = 0x31;
    uint8_t crc = 0xff;
    uint32_t idx1, idx2;

    for (idx1 = len; idx1 > 0; idx1--) {
        crc ^= *data++;
        for (idx2 = 8; idx2 > 0; idx2--) {
            crc = (crc & 0x80) ? (crc << 1) ^ polynomial : (crc << 1);
        }
    }
    return crc;
}
```

**Test it:**
From datasheet: `crc8(0xBEEF) should equal 0x92`

```c
uint8_t test[2] = {0xBE, 0xEF};
if (crc8(test, 2) == 0x92)
    printc("CRC test PASS\n");
```

---

### **Step 10: Understand Data Conversion** (20 min)

**Temperature formula from datasheet:**
```
T[°C] = -45 + 175 × (raw_value / 65535)
```

**Problem:** Floating point is slow!

**Solution:** Use integer math, store as degC × 10
```c
temp = -450 + (1750 * temp + 32767) / 65535;
```

**Why this works:**
- `-45°C` becomes `-450` (×10)
- `175` becomes `1750` (×10)
- `+ 32767` is rounding (`divisor/2`)
- Result is in degC × 10

**Example:**
- Raw value: `32768` (middle of range)
- `temp = -450 + (1750 × 32768 + 32767) / 65535`
- `temp = -450 + 875`
- `temp = 425` → **42.5°C**

Same pattern for humidity (0-100% → 0-1000)

---

### **Step 11: Implement Get Last Measurement API** (10 min)

```c
int32_t tmphm_get_last_meas(enum tmphm_instance_id instance_id,
                            struct tmphm_meas* meas, uint32_t* meas_age_ms)
{
    if (meas == NULL)
        return MOD_ERR_ARG;
    
    if (!st.got_meas)
        return MOD_ERR_UNAVAIL;

    *meas = st.last_meas;
    if (meas_age_ms != NULL)
        *meas_age_ms = tmr_get_ms() - st.last_meas_ms;

    return 0;
}
```

**Key points:**
- Check for NULL pointer
- Don't return garbage if no measurement yet
- Report age so caller knows if data is stale

---

### **Step 12: Test Everything** (45 min)

**12a. Enable in app_main.c:**
Uncomment `tmphm_start()` and `tmphm_run()`

**12b. Build and flash**

**12c. Expected console output:**
```
temp=235 degC*10 hum=450 %*10
temp=234 degC*10 hum=451 %*10
temp=236 degC*10 hum=449 %*10
```

**12d. Troubleshooting:**

| Problem | Likely Cause | Fix |
|---------|--------------|-----|
| No output | Timer not triggering | Check `tmr_run()` in super loop |
| "Timer overrun" | Measurement taking too long | Check I2C driver working |
| "CRC error" | I2C read failed | Check sensor wiring |
| Wrong values | Data conversion bug | Check formula, divisor |

---

## **Summary: Critical Concepts Learned**

### 1. **Non-Blocking State Machine**
Never block the super loop. Each `run()` call advances the state by one step.

### 2. **Async I2C Usage**
Start operation → Poll status → Process result. Don't wait!

### 3. **Resource Management**
Reserve → Use → Release. Even on error paths!

### 4. **Data Validation**
Always validate CRC before trusting sensor data.

### 5. **Integer Math**
Store `temp × 10` instead of float. Faster, smaller code.

### 6. **Timer-Driven Sampling**
Timer just kicks off the cycle. State machine does the work.

---

## **Next Steps After Understanding**

1. **Add console commands** - For debugging
2. **Add performance counters** - Track errors
3. **Add multiple instances** - If you have >1 sensor
4. **Add LWL logging** - Day 3 afternoon task!

---

## **Time Estimate for Development**

| Step | Task | Time |
|------|------|------|
| 1 | Design state machine | 15 min |
| 2 | Data structures | 10 min |
| 3 | Init/Start | 15 min |
| 4-8 | Implement 5 states | 85 min |
| 9 | CRC-8 | 20 min |
| 10 | Data conversion | 20 min |
| 11 | Get measurement API | 10 min |
| 12 | Test and debug | 45 min |
| **Total** | | **~3.5 hours** |

---

**You're ready to build this from scratch now!** 🚀

