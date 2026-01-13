"""
HIL Test: Version and Help Commands Verification

Verifies firmware version and help commands via serial communication.

Requirements:
    - Python 3.6+, pexpect, PuTTY plink

Exit Codes:
    0: Test passed
    1: Test failed

Tests
- Version command
- Help command

Lesson 1: Basic Testing Pattern
Lesson 2: Error Handling and Helper Functions

========================================
Core testing pattern:
========================================

console.sendline('command')
result = console.expect([expected_pattern, TIMEOUT, EOF])
if result == 0:  # Pattern matched = success
    test_passed = True
else:            # Timeout or EOF = failure
    test_passed = False

run
$ python MBC_HIL_help.py 
Sending: main version
Version: PASS
Sending: help
Help: PASS
Overall: PASS
"""

"""
========================================
Lesson 1: Basic Testing Pattern
========================================
"""
"""
import sys
import signal
import pexpect
import pexpect.popen_spawn as popen_spawn


def main():
    # Configuration
    SERIAL_PORT = 'COM4'
    BAUD_RATE = 115200
    EXPECTED_VERSION = 'v1.0.0'

    #========================================
    # Connect to serial port
    #========================================
    console = popen_spawn.PopenSpawn(f'plink -serial {SERIAL_PORT} -sercfg {BAUD_RATE}')
    console.timeout = 5
    
    # Wait for initial prompt
    console.expect([r'>', pexpect.TIMEOUT], timeout=2)
    
    #========================================
    # Test 1: Version command
    #========================================
    command = 'main version'
    print(f'Sending: {command}')
    console.sendline(command)
    
    # Wait for version pattern (non-greedy to not consume everything)
    version_pattern = fr'.*?Version="{EXPECTED_VERSION}"'
    result = console.expect([version_pattern, pexpect.TIMEOUT, pexpect.EOF])
    
    # Check if version found
    if result == 0:
        # Wait for prompt (flexible pattern to handle newlines/whitespace)
        prompt_result = console.expect([r'.*?>', pexpect.TIMEOUT, pexpect.EOF])
        version_test_passed = (prompt_result == 0)
    else:
        version_test_passed = False
    
    # Print result for version test
    print('Version: PASS' if version_test_passed else 'Version: FAIL')
    
    #========================================
    # Test 2: Help command
    #========================================
    command = 'help'
    print(f'Sending: {command}')
    console.sendline(command)
    
    # Wait for help pattern (non-greedy to not consume everything)
    help_pattern = r'.*?main \(status, version, reset\)'
    result = console.expect([help_pattern, pexpect.TIMEOUT, pexpect.EOF])
    
    # Check if help output found
    if result == 0:
        # Wait for prompt (flexible pattern to handle newlines/whitespace)
        prompt_result = console.expect([r'.*?>', pexpect.TIMEOUT, pexpect.EOF])
        help_test_passed = (prompt_result == 0)
    else:
        help_test_passed = False
    
    # Print result for help test
    print('Help: PASS' if help_test_passed else 'Help: FAIL')
    
    # Both tests must pass
    test_passed = version_test_passed and help_test_passed
    
    #========================================
    # Cleanup
    #========================================
    console.kill(signal.SIGTERM)
    
    # Print result
    print('Overall: PASS' if test_passed else 'Overall: FAIL')
    
    # Exit with result
    sys.exit(0 if test_passed else 1)

if __name__ == '__main__':
    # This runs when you execute: python MBC_HIL_help.py
    main()

"""




"""
========================================
Lesson 2: Add Error Handling and Helper Functions
========================================
Error Handling Pattern:

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt: 
        sys.exit(1)
    except Exception: 
        sys.exit(1)
    finally:
        pass

Helper Functions Pattern:

def helper_function(param1, param2):
    # Do work
    return result

def main():
    result = helper_function(value1, value2)
"""

import sys
import signal
import pexpect
import pexpect.popen_spawn as popen_spawn


def connect_serial(serial_port, baud_rate):
    """Connect to serial port and wait for prompt."""
    console = popen_spawn.PopenSpawn(f'plink -serial {serial_port} -sercfg {baud_rate}')
    console.timeout = 5
    console.expect([r'>', pexpect.TIMEOUT], timeout=2)
    return console


def test_version(console, expected_version):
    """Test version command."""
    command = 'main version'
    print(f'Sending: {command}')
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
    print(f'Sending: {command}')
    console.sendline(command)
    
    help_pattern = r'.*?main \(status, version, reset\)'
    result = console.expect([help_pattern, pexpect.TIMEOUT, pexpect.EOF])
    
    if result == 0:
        prompt_result = console.expect([r'.*?>', pexpect.TIMEOUT, pexpect.EOF])
        return (prompt_result == 0)
    return False


def main():
    # Configuration
    SERIAL_PORT = 'COM4'
    BAUD_RATE = 115200
    EXPECTED_VERSION = 'v1.0.0'

    # Connect to serial port
    console = connect_serial(SERIAL_PORT, BAUD_RATE)
    
    # Test 1: Version command
    version_test_passed = test_version(console, EXPECTED_VERSION)
    print('Version: PASS' if version_test_passed else 'Version: FAIL')
    
    # Test 2: Help command
    help_test_passed = test_help(console)
    print('Help: PASS' if help_test_passed else 'Help: FAIL')
    
    # Both tests must pass
    test_passed = version_test_passed and help_test_passed
    
    # Cleanup
    console.kill(signal.SIGTERM)
    
    # Print overall result
    print('Overall: PASS' if test_passed else 'Overall: FAIL')
    
    # Exit with result
    sys.exit(0 if test_passed else 1)


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        sys.exit(1)
    except Exception:
        sys.exit(1)
    finally:
        # This always runs, even if error occurred
        # (useful for cleanup, but not essential for minimal version)
        pass