"""
HIL Test: Version Command Verification

Verifies firmware version via serial communication.

Requirements:
    - Python 3.6+, pexpect, PuTTY plink

Exit Codes:
    0: Test passed
    1: Test failed

Tests
- Version command

Lesson 1: Basic Testing Pattern
Lesson 2: Add Error Handling and Helper Functions

========================================
Core testing pattern:
========================================
# 1. SEND command
console.sendline('command')

# 2. EXPECT response (wait for pattern)
result = console.expect([expected_pattern, TIMEOUT, EOF])

# 3. VERIFY result
if result == 0:  # Pattern matched = success
    test_passed = True
else:            # Timeout or EOF = failure
    test_passed = False

run
- $python MBC_HIL.py
// Sending: main version
// PASS
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

    # Connect to serial port
    console = popen_spawn.PopenSpawn(f'plink -serial {SERIAL_PORT} -sercfg {BAUD_RATE}')
    console.timeout = 5
    
    # Wait for initial prompt
    console.expect([r'>', pexpect.TIMEOUT], timeout=2)
    
    # Send version command
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
        test_passed = (prompt_result == 0)
    else:
        test_passed = False
    
    # Cleanup
    console.kill(signal.SIGTERM)
    
    # Print result
    print('PASS' if test_passed else 'FAIL')
    
    # Exit with result
    sys.exit(0 if test_passed else 1)

if __name__ == '__main__':
    # This runs when you execute: python day6_python_test_version_HIL.py
    main()




# i2c test reserve 1

# i2c test write 1 0x44 0x2c 0x06

# i2c test read 1 0x44 6

# i2c test release 1


"""
Lesson 2: Add Error Handling and Helper Functions
"""

'''
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


def main():
    # Configuration
    SERIAL_PORT = 'COM4'
    BAUD_RATE = 115200
    EXPECTED_VERSION = 'v1.0.0'

    # Connect
    console = connect_serial(SERIAL_PORT, BAUD_RATE)
    
    # Run test
    test_passed = test_version(console, EXPECTED_VERSION)
    
    # Cleanup
    console.kill(signal.SIGTERM)
    
    # Print result
    print('PASS' if test_passed else 'FAIL')
    sys.exit(0 if test_passed else 1)


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        sys.exit(1)
    except Exception:
        sys.exit(1)
    finally:
        pass
'''