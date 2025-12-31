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

1. **Step 6: Timeout Management** (prevents hangs)
   - Set timeouts before operations
   - Prevents infinite waits
   - Critical for production reliability

2. **Step 7: Return Value Checking** (catches failures)
   - Check every `expect()` return value
   - Log specific error details
   - Return False immediately on failure

3. **Step 9: Verbose Mode** (helps with debugging)
   - Add `--verbose` flag
   - Enable DEBUG logging when needed
   - Essential for troubleshooting

4. **Step 10: Better Error Messages** (improves diagnostics)
   - Contextual error information
   - Suggests debugging steps
   - Reduces troubleshooting time

5. **Step 8: Reset Functionality** (adds capability)
   - Reset device before tests
   - Ensures clean state
   - Common in production test suites

**Why These Steps Matter:**
- Address issues from goal file (reset hang problem)
- Move script toward production-ready reliability
- Prevent common failure scenarios
- Improve debugging capabilities

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

## Advanced Steps Implementation Order

1. **Step 11: Process Cleanup** (prevents conflicts)
   - Kill existing plink processes before connecting
   - Ensures clean connection state

2. **Step 14: Input Flushing** (ensures clean state)
   - Clear serial buffer before each test
   - Prevents pattern matching on stale data

3. **Step 12: Test Framework** (organizes tests)
   - Implement `start_test()`, `test_pass()`, `test_fail()`
   - Centralized test result tracking

4. **Step 15: Configurable Logs** (improves control)
   - Add `--log` argument with level options
   - Professional log level management

5. **Step 13: JUnit XML** (CI/CD integration)
   - Generate JUnit XML output
   - Enable CI/CD system integration

**Result:** These steps move your script from intermediate to production-ready with full CI/CD integration capabilities.
