
"""
HIL Test: Version and Help Commands Verification

Verifies firmware version and help commands via serial communication.

Requirements:
    - Python 3.6+, pexpect, PuTTY plink

Exit Codes:
    0: Test passed
    1: Test failed

Features:
    - Error handling: try/except blocks for graceful error handling and cleanup
    - Helper functions: Modular design with reusable functions (connect_serial, test_version, test_help)
    - Logging: Uses logging module with INFO level for test output and ERROR for failures
    - Command line arguments: Configurable via argparse (--port, --baud, --version)

Tests:
    - Version command
    - Help command

Usage:
    python MBC_HIL_Full_Suit.py
    python MBC_HIL_Full_Suit.py --port COM5 --baud 9600 --version v2.0.0
    python MBC_HIL_Full_Suit.py --help

Core Testing Pattern:
    1. SEND command
    2. EXPECT response (wait for pattern)
    3. VERIFY result
"""

import sys
import signal
import logging
import argparse
import pexpect
import pexpect.popen_spawn as popen_spawn

# Configure logging (minimal setup)
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
    logging.info(f'Sending: {command}')  # Changed from print()
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
    logging.info(f'Sending: {command}')  # Changed from print()
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
    logging.info('Version: PASS' if version_test_passed else 'Version: FAIL')  # Changed
    
    # Test 2: Help command
    help_test_passed = test_help(console)
    logging.info('Help: PASS' if help_test_passed else 'Help: FAIL')  # Changed
    
    # Both tests must pass
    test_passed = version_test_passed and help_test_passed
    
    # Cleanup
    console.kill(signal.SIGTERM)
    
    # Print overall result
    logging.info('Overall: PASS' if test_passed else 'Overall: FAIL')  # Changed
    
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