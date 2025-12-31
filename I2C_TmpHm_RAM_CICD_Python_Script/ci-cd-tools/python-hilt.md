# Python HIL Testing - Reset Hang Problem

## Problem Description

The `badweh-hilt.py` script was hanging at the "Resetting board before tests..." step, causing the script to become unresponsive and requiring manual interruption.

## Initial Code

The problematic code was in the `run_tests()` function:

```python
def run_tests(tver):
    """ Run all HIL tests. """

    all_passed = True

    print('\n' + '='*60)
    print('Badweh Development HIL Test Suite')
    print('Day 6: Static Analysis and HIL Testing')
    print('='*60 + '\n')

    # Reset board before testing
    print('Resetting board before tests...')
    g_dut.do_reset()
    time.sleep(0.5)

    # Test 1-3: Basic console tests
    all_passed &= test_console_prompt()
    all_passed &= test_version(tver)  # CRITICAL: Ring Doorbell lesson!
    all_passed &= test_help()
```

## Problem Encountered

When running the script, it would hang at the reset step:

```bash
$ python badweh-hilt.py
Badweh HIL Testing
  DUT Serial: COM4
  Test Version: v1.0.0
  JUnit File: badweh-test-results.xml

  Test Version: v1.0.0
  JUnit File: badweh-test-results.xml

  JUnit File: badweh-test-results.xml

============================================================
Badweh Development HIL Test Suite
Day 6: Static Analysis and HIL Testing
============================================================

Resetting board before tests...

Interrupted by user
```

## Root Cause

The issue was caused by:

1. **No timeout set**: The `do_reset()` method was called without setting a timeout first. Timeouts are only set in `start_test()`, which happens after the reset.

2. **No error handling**: The return value from `do_reset()` (which returns `(rc, failed_pat)`) was not being checked, so if the reset failed or patterns didn't match, the script would hang indefinitely waiting for expected patterns.

3. **Pattern mismatch**: The `do_reset()` method waits for specific patterns (`'Resetting MCU'`, `'READY.*Entering super loop'`, and the prompt). If these patterns don't match the actual device output, `pexpect` will wait indefinitely (or until timeout, if one is set).

## Solution

The fix involves:

1. Setting a timeout before calling `do_reset()`
2. Checking the return value from `do_reset()` to detect failures
3. Providing clear error messages and graceful failure handling

### Fixed Code

```python
def run_tests(tver):
    """ Run all HIL tests. """

    all_passed = True

    print('\n' + '='*60)
    print('Badweh Development HIL Test Suite')
    print('Day 6: Static Analysis and HIL Testing')
    print('='*60 + '\n')

    # Reset board before testing
    print('Resetting board before tests...')
    g_dut.set_timeout(10)  # Set a timeout before reset (10 seconds should be enough)
    rc, failed_pat = g_dut.do_reset()
    if rc != 0:
        print('ERROR: Reset failed! rc=%d, failed pattern: %s' % (rc, failed_pat))
        print('The device may not be responding correctly.')
        print('Try running with --verbose to see what the device is sending.')
        return False
    time.sleep(0.5)

    # Test 1-3: Basic console tests
    all_passed &= test_console_prompt()
    all_passed &= test_version(tver)  # CRITICAL: Ring Doorbell lesson!
    all_passed &= test_help()
```

## Key Changes

1. **Added timeout**: `g_dut.set_timeout(10)` ensures the reset operation will timeout after 10 seconds instead of hanging indefinitely.

2. **Error checking**: The return value `(rc, failed_pat)` is now captured and checked. If `rc != 0`, it means the reset failed.

3. **Graceful failure**: Instead of continuing when reset fails, the function now prints an error message and returns `False`, allowing the main script to handle the error appropriately.

4. **Debugging guidance**: The error message suggests using `--verbose` flag to see what the device is actually sending, which helps diagnose pattern mismatch issues.

## Additional Debugging

If the problem persists, run with the `--verbose` flag to see detailed debug output:

```bash
python badweh-hilt.py --verbose
```

This will show what patterns the device is actually sending, which can help identify if the expected patterns in `do_reset()` need to be adjusted to match the actual device output.

