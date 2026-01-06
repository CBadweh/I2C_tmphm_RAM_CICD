"""
HIL Test: Version and Help Commands Verification with Advanced Features

Verifies firmware version and help commands via serial communication with
professional testing framework features.

Requirements:
    - Python 3.6+, pexpect, PuTTY plink, psutil, junit-xml

Exit Codes:
    0: All tests passed
    1: One or more tests failed

Features:
    - Error handling: try/except blocks for graceful error handling and cleanup
    - Helper functions: Modular design with reusable functions
    - Test framework: Structured test execution with start_test(), test_pass(), test_fail()
    - Input buffer flushing: Clears stale data from serial buffer before/after tests
    - Process cleanup: Kills existing plink processes before connecting
    - JUnit XML output: Generates CI/CD compatible test results XML file
    - Command line arguments: Configurable via argparse (--port, --baud, --version, --jfile)

Tests:
    - Version command
    - Help command

Usage:
    python MBC_HIL_Full_Suit_Advanced.py
    python MBC_HIL_Full_Suit_Advanced.py --port COM5 --baud 9600 --version v2.0.0
    python MBC_HIL_Full_Suit_Advanced.py --jfile custom-results.xml
    python MBC_HIL_Full_Suit_Advanced.py --help

Core Testing Pattern:
    1. SEND command
    2. EXPECT response (wait for pattern)
    3. VERIFY result
    4. RECORD in JUnit XML
"""

import sys
import signal
import psutil
import pexpect
import pexpect.popen_spawn as popen_spawn
import junit_xml as jux
import argparse

# Global test cases list for JUnit XML output
test_cases = []

# Global variables for test framework
g_test_name = None
g_test_junit = None
g_console = None


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
        flush_input(g_console)


def test_pass():
    """Handle a test pass, recording info."""
    global g_test_name, g_console
    
    print(f'Test "{g_test_name}" passes')
    if g_console:
        flush_input(g_console)


def test_fail(failure_info):
    """Handle a test failure, recording info.
    
    Args:
        failure_info: String describing how it failed
    """
    global g_test_name, g_test_junit
    
    print(f'Test "{g_test_name}" fails: {failure_info}')
    if g_test_junit:
        g_test_junit.add_failure_info(failure_info)


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


def main():
    global g_console
    
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='HIL Test: Version Command with JUnit XML Output')
    parser.add_argument('--port', default='COM4', help='Serial port (default: COM4)')
    parser.add_argument('--baud', type=int, default=115200, help='Baud rate (default: 115200)')
    parser.add_argument('--version', default='v1.0.0', help='Expected version (default: v1.0.0)')
    parser.add_argument('--jfile', default='test-results.xml', help='JUnit XML output file (default: test-results.xml)')
    args = parser.parse_args()
    
    # Configuration
    SERIAL_PORT = args.port
    BAUD_RATE = args.baud
    EXPECTED_VERSION = args.version
    JUNIT_FILE = args.jfile

    # Cleanup BEFORE connecting
    cleanup_existing_processes(SERIAL_PORT)

    # Connect
    g_console = connect_serial(SERIAL_PORT, BAUD_RATE)
    console = g_console
    
    # Test 1: Version command
    version_test_passed = test_version(console, EXPECTED_VERSION)
    
    # Test 2: Help command
    help_test_passed = test_help(console)
    
    # Both tests must pass
    test_passed = version_test_passed and help_test_passed
    
    # Generate JUnit XML
    test_suite = jux.TestSuite('MBC HIL Tests', test_cases)
    with open(JUNIT_FILE, 'w') as f:
        jux.TestSuite.to_file(f, [test_suite], prettyprint=True)
    
    print(f'\nJUnit XML written to: {JUNIT_FILE}')
    
    # Cleanup
    console.kill(signal.SIGTERM)
    
    # Print result
    print('Overall: PASS' if test_passed else 'Overall: FAIL')
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