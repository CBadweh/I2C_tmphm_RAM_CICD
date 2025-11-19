# Fault Injection Testing Guide

**Purpose:** Simulate real-world failures in software without physical hardware changes

**Why we have this:** Testing error handling code requires triggering errors. Fault injection lets you test "what happens if the sensor is unplugged?" without actually unplugging it.

---

## Quick Reference

| Command | What It Does | Use Case |
|---------|--------------|----------|
| `i2c test wrong_addr` | Toggle wrong address fault | Test "device doesn't exist" error |
| `i2c test nack` | Toggle NACK fault | Test "sensor unplugged" error |
| `i2c test auto` | Run I2C test | Execute test with active faults |
| `i2c test not_reserved` | Test without reserve | Validate state machine protection |

---

## What is Fault Injection?

**Problem:** You need to test error handling, but creating real errors is hard:
- ❌ Unplugging sensor during test (manual, not repeatable)
- ❌ Hardcoding wrong address in code (requires rebuild, easy to forget to revert)
- ❌ Physically damaging hardware (expensive, dangerous)

**Solution:** Fault injection simulates errors in software:
- ✅ Toggle faults on/off via console commands
- ✅ No hardware changes needed
- ✅ Repeatable and automatable (CI/CD friendly)
- ✅ Safe (can't accidentally commit test code)

---

## Available Fault Types

### 1. Wrong Address Fault

**Command:** `i2c test wrong_addr`

**What it does:** Uses I2C address `0x45` instead of the real sensor address (`0x44`)

**Simulates:** Trying to communicate with a non-existent device

**Expected error:** `I2C_ERR_ACK_FAIL` (slave doesn't acknowledge)

**Real-world scenario:** You hardcoded the wrong I2C address in your code

---

### 2. NACK Fault (Unplugged Sensor)

**Command:** `i2c test nack`

**What it does:** Forces any I2C error to become `ACK_FAIL`

**Simulates:** Sensor is unplugged or not responding

**Expected error:** `I2C_ERR_ACK_FAIL`

**Real-world scenario:** Sensor loses power, wire disconnected, sensor failure

---

### 3. Not Reserved Test (State Violation)

**Command:** `i2c test not_reserved`

**What it does:** Attempts I2C operation without calling `i2c_reserve()` first

**Simulates:** Incorrect API usage (programming error)

**Expected error:** `MOD_ERR_NOT_RESERVED`

**Real-world scenario:** Developer forgets to reserve bus before using it

**Note:** This uses Pattern B (execute immediately) because it's a simple validation test, not a realistic failure scenario.

---

## Usage Pattern: Toggle vs Execute

### Pattern A: Toggle Flag (wrong_addr, nack)

**Usage:**
```
>> i2c test wrong_addr    # Step 1: Enable fault
>> i2c test auto          # Step 2: Run test (with fault active)
>> i2c test auto          # Can run multiple times
>> i2c test wrong_addr    # Step 3: Disable fault
```

**Why this pattern:**
- One fault can affect multiple test runs
- Enables CI/CD automation (script multiple tests under same fault)
- Simulates how real faults behave (persist across operations)

### Pattern B: Execute Immediately (not_reserved)

**Usage:**
```
>> i2c test not_reserved  # Runs test immediately, one-shot
```

**Why this pattern:**
- Simple validation test
- No need to persist across operations
- Self-contained (stateless)

**For detailed comparison**, see [i2c_module.md - Pattern Comparison](../i2c_module.md#fault-injection-pattern-comparison-toggle-vs-execute-immediately)

---

## Step-by-Step Test Procedures

### Test 1: Wrong Address Error Handling

**Goal:** Verify driver detects when I2C address doesn't exist

**Steps:**
1. Open serial console (115200 baud)
2. Enable wrong address fault:
   ```
   >> i2c test wrong_addr
   ```
3. Observe output:
   ```
   ========================================
     Fault Injection: Wrong Address
   ========================================
     Status: ENABLED
     Next I2C operation will use address 0x45 instead of actual address
     This simulates addressing a non-existent device
   ========================================
   ```
4. Run auto test:
   ```
   >> i2c test auto
   ```
5. Expected result: Test should detect `I2C_ERR_ACK_FAIL`
6. Disable fault:
   ```
   >> i2c test wrong_addr
   ```
7. Observe output:
   ```
   ========================================
     Fault Injection: Wrong Address
   ========================================
     Status: DISABLED
     Normal addressing restored
   ========================================
   ```

**What you learned:** Driver correctly detects non-existent devices

---

### Test 2: Unplugged Sensor Error Handling

**Goal:** Verify driver handles sensor failures gracefully

**Steps:**
1. Enable NACK fault:
   ```
   >> i2c test nack
   ```
2. Observe output:
   ```
   ========================================
     Fault Injection: NACK (Unplugged Sensor)
   ========================================
     Status: ENABLED
     Next I2C error will be forced to ACK_FAIL
     This simulates an unplugged or non-responsive sensor
   ========================================
   ```
3. Run auto test:
   ```
   >> i2c test auto
   ```
4. Expected result: Any error becomes `I2C_ERR_ACK_FAIL` (simulates unplugged sensor)
5. Disable fault:
   ```
   >> i2c test nack
   ```

**What you learned:** Driver correctly handles sensor disconnection

---

### Test 3: State Violation Detection

**Goal:** Verify driver rejects operations when bus not reserved

**Steps:**
1. Run test:
   ```
   >> i2c test not_reserved
   ```
2. Expected result: Test detects `MOD_ERR_NOT_RESERVED` error

**What you learned:** Driver enforces proper API usage (reserve before use)

---

## Expected Console Output

### Successful Wrong Address Test

```
>> i2c test wrong_addr
========================================
  Fault Injection: Wrong Address
========================================
  Status: ENABLED
  Next I2C operation will use address 0x45 instead of actual address
  This simulates addressing a non-existent device
========================================

>> i2c test auto
[Test runs, detects ACK_FAIL error as expected]

>> i2c test wrong_addr
========================================
  Fault Injection: Wrong Address
========================================
  Status: DISABLED
  Normal addressing restored
========================================
```

---

## CI/CD Automation (Future)

While not implemented yet, fault injection enables automated testing via UART:

```python
# Python test script example (documentation only)
uart.send("i2c test wrong_addr\n")   # Enable fault
uart.send("i2c test auto\n")         # Run test
response = uart.read_until_timeout()
assert "ACK_FAIL" in response        # Verify error detected
uart.send("i2c test wrong_addr\n")   # Disable fault
```

**This enables:**
- Automated regression testing
- CI/CD pipeline integration
- Repeatable error path coverage
- Professional development workflow

---

## Implementation Details

### File Locations

**Header file:**
- `Badweh_Development/modules/include/i2c.h` (lines 41-45, 106-111)
- Defines `ENABLE_FAULT_INJECTION` (only when `DEBUG` is defined)

**Implementation:**
- `Badweh_Development/modules/i2c/i2c.c`
- State variables: lines 107-112
- Wrong address injection: lines 321, 382
- NACK injection: lines 612-623
- Test functions: lines 831-873
- Console commands: lines 889-892, 933-953, 966-981

### Conditional Compilation

**Debug builds (fault injection enabled):**
```c
#ifdef DEBUG
    #define ENABLE_FAULT_INJECTION
#endif
```

**Release builds (fault injection removed):**
- No `DEBUG` define → No `ENABLE_FAULT_INJECTION` → Zero overhead
- Fault injection code completely removed from binary
- Production-safe (test code cannot exist in Release builds)

### How It Works

**Wrong Address Fault:**
```c
#ifdef ENABLE_FAULT_INJECTION
    st->dest_addr = fault_inject_wrong_addr ? 0x45 : dest_addr;
#else
    st->dest_addr = dest_addr;  // Zero overhead in Release!
#endif
```

**NACK Fault:**
```c
#ifdef ENABLE_FAULT_INJECTION
    if (fault_inject_nack) {
        // Force ACK_FAIL error on any I2C error
        op_stop_fail(st, I2C_ERR_ACK_FAIL);
        return;
    }
#endif
```

---

## Debug vs Release Builds

### Debug Build (Development/Testing)

**How to build:**
```bash
cd Badweh_Development/ci-cd-tools
build.bat  # Uses Debug configuration by default
```

**Features:**
- ✅ Fault injection enabled
- ✅ Debug symbols included
- ✅ No optimization (easier debugging)
- ✅ Console test commands available

**When to use:** Development, testing, CI/CD automation

---

### Release Build (Production)

**How to build:** Configure STM32CubeIDE for Release configuration

**Features:**
- ✅ Fault injection completely removed
- ✅ Optimized for size/speed
- ✅ No debug symbols
- ✅ Zero testing overhead

**When to use:** Production deployment, final product

---

## Professional Context

### Industry Usage

This fault injection pattern is used by:

**Automotive (ISO 26262):**
- Fault injection required for safety certification
- Must test error paths across all scenarios
- Automated testing mandatory

**Aerospace (NASA/JPL):**
- Software fault injection for Mars rovers
- Test mission-critical error handling
- Repeatable, documented test evidence

**Medical Devices (FDA):**
- Fault injection testing for regulatory approval
- Automated, traceable test results
- Error path coverage requirements

### Why Toggle Pattern (Pattern A)?

**Pattern A (our implementation):**
- ✅ One fault affects multiple tests
- ✅ CI/CD automation friendly
- ✅ Industry standard
- ✅ Required for certification

**Pattern B (execute immediately):**
- ❌ Limited to single test
- ❌ Hard to automate complex scenarios
- ❌ Not used in safety-critical systems

**For detailed analysis**, see [i2c_module.md](../i2c_module.md) (lines 5870-6275)

---

## Troubleshooting

### Fault injection commands not found

**Problem:** Commands like `i2c test wrong_addr` don't work

**Solution:** Check that you're running a Debug build
```bash
# Rebuild with Debug configuration
cd Badweh_Development/ci-cd-tools
build.bat
```

### Fault stays enabled after test

**Problem:** Forgot to disable fault, subsequent tests fail

**Solution:** Toggle the fault command again to disable:
```
>> i2c test wrong_addr    # If enabled, this disables it
```

**Tip:** Faults are toggle switches - call once to enable, call again to disable

### Test doesn't trigger error

**Problem:** Enabled fault but test still passes

**Solution:** Make sure you run `i2c test auto` AFTER enabling the fault:
```
# Correct order:
>> i2c test wrong_addr    # Enable fault first
>> i2c test auto          # Then run test
```

---

## Quick Reference Card

**Print this for quick reference:**

```
Fault Injection Commands:

Enable/Disable:
  i2c test wrong_addr    Toggle wrong address (0x45)
  i2c test nack          Toggle NACK fault

Execute Test:
  i2c test auto          Run I2C test with active faults
  i2c test not_reserved  Test state violation (one-shot)

Pattern:
  1. Enable fault (toggle ON)
  2. Run test (i2c test auto)
  3. Disable fault (toggle OFF)

Remember:
  - Only in Debug builds
  - Faults are toggles (call twice to disable)
  - Zero production overhead (Release builds)
```

---

## Learning Resources

**Related documentation:**
- [i2c_module.md](../i2c_module.md) - Complete I2C driver reference (5800+ lines)
  - Fault injection deep dive (lines 1380-2007)
  - Pattern comparison (lines 5870-6275)
  - Conditional compilation explanation (lines 2011-2442)

**Why this matters:**
- Professional embedded systems require error testing
- Manual testing is not scalable or repeatable
- CI/CD automation is industry standard
- This pattern is used in automotive, aerospace, and medical devices

---

## Summary

**What you learned:**
- ✅ Fault injection simulates real-world failures in software
- ✅ Two patterns: Toggle (flexible) vs Execute (simple)
- ✅ Debug builds include testing, Release builds don't
- ✅ Industry-standard approach for safety-critical systems

**Why it matters:**
- Testing error paths is as important as testing happy paths
- Automated testing saves time and catches regressions
- Professional embedded development requires repeatable tests
- This approach scales from hobby projects to production systems

**Next steps:**
- Try running the fault injection tests yourself
- Experiment with different test sequences
- Consider adding CI/CD automation in the future
- Apply this pattern to other modules (UART, SPI, etc.)

---

**Documentation maintained:** 2025-11-19
**Related files:** `README.md`, `docs/build.md`, `i2c_module.md`
