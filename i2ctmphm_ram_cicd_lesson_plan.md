# Week-Long Lesson Plan: I2C Temperature/Humidity Sensor with RAM and CI/CD
## Gene Schrader's Embedded Systems Integration Course

**Target Student**: Junior-level robotics and embedded systems engineer  
**Hardware**: STM32F401RE Nucleo board, SHT31-D sensor, Blue Pill (optional for HIL)  
**Duration**: 7 days (approximately 3-4 hours per day)  
**Goal**: Build a production-quality I2C temperature/humidity monitoring system with RAM features and CI/CD automation

---

## **Day 1: Foundations - I2C Theory and Git Repository Setup**

### Morning Session (2 hours)
**I2C Theory and Protocol Understanding**

**Topics:**
- I2C bus fundamentals (2-wire communication, master/slave architecture)
- Open-drain signals and pull-up resistors
- START/STOP conditions, ACK/NACK mechanism
- 7-bit addressing scheme
- Clock stretching and combined format

**Key Concepts to Master:**
- Why I2C uses only 2 signals (SDA, SCL) vs. SPI
- How multiple devices share the same bus
- Bus arbitration and speed considerations (100kHz standard, 400kHz fast)

**Practical Exercise:**
- Review SHT31-D datasheet sections:
  - Address configuration (0x44 vs 0x45)
  - Command structure (Table 8)
  - Measurement timing (Table 4: 15ms for high repeatability)
  - Data format (6 bytes: temp MSB/LSB/CRC, humidity MSB/LSB/CRC)

### Afternoon Session (2 hours)
**Git Repository and Project Structure Setup**

**Topics:**
- Creating STM32CubeIDE project for bare-metal I2C
- Git fundamentals for embedded projects
- What belongs in source control vs. build outputs

**Hands-On:**
```bash
# Initialize repository
cd [project_folder]
git init
git config user.name "Your Name"
git config user.email "your@email.com"

# Create .gitignore for embedded project
# Exclude: *.o, *.elf, *.map, *.su
# Include: makefiles, linker scripts, source code

# First commit
git add .
git commit -m "initial STM32CubeIDE project with I2C3 configured"
```

**IDE Configuration:**
- Configure I2C3 peripheral (PB4 for SDA, 100kHz standard mode)
- Select **Low Level (LL) library** (not HAL) for I2C
- Generate initialization code

**Deliverable:** Working repository with configured I2C peripheral, ready for driver development

---

## **Day 2: I2C Driver Module - Design and Core Implementation**

### Morning Session (2 hours)
**I2C Driver Requirements and State Machine Design**

**Minimum Viable Product Requirements:**
1. Master operation only (no slave mode)
2. Simple read-only and write-only operations
3. 7-bit addressing support
4. Non-blocking APIs (critical for super loop)
5. Shared bus support (reservation mechanism)

**State Machine Design:**
```c
enum states {
    STATE_IDLE,
    STATE_MSTR_WR_GEN_START,
    STATE_MSTR_WR_SENDING_ADDR,
    STATE_MSTR_WR_SENDING_DATA,
    STATE_MSTR_RD_GEN_START,
    STATE_MSTR_RD_SENDING_ADDR,
    STATE_MSTR_RD_READING_DATA,
};
```

**Data Structure:**
```c
struct i2c_state {
    struct i2c_cfg cfg;
    I2C_TypeDef* i2c_reg_base;
    int32_t guard_tmr_id;
    uint8_t* msg_bfr;
    uint32_t msg_len;
    uint32_t msg_bytes_xferred;
    uint16_t dest_addr;
    bool reserved;
    enum states state;
    enum i2c_errors last_op_error;
};
```

### Afternoon Session (2 hours)
**Interrupt-Based I2C Driver Implementation**

**Key Implementation Points:**
1. **Reservation System** - Simple boolean flag (honor-based)
2. **Interrupt Handlers** - Separate for events (EV) and errors (ER)
3. **Guard Timer** - Overall transaction timeout
4. **Error Handling** - Stop transaction on unexpected events

**Critical Code:**
```c
// Override weak default interrupt handlers
void I2C3_EV_IRQHandler(void)
{
    i2c_interrupt(I2C_INSTANCE_3, INTER_TYPE_EVT, I2C3_EV_IRQn);
}

void I2C3_ER_IRQHandler(void)
{
    i2c_interrupt(I2C_INSTANCE_3, INTER_TYPE_ERR, I2C3_ER_IRQn);
}
```

**Testing Strategy:**
- Console test commands for manual validation
- Logic analyzer verification (Saleae Logic 2)
- Test write operations (2 bytes to sensor)
- Test read operations (6 bytes from sensor)

**Deliverable:** Functioning I2C driver module with console test interface

**Commit:** `git commit -m "add I2C driver module with interrupt support"`

---

## **Day 3: Temperature/Humidity Module and Lightweight Logging**

### Morning Session (2.5 hours)
**TMPHM Module - Sensor-Specific Driver**

**State Machine for Measurement Cycle:**
```c
enum states {
    STATE_IDLE,
    STATE_RESERVE_I2C,
    STATE_WRITE_MEAS_CMD,
    STATE_WAIT_MEAS,
    STATE_READ_MEAS_VALUE
};
```

**Measurement Command:**
```c
const char sensor_i2c_cmd[2] = {0x2c, 0x06};  // Clock stretching, high repeatability
```

**Data Conversion (integer math to avoid floating point):**
```c
// Temperature: -45 to +130°C, stored as degC × 10
temp = -450 + (1750 * raw_temp + 32767) / 65535;

// Humidity: 0 to 100%, stored as % × 10
hum = (1000 * raw_hum + 32767) / 65535;
```

**CRC-8 Validation:**
```c
static uint8_t crc8(const uint8_t *data, int len)
{
    const uint8_t polynomial = 0x31;
    uint8_t crc = 0xff;
    
    for (idx1 = len; idx1 > 0; idx1--) {
        crc ^= *data++;
        for (idx2 = 8; idx2 > 0; idx2--) {
            crc = (crc & 0x80) ? (crc << 1) ^ polynomial : (crc << 1);
        }
    }
    return crc;
}
```

### Afternoon Session (1.5 hours)
**Lightweight Logging (LWL) Integration**

**Why LWL is Critical:**
- Flight recorder for embedded systems
- Minimal runtime overhead (no string formatting)
- Captures pre-fault activity in circular buffer
- Essential for maintainability

**Data Structure:**
```c
struct lwl_data {
    uint32_t magic;
    uint32_t num_section_bytes;
    uint32_t buf_size;
    uint32_t put_idx;
    uint8_t buf[LWL_BUF_SIZE];  // Default 1008 bytes
};
```

**Usage Pattern:**
```c
// In your module (.c file)
#define LWL_BASE_ID 20
#define LWL_NUM 10

// In code
LWL("state=%d mask=0x%04x", 3, LWL_1(state), LWL_2(0xFF00));
```

**Integration Points:**
- Add LWL statements to I2C state transitions
- Add LWL statements to TMPHM measurement cycle
- Enable early in `app_main.c`: `lwl_enable(true);`

**Deliverable:** Temperature/humidity module with background sampling + LWL integration

**Commit:** `git commit -m "add tmphm module with LWL instrumentation"`

---

## **Day 4: Fault Handling and Watchdog Protection**

### Morning Session (2.5 hours)
**Fault Module - Panic Mode and Data Collection**

**Supported Fault Types:**
1. CPU-detected faults (memory violations, stack overflow, divide-by-zero)
2. Watchdog triggers
3. Application-detected faults (data structure corruption)

**Fault Data Recording:**
```c
struct fault_data {
    uint32_t magic;
    uint32_t fault_type;
    uint32_t fault_param;
    
    // Exception stack frame (automatic by ARM)
    uint32_t excpt_stk_r0;
    uint32_t excpt_stk_r1;
    uint32_t excpt_stk_r2;
    uint32_t excpt_stk_r3;
    uint32_t excpt_stk_r12;
    uint32_t excpt_stk_lr;
    uint32_t excpt_stk_rtn_addr;
    uint32_t excpt_stk_xpsr;
    
    uint32_t sp;
    uint32_t lr;
    
    // ARM system registers
    uint32_t ipsr;
    uint32_t icsr;
    uint32_t shcsr;
    uint32_t cfsr;
    uint32_t hfsr;
    uint32_t mmfar;
    uint32_t bfar;
    uint32_t tick_ms;
};
```

**Panic Mode Actions:**
1. Disable interrupts immediately
2. Reset stack pointer to top of RAM
3. Use polling mode for all hardware
4. Disable MPU to avoid secondary faults
5. Feed hardware watchdog
6. Collect diagnostic data
7. Write to flash AND console
8. Reset MCU via `NVIC_SystemReset()`

**Flash Storage Strategy:**
- Reserve second flash page (16KB) for fault data
- Don't overwrite existing fault data (prevents flash wear in boot loops)
- Section-based format: Fault data + LWL buffer + End marker

### Afternoon Session (1.5 hours)
**Watchdog Module - Software and Hardware Protection**

**Multiple Software Watchdogs:**
```c
struct soft_wdg {
    uint32_t period_ms;
    uint32_t last_feed_time_ms;
};
```

**Key Principle:** Watchdogs ensure critical work is DONE, not just code executing

**Timer Callback (Heart of the System):**
```c
static enum tmr_cb_action wdg_tmr_cb(int32_t tmr_id, uint32_t user_data)
{
    // Check all software watchdogs
    for (idx = 0; idx < CONFIG_WDG_NUM_WDGS; idx++) {
        if (tmr_get_ms() - soft_wdg->last_feed_time_ms > soft_wdg->period_ms) {
            wdg_triggered = true;
            state.triggered_cb(idx);  // Call fault module
        }
    }
    
    // Only feed hardware watchdog if no software watchdog triggered
    if (!wdg_triggered) {
        wdg_feed_hdw();
    }
}
```

**No-Init RAM for Initialization Watchdog:**
- Track consecutive failed initializations across resets
- Linker script modification: `.no.init.vars` section
- After N failures, stop enabling watchdog (let system try without time limit)

**Integration:**
- Register TMPHM watchdog: `wdg_register(WDG_TMPHM, 5000);`  // 5 second timeout
- Feed on successful measurement: `wdg_feed(WDG_TMPHM);`
- Fault module registers callback: `wdg_register_triggered_cb(wdg_triggered_handler);`

**Deliverable:** Complete fault handling and watchdog protection system

**Commit:** `git commit -m "add fault module and watchdog protection"`

---

## **Day 5: Stack Protection and CI/CD Foundation**

### Morning Session (2 hours)
**MPU-Based Stack Guard and High Water Mark**

**Memory Map Modification (linker script):**
```ld
_min_mpu_region_size = 32;

. = ALIGN(4);
PROVIDE(end = .);
. = . + _Min_Heap_Size;
. = ALIGN(_min_mpu_region_size);
_s_stack_guard = .;
. = . + _min_mpu_region_size;
_e_stack_guard = .;
. = . + _Min_Stack_Size;
. = ALIGN(8);
PROVIDE(_estack = .);
```

**Stack Initialization Pattern:**
```c
#define STACK_INIT_PATTERN 0xcafebadd

__ASM volatile("MOV  %0, sp" : "=r" (sp) : : "memory");
sp--;
while (sp >= &_s_stack_guard)
    *sp-- = STACK_INIT_PATTERN;
```

**MPU Configuration:**
- 32-byte read-only guard region
- Triggers MemManage fault on stack overflow
- Provides immediate detection and safe reset

**High Water Mark Detection:**
```c
sp = &_e_stack_guard;
while (sp < &_estack && *sp == STACK_INIT_PATTERN)
    sp++;
// sp now points to high water mark
```

### Afternoon Session (2 hours)
**Git Remote Repository and Automation Basics**

**Create Bare Repository (Git Server):**
```bash
cd c:/repos
mkdir project.git
cd project.git
git init --bare
```

**Configure Remote in IDE Project:**
```bash
cd [ide_project]
git remote add origin file:///C:/repos/project.git
git push -u origin master
```

**Build Automation - Direct Make Approach:**
```batch
@echo off
setlocal

set CONFIG=%1
set TARGET=%2
set WORKSPACE=%3

cd "%WORKSPACE%\Debug"
make -j4 %TARGET%

exit /b %ERRORLEVEL%
```

**Flash Automation - STM32CubeProgrammer CLI:**
```batch
@echo off
setlocal

set CONFIG=%1
set WORKSPACE=%2
set IMAGE=%WORKSPACE%\Debug\project.elf
set STLINK=<serial_number>

STM32_Programmer_CLI.exe -c port=SWD sn=%STLINK% -d %IMAGE% 0x08000000 -hardRst

exit /b %ERRORLEVEL%
```

**Critical Fix - Linker Script Path:**
- Change from absolute path to relative path in IDE settings
- `../STM32F401RETX_FLASH.ld` instead of full workspace path
- Required for Jenkins builds to work

**Deliverable:** Stack overflow protection + automation scripts ready for Jenkins

**Commit:** `git commit -m "add stack protection and build automation scripts"`

---

## **Day 6: Static Analysis and HIL Testing**

### Morning Session (2 hours)
**Static Code Analysis with Cppcheck**

**Benefits in CI/CD Pipeline:**
- Automated analysis on every code change
- Finds potential bugs without running code
- Quick feedback while code is fresh

**Cppcheck Script:**
```batch
@echo off
cppcheck --enable=all --suppress=missingIncludeSystem ^
         --xml --xml-version=2 ^
         --output-file=cppcheck-results.xml ^
         %WORKSPACE%\app1\*.c
         
exit /b %ERRORLEVEL%
```

**Handling False Positives:**
```c
// cppcheck-suppress comparePointers
if (sp < &_estack) { ... }
```

### Afternoon Session (2.5 hours)
**Hardware-in-the-Loop Testing with Python**

**Test Architecture:**
- Build server runs Python test script
- plink (PuTTY serial terminal) connects to boards
- Test script sends commands, verifies responses
- Results saved as JUnit XML for Jenkins

**Critical Test - Version Verification:**
```python
def test_version(tver):
    """Ensure testing correct software version (Ring Doorbell lesson!)"""
    passed = True
    start_test('version')
    g_dut.send_line('version')
    pat_list = ['Version="' + tver + '"', g_prompt]
    rc, failed_pat = g_dut.get_pattern_list(pat_list)
    if rc != 0:
        test_fail('Did not find pattern "%s"' % failed_pat)
        passed = False
    else:
        test_pass()
    return passed
```

**Data-Driven GPIO Tests:**
```python
'temp-read' : [
    [g_dut, 'i2c test reserve 0', 'OK'],
    [g_dut, 'i2c test write 0 44 2c 06', 'OK'],
    [g_dut, 'delay 20', 'OK'],
    [g_dut, 'i2c test read 0 44 6', 'OK'],
    [g_dut, 'tmphm test lastmeas 0', r'Temp=\d+\.\d+ C'],
    [g_dut, 'i2c test release 0', 'OK'],
],
```

**JUnit XML Generation:**
```python
import junit_xml as jux

test_suite = jux.TestSuite('HIL Tests', g_test_junits)
with open('test-results.xml', 'w') as f:
    jux.TestSuite.to_file(f, [test_suite], prettyprint=True)
```

**Deliverable:** Static analysis script + Python HIL test suite with 15+ tests

**Commit:** `git commit -m "add static analysis and HIL test automation"`

---

## **Day 7: Jenkins Integration and End-to-End CI/CD**

### Morning Session (2.5 hours)
**Jenkins Pipeline Setup**

**Install Required Plugins:**
- Extended Email Notification
- Generic Webhook Trigger Plugin
- JUnit Plugin

**Jenkinsfile Structure (Declarative):**
```groovy
pipeline {
    agent any
    
    parameters {
        string(name: 'COM_PORT_DUT', defaultValue: 'COM3', description: 'DUT serial port')
        string(name: 'STLINK_SERIAL', defaultValue: '066EFF505251727867233656', description: 'ST-Link serial')
    }
    
    environment {
        TOOL_DIR = "${WORKSPACE}/ci-cd-tools"
    }
    
    stages {
        stage('Build') {
            steps {
                bat "\"${TOOL_DIR}/build.bat\" Debug all \"${WORKSPACE}\""
                bat "\"${TOOL_DIR}/build.bat\" Release all \"${WORKSPACE}\""
            }
        }
        
        stage('Static Code Analysis') {
            steps {
                catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
                    bat "\"${TOOL_DIR}/static-analysis.bat\""
                }
            }
        }
        
        stage('Program Flash Debug') {
            steps {
                bat "\"${TOOL_DIR}/flash.bat\" Debug \"${WORKSPACE}\""
            }
        }
        
        stage('Test Debug') {
            steps {
                bat "python3 \"${TOOL_DIR}/base-hilt.py\" --jfile test-results.xml --tver ${BUILD_ID} --dut-serial ${COM_PORT_DUT}"
                junit 'test-results.xml'
                bat "\"${TOOL_DIR}/check-test-results.bat\" test-results.xml"
            }
        }
        
        stage('Program Flash Release') {
            steps {
                bat "\"${TOOL_DIR}/flash.bat\" Release \"${WORKSPACE}\""
            }
        }
        
        stage('Test Release') {
            steps {
                bat "python3 \"${TOOL_DIR}/base-hilt.py\" --jfile test-results.xml --tver ${BUILD_ID} --dut-serial ${COM_PORT_DUT}"
                junit 'test-results.xml'
                bat "\"${TOOL_DIR}/check-test-results.bat\" test-results.xml"
            }
        }
    }
    
    post {
        success {
            bat "\"${TOOL_DIR}/deliver.bat\" \"${WORKSPACE}\""
        }
        
        unsuccessful {
            emailext (
                subject: "${env.JOB_NAME} - Build #${env.BUILD_NUMBER} - ${currentBuild.result}",
                body: "Check console output at ${env.BUILD_URL}",
                to: "your.email@example.com"
            )
        }
    }
}
```

### Afternoon Session (1.5 hours)
**Git Hooks and End-to-End Testing**

**Post-Update Hook (Server-Side):**
```bash
#!/bin/sh
curl http://jenkins:password@localhost:8080/generic-webhook-trigger/invoke?token=YOUR_TOKEN
```

**File Location:**
```
C:/repos/project.git/hooks/post-update
```

**End-to-End Workflow Test:**
1. Introduce deliberate bug (null pointer dereference)
2. Commit and push to Git
3. Verify Git hook triggers Jenkins
4. Monitor Jenkins pipeline execution
5. Confirm static analysis catches bug (build marked UNSTABLE)
6. Verify email notification received
7. Fix bug and push again
8. Confirm successful build and delivery

**Final Integration Test:**
- All modules running together
- TMPHM taking measurements every 1 second
- Watchdog protecting system
- LWL recording activity
- Fault module ready for panic mode
- Stack guard active
- CI/CD pipeline automated

**Deliverable:** Fully functional CI/CD pipeline with automated build, test, and delivery

**Commit:** `git commit -m "complete CI/CD pipeline with Jenkins automation"`

---

## **Week Summary and Key Takeaways**

### Technical Achievements
✓ **I2C Communication**: Interrupt-driven driver with state machine  
✓ **Sensor Integration**: SHT31-D with CRC validation and integer math  
✓ **Lightweight Logging**: Flight recorder with minimal overhead  
✓ **Fault Handling**: Comprehensive panic mode with flash storage  
✓ **Watchdog Protection**: Hierarchical software + hardware watchdogs  
✓ **Stack Protection**: MPU guard + high water mark detection  
✓ **Static Analysis**: Automated Cppcheck integration  
✓ **HIL Testing**: Python-driven hardware validation  
✓ **CI/CD Pipeline**: Full Jenkins automation with Git hooks

### Production-Quality Practices Learned

**Reliability:**
- Fault detection via watchdogs and MPU
- CRC data validation
- Guard timers for all operations
- Comprehensive error handling

**Availability:**
- Automatic fault recovery via MCU reset
- Non-blocking APIs throughout
- Resource reservation mechanisms
- Watchdog hierarchy (software checks work, hardware checks software)

**Maintainability:**
- Lightweight logging captures pre-fault activity
- Fault data preserved in flash across resets
- Console test commands for all modules
- Python offline tools for data formatting
- Static analysis catches bugs before runtime
- HIL testing validates every build

**CI/CD Integration:**
- Automated builds prevent human error
- Early feedback on every commit
- Consistent test execution
- Automated delivery of validated releases

### Lessons from 40 Years of Experience

**From Gene Schrader:**

1. **"Bugs are inevitable in complex systems"** - Focus on detection and recovery, not prevention alone

2. **"Watchdogs ensure critical work is DONE, not just that code is EXECUTING"** - Monitor outcomes, not activity

3. **"Even simple HIL testing adds significant value"** - Don't let perfect be the enemy of good enough

4. **"The Ring Doorbell story"** - Always verify build ID in tests to avoid shipping untested software

5. **"Stack overflow is a nasty problem"** - Developers spend considerable time before realizing it's the issue

6. **"Lightweight logging is a flight recorder"** - Critical for understanding what happened before crashes

7. **"Keep Jenkinsfile lean"** - More logic in your own scripts = less Jenkins dependency

8. **"Field conditions are much more complex than test labs"** - What works in development may trigger unexpected behaviors in production

### Next Steps for Continued Learning

**Week 2 Focus Areas:**
- RTOS integration (migrate from super loop)
- DMA optimization for I2C transfers
- Additional sensor integration (pressure, acceleration)
- Network connectivity (IoT backend)
- Power management and low-power modes
- Code coverage analysis
- MISRA-C compliance checking

**Advanced Topics:**
- Bootloader design for field updates
- Secure firmware updates
- Multi-board systems with CAN bus
- Real-time plotting of sensor data
- Advanced fault analysis techniques
- Performance optimization with profiling

---

## **Daily Time Allocation Reference**

| Day | Morning (2-2.5h) | Afternoon (1.5-2h) | Total |
|-----|------------------|-------------------|-------|
| 1 | I2C Theory | Git Setup | 4h |
| 2 | I2C Driver Design | I2C Implementation | 4h |
| 3 | TMPHM Module | LWL Integration | 4h |
| 4 | Fault Handling | Watchdog Protection | 4h |
| 5 | Stack Protection | CI/CD Foundation | 4h |
| 6 | Static Analysis | HIL Testing | 4.5h |
| 7 | Jenkins Setup | End-to-End Testing | 4h |

**Total Course Time:** ~28.5 hours

---

## **Required Tools and Setup**

**Hardware:**
- STM32F401RE Nucleo board
- Adafruit SHT31-D sensor breakout
- USB cables (Nucleo has built-in ST-Link)
- Logic analyzer (optional: Saleae compatible)
- Blue Pill board (optional: for advanced HIL)

**Software:**
- STM32CubeIDE (with LL library support)
- Git for Windows
- Python 3.x (with pexpect, junit_xml, plink)
- Jenkins
- Cppcheck
- ST-Link utilities / STM32CubeProgrammer CLI
- Serial terminal (PuTTY/TeraTerm)

**Configuration Files Provided:**
- .gitignore for embedded projects
- Linker script modifications
- Jenkinsfile template
- Python HIL test framework
- Build automation batch scripts

---

**Course Philosophy:** *Build production-quality embedded systems by integrating communication protocols, reliability features, and modern DevOps practices. Learn from 40 years of real-world experience - not just theory, but battle-tested techniques that prevent the "gut-wrenching" moments that happen when untested code ships to customers.*

*Remember: Start small, test early, automate everything, and always verify your build ID!*

---

**End of Lesson Plan**

