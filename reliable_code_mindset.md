# The Reliable Code Mindset: From Scratch to Senior
## 40 Years of Lessons in Building Dependable Embedded Systems

**By Gene Schrader's Principles**

---

## **The Central Truth:**

> **"Bugs are inevitable in complex systems. Focus on detection and recovery, not prevention alone."**

This document shows you **HOW** to think as you build code from scratch, and **WHY** certain practices matter at each skill level.

---

# Part 1: The Three-Level Reliability Pyramid

```
┌────────────────────────────────────────────────────┐
│ SENIOR LEVEL (5-40 years)                          │
│ Mindset: "What will fail that I haven't imagined?" │
│ • Fault recovery strategies                        │
│ • Watchdog integration                             │
│ • Performance optimization                         │
│ • System-wide failure analysis                     │
│ • Graceful degradation                             │
└────────────────────────────────────────────────────┘
┌────────────────────────────────────────────────────┐
│ MID-LEVEL (1-5 years)                              │
│ Mindset: "What can go wrong?"                      │
│ • Error counters                                   │
│ • Sanity checks                                    │
│ • Timeout protection                               │
│ • Defensive programming                            │
│ • Instrumentation for debugging                    │
└────────────────────────────────────────────────────┘
┌────────────────────────────────────────────────────┐
│ ENTRY LEVEL (0-1 years)                            │
│ Mindset: "Make it work"                            │
│ • Clean code structure                             │
│ • Correct patterns                                 │
│ • Basic error handling                             │
│ • Resource cleanup                                 │
│ • Non-blocking design                              │
└────────────────────────────────────────────────────┘
```

---

# Part 2: The Practices - When and Why

## **Practice 1: Clean Code Structure**

### **When to Apply:** ✅ **ALWAYS - From Day 1**

### **What It Means:**
- Meaningful variable names
- Clear function responsibilities
- Consistent formatting
- Logical organization
- Comments explaining "why", not "what"

### **Why It Matters:**
**You'll read code 10x more than you write it.** In 6 months, you'll forget why you wrote it. Clean code is a gift to your future self.

### **Example - BAD:**
```c
int f(int x) {
    int t=x*175; t=t/65535; t-=45;  // calc temp
    return t;
}
```

### **Example - GOOD:**
```c
// Convert raw sensor value to temperature in degrees Celsius
int32_t convert_temperature(uint16_t raw_value)
{
    const int32_t min_temp = -45;
    const int32_t temp_range = 175;
    const uint32_t max_raw = 65535;
    
    return min_temp + (temp_range * raw_value) / max_raw;
}
```

### **Skill Level:** Entry-level should master this **FIRST**

---

## **Practice 2: Correct Patterns**

### **When to Apply:** ✅ **ALWAYS - From Day 1**

### **What It Means:**
- State machines for async operations
- Non-blocking APIs in super loop
- Resource reservation/release
- Separate init/start/run phases
- Interrupt-driven I/O

### **Why It Matters:**
**Wrong patterns create unfixable problems.** You can't add non-blocking to blocking code later. The architecture must be right from the start.

### **Example - BAD (Blocking):**
```c
void read_sensor(void)
{
    i2c_write_blocking(0x44, cmd, 2);  // Blocks for 5ms
    delay_ms(15);                       // Blocks for 15ms
    i2c_read_blocking(0x44, data, 6);   // Blocks for 5ms
    // Total: 25ms where system is frozen!
}
```

### **Example - GOOD (Non-Blocking):**
```c
void sensor_run(void)
{
    switch (state) {
        case IDLE: break;
        case WRITE: i2c_write(...); state = WRITE_WAIT; break;
        case WRITE_WAIT: if (done) state = DELAY; break;
        // System keeps running!
    }
}
```

### **Skill Level:** Entry-level must learn this **pattern-first**

### **📖 WAR STORY:**
I once inherited a codebase where a junior engineer used `delay_ms()` everywhere. System had to read 10 sensors = 250ms blocked. Customer complained about "sluggish response." **Complete rewrite required.** Cost: 3 engineer-months. **Patterns matter from day 1.**

---

## **Practice 3: Basic Error Handling**

### **When to Apply:** ✅ **ALWAYS - From Day 1**

### **What It Means:**
- Check return codes
- Clean up on errors
- Return to safe state
- Don't continue with invalid data

### **Why It Matters:**
**Unchecked errors cascade into disasters.** One failed I2C operation → use stale data → make wrong decision → motor crashes robot.

### **Example - BAD:**
```c
i2c_reserve(bus);
i2c_write(bus, addr, data, 2);  // What if reserve failed?
// Keep going anyway...
```

### **Example - GOOD:**
```c
rc = i2c_reserve(bus);
if (rc != 0) {
    return ERROR;  // Stop! Don't continue with failed reserve
}

rc = i2c_write(bus, addr, data, 2);
if (rc != 0) {
    i2c_release(bus);  // Clean up!
    return ERROR;
}
```

### **Skill Level:** **ENTRY-LEVEL MINIMUM REQUIREMENT**

### **Your Current Status:** You said "I only think of error handling case" - **GOOD START!** This is the foundation.

---

## **Practice 4: Error Counters (Instrumentation)**

### **When to Apply:** 🔶 **MID-LEVEL - Add after basic functionality works**

### **What It Means:**
- Count each error type
- Track success/failure rates
- Provide visibility into runtime behavior
- **Don't just handle errors - COUNT them**

### **Why It Matters:**
**You can't fix what you can't measure.** In the field, you need data to diagnose problems.

### **Example - ENTRY LEVEL (You):**
```c
if (crc_valid) {
    // Use data
} else {
    log_error("CRC error\n");  // Logged once, then lost in scrolling console
}
```

### **Example - MID LEVEL:**
```c
if (crc_valid) {
    // Use data
} else {
    INC_SAT_U16(cnts_u16[CNT_CRC_FAIL]);  // Counted!
    log_error("CRC error\n");
}

// Later, user can check:
// > tmphm status
// CRC failures: 847  ← "Aha! Noisy I2C bus!"
```

### **The Difference:**
- **Entry:** "There was an error" (one data point)
- **Mid:** "There were 847 errors in 3 hours" (pattern emerges)

### **Skill Level:** Mid-level engineers add **visibility into system health**

### **📖 WAR STORY:**
System in production had intermittent sensor failures. Customer: "Sometimes wrong." No counters, no data. Took 6 weeks of field testing to find: loose I2C connector. **With counters?** "CRC fails correlate with vibration" → 1 day to diagnose.

**Cost of no counters: 6 weeks. Cost of adding counters: 30 minutes.**

---

## **Practice 5: Sanity Checks (Defensive Programming)**

### **When to Apply:** 🔶 **MID-LEVEL - After core functionality proven**

### **What It Means:**
- Validate data is within expected range
- Check for impossible values
- Don't trust external inputs blindly
- Fail fast on nonsense data

### **Why It Matters:**
**CRC can pass on corrupted data** (1 in 256 chance). Sanity checks catch what CRC misses.

### **Example - ENTRY LEVEL:**
```c
if (crc_valid) {
    temperature = convert(raw);  // Could be -999°C or +999°C!
    store(temperature);           // Garbage in, garbage out
}
```

### **Example - MID LEVEL:**
```c
if (crc_valid) {
    temperature = convert(raw);
    
    // SHT31-D operates -40°C to +125°C. If outside, something's wrong!
    if (temperature < -400 || temperature > 1250) {
        INC_SAT_U16(cnts_u16[CNT_RANGE_ERR]);
        log_error("Temp out of range: %ld\n", temperature);
        return;  // Don't store garbage
    }
    
    store(temperature);  // Only store sane values
}
```

### **The Defense Layers:**
1. **CRC:** Catches ~99.6% of transmission errors
2. **Sanity Check:** Catches impossible values (bad sensor, bit flips)
3. **Together:** Much more robust!

### **Skill Level:** Mid-level engineers **don't trust anyone** (including hardware)

### **🎯 PITFALL:**
Junior engineers think: "CRC passed, data must be good!"  
Mid-level knows: "CRC passed, data is *probably* good. Let me verify it makes sense."

---

## **Practice 6: Timeout Protection**

### **When to Apply:** 🔶 **MID-LEVEL - Add to any waiting/polling code**

### **What It Means:**
- Every "wait for X" needs a timeout
- Every polling loop needs an exit
- No infinite retries without limit
- State machines can't be stuck forever

### **Why It Matters:**
**Hardware WILL fail in unexpected ways.** A timeout is your escape hatch.

### **Example - ENTRY LEVEL:**
```c
case STATE_RESERVE_I2C:
    rc = i2c_reserve(bus);
    if (rc == 0) {
        state = WRITE;
    }
    // If rc != 0, stay here and retry next loop
    // Problem: Could retry FOREVER if I2C stuck!
    break;
```

### **Example - MID LEVEL:**
```c
case STATE_RESERVE_I2C:
    static uint32_t reserve_start_ms = 0;
    
    // First entry to this state?
    if (reserve_start_ms == 0) {
        reserve_start_ms = tmr_get_ms();
    }
    
    rc = i2c_reserve(bus);
    if (rc == 0) {
        reserve_start_ms = 0;  // Reset for next time
        state = WRITE;
    } else if (tmr_get_ms() - reserve_start_ms > 5000) {
        // Failed to reserve for 5 seconds - something's stuck!
        log_error("Reserve timeout - I2C deadlock?\n");
        INC_SAT_U16(cnts_u16[CNT_RESERVE_TIMEOUT]);
        reserve_start_ms = 0;
        state = IDLE;  // Give up and retry next cycle
    }
    break;
```

### **Skill Level:** Mid-level engineers add **escape hatches** to every wait

### **⚠️ WATCH OUT:**
Your current code **relies on I2C driver's guard timer** (100ms). That's good! But what if:
- You're stuck in `STATE_RESERVE_I2C` retrying?
- I2C reserve never succeeds (bug in another module)?

**No local timeout = potential infinite retry loop.**

---

## **Practice 7: Measurement Staleness**

### **When to Apply:** 🔶 **MID-LEVEL - Add to any cached data**

### **What It Means:**
- Data has a shelf life
- Inform callers if data is old
- Don't pretend fresh when stale

### **Why It Matters:**
**Old data is often worse than no data.** Using yesterday's temperature for today's control decision = wrong actions.

### **Example - ENTRY LEVEL (Your Code):**
```c
int32_t get_last_meas(struct meas* out)
{
    if (!got_meas)
        return ERROR_NO_DATA;
    
    *out = last_meas;  // Could be from 3 days ago!
    return SUCCESS;
}
```

### **Example - MID LEVEL:**
```c
int32_t get_last_meas(struct meas* out, uint32_t* age_ms)
{
    if (!got_meas)
        return ERROR_NO_DATA;
    
    uint32_t age = tmr_get_ms() - last_meas_ms;
    
    // Data older than 5 seconds? Something's wrong!
    if (age > 5000) {
        return ERROR_STALE_DATA;  // Let caller decide what to do
    }
    
    *out = last_meas;
    if (age_ms != NULL)
        *age_ms = age;  // Caller can judge staleness
    
    return SUCCESS;
}
```

### **Skill Level:** Mid-level engineers think about **data freshness**

---

## **Practice 8: Watchdog Integration**

### **When to Apply:** 🔴 **SENIOR LEVEL - System-wide reliability**

### **What It Means:**
- Prove critical work is DONE, not just executing
- Feed watchdog only on successful outcomes
- Detect when system appears alive but isn't working

### **Why It Matters:**
**Code can run perfectly while doing nothing useful.** Watchdog ensures OUTCOMES, not just activity.

### **Example - MID LEVEL:**
```c
while (1) {
    sensor_run();
    wdg_feed();  // Feed every loop - BAD!
}
```

### **Example - SENIOR LEVEL:**
```c
// In tmphm module
case STATE_READ_MEAS_VALUE:
    if (measurement_successful) {
        wdg_feed(WDG_SENSOR);  // Feed ONLY on success
        // If sensor fails, watchdog NOT fed → system resets
    }
    break;
```

### **The Principle:**
> **"Watchdogs ensure critical work is DONE, not that code is EXECUTING"** - Gene Schrader

### **Skill Level:** Senior engineers design **outcome-based monitoring**

### **📖 WAR STORY:**
Medical device ran for 48 hours with sensor disconnected. Code was running (so watchdog was fed), but no measurements taken. Patient monitoring **completely failed** for 2 days before nurses noticed. **Fix:** Only feed watchdog on successful measurement. System would've reset in 30 seconds and recovered.

---

# Part 3: Building From Scratch - The Mindset at Each Stage

## **Stage 1: Make It Work (Entry-Level Thinking)**

### **Mindset:** "Get the happy path working"

### **What You Focus On:**
1. **Core algorithm** - Does it read the sensor?
2. **Clean structure** - Is it readable?
3. **Correct pattern** - Non-blocking state machine?
4. **Basic errors** - Release resources on failure?

### **What You Code:**

```c
// Entry-level first draft
int32_t sensor_run(void)
{
    switch (state) {
        case IDLE:
            break;
            
        case RESERVE:
            if (i2c_reserve(BUS) == 0) {  // Basic error check ✅
                i2c_write(BUS, ADDR, cmd, 2);
                state = WRITE_WAIT;
            }
            break;
            
        case WRITE_WAIT:
            if (i2c_get_status(BUS) == 0) {  // Check completion ✅
                state = READ;
            }
            break;
            
        case READ:
            i2c_read(BUS, ADDR, data, 6);
            state = READ_WAIT;
            break;
            
        case READ_WAIT:
            if (i2c_get_status(BUS) == 0) {
                temperature = convert(data);
                i2c_release(BUS);  // Clean up ✅
                state = IDLE;
            }
            break;
    }
}
```

### **What's Good:**
- ✅ State machine structure
- ✅ Non-blocking
- ✅ Basic error checks
- ✅ Resource cleanup

### **What's Missing:**
- ❌ CRC validation
- ❌ Error counters
- ❌ Sanity checks
- ❌ Timeout protection
- ❌ Diagnostic logging

### **Verdict:** **This is acceptable for a first working version!**

---

## **Stage 2: Make It Robust (Mid-Level Thinking)**

### **Mindset:** "What can go wrong? How do I detect it?"

### **What You Add:**

```c
// Mid-level hardened version
int32_t sensor_run(void)
{
    switch (state) {
        case IDLE:
            break;
            
        case RESERVE:
            rc = i2c_reserve(BUS);
            if (rc == 0) {
                i2c_write(BUS, ADDR, cmd, 2);
                state = WRITE_WAIT;
                state_entry_ms = tmr_get_ms();  // ← Timeout tracking
            } else {
                INC_SAT_U16(reserve_fail_count);  // ← Error counter
                
                // ← Retry limit (don't retry forever!)
                if (reserve_retry_count++ > 10) {
                    log_error("Reserve timeout - I2C stuck?\n");
                    state = IDLE;
                    reserve_retry_count = 0;
                }
            }
            break;
            
        case WRITE_WAIT:
            // ← State timeout protection
            if (tmr_get_ms() - state_entry_ms > 1000) {
                log_error("Write timeout\n");
                INC_SAT_U16(write_timeout_count);
                i2c_release(BUS);
                state = IDLE;
                break;
            }
            
            rc = i2c_get_status(BUS);
            if (rc == 0) {
                state = DELAY;
                delay_start_ms = tmr_get_ms();
            } else if (rc != IN_PROGRESS) {
                INC_SAT_U16(write_fail_count);  // ← Count failures
                i2c_release(BUS);
                state = IDLE;
            }
            break;
            
        case READ_WAIT:
            rc = i2c_get_status(BUS);
            if (rc == 0) {
                // ← CRC validation
                if (crc8(&data[0], 2) == data[2] &&
                    crc8(&data[3], 2) == data[5]) {
                    
                    temperature = convert(data);
                    
                    // ← Sanity check
                    if (temperature < -400 || temperature > 1250) {
                        log_error("Temp range error: %ld\n", temperature);
                        INC_SAT_U16(range_error_count);
                    } else {
                        // ← Only store sane, validated data
                        last_temperature = temperature;
                        last_meas_ms = tmr_get_ms();
                        got_meas = true;
                        INC_SAT_U16(success_count);  // ← Count successes too!
                    }
                } else {
                    INC_SAT_U16(crc_fail_count);  // ← Count CRC failures
                    log_error("CRC fail\n");
                }
                
                i2c_release(BUS);
                state = IDLE;
            }
            break;
    }
}
```

### **What Was Added:**
- ✅ Error counters (every failure type)
- ✅ Success counters (measure reliability)
- ✅ CRC validation
- ✅ Sanity checks on data
- ✅ Retry limits
- ✅ State timeouts

### **Why Each Addition:**

| Addition | Detects | Example Field Issue |
|----------|---------|---------------------|
| Error counters | Pattern of failures | "CRC fails every 100 samples" → EMI problem |
| Success counters | Reliability percentage | "99.8% success rate" → meets spec |
| CRC validation | Transmission errors | Noisy I2C bus, loose wiring |
| Sanity checks | Sensor malfunction | Sensor returns 500°C (impossible) |
| Retry limits | Stuck states | I2C deadlock |
| State timeouts | Unexpected hangs | I2C driver bug |

### **Skill Level:** Mid-level engineers think **"What can I measure to diagnose problems later?"**

---

## **Stage 3: Make It Bulletproof (Senior-Level Thinking)**

### **Mindset:** "How does this recover from catastrophic failures?"

### **What You Add:**

```c
// Senior-level production version
int32_t sensor_run(void)
{
    // ← Watchdog: Prove work is done, not just executing
    static uint32_t last_successful_meas_ms = 0;
    
    switch (state) {
        case IDLE:
            // ← Check if we're stuck not measuring
            if (got_meas && (tmr_get_ms() - last_successful_meas_ms > 10000)) {
                // No successful measurement in 10 seconds!
                log_error("Measurement stall detected\n");
                INC_SAT_U16(stall_count);
                
                // ← Forced recovery: Reset state machine
                i2c_release(BUS);  // Force release in case stuck
                state = IDLE;
                reserve_retry_count = 0;
            }
            break;
            
        case READ_WAIT:
            if (rc == 0) {
                if (validate_crc_and_sanity(data)) {
                    temperature = convert(data);
                    
                    // ← Trend analysis (detect sensor degradation)
                    if (abs(temperature - last_temperature) > 100) {
                        // 10°C jump in 1 second? Suspicious!
                        INC_SAT_U16(jump_count);
                        log_warn("Large temp jump: %ld to %ld\n", 
                                 last_temperature, temperature);
                    }
                    
                    // ← Moving average (noise filtering)
                    temperature = (temperature + last_temperature * 3) / 4;
                    
                    last_temperature = temperature;
                    last_meas_ms = tmr_get_ms();
                    last_successful_meas_ms = tmr_get_ms();  // ← Watchdog proof
                    got_meas = true;
                    
                    // ← Feed watchdog only on success
                    wdg_feed(WDG_SENSOR);
                    
                    INC_SAT_U16(success_count);
                }
                i2c_release(BUS);
                state = IDLE;
            }
            break;
    }
}
```

### **What Was Added:**

| Feature | Purpose | When You Need It |
|---------|---------|------------------|
| **Stall detection** | Catch "running but not working" | Mission-critical systems |
| **Forced recovery** | Escape from unknown stuck states | Safety-critical systems |
| **Trend analysis** | Detect sensor degradation | Predictive maintenance |
| **Noise filtering** | Smooth out spikes | Control systems |
| **Watchdog feeding** | Prove work completed | All production systems |

### **Skill Level:** Senior engineers design **self-healing systems**

---

# Part 4: The Build-From-Scratch Roadmap

## **Your Current Status:**
> "For now when I develop code from scratch, I only think of error handling case."

**That's Step 3 of 10!** Here's the full roadmap:

---

## **The 10-Step Reliability Checklist**

### **ALWAYS DO (Entry-Level Minimum):**

- **Step 1: Clean Code Structure**
  - [ ] Meaningful names
  - [ ] Clear functions (one purpose each)
  - [ ] Consistent formatting
  - [ ] Comments on "why"

- **Step 2: Correct Patterns**
  - [ ] Non-blocking state machine
  - [ ] Separate init/start/run
  - [ ] Resource reservation/release
  - [ ] Interrupt-driven I/O

- **Step 3: Basic Error Handling** ← **YOU ARE HERE**
  - [ ] Check all return codes
  - [ ] Clean up on errors
  - [ ] Return to safe state
  - [ ] Don't use invalid data

- **Step 4: Data Validation**
  - [ ] CRC/checksum on external data
  - [ ] Validate before using
  - [ ] Handle validation failures gracefully

---

### **ADD NEXT (Mid-Level Practices):**

- **Step 5: Error Counters**
  - [ ] Count each error type
  - [ ] Count successes too
  - [ ] Make counters queryable (console command)
  - [ ] Track patterns over time

- **Step 6: Sanity Checks**
  - [ ] Range validation on measurements
  - [ ] Detect impossible values
  - [ ] Fail fast on nonsense

- **Step 7: Timeout Protection**
  - [ ] Timeout on all waits/retries
  - [ ] Escape from stuck states
  - [ ] Don't retry forever

- **Step 8: Staleness Detection**
  - [ ] Track data age
  - [ ] Reject overly old data
  - [ ] Report age to caller

---

### **LEARN LATER (Senior-Level Practices):**

- **Step 9: Self-Healing**
  - [ ] Stall detection
  - [ ] Automatic recovery
  - [ ] Forced state reset

- **Step 10: System-Wide Reliability**
  - [ ] Watchdog integration
  - [ ] Fault module integration
  - [ ] Graceful degradation

---

# Part 5: Practical Application Guide

## **When Building Code From Scratch:**

### **Phase 1: Core Functionality (Day 1)**

**Focus:** Make the happy path work

**Apply:**
1. ✅ Clean code structure
2. ✅ Correct patterns
3. ✅ Basic error handling

**Skip (For Now):**
- Counters
- Sanity checks
- Timeouts
- Advanced features

**Why:** You need to prove the concept works before hardening it.

### **Test Criteria:**
```
✓ Does it work with sensor connected?
✓ Does it compile without warnings?
✓ Is the code readable?
✓ Does it handle basic errors?
```

---

### **Phase 2: Error Hardening (Day 2)**

**Focus:** Make it fail gracefully

**Apply:**
4. ✅ Data validation (CRC)
5. ✅ Error counters
6. ✅ Sanity checks
7. ✅ Timeout protection

**Why:** Now that core works, add defensive layers.

### **Test Criteria:**
```
✓ Disconnect sensor - does it recover?
✓ Send garbage data - does it detect?
✓ Hold I2C busy - does it timeout?
✓ Check counters - do they track errors?
```

---

### **Phase 3: Production Hardening (Week 2)**

**Focus:** Make it debuggable and self-healing

**Apply:**
8. ✅ Staleness detection
9. ✅ Self-healing mechanisms
10. ✅ Watchdog integration

**Why:** Field debugging and reliability.

### **Test Criteria:**
```
✓ Run for 24 hours - any crashes?
✓ Induce failures - does it recover?
✓ Review counters - what's the pattern?
✓ Trigger watchdog - does system restart?
```

---

# Part 6: Real-World Development Sequence

## **Example: Building Your TMPHM Module From Scratch**

### **Day 1: Core Functionality**

**What I'd Code:**

```c
// PHASE 1: Bare minimum to read sensor
static struct {
    enum states state;
    int32_t timer_id;
    uint8_t data[6];
    int32_t temperature;
} st;

int32_t tmphm_run(void)
{
    switch (st.state) {
        case IDLE: break;
        
        case RESERVE:
            if (i2c_reserve(3) == 0) {          // ← Basic error check
                i2c_write(3, 0x44, cmd, 2);
                state = WRITE_WAIT;
            }
            break;
            
        case WRITE_WAIT:
            if (i2c_get_status(3) == 0) {
                state = READ;
            }
            break;
        
        // ... etc
    }
}
```

**Mindset:** "Does this read temperature? Yes? Move to Phase 2."

---

### **Day 2: Add Defensive Layers**

**What I'd Add:**

```c
// PHASE 2: Add CRC, counters, sanity
static uint16_t crc_fail_count = 0;
static uint16_t range_error_count = 0;
static uint16_t success_count = 0;

case READ_WAIT:
    if (i2c_get_status(3) == 0) {
        // ← ADD: CRC validation
        if (crc8(&data[0], 2) != data[2]) {
            crc_fail_count++;           // ← ADD: Counter
            log_error("CRC fail\n");
            i2c_release(3);
            state = IDLE;
            break;
        }
        
        temperature = convert(data);
        
        // ← ADD: Sanity check
        if (temperature < -400 || temperature > 1250) {
            range_error_count++;        // ← ADD: Counter
            log_error("Range error\n");
            i2c_release(3);
            state = IDLE;
            break;
        }
        
        // All good!
        last_temperature = temperature;
        success_count++;                // ← ADD: Success counter
        i2c_release(3);
        state = IDLE;
    }
    break;
```

**Mindset:** "Now add detection for common failure modes."

---

### **Day 3: Add Timeout Protection**

**What I'd Add:**

```c
// PHASE 3: Prevent infinite retries and stuck states
static uint32_t reserve_attempts = 0;
static uint32_t state_entry_time = 0;

case RESERVE:
    rc = i2c_reserve(3);
    if (rc == 0) {
        reserve_attempts = 0;  // Success, reset counter
        i2c_write(3, 0x44, cmd, 2);
        state = WRITE_WAIT;
        state_entry_time = tmr_get_ms();  // ← Track state entry
    } else {
        reserve_attempts++;
        
        // ← Retry limit
        if (reserve_attempts > 20) {
            log_error("Reserve stuck after 20 attempts\n");
            reserve_attempts = 0;
            state = IDLE;  // Give up, try next cycle
        }
    }
    break;

case WRITE_WAIT:
    // ← State timeout
    if (tmr_get_ms() - state_entry_time > 500) {
        log_error("Write state timeout\n");
        i2c_release(3);
        state = IDLE;
        break;
    }
    
    if (i2c_get_status(3) == 0) {
        state = READ;
    }
    break;
```

**Mindset:** "What if something gets stuck? How do I escape?"

---

### **Week 2: Watchdog & Self-Healing**

**What I'd Add:**

```c
// PHASE 4: Prove work is done
static uint32_t last_successful_meas = 0;

case READ_WAIT:
    if (measurement_successful) {
        last_successful_meas = tmr_get_ms();
        wdg_feed(WDG_SENSOR);  // ← Feed only on success
    }
    break;

// In tmphm_run() entry:
// ← Stall detection
if (got_meas && (tmr_get_ms() - last_successful_meas > 10000)) {
    log_error("Measurement stalled for 10 seconds\n");
    // Force recovery
    i2c_release(3);
    state = IDLE;
}
```

**Mindset:** "System must auto-recover from failures."

---

# Part 7: The Mindset Evolution

## **Entry-Level Mindset:**

**Questions You Ask:**
- "How do I make this work?"
- "What's the algorithm?"
- "Is my code clean?"

**Focus:**
- Getting it working
- Understanding the domain
- Learning patterns

**Typical Code:**
- Core functionality ✅
- Basic error handling ✅
- Missing instrumentation ❌
- Missing edge case protection ❌

---

## **Mid-Level Mindset:**

**Questions You Ask:**
- "What can go wrong?"
- "How will I debug this in the field?"
- "What happens if the sensor fails?"
- "How do I know if this is working?"

**Focus:**
- Error detection
- Instrumentation
- Defensive programming
- Field debuggability

**Typical Code:**
- Core functionality ✅
- Comprehensive error handling ✅
- Error counters ✅
- Sanity checks ✅
- Missing system-wide integration ❌

---

## **Senior-Level Mindset:**

**Questions You Ask:**
- "What will fail that I haven't imagined?"
- "How does this recover without human intervention?"
- "What's the failure mode when X AND Y both fail?"
- "How does this degrade gracefully?"
- "What data do I need to diagnose unknown future problems?"

**Focus:**
- System reliability
- Fault recovery
- Self-healing
- Unknown unknowns

**Typical Code:**
- Everything from entry/mid ✅
- Watchdog integration ✅
- Automatic recovery ✅
- Graceful degradation ✅
- Comprehensive diagnostics ✅

---

# Part 8: Practical Roadmap for YOU

## **Your Current State:**
> "I only think of error handling case"

**That's ~30% of the reliability journey!**

---

## **Your Growth Roadmap:**

### **Month 1: Entry-Level Mastery**
**Add to your practice:**
- [x] Clean code ← **You already do this**
- [x] Correct patterns ← **You understand this**
- [x] Basic error handling ← **You're doing this**
- [ ] Data validation (CRC) ← **ADD THIS WEEK**

**Goal:** Build a simple module from scratch that handles basic errors

---

### **Month 2-3: Mid-Level Thinking**
**Add to your practice:**
- [ ] Error counters on EVERY error path
- [ ] Sanity checks on EVERY external data
- [ ] Timeout on EVERY wait/retry
- [ ] Staleness checks on EVERY cached data

**Goal:** Build a module that's field-debuggable

**Exercise:** 
- Intentionally disconnect sensor during operation
- Watch your error counters
- Use counters to diagnose the problem
- Feel the power of instrumentation!

---

### **Month 4-6: Senior-Level Awareness**
**Study (don't implement yet):**
- Watchdog patterns
- Fault recovery strategies
- System-wide reliability
- Graceful degradation

**Goal:** Understand the principles

---

### **Year 1: Apply Senior Patterns**
**Implement:**
- Watchdog integration
- Self-healing mechanisms
- Comprehensive diagnostics

**Goal:** Build production-quality modules

---

# Part 9: The Cheat Sheet

## **When Building Code From Scratch - Your Checklist:**

### **PHASE 1: Make It Work (First)**
```
[ ] Clean structure
[ ] Correct pattern (state machine)
[ ] Basic error handling (check returns)
[ ] Resource cleanup (release on error)
```
**Stop and Test:** Does happy path work?

---

### **PHASE 2: Make It Robust (Second)**
```
[ ] Add error counters (every error type)
[ ] Add CRC/data validation
[ ] Add sanity checks (range validation)
[ ] Add timeout protection (retry limits)
[ ] Add staleness detection
```
**Stop and Test:** Disconnect sensor - does it recover?

---

### **PHASE 3: Make It Debuggable (Third)**
```
[ ] Add console commands (status, counters)
[ ] Add detailed logging
[ ] Add performance metrics
[ ] Make all counters queryable
```
**Stop and Test:** Can you diagnose issues via console?

---

### **PHASE 4: Make It Bulletproof (Fourth)**
```
[ ] Add watchdog integration
[ ] Add stall detection
[ ] Add forced recovery
[ ] Add graceful degradation
```
**Stop and Test:** Run for 24 hours, induce failures

---

# Part 10: Summary - The Core Principles

## **The Three Questions to Ask at Each Level:**

### **Entry-Level:**
1. "Does it work?"
2. "Is it clean?"
3. "Did I check for errors?"

### **Mid-Level:**
1. "What can go wrong?"
2. "How will I know when it goes wrong?"
3. "Can I debug this 6 months from now?"

### **Senior-Level:**
1. "How does this fail safely?"
2. "Can it recover automatically?"
3. "What happens when multiple things fail simultaneously?"

---

## **Your Development Mantra:**

### **BUILD Phase:**
```
1. Make it work (core algorithm)
2. Make it clean (readable, maintainable)
3. Make it correct (patterns, non-blocking)
```

### **HARDEN Phase:**
```
4. Make it validated (CRC, sanity checks)
5. Make it visible (error counters)
6. Make it escapable (timeouts, limits)
```

### **BULLETPROOF Phase:**
```
7. Make it recoverable (watchdogs)
8. Make it diagnosable (comprehensive logging)
9. Make it self-healing (auto-recovery)
10. Make it field-ready (all of the above)
```

---

## **Final Wisdom from 40 Years:**

### **Entry-Level Engineers:**
- Focus on **functionality**
- Learn **patterns**
- Build **clean code**
- **This is the foundation** - get it right!

### **Mid-Level Engineers:**
- Focus on **robustness**
- Add **instrumentation**
- Think **defensively**
- **This is where reliability comes from**

### **Senior Engineers:**
- Focus on **recovery**
- Design **self-healing**
- Anticipate **unknown failures**
- **This is what separates good from great**

---

## **Your Immediate Action Plan:**

### **This Week:**
1. ✅ Keep studying the reference code (understand patterns)
2. ✅ Trace execution with concrete values (annotations help)
3. ⚠️ **Build a simple module from scratch** (prove competency)
4. ⚠️ **Add ONE counter** to your code (practice instrumentation)

### **Next Week:**
1. Add sanity checks to TMPHM
2. Add timeout to I2C reserve retry
3. Add staleness check to get_last_meas()
4. Compare your code to reference

### **Next Month:**
1. Build a new module from scratch (different sensor)
2. Apply all practices up to Step 8
3. Review with senior engineer
4. Iterate based on feedback

---

## **Remember:**

**You're not behind.**  
**You're learning in the right order.**  
**Entry → Mid → Senior is a JOURNEY, not a destination.**

**The fact you're asking these questions?**  
**That's the mindset that will make you a senior engineer someday!** 🚀

---

**End of Document**

---

**Next Steps:**
1. Read this document
2. Identify where you are (Entry-Level, learning fundamentals)
3. Pick ONE practice from the next level to add this week
4. Build something from scratch to prove understanding
5. Iterate and grow!

**You've got this!** 💪

