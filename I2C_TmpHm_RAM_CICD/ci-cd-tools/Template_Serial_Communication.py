"""
Template 1: Complete Serial Communication & Hardware Testing
Merges: console.expect() + buffer flushing + process cleanup + error handling

Use this when: Talking to hardware via serial/UART/USB with robust error handling
"""

import sys
import signal
import psutil
import pexpect
import pexpect.popen_spawn as popen_spawn


def cleanup_existing_processes(serial_port):
    """Kill any existing plink processes using the serial port."""
    for p in psutil.process_iter():  # iterates over running processes
        if p.name() == 'plink.exe':  # process name
            if serial_port.upper() in [s.upper() for s in p.cmdline()]:  # command line arguments (includes COM port)
                p.kill()  # terminates the process


def flush_input(console):
    """Clear stale data from serial buffer."""
    while console.expect(['.*', pexpect.TIMEOUT, pexpect.EOF], timeout=0.1) == 0:
        if len(console.after) == 0:
            break
    # if console.expect() see any characters, ".*", it return 0, and while loop enter the while loop
    # if console.expect() TIMEOUT-after 0.1s or EOF, while loop breaks and then leave the flush_input() function


def connect_serial(serial_port, baud_rate):
    """Connect to serial port and wait for prompt."""
    console = popen_spawn.PopenSpawn(f'plink -serial {serial_port} -sercfg {baud_rate}')
    console.timeout = 5
    console.expect([r'>', pexpect.TIMEOUT], timeout=2)
    return console


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
