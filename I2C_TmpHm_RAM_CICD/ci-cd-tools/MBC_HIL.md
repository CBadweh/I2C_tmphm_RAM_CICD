# MBC HIL Testing - Learning Summary

## Reference Files

**For Basic (Note: 2 lessons in the file):**
- `C:\Users\Sheen\Desktop\Embedded_System\gene_Baremetal_I2CTmphm_RAM_CICD\I2C_TmpHm_RAM_CICD\ci-cd-tools\MBC_HIL.py`

**For Intermediate (Note: 2 lessons in the file):**
- `C:\Users\Sheen\Desktop\Embedded_System\gene_Baremetal_I2CTmphm_RAM_CICD\I2C_TmpHm_RAM_CICD\ci-cd-tools\MBC_HIL_help.py`

**For Full Suit:**
- `C:\Users\Sheen\Desktop\Embedded_System\gene_Baremetal_I2CTmphm_RAM_CICD\I2C_TmpHm_RAM_CICD\ci-cd-tools\MBC_HIL_Full_Suit.py`

---

## Comprehensive Learning Summary

### 1. Understanding Test Structure

**Initial Analysis:**
- Analyzed a basic HIL test script that performs version command verification
- Identified that the script performs **1 test**: version command verification
- The test follows a simple pattern: connect → send command → verify response

**Key Insight:** A single test can have multiple verification steps (version pattern + prompt), but it's still considered one test case.

---

### 2. Pattern Matching Issues and Solutions

**Problem Encountered:**
- Initial timeout issues when matching version pattern
- Pattern `Version="v1.0.0"` was too strict and failed to match due to newlines/carriage returns in serial output

**Solutions Applied:**

1. **Non-greedy pattern matching:**
   - Changed from `.*Version=...` to `.*?Version=...`
   - Non-greedy (`.*?`) stops at first match, preventing consumption of entire buffer
   - This preserves the prompt in the buffer for subsequent checks

2. **Flexible prompt matching:**
   - Changed from `r'>'` to `r'.*?>'`
   - Allows any characters (including newlines/whitespace) before the prompt
   - Handles various serial output formats

3. **Initial prompt wait:**
   - Added wait for initial prompt after connection
   - Ensures connection is ready before sending commands
   - Prevents race conditions

4. **Increased timeout:**
   - Changed from 3 to 5 seconds
   - Provides more time for responses on slower systems

**Result:** Test successfully passes with robust pattern matching.

---

### 3. Code Version Comparison

**Two Versions Analyzed:**

**Version 1 (Function-based):**
- Uses `main()` function structure
- More verbose output for debugging
- Better error handling and debug output
- Initial prompt wait
- Non-greedy patterns (`.*?Version=...`, `.*?>`)
- 5-second timeout
- Better for learning and development

**Version 2 (Simplified):**
- Direct script execution (no function)
- Minimal output
- Simpler patterns (may fail in edge cases)
- No initial prompt wait
- 3-second timeout
- Better for CI/CD if stable

**Recommendation:** Version 1 is better for learning and reference because:
- Teaches proper Python structure
- More robust patterns
- Better debugging capabilities
- Easier to extend
- Industry-standard practices

---

### 4. Exit Codes in CI/CD

**Understanding Exit Codes:**
- Exit codes are **not printed** to terminal - they're returned to the shell
- Check with `echo $?` (Git Bash) or `echo %ERRORLEVEL%` (Windows CMD)
- Exit code 0 = success, 1 = failure

**Why Essential:**
- CI/CD systems automatically check exit codes
- Exit code 0 → Continue pipeline
- Exit code 1 → Stop pipeline, mark as failed
- Standard practice in embedded testing

**Implementation:**
```python
sys.exit(0 if test_passed else 1)
```

---

### 5. Professional Documentation Standards

**Top 3 Essential Elements:**

1. **Exit Codes Documentation:**
   ```python
   """
   Exit Codes:
       0: Test passed
       1: Test failed
   """
   ```
   - Essential for CI/CD automation
   - Standard practice in embedded testing
   - Quick reference without reading code

2. **Requirements/Dependencies:**
   ```python
   """
   Requirements:
       - Python 3.6+, pexpect, PuTTY plink
   """
   ```
   - Prevents "it doesn't work" issues
   - Saves time for others
   - Professional standard

3. **Clear Purpose Statement:**
   ```python
   """
   HIL Test: Version Command Verification
   
   Verifies firmware version via serial communication.
   """
   ```
   - Immediate understanding of script purpose
   - Better than vague titles
   - Industry standard

**Minimal Professional Version:**
```python
"""
HIL Test: Version Command Verification

Verifies firmware version via serial communication.

Requirements:
    - Python 3.6+, pexpect, PuTTY plink

Exit Codes:
    0: Test passed
    1: Test failed
"""
```

---

### 6. Core Testing Pattern

**The Essential Pattern: Send → Expect → Verify**

This is the fundamental pattern for all HIL tests:

```python
# 1. SEND command
console.sendline('command')

# 2. EXPECT response (wait for pattern)
result = console.expect([expected_pattern, TIMEOUT, EOF])

# 3. VERIFY result
if result == 0:  # Pattern matched = success
    test_passed = True
else:            # Timeout or EOF = failure
    test_passed = False
```

**Why This is Essential:**
- `sendline()` - Sends the command
- `expect()` - Waits for and verifies the response
- Result check - Determines pass/fail

**Without `expect()` verification, you're only sending commands, not testing.**

---

### 7. Learning Path Recommendations

**Incremental Learning Approach:**

**Step 1: Error Handling (Essential)**
- Add try/except around `main()`
- Handle KeyboardInterrupt (Ctrl+C)
- Handle general exceptions
- Time: 15 minutes
- **Why First:** Small change, prevents crashes, essential for production

**Step 2: Helper Functions (Next)**
- Extract connection logic into function
- Extract test logic into function
- Makes code reusable
- Time: 30 minutes
- **Why Second:** Natural progression after error handling

**Step 3: Multiple Tests (After Helpers)**
- Add second test (e.g., reset command)
- Call multiple test functions
- Time: 20 minutes
- **Why Third:** Builds on helper functions

**Step 4: Logging (Later)**
- Replace `print()` with logging
- Add debug/info levels
- Time: 30 minutes
- **Why Fourth:** More complex, can wait

**Step 5: Command Line Args (Much Later)**
- Use argparse for configuration
- Make it configurable
- Time: 45 minutes
- **Why Last:** Most complex, do last

---

### 8. Minimal Essential Error Handling

**Bare Minimum Code:**

```python
if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        sys.exit(1)
    except Exception:
        sys.exit(1)
```

**What Each Part Does:**

1. **`try:` block**
   - Runs your code
   - If error occurs, Python jumps to `except`

2. **`except KeyboardInterrupt:`**
   - Handles Ctrl+C
   - Exits cleanly instead of showing traceback

3. **`except Exception:`**
   - Catches any other error
   - Prevents crashes
   - Provides clean exit

**Why Essential:**
- Prevents crashes from unexpected errors
- Handles Ctrl+C gracefully
- Exits cleanly on errors
- Works for CI/CD

**Optional: `finally` block for cleanup:**
```python
finally:
    # This always runs, even if error occurred
    # Useful for cleanup, but not essential for minimal version
    pass
```

---

### 9. Key Takeaways

**Pattern Matching Best Practices:**
- Use non-greedy patterns (`.*?`) to avoid consuming entire buffer
- Use flexible patterns (`.*?>`) to handle whitespace/newlines
- Wait for initial prompt to ensure connection readiness
- Increase timeout for slower systems

**Code Structure:**
- Function-based structure (`main()`) is better for learning
- More robust patterns are worth the extra complexity
- Error handling is essential for production code
- Helper functions make code reusable and maintainable

**Documentation:**
- Always document exit codes
- List requirements/dependencies
- Provide clear purpose statement
- Keep it minimal but informative

**Testing Philosophy:**
- Every test follows: Send → Expect → Verify
- Pattern matching is the core of HIL testing
- Exit codes are essential for automation
- Incremental learning is the best approach

---

### 10. Code Evolution Path

**Basic → Intermediate → Full Suit:**

1. **Basic (`MBC_HIL.py`):**
   - Single test (version command)
   - Hardcoded configuration
   - Minimal error handling
   - Essential patterns only

2. **Intermediate (`MBC_HIL_help.py`):**
   - Multiple tests (version + help)
   - Helper functions
   - Error handling
   - Better organization

3. **Full Suit (`MBC_HIL_Full_Suit.py`):**
   - All features from intermediate
   - Logging module
   - Command line arguments (argparse)
   - Production-ready

**Learning Strategy:**
- Master each level before moving to next
- Understand why each feature is added
- Practice the patterns until they become natural
- Build incrementally, don't jump ahead

---

## Conclusion

This learning journey covered:
- Understanding test structure and counting tests
- Fixing pattern matching issues with robust regex patterns
- Comparing code versions and choosing best practices
- Understanding exit codes and CI/CD integration
- Professional documentation standards
- Core testing pattern (Send → Expect → Verify)
- Incremental learning path recommendations
- Minimal essential error handling
- Code evolution from basic to production-ready

The key insight: **Start simple, understand the core pattern, then incrementally add features.** The fundamental pattern of Send → Expect → Verify is the foundation of all HIL testing.

---

## Complete Pattern Reference Guide

### 11. Helper Functions Pattern (Step 2)

**Purpose:** Extract reusable code into functions for modularity and maintainability.

**Minimal Essential Pattern:**

```python
# Helper function pattern:
def helper_function(param1, param2):
    # Do work
    return result

# Main function pattern:
def main():
    result = helper_function(value1, value2)
    # Use result
```

**Essential Helper Functions for HIL Testing:**

```python
def connect_serial(serial_port, baud_rate):
    """Connect to serial port and wait for prompt."""
    console = popen_spawn.PopenSpawn(f'plink -serial {serial_port} -sercfg {baud_rate}')
    console.timeout = 5
    console.expect([r'>', pexpect.TIMEOUT], timeout=2)
    return console


def test_version(console, expected_version):
    """Test version command."""
    command = 'main version'
    console.sendline(command)
    
    version_pattern = fr'.*?Version="{expected_version}"'
    result = console.expect([version_pattern, pexpect.TIMEOUT, pexpect.EOF])
    
    if result == 0:
        prompt_result = console.expect([r'.*?>', pexpect.TIMEOUT, pexpect.EOF])
        return (prompt_result == 0)
    return False
```

**Benefits:**
- Reusable: Functions can be called multiple times
- Testable: Each function can be tested independently
- Readable: `main()` shows high-level flow
- Extensible: Easy to add more test functions

**Usage in main():**
```python
def main():
    console = connect_serial(SERIAL_PORT, BAUD_RATE)
    test_passed = test_version(console, EXPECTED_VERSION)
    # cleanup and exit
```

---

### 12. Multiple Tests Pattern (Step 3)

**Purpose:** Run multiple test cases in sequence and combine results.

**Minimal Essential Pattern:**

```python
def main():
    console = connect_serial(SERIAL_PORT, BAUD_RATE)
    
    # Test 1
    test1_passed = test_version(console, EXPECTED_VERSION)
    
    # Test 2
    test2_passed = test_help(console)
    
    # Combine results
    test_passed = test1_passed and test2_passed
    
    # Cleanup and exit
    console.kill(signal.SIGTERM)
    sys.exit(0 if test_passed else 1)
```

**Key Points:**
- Reuse the same console connection for all tests
- Each test function returns True/False
- Combine results with `and` (all must pass) or `or` (any can pass)
- Cleanup once after all tests complete

**Example with Individual Test Results:**
```python
# Test 1: Version command
version_test_passed = test_version(console, EXPECTED_VERSION)
print('Version: PASS' if version_test_passed else 'Version: FAIL')

# Test 2: Help command
help_test_passed = test_help(console)
print('Help: PASS' if help_test_passed else 'Help: FAIL')

# Both tests must pass
test_passed = version_test_passed and help_test_passed
print('Overall: PASS' if test_passed else 'Overall: FAIL')
```

---

### 13. Logging Pattern (Step 4)

**Purpose:** Replace `print()` with logging for better control and professional output.

**Minimal Essential Setup:**

```python
import logging

# Configure logging (one line)
logging.basicConfig(level=logging.INFO, format='%(message)s')
```

**Essential Logging Levels:**

```python
logging.debug('Detailed diagnostic info')    # Only shown if level=DEBUG
logging.info('Normal flow information')       # Shown for INFO and above
logging.error('Errors and failures')          # Always shown
```

**Replacing print() with logging:**

```python
# Before:
print(f'Sending: {command}')

# After:
logging.info(f'Sending: {command}')  # Normal output
logging.error('Error occurred')       # Errors
```

**Where to Use Each Level:**

**DEBUG** - Detailed diagnostic info:
- Connection start/status
- Pattern matching details
- Step-by-step execution
- Configuration values
- Internal state changes

**INFO** - Normal flow:
- Commands being sent
- Test results (PASS/FAIL)
- Overall status

**ERROR** - Failures and errors:
- Test failures
- Pattern not found
- Timeouts
- Connection failures
- Exceptions

**Complete Logging Example:**

```python
import logging

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(message)s')

def test_version(console, expected_version):
    command = 'main version'
    logging.info(f'Sending: {command}')  # INFO: Command being sent
    console.sendline(command)
    
    logging.debug(f'Waiting for pattern: {version_pattern}')  # DEBUG: Pattern details
    result = console.expect([version_pattern, pexpect.TIMEOUT, pexpect.EOF])
    
    if result == 0:
        logging.debug('Version pattern matched')  # DEBUG: Success
        return True
    else:
        logging.error(f'Version pattern not found (result={result})')  # ERROR: Failure
        return False
```

**Output Control:**
```bash
# Normal output (INFO level)
python script.py

# Show debug info too
# Change: logging.basicConfig(level=logging.DEBUG)
```

---

### 14. Command Line Arguments Pattern (Step 5)

**Purpose:** Make script configurable via command line arguments.

**Minimal Essential Pattern:**

```python
import argparse

# Create parser
parser = argparse.ArgumentParser()

# Add arguments
parser.add_argument('--port', default='COM4', help='Serial port')
parser.add_argument('--baud', type=int, default=115200, help='Baud rate')
parser.add_argument('--version', default='v1.0.0', help='Expected version')

# Parse arguments
args = parser.parse_args()

# Use arguments
SERIAL_PORT = args.port
BAUD_RATE = args.baud
EXPECTED_VERSION = args.version
```

**Essential argparse Patterns:**

```python
# Basic argument with default
parser.add_argument('--name', default='value', help='Description')

# With type conversion
parser.add_argument('--number', type=int, default=100, help='Number')

# Required argument
parser.add_argument('--required', required=True, help='Required argument')
```

**Complete Example:**

```python
def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='HIL Test: Version and Help Commands')
    parser.add_argument('--port', default='COM4', help='Serial port (default: COM4)')
    parser.add_argument('--baud', type=int, default=115200, help='Baud rate (default: 115200)')
    parser.add_argument('--version', default='v1.0.0', help='Expected version (default: v1.0.0)')
    args = parser.parse_args()
    
    # Use parsed arguments
    SERIAL_PORT = args.port
    BAUD_RATE = args.baud
    EXPECTED_VERSION = args.version
    
    # Rest of your code...
```

**Usage Examples:**
```bash
# Use defaults
python script.py

# Override port
python script.py --port COM5

# Override multiple
python script.py --port COM5 --baud 9600 --version v2.0.0

# Get help
python script.py --help
```

**Benefits:**
- Configurable: Change settings without editing code
- Professional: Standard way to handle CLI arguments
- User-friendly: Automatic help with `--help`
- Flexible: Defaults make arguments optional

---

### 15. Error Handling and Logging Relationship

**Key Insight:** Error handling and logging work together but serve different purposes.

**Error Handling (try/except):**
- **Purpose:** Prevents crashes, controls program flow
- **What it does:** Catches errors and decides what to do next
- **Without it:** Program crashes with traceback

**Logging:**
- **Purpose:** Records what happened (success or failure)
- **What it does:** Provides visibility into program execution
- **Without it:** No record of what happened

**How They Work Together:**

```python
try:
    main()
except KeyboardInterrupt:
    logging.error('Interrupted by user')  # ← Logging records the error
    sys.exit(1)                            # ← Error handling decides what to do
except Exception as e:
    logging.error(f'Unexpected error: {e}')  # ← Logging records the error
    sys.exit(1)                                # ← Error handling decides what to do
```

**The Pattern:**
1. **Error handling** catches the error
2. **Logging** records what happened
3. **Error handling** decides the next action (exit, retry, etc.)

**Best Practice Pattern:**

```python
try:
    # Your code
    result = do_something()
    logging.info('Operation successful')  # Log success
except SpecificError as e:
    logging.error(f'Specific error occurred: {e}')  # Log the error
    sys.exit(1)  # Handle the error
except Exception as e:
    logging.error(f'Unexpected error: {e}')  # Log unexpected errors
    sys.exit(1)
```

**In Test Functions:**

```python
if result == 0:
    logging.debug('Version pattern matched')  # Log success
    return True
else:
    logging.error(f'Version pattern not found (result={result})')  # Log failure
    return False  # Error handling: return False
```

**Why They Go Together:**
- **Error handling** prevents crashes
- **Logging** provides visibility
- **Together:** Graceful failure with record of what happened

---

### 16. Complete Minimal Code Reference

**Complete Minimal Script with All Features:**

```python
"""
HIL Test: Version and Help Commands Verification

Requirements:
    - Python 3.6+, pexpect, PuTTY plink

Exit Codes:
    0: Test passed
    1: Test failed
"""

import sys
import signal
import logging
import argparse
import pexpect
import pexpect.popen_spawn as popen_spawn

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(message)s')


def connect_serial(serial_port, baud_rate):
    """Connect to serial port and wait for prompt."""
    console = popen_spawn.PopenSpawn(f'plink -serial {serial_port} -sercfg {baud_rate}')
    console.timeout = 5
    console.expect([r'>', pexpect.TIMEOUT], timeout=2)
    return console


def test_version(console, expected_version):
    """Test version command."""
    command = 'main version'
    logging.info(f'Sending: {command}')
    console.sendline(command)
    
    version_pattern = fr'.*?Version="{expected_version}"'
    result = console.expect([version_pattern, pexpect.TIMEOUT, pexpect.EOF])
    
    if result == 0:
        prompt_result = console.expect([r'.*?>', pexpect.TIMEOUT, pexpect.EOF])
        return (prompt_result == 0)
    return False


def test_help(console):
    """Test help command."""
    command = 'help'
    logging.info(f'Sending: {command}')
    console.sendline(command)
    
    help_pattern = r'.*?main \(status, version, reset\)'
    result = console.expect([help_pattern, pexpect.TIMEOUT, pexpect.EOF])
    
    if result == 0:
        prompt_result = console.expect([r'.*?>', pexpect.TIMEOUT, pexpect.EOF])
        return (prompt_result == 0)
    return False


def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='HIL Test: Version and Help Commands')
    parser.add_argument('--port', default='COM4', help='Serial port (default: COM4)')
    parser.add_argument('--baud', type=int, default=115200, help='Baud rate (default: 115200)')
    parser.add_argument('--version', default='v1.0.0', help='Expected version (default: v1.0.0)')
    args = parser.parse_args()
    
    # Use parsed arguments
    SERIAL_PORT = args.port
    BAUD_RATE = args.baud
    EXPECTED_VERSION = args.version

    # Connect to serial port
    console = connect_serial(SERIAL_PORT, BAUD_RATE)
    
    # Test 1: Version command
    version_test_passed = test_version(console, EXPECTED_VERSION)
    logging.info('Version: PASS' if version_test_passed else 'Version: FAIL')
    
    # Test 2: Help command
    help_test_passed = test_help(console)
    logging.info('Help: PASS' if help_test_passed else 'Help: FAIL')
    
    # Both tests must pass
    test_passed = version_test_passed and help_test_passed
    
    # Cleanup
    console.kill(signal.SIGTERM)
    
    # Print overall result
    logging.info('Overall: PASS' if test_passed else 'Overall: FAIL')
    
    # Exit with result
    sys.exit(0 if test_passed else 1)


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        logging.error('Interrupted by user')
        sys.exit(1)
    except Exception as e:
        logging.error(f'Error: {e}')
        sys.exit(1)
    finally:
        pass
```

---

### 17. Learning Progression Summary

**Complete Step-by-Step Learning Path:**

1. **Basic Test (No features)**
   - Core pattern: Send → Expect → Verify
   - Hardcoded configuration
   - Single test
   - Minimal output

2. **Add Error Handling**
   - Wrap `main()` with try/except
   - Handle KeyboardInterrupt
   - Handle general exceptions
   - **Time: 15 minutes**

3. **Add Helper Functions**
   - Extract `connect_serial()`
   - Extract test functions
   - Modularize code
   - **Time: 30 minutes**

4. **Add Multiple Tests**
   - Add second test function
   - Combine results
   - Reuse connection
   - **Time: 20 minutes**

5. **Add Logging**
   - Replace `print()` with `logging.info()`
   - Add `logging.error()` for failures
   - Add `logging.debug()` for details
   - **Time: 30 minutes**

6. **Add Command Line Args**
   - Import argparse
   - Add argument definitions
   - Parse and use arguments
   - **Time: 45 minutes**

**Total Learning Time:** ~2.5 hours for complete progression

---

### 18. Key Patterns Quick Reference

**Core Testing Pattern:**
```python
console.sendline('command')                                    # SEND
result = console.expect([pattern, TIMEOUT, EOF])              # EXPECT
if result == 0: return True else: return False                # VERIFY
```

**Error Handling Pattern:**
```python
try:
    main()
except KeyboardInterrupt:
    sys.exit(1)
except Exception:
    sys.exit(1)
```

**Helper Function Pattern:**
```python
def helper_function(param1, param2):
    # Do work
    return result
```

**Logging Pattern:**
```python
logging.basicConfig(level=logging.INFO, format='%(message)s')
logging.info('Normal output')
logging.error('Error output')
```

**Command Line Args Pattern:**
```python
parser = argparse.ArgumentParser()
parser.add_argument('--name', default='value', help='Description')
args = parser.parse_args()
value = args.name
```

---

## Final Summary

This comprehensive guide covers the complete journey from basic HIL testing to production-ready scripts:

1. **Core Pattern:** Send → Expect → Verify (foundation of all HIL testing)
2. **Error Handling:** Essential for graceful failure and CI/CD integration
3. **Helper Functions:** Modular design for reusability and maintainability
4. **Multiple Tests:** Sequential execution with combined results
5. **Logging:** Professional output control with DEBUG/INFO/ERROR levels
6. **Command Line Args:** Configurable scripts via argparse

**Remember:** Start simple, understand each pattern, then incrementally add features. The fundamental pattern of Send → Expect → Verify is the foundation of all HIL testing.



# Future Steps

## Intermediate Steps (6-10)

### Recommended Implementation Order

**Step 6: Timeout Management** (prevents hangs)
   - Set timeouts before operations
   - Prevents infinite waits
   - Critical for production reliability
   
   Pattern:
   - Always set console.timeout before any expect() operation
   - Different operations may need different timeouts

```python
def connect_serial(serial_port, baud_rate):
    console = popen_spawn.PopenSpawn(f'plink -serial {serial_port} -sercfg {baud_rate}')
    console.timeout = 5  # Set timeout BEFORE operations
    console.expect([r'>', pexpect.TIMEOUT], timeout=2)
    return console

def test_version(console, expected_version):
    console.timeout = 5  # Set timeout before each test
    # ... rest of test
```

**Step 7: Return Value Checking** (catches failures)
   - Check every `expect()` return value
   - Log specific error details
   - Return False immediately on failure

   Pattern:
   - Check every expect() return value
   - Log specific error details (result code, expected pattern)
   - Return False immediately on failure

```python
def test_version(console, expected_version):
    command = 'main version'
    logging.info(f'Sending: {command}')
    console.sendline(command)
    
    version_pattern = fr'.*?Version="{expected_version}"'
    result = console.expect([version_pattern, pexpect.TIMEOUT, pexpect.EOF])
    
    if result == 0:
        prompt_result = console.expect([r'.*?>', pexpect.TIMEOUT, pexpect.EOF])
        if prompt_result == 0:
            return True
        else:
            logging.error(f'Prompt not found after version (result={prompt_result})')
            return False
    else:
        logging.error(f'Version pattern not found (result={result}, expected={expected_version})')
        return False
```


**Step 9: Verbose Mode** (helps with debugging)
   - Add `--verbose` flag
   - Enable DEBUG logging when needed
   - Essential for troubleshooting

**Step 10: Better Error Messages** (improves diagnostics)
   - Contextual error information
   - Suggests debugging steps
   - Reduces troubleshooting time

**Step 8: Reset Functionality** (adds capability)
   - Reset device before tests
   - Ensures clean state
   - Common in production test suites

**Why These Steps Matter:**
- Address issues from goal file (reset hang problem)
- Move script toward production-ready reliability
- Prevent common failure scenarios
- Improve debugging capabilities



---

## Advanced Steps Implementation Order

**Step 11: Process Cleanup** (prevents conflicts)
   - Kill existing plink processes before connecting
   - Ensures clean connection state

   Pattern:
   - Check for existing processes before connecting
   - Kill conflicting processes
   - Ensures clean connection state

```python
import psutil

def cleanup_existing_processes(serial_port):
    """Kill any existing plink processes using the serial port."""
    kill_procs = []
    for p in psutil.process_iter():
        if p.name() == 'plink.exe':
            if serial_port.upper() in [s.upper() for s in p.cmdline()]:
                kill_procs.append(p)
    
    for p in kill_procs:
        logging.debug(f'Killing existing process: {p.name()} (pid={p.pid})')
        p.kill()

def main():
    # Cleanup before connecting
    cleanup_existing_processes(SERIAL_PORT)
    
    # Then connect
    console = connect_serial(SERIAL_PORT, BAUD_RATE)
```


**Step 14: Input Flushing** (ensures clean state)
   - Clear serial buffer before each test
   - Prevents pattern matching on stale data

```python
def flush_input(console):
    """Flush any pending input from serial buffer."""
    logging.debug('Flushing input buffer...')
    while True:
        try:
            result = console.expect([pexpect.TIMEOUT, pexpect.EOF, r'.*'], timeout=0.1)
            if result == 0:  # TIMEOUT - no more data
                break
        except:
            break
    logging.debug('Input buffer flushed')

def test_version(console, expected_version):
    """Test version command."""
    flush_input(console)  # Clear buffer before test
    command = 'main version'
    # ... rest of test
```




**Step 12: Test Framework** (organizes tests)
   - Implement `start_test()`, `test_pass()`, `test_fail()`
   - Centralized test result tracking

   Pattern:
   - start_test() - Initialize test context
   - test_pass() - Record success
   - test_fail() - Record failure with reason
   - Centralized test result tracking

```python
test_results = []

def start_test(test_name, timeout=5):
    """Initialize a test with name and timeout."""
    logging.info(f'Starting test: {test_name}')
    console.timeout = timeout
    return {'name': test_name, 'timeout': timeout}

def test_pass(test_name):
    """Record test pass."""
    logging.info(f'Test "{test_name}" PASSED')
    test_results.append({'name': test_name, 'status': 'PASS'})

def test_fail(test_name, failure_info):
    """Record test failure with reason."""
    logging.error(f'Test "{test_name}" FAILED: {failure_info}')
    test_results.append({'name': test_name, 'status': 'FAIL', 'reason': failure_info})

def test_version(console, expected_version):
    """Test version command."""
    test_info = start_test('version')
    command = 'main version'
    logging.info(f'Sending: {command}')
    console.sendline(command)
    
    version_pattern = fr'.*?Version="{expected_version}"'
    result = console.expect([version_pattern, pexpect.TIMEOUT, pexpect.EOF])
    
    if result == 0:
        prompt_result = console.expect([r'.*?>', pexpect.TIMEOUT, pexpect.EOF])
        if prompt_result == 0:
            test_pass('version')
            return True
        else:
            test_fail('version', f'Prompt timeout (result={prompt_result})')
            return False
    else:
        test_fail('version', f'Pattern not found (result={result})')
        return False
```




**Step 15: Configurable Logs** (improves control)
   - Add `--log` argument with level options
   - Professional log level management

**Step 13: JUnit XML** (CI/CD integration)
   - Generate JUnit XML output
   - Enable CI/CD system integration
```python
import junit_xml as jux

test_cases = []

def test_version(console, expected_version):
    """Test version command."""
    test_case = jux.TestCase('version')
    test_cases.append(test_case)
    
    command = 'main version'
    logging.info(f'Sending: {command}')
    console.sendline(command)
    
    version_pattern = fr'.*?Version="{expected_version}"'
    result = console.expect([version_pattern, pexpect.TIMEOUT, pexpect.EOF])
    
    if result == 0:
        prompt_result = console.expect([r'.*?>', pexpect.TIMEOUT, pexpect.EOF])
        if prompt_result == 0:
            return True
        else:
            test_case.add_failure_info(f'Prompt timeout (result={prompt_result})')
            return False
    else:
        test_case.add_failure_info(f'Pattern not found (result={result})')
        return False

def main():
    # ... run tests ...
    
    # Generate JUnit XML
    test_suite = jux.TestSuite('MBC_HIL_Tests', test_cases)
    
    # Write to file
    with open('test-results.xml', 'w') as f:
        jux.TestSuite.to_file(f, [test_suite], prettyprint=True)
    
    logging.info('Test results written to test-results.xml')
```


**Result:** These steps move your script from intermediate to production-ready with full CI/CD integration capabilities.

---

## Complete Learning Path Summary

### Foundation (Steps 1-5):
- Error handling
- Helper functions
- Multiple tests
- Logging
- Command line args

### Intermediate (Steps 6-10):
- Timeout management
- Return value checking
- Reset functionality
- Verbose mode
- Better error messages

### Advanced (Steps 11-15):
- Process cleanup
- Test framework structure
- JUnit XML output
- Input buffer flushing
- Configurable log levels


---

## Why Advanced Steps Matter

1. **Process Cleanup:** Prevents connection conflicts from leftover processes
2. **Test Framework:** Standardizes test execution and reporting
3. **JUnit XML:** Enables CI/CD integration and test result visualization
4. **Input Flushing:** Ensures clean test state by clearing serial buffer
5. **Configurable Logs:** Professional flexibility for different use cases


---

## 19. Template-Based Testing Guide

### 19.1 Overview

This section provides comprehensive guides for using the two main template files that consolidate multiple patterns into reusable testing frameworks.

**Template Files:**

1. **Template 1: Complete Serial Communication & Hardware Testing**
   - File: `C:\Users\Sheen\Desktop\Embedded_System\gene_Baremetal_I2CTmphm_RAM_CICD\I2C_TmpHm_RAM_CICD\ci-cd-tools\Template_Serial_Communication.py`
   - Merges: `console.expect()` + `buffer flushing` + `process cleanup` + `error handling`
   - Use when: Talking to hardware via serial/UART/USB with robust error handling

2. **Template 2: Test Framework & Reporting**
   - File: `C:\Users\Sheen\Desktop\Embedded_System\gene_Baremetal_I2CTmphm_RAM_CICD\I2C_TmpHm_RAM_CICD\ci-cd-tools\Template_Test_Framework_Reporting.py`
   - Merges: `start_test/test_pass/test_fail` + `JUnit XML output`
   - Use when: Need structured, CI/CD-compatible test reporting

**When to Use Each Template:**

- **Template 1 Only**: Simple hardware tests without CI/CD reporting
- **Template 2 Only**: Software tests that need JUnit XML but don't use serial communication
- **Both Templates**: Production HIL tests with full CI/CD integration

---

### 19.2 Template 1: Complete Serial Communication & Hardware Testing

#### What This Template Provides

Template 1 consolidates **4 essential patterns** into a single, copy-paste ready file:

1. **console.expect()** - Send commands and verify responses
2. **Buffer flushing** - Clear stale data from serial buffer
3. **Process cleanup** - Kill existing plink processes before connecting
4. **Error handling** - Graceful failure with try/except/finally

#### Complete Code Walkthrough

**Imports:**
```python
import sys
import signal
import psutil
import pexpect
import pexpect.popen_spawn as popen_spawn
```

**Why these imports:**
- `sys` - For exit codes (CI/CD integration)
- `signal` - For clean process termination
- `psutil` - For finding and killing existing processes
- `pexpect` - For serial communication and pattern matching
- `popen_spawn` - For spawning plink process

---

#### Function 1: cleanup_existing_processes()

```python
def cleanup_existing_processes(serial_port):
    """Kill any existing plink processes using the serial port."""
    for p in psutil.process_iter():  # iterates over running processes
        if p.name() == 'plink.exe':  # process name
            if serial_port.upper() in [s.upper() for s in p.cmdline()]:  # command line arguments (includes COM port)
                p.kill()  # terminates the process
```

**What it does:**
- Iterates through all running processes
- Finds plink.exe processes using your COM port
- Kills them to prevent "port already in use" errors

**When to call:**
- BEFORE connecting to serial port
- At the start of main()

**Why essential:**
- Previous test crashes may leave plink.exe running
- Old processes lock the COM port
- Ensures clean connection state

---

#### Function 2: flush_input()

```python
def flush_input(console):
    """Clear stale data from serial buffer."""
    while console.expect(['.*', pexpect.TIMEOUT, pexpect.EOF], timeout=0.1) == 0:
        if len(console.after) == 0:
            break
    # if console.expect() see any characters, ".*", it return 0, and while loop enter the while loop
    # if console.expect() TIMEOUT-after 0.1s or EOF, while loop breaks and then leave the flush_input() function
```

**What it does:**
- Reads and discards any data in the serial buffer
- Continues until buffer is empty (timeout or no data)

**When to call:**
- BEFORE sending a test command
- AFTER receiving a response
- Anytime you need a clean buffer state

**Why essential:**
- Prevents matching patterns against old/stale data
- Ensures you're reading fresh responses
- Eliminates intermittent test failures

**How it works:**
1. `console.expect(['.*', ...])` tries to read any characters
2. If it finds characters (result == 0), loop continues
3. If timeout (0.1s with no data) or EOF, loop breaks
4. Buffer is now clean

---

#### Function 3: connect_serial()

```python
def connect_serial(serial_port, baud_rate):
    """Connect to serial port and wait for prompt."""
    console = popen_spawn.PopenSpawn(f'plink -serial {serial_port} -sercfg {baud_rate}')
    console.timeout = 5
    console.expect([r'>', pexpect.TIMEOUT], timeout=2)
    return console
```

**What it does:**
- Spawns plink process to connect to serial port
- Sets default timeout to 5 seconds
- Waits for initial prompt ('>') to ensure connection is ready
- Returns console object for testing

**When to call:**
- AFTER cleanup_existing_processes()
- At the start of main(), once per test session

**Why essential:**
- Encapsulates connection logic
- Ensures connection is ready before tests
- Reusable across different test scripts

**Parameters:**
- `serial_port`: COM port (e.g., 'COM4')
- `baud_rate`: Baud rate (e.g., 115200)

---

#### Function 4: test_command() - The Core Test Pattern

```python
def test_command(console, command, expected_pattern):
    """
    Template function: Send command and verify response

    Args:
        console: pexpect console object
        command: Command string to send (e.g., 'main version')
        expected_pattern: Regex pattern to match (e.g., r'.*?Version="v1.0.0"')

    Returns:
        bool: True if test passed, False otherwise
    """
    # 1. Flush buffer before test
    flush_input(console)

    # 2. Send command
    print(f'Sending: {command}')
    console.sendline(command)

    # 3. Wait for expected pattern
    result = console.expect([expected_pattern, pexpect.TIMEOUT, pexpect.EOF])

    # 4. Check result
    if result == 0:  # expected_pattern matched = success
        # Continue - wait for prompt to confirm command completed
        prompt_result = console.expect([r'.*?>', pexpect.TIMEOUT, pexpect.EOF], timeout=2)
        test_passed = (prompt_result == 0)
    else:  # result == 1 (TIMEOUT) or result == 2 (EOF) = failure
        test_passed = False

    # 5. Flush buffer after test
    flush_input(console)

    return test_passed
```

**The 5-Step Test Pattern:**

**Step 1: Flush Before**
- Clears any stale data in buffer
- Ensures clean starting state

**Step 2: Send Command**
- Uses `sendline()` to send command + newline
- Logs what was sent

**Step 3: Wait for Pattern**
- `expect()` waits for regex match or timeout
- Returns 0 if pattern matched, 1 if timeout, 2 if EOF

**Step 4: Check Result**
- If pattern matched (result == 0):
  - Wait for prompt to confirm completion
  - Return True if prompt found
- If timeout/EOF:
  - Return False immediately

**Step 5: Flush After**
- Clears buffer for next test
- Prevents contamination between tests

**Usage Example:**
```python
# Test version command
version_pattern = r'.*?Version="v1.0.0"'
version_passed = test_command(console, 'main version', version_pattern)

# Test help command
help_pattern = r'.*?main \(status, version, reset\)'
help_passed = test_command(console, 'help', help_pattern)
```

---

#### Function 5: main() - Execution Flow

```python
def main():
    # Configuration
    SERIAL_PORT = 'COM4'
    BAUD_RATE = 115200

    # PATTERN: Process cleanup BEFORE connecting
    cleanup_existing_processes(SERIAL_PORT)

    # Connect
    console = connect_serial(SERIAL_PORT, BAUD_RATE)

    # Test 1: Version command
    version_pattern = r'.*?Version="v1.0.0"'
    version_passed = test_command(console, 'main version', version_pattern)
    print(f'Version test: {"PASS" if version_passed else "FAIL"}')

    # Test 2: Help command
    help_pattern = r'.*?main \(status, version, reset\)'
    help_passed = test_command(console, 'help', help_pattern)
    print(f'Help test: {"PASS" if help_passed else "FAIL"}')

    # Cleanup
    console.kill(signal.SIGTERM)

    # Exit based on test results
    test_passed = version_passed and help_passed
    print('Overall: PASS' if test_passed else 'Overall: FAIL')
    sys.exit(0 if test_passed else 1)
```

**Execution Flow:**

1. **Configuration**: Set COM port and baud rate
2. **Cleanup**: Kill old plink processes
3. **Connect**: Establish serial connection
4. **Test 1**: Run version test
5. **Test 2**: Run help test
6. **Combine Results**: Both must pass for overall PASS
7. **Cleanup**: Kill plink process
8. **Exit**: Return 0 (pass) or 1 (fail) for CI/CD

---

#### Error Handling Wrapper

```python
# PATTERN: Error handling wraps everything
if __name__ == '__main__':
    try:  # Runs your code
        main()
    except KeyboardInterrupt:  # Handles Ctrl+C
        sys.exit(1)
    except Exception:  # Catches any other error, Prevents crashes and provides a clean exit
        sys.exit(1)
    finally:
        # This always runs, even if error occurred
        # (useful for cleanup, but not essential for minimal version)
        pass
```

**What it does:**
- `try`: Runs main()
- `except KeyboardInterrupt`: Handles Ctrl+C gracefully
- `except Exception`: Catches any other error, exits with code 1
- `finally`: Always runs (useful for cleanup)

**Why essential:**
- Prevents crashes from unexpected errors
- Provides clean exit codes for CI/CD
- Handles user interruption gracefully

---

#### Customization Guide

**To adapt Template 1 for your tests:**

**1. Update Configuration:**
```python
def main():
    # Change these values
    SERIAL_PORT = 'COM5'  # Your COM port
    BAUD_RATE = 9600       # Your baud rate
```

**2. Change Expected Patterns:**
```python
# Replace with your firmware's version format
version_pattern = r'.*?FW_Version="v2.0.0"'

# Replace with your firmware's help format
help_pattern = r'.*?Commands: help, status, reset'
```

**3. Add More Tests:**
```python
# Test 3: Your custom command
custom_pattern = r'.*?YourExpectedResponse'
custom_passed = test_command(console, 'your command', custom_pattern)
print(f'Custom test: {"PASS" if custom_passed else "FAIL"}')

# Update overall result
test_passed = version_passed and help_passed and custom_passed
```

**4. Change Prompt Pattern (if needed):**
```python
# In connect_serial() and test_command()
# Change r'.*?>' to match your firmware's prompt
console.expect([r'.*?\$', pexpect.TIMEOUT], timeout=2)  # Example: $ prompt
```

---

### 19.3 Template 2: Test Framework & Reporting

#### What This Template Provides

Template 2 consolidates **2 essential patterns** for professional test reporting:

1. **Test framework structure** - start_test(), test_pass(), test_fail()
2. **JUnit XML output** - CI/CD compatible test results

#### Complete Code Walkthrough

**Imports:**
```python
import junit_xml as jux
```

**Why this import:**
- `junit_xml` - For generating CI/CD compatible XML test results

---

#### Global Variables

```python
# Global test cases list for JUnit XML output
test_cases = []

# Global variables for test framework
g_test_name = None
g_test_junit = None
g_console = None
```

**What they do:**
- `test_cases`: List storing all test case objects for final XML output
- `g_test_name`: Current test name (for logging)
- `g_test_junit`: Current JUnit TestCase object
- `g_console`: Console object (optional, for timeout management)

**Why global:**
- Allows test framework functions to share state
- Simplifies test function signatures
- Centralized test result tracking

---

#### Function 1: start_test()

```python
def start_test(name, timeout=3):
    """Start a test, including recording the test name and junit object.

    Args:
        name: The name of the test
        timeout: How long to wait for results (default: 3 seconds)
    """
    global g_test_name, g_test_junit, g_console

    g_test_junit = jux.TestCase(name)
    test_cases.append(g_test_junit)
    g_test_name = name
    print(f'Test "{g_test_name}" starting')
    if g_console:
        g_console.timeout = timeout
```

**What it does:**
1. Creates a new JUnit TestCase object
2. Adds it to the test_cases list
3. Sets current test name
4. Logs test start
5. Sets timeout on console (if provided)

**When to call:**
- At the beginning of every test function
- BEFORE sending any commands

**Parameters:**
- `name`: Test name (e.g., 'version', 'help', 'reset')
- `timeout`: How long to wait for responses (default: 3 seconds)

**Usage:**
```python
def test_version(console, expected_version):
    start_test('version', timeout=5)  # Initialize test
    # ... rest of test logic ...
```

---

#### Function 2: test_pass()

```python
def test_pass():
    """Handle a test pass, recording info."""
    global g_test_name

    print(f'Test "{g_test_name}" passes')
    # Test passes if no failure info is added to g_test_junit
```

**What it does:**
- Logs that the test passed
- Does NOT modify g_test_junit (no failure = pass in JUnit)

**When to call:**
- After all test checks succeed
- BEFORE returning True from test function

**Key Insight:**
- JUnit XML: Test passes if no failure info is added
- You don't need to explicitly mark success

**Usage:**
```python
if result == 0:
    prompt_result = console.expect([r'.*?>', pexpect.TIMEOUT, pexpect.EOF])
    if prompt_result == 0:
        test_pass()  # Mark as passed
        return True
```

---

#### Function 3: test_fail()

```python
def test_fail(failure_info):
    """Handle a test failure, recording info.

    Args:
        failure_info: String describing how it failed
    """
    global g_test_name, g_test_junit

    print(f'Test "{g_test_name}" fails: {failure_info}')
    if g_test_junit:
        g_test_junit.add_failure_info(failure_info)
```

**What it does:**
1. Logs the failure with reason
2. Adds failure info to JUnit TestCase object
3. This marks the test as FAILED in XML output

**When to call:**
- When any test check fails
- BEFORE returning False from test function

**Parameters:**
- `failure_info`: String describing why test failed (be specific!)

**Usage Examples:**
```python
# Timeout on pattern
if result != 0:
    test_fail(f'Version pattern not found. Expected: {expected_version}')
    return False

# Prompt not found
if prompt_result != 0:
    test_fail('Prompt not found after version command')
    return False
```

---

#### Function 4: write_junit_xml()

```python
def write_junit_xml(junit_file):
    """Write JUnit XML file for CI/CD integration.

    Args:
        junit_file: Path to output XML file (e.g., 'test-results.xml')
    """
    # Create TestSuite and write to file
    test_suite = jux.TestSuite('MBC HIL Tests', test_cases)
    with open(junit_file, 'w') as f:
        jux.TestSuite.to_file(f, [test_suite], prettyprint=True)

    print(f'\nJUnit XML written to: {junit_file}')
```

**What it does:**
1. Creates a TestSuite containing all test cases
2. Writes XML file with test results
3. Logs where file was written

**When to call:**
- At the END of main(), after all tests complete
- BEFORE sys.exit()

**Parameters:**
- `junit_file`: Output XML filename (e.g., 'test-results.xml')

**Usage:**
```python
def main():
    # ... run all tests ...

    # Write JUnit XML
    write_junit_xml('test-results.xml')

    # Then exit
    sys.exit(0 if test_passed else 1)
```

**XML Output Example:**
```xml
<?xml version="1.0" ?>
<testsuites>
    <testsuite name="MBC HIL Tests" tests="2" failures="0">
        <testcase name="version"/>
        <testcase name="help"/>
    </testsuite>
</testsuites>
```

---

#### JUnit XML Explanation

**What is JUnit XML?**
- Industry-standard format for test results
- Used by CI/CD systems (Jenkins, GitHub Actions, GitLab CI)
- Shows pass/fail for each test

**How CI/CD Systems Use It:**
1. Your script runs and generates XML file
2. CI/CD system reads the XML
3. System displays test results in UI
4. Pipeline passes/fails based on results

**Key XML Elements:**
- `<testsuite>`: Collection of tests
- `<testcase name="...">`: Individual test
- `<failure>`: Present if test failed (contains failure_info)
- If no `<failure>` tag: Test passed

**Benefits:**
- Visual test reports in CI/CD
- Historical trend analysis
- Automatic pass/fail detection
- Standard format across teams

---

#### Example Test Functions

**test_version() - Complete Example:**

```python
def test_version(console, expected_version):
    """Test version command."""
    start_test('version', timeout=5)

    command = 'main version'
    print(f'Sending: {command}')
    console.sendline(command)

    version_pattern = fr'.*?Version="{expected_version}"'
    result = console.expect([version_pattern, pexpect.TIMEOUT, pexpect.EOF])

    if result == 0:
        prompt_result = console.expect([r'.*?>', pexpect.TIMEOUT, pexpect.EOF])
        if prompt_result == 0:
            test_pass()
            return True
        else:
            test_fail('Prompt not found after version command')
            return False
    else:
        test_fail(f'Version pattern not found. Expected: {expected_version}')
        return False
```

**Flow:**
1. `start_test('version', timeout=5)` - Initialize test
2. Send command and wait for pattern
3. If pattern matched (result == 0):
   - Wait for prompt
   - If prompt found: `test_pass()` and return True
   - If no prompt: `test_fail()` and return False
4. If pattern not matched: `test_fail()` and return False

---

**test_help() - Complete Example:**

```python
def test_help(console):
    """Test help command."""
    start_test('help', timeout=5)

    command = 'help'
    print(f'Sending: {command}')
    console.sendline(command)

    help_pattern = r'.*?main \(status, version, reset\)'
    result = console.expect([help_pattern, pexpect.TIMEOUT, pexpect.EOF])

    if result == 0:
        prompt_result = console.expect([r'.*?>', pexpect.TIMEOUT, pexpect.EOF])
        if prompt_result == 0:
            test_pass()
            return True
        else:
            test_fail('Prompt not found after help command')
            return False
    else:
        test_fail('Help pattern not found')
        return False
```

**Same pattern, different command:**
- Same structure as test_version()
- Different command ('help' instead of 'main version')
- Different expected pattern
- Reusable framework

---

### 19.4 Combining Both Templates

#### Complete Working Example

Here's how to merge Template 1 + Template 2 into a production-ready test script:

```python
"""
Complete HIL Test with Framework and JUnit XML Output

Merges Template 1 + Template 2 for production-ready testing.

Requirements:
    - Python 3.6+, pexpect, PuTTY plink, psutil, junit-xml

Exit Codes:
    0: All tests passed
    1: One or more tests failed
"""

import sys
import signal
import psutil
import pexpect
import pexpect.popen_spawn as popen_spawn
import junit_xml as jux

# ==================
# TEMPLATE 2: Global variables for test framework
# ==================
test_cases = []
g_test_name = None
g_test_junit = None
g_console = None


# ==================
# TEMPLATE 1: Serial Communication Functions
# ==================
def cleanup_existing_processes(serial_port):
    """Kill any existing plink processes using the serial port."""
    for p in psutil.process_iter():
        if p.name() == 'plink.exe':
            if serial_port.upper() in [s.upper() for s in p.cmdline()]:
                p.kill()


def flush_input(console):
    """Clear stale data from serial buffer."""
    while console.expect(['.*', pexpect.TIMEOUT, pexpect.EOF], timeout=0.1) == 0:
        if len(console.after) == 0:
            break


def connect_serial(serial_port, baud_rate):
    """Connect to serial port and wait for prompt."""
    console = popen_spawn.PopenSpawn(f'plink -serial {serial_port} -sercfg {baud_rate}')
    console.timeout = 5
    console.expect([r'>', pexpect.TIMEOUT], timeout=2)
    return console


# ==================
# TEMPLATE 2: Test Framework Functions
# ==================
def start_test(name, timeout=3):
    """Start a test, including recording the test name and junit object."""
    global g_test_name, g_test_junit, g_console

    g_test_junit = jux.TestCase(name)
    test_cases.append(g_test_junit)
    g_test_name = name
    print(f'Test "{g_test_name}" starting')
    if g_console:
        g_console.timeout = timeout


def test_pass():
    """Handle a test pass, recording info."""
    global g_test_name
    print(f'Test "{g_test_name}" passes')


def test_fail(failure_info):
    """Handle a test failure, recording info."""
    global g_test_name, g_test_junit
    print(f'Test "{g_test_name}" fails: {failure_info}')
    if g_test_junit:
        g_test_junit.add_failure_info(failure_info)


def write_junit_xml(junit_file):
    """Write JUnit XML file for CI/CD integration."""
    test_suite = jux.TestSuite('MBC HIL Tests', test_cases)
    with open(junit_file, 'w') as f:
        jux.TestSuite.to_file(f, [test_suite], prettyprint=True)
    print(f'\nJUnit XML written to: {junit_file}')


# ==================
# TEST FUNCTIONS (Using Both Templates)
# ==================
def test_version(console, expected_version):
    """Test version command."""
    start_test('version', timeout=5)  # TEMPLATE 2
    flush_input(console)              # TEMPLATE 1

    command = 'main version'
    print(f'Sending: {command}')
    console.sendline(command)

    version_pattern = fr'.*?Version="{expected_version}"'
    result = console.expect([version_pattern, pexpect.TIMEOUT, pexpect.EOF])

    if result == 0:
        prompt_result = console.expect([r'.*?>', pexpect.TIMEOUT, pexpect.EOF])
        if prompt_result == 0:
            test_pass()  # TEMPLATE 2
            flush_input(console)  # TEMPLATE 1
            return True
        else:
            test_fail('Prompt not found after version command')  # TEMPLATE 2
            return False
    else:
        test_fail(f'Version pattern not found. Expected: {expected_version}')  # TEMPLATE 2
        return False


def test_help(console):
    """Test help command."""
    start_test('help', timeout=5)  # TEMPLATE 2
    flush_input(console)           # TEMPLATE 1

    command = 'help'
    print(f'Sending: {command}')
    console.sendline(command)

    help_pattern = r'.*?main \(status, version, reset\)'
    result = console.expect([help_pattern, pexpect.TIMEOUT, pexpect.EOF])

    if result == 0:
        prompt_result = console.expect([r'.*?>', pexpect.TIMEOUT, pexpect.EOF])
        if prompt_result == 0:
            test_pass()  # TEMPLATE 2
            flush_input(console)  # TEMPLATE 1
            return True
        else:
            test_fail('Prompt not found after help command')  # TEMPLATE 2
            return False
    else:
        test_fail('Help pattern not found')  # TEMPLATE 2
        return False


# ==================
# MAIN EXECUTION (Using Both Templates)
# ==================
def main():
    global g_console

    # Configuration
    SERIAL_PORT = 'COM4'
    BAUD_RATE = 115200
    EXPECTED_VERSION = 'v1.0.0'
    JUNIT_FILE = 'test-results.xml'

    # TEMPLATE 1: Cleanup before connecting
    cleanup_existing_processes(SERIAL_PORT)

    # TEMPLATE 1: Connect
    console = connect_serial(SERIAL_PORT, BAUD_RATE)
    g_console = console

    # Run tests
    version_passed = test_version(console, EXPECTED_VERSION)
    help_passed = test_help(console)

    # Both tests must pass
    test_passed = version_passed and help_passed

    # TEMPLATE 2: Generate JUnit XML
    write_junit_xml(JUNIT_FILE)

    # TEMPLATE 1: Cleanup
    console.kill(signal.SIGTERM)

    # Print result
    print('Overall: PASS' if test_passed else 'Overall: FAIL')
    sys.exit(0 if test_passed else 1)


# TEMPLATE 1: Error handling wrapper
if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        sys.exit(1)
    except Exception:
        sys.exit(1)
    finally:
        pass
```

---

#### Step-by-Step Workflow

**Step 1: Process Cleanup (Template 1)**
```python
cleanup_existing_processes(SERIAL_PORT)
```
- Kills old plink processes
- Prevents "port in use" errors

**Step 2: Connect to Serial (Template 1)**
```python
console = connect_serial(SERIAL_PORT, BAUD_RATE)
g_console = console
```
- Establishes connection
- Sets global console for framework

**Step 3: Run Tests (Both Templates)**
```python
version_passed = test_version(console, EXPECTED_VERSION)
help_passed = test_help(console)
```
- Each test uses:
  - `start_test()` - Template 2
  - `flush_input()` - Template 1
  - `test_pass()/test_fail()` - Template 2

**Step 4: Generate JUnit XML (Template 2)**
```python
write_junit_xml(JUNIT_FILE)
```
- Creates test-results.xml
- Contains all test results

**Step 5: Cleanup and Exit (Template 1)**
```python
console.kill(signal.SIGTERM)
sys.exit(0 if test_passed else 1)
```
- Closes connection
- Returns exit code for CI/CD

---

#### Real-World Usage Scenarios

**Scenario 1: Local Development**
```bash
# Run tests locally
python combined_test.py

# Check results
cat test-results.xml
```

**Scenario 2: CI/CD Pipeline (GitHub Actions)**
```yaml
- name: Run HIL Tests
  run: python combined_test.py

- name: Upload Test Results
  uses: actions/upload-artifact@v3
  with:
    name: test-results
    path: test-results.xml
```

**Scenario 3: Different COM Ports**
```python
# Modify main() configuration
SERIAL_PORT = 'COM5'  # Different port
BAUD_RATE = 9600      # Different baud rate
```

**Scenario 4: Adding More Tests**
```python
def test_reset(console):
    """Test reset command."""
    start_test('reset', timeout=10)  # Longer timeout
    flush_input(console)

    command = 'main reset'
    print(f'Sending: {command}')
    console.sendline(command)

    reset_pattern = r'.*?System Reset'
    result = console.expect([reset_pattern, pexpect.TIMEOUT, pexpect.EOF])

    if result == 0:
        test_pass()
        flush_input(console)
        return True
    else:
        test_fail('Reset pattern not found')
        return False

# In main()
reset_passed = test_reset(console)
test_passed = version_passed and help_passed and reset_passed
```

---

### 19.5 Quick Reference

#### Template 1 Function Signatures

```python
# Process cleanup
cleanup_existing_processes(serial_port: str) -> None

# Buffer flushing
flush_input(console: pexpect.PopenSpawn) -> None

# Connection
connect_serial(serial_port: str, baud_rate: int) -> pexpect.PopenSpawn

# Generic test (optional)
test_command(console: pexpect.PopenSpawn, command: str, expected_pattern: str) -> bool
```

#### Template 2 Function Signatures

```python
# Test framework
start_test(name: str, timeout: int = 3) -> None
test_pass() -> None
test_fail(failure_info: str) -> None

# JUnit XML
write_junit_xml(junit_file: str) -> None
```

#### Common Customization Patterns

**Pattern 1: Change Serial Settings**
```python
# In main()
SERIAL_PORT = 'COM5'
BAUD_RATE = 9600
```

**Pattern 2: Custom Prompt**
```python
# In connect_serial() and test functions
console.expect([r'.*?\$', pexpect.TIMEOUT], timeout=2)  # $ instead of >
```

**Pattern 3: Add Test**
```python
# 1. Write test function
def test_custom(console):
    start_test('custom', timeout=5)
    flush_input(console)
    # ... test logic ...
    test_pass() or test_fail('reason')
    return True/False

# 2. Call in main()
custom_passed = test_custom(console)

# 3. Include in overall result
test_passed = version_passed and help_passed and custom_passed
```

**Pattern 4: Different XML Filename**
```python
# In main()
JUNIT_FILE = 'my-test-results.xml'
write_junit_xml(JUNIT_FILE)
```

**Pattern 5: Longer Timeout for Slow Commands**
```python
def test_slow_command(console):
    start_test('slow', timeout=30)  # 30 second timeout
    # ... test logic ...
```

---

## Summary

**Template 1** provides robust serial communication with:
- Process cleanup
- Buffer flushing
- Connection management
- Error handling

**Template 2** provides professional test reporting with:
- Structured test framework
- JUnit XML output
- CI/CD integration

**Combined**: Production-ready HIL testing with full automation support.

**Next Steps:**
1. Copy Template 1 for simple serial tests
2. Copy Template 2 when you need JUnit XML
3. Combine both for production CI/CD integration
4. Customize for your specific hardware and commands

