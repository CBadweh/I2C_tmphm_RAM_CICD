"""
Hardware-in-the-Loop (HIL) Testing for Badweh_Development
Day 6: Static Analysis and HIL Testing

CURRENT STATUS:
==============
Implemented and Active Tests:
1. Reset - Board reset via 'main reset' command
2. Version - Software version verification (CRITICAL: Ring Doorbell lesson!)
3. Help - Help command output verification
4. I2C Status - I2C status table verification
5. TMPHM Measurement - Temperature and humidity sensor measurement
6. TMPHM CRC8 - CRC8 calculation verification
7. TMPHM Status - TMPHM status command

Tests to be Implemented (currently commented out):
1. Console Prompt - Basic prompt response test
2. I2C Reserve/Release - I2C bus reservation and release
3. I2C Write - I2C write operation to SHT31-D sensor
4. I2C Read - I2C read operation from SHT31-D sensor
5. Fault Status - Fault status command
6. LWL Enable/Dump - LWL enable and dump commands

USAGE:
======
Run the test suite:
    python badweh-hilt.py --verbose

Arguments:
    --dut-serial SERIAL    Serial port for DUT (default: COM4)
    --tver VERSION         Test version string (default: v1.0.0)
    --jfile FILE           JUnit XML output file (default: badweh-test-results.xml)
    --verbose              Enable verbose logging for debugging

Example:
    python badweh-hilt.py --verbose --dut-serial COM4 --tver v1.0.0

EXPECTED PATTERNS FOR ACTIVE TESTS:
===================================
1. Reset (do_reset):
   Command: 'main reset'
   Expected Pattern: 'Init: Enter super loop' followed by prompt '> '
   Output: Board initialization messages ending with "Init: Enter super loop"

2. Version (test_version):
   Command: 'version'
   Expected Pattern: 'Version="v1.0.0"' followed by prompt '> '
   Example Output: Version="v1.0.0"

3. Help (test_help):
   Command: 'help'
   Expected Pattern: 'i2c (status, test)' then 'main (status, version, reset)' then prompt '> '
   Example Output:
       ttys (status, test, log, pm)
       fault (data, status, test, log)
       ...
       i2c (status, test)
       tmphm (status, test)
       main (status, version, reset)

4. I2C Status (test_i2c_status):
   Command: 'i2c status'
   Expected Pattern: 'ID' column header, then table row with whitespace '0' (r'\s+0\s+'), then prompt '> '
   Example Output:
          Rsr Sta Dest Msg Byte I2C Err  Register
      ID vrd te  Addr Len Xfrd Err Sta  BaseAddr
      -- --- --- ---- --- ---- --- --- ----------
       0   0   0 0x00   0    0   0   0          0
       1   0   0 0x44   6    6   0   0 0x40005c00

5. TMPHM Measurement (test_tmphm_measurement):
   Command: 'tmphm test lastmeas 0'
   Expected Pattern: 'Temp=.*C', then 'Hum=.*%', then 'age=.*ms', then prompt '> '
   Example Output: Temp=27.0 C Hum=47.4 % age=191 ms

6. TMPHM CRC8 (test_tmphm_crc8):
   Command: 'tmphm test crc8 0xBE 0xEF'
   Expected Pattern: 'crc8: 0x92' then prompt '> '
   Example Output: crc8: 0x92

7. TMPHM Status (test_tmphm_status):
   Command: 'tmphm status'
   Expected Pattern: 'ID' column header, then table row with whitespace '0' (r'\s+0\s+'), then prompt '> '
   Example Output:
              Got  Last Last Meas Meas
         ID State Meas Temp Hum  Age  Time
         -- ----- ---- ---- ---- ---- ----
          0     0    1  270  474   30   17

Author: Based on Gene Schrader's base-hilt.py
Date: 2025-11-13
"""

import argparse
import junit_xml as jux
import logging
import pexpect as pex
import pexpect.popen_spawn as pos
import psutil
import signal
import sys
import time

_log = logging.getLogger()
_log_handler = logging.StreamHandler(sys.stdout)
_log.addHandler(_log_handler)
_log.setLevel(logging.ERROR)

g_dut = None
g_test_name = None
g_prompt = '> '
g_test_junit = None
g_test_junits = []

################################################################################
class BadwehDev:
    """ This class represents the Badweh Development board via serial console. """

    def __init__(self, name, serial_dev, baud_rate=115200):
        self.name = name
        _log.debug('[%s] Connecting to %s at %d', self.name, serial_dev,
                   baud_rate)
        self.console = pos.PopenSpawn('plink -serial %s -sercfg %d' %
                                      (serial_dev, baud_rate))

    def set_timeout(self, timeout):
        self.console.timeout = timeout

    def flush_input(self):
        _log.debug('[%s] flush_input()', self.name)
        while self.console.expect([pex.TIMEOUT, pex.EOF, '.*']) == 2:
            if len(self.console.after) == 0:
                break
            _log.debug('[%s] In flush_input() got %s', self.name,
                       self.console.after)

    def send_line(self, msg):
        _log.debug('[%s] Sending "%s"', self.name, msg)
        self.console.sendline(msg)

    def get_pattern_list(self, pattern_list):
        """ Wait for a series of patterns from the device, with an arbitrary amount
        of text allowed between the patterns.

        pattern_list - a list of text strings (can be regex) to wait for, in order.
        """

        rc = 0
        for pat in pattern_list:
            rc = self.console.expect([pat, pex.TIMEOUT, pex.EOF])
            if rc != 0:
                _log.debug('[%s] In get_pattern_list() failure for "%s" (rc=%d) for test "%s"',
                           self.name, pat, rc, g_test_name)
                return rc, pat
            _log.debug('[%s] Expecting "%s" got "%s%s"', self.name, pat,
                       self.console.before, self.console.after)
        return rc, None

    def get_prompt(self):
        rc, failed_pat = self.get_pattern_list([g_prompt])
        if rc != 0:
            _log.debug('[%s] In get_prompt() failure rc=%d', self.name, rc)
        return rc

    def do_reset(self):
        self.flush_input()
        self.send_line('main reset')
        # Wait for "Resetting MCU..." and then for the boot sequence
        rc, failed_pat = self.get_pattern_list(['Init: Enter super loop', g_prompt])
        if rc != 0:
            _log.debug('[%s] In do_reset() failure rc=%d pat=%s', self.name,
                       rc, failed_pat)
        return rc, failed_pat

    def terminate(self):
        if self.console is not None:
            _log.debug('[%s] Killing', self.name)
            try:
                self.console.kill(signal.SIGTERM)
            except Exception as e:
                _log.debug('[%s] Exception on kill: %s', self.name, str(e))

################################################################################
def testing_init(dut_serial):
    """ Initialize testing software. """

    global g_dut

    # Kill any existing plink processes for this serial port
    kill_procs = []
    for p in psutil.process_iter():
        if p.name() == 'plink.exe':
            if dut_serial.upper() in [s.upper() for s in p.cmdline()]:
                kill_procs.append(p)

    for p in kill_procs:
        _log.debug('Killing process %s %s (pid=%d)', p.name(), p.cmdline(),
                   p.pid)
        p.kill()

    g_dut = BadwehDev('dut', dut_serial)

################################################################################
def start_test(name, timeout=3):
    """ Start a test, including recording the test name and junit object.

    name - the name of the test
    timeout - how long to wait for results.
    """

    global g_test_name
    global g_test_junit

    g_test_junit = jux.TestCase(name)
    g_test_junits.append(g_test_junit)
    g_test_name = name
    _log.debug('Test "%s" starting', g_test_name)
    g_dut.set_timeout(timeout)
    g_dut.flush_input()

################################################################################
def test_fail(failure_info):
    """ Handle a test failure, recording info and restarting the board.

    failure_info - string describing how it failed.
    """

    _log.info('Test "%s" fails: %s', g_test_name, failure_info)
    g_test_junit.add_failure_info(failure_info)

    # After a test fail, try to reset the MCU to clean things up for the next test
    g_dut.do_reset()

################################################################################
def test_pass():
    """ Handle a test pass, recording info. """

    _log.info('Test "%s" passes', g_test_name)
    g_dut.flush_input()

################################################################################
# Test: console_prompt
################################################################################
def test_console_prompt():
    """ Send a carriage return and wait for the console prompt. """

    passed = True
    start_test('console_prompt')
    g_dut.send_line('')
    rc = g_dut.get_prompt()
    if rc != 0:
        test_fail('Did not receive prompt rc=%d' % rc)
        passed = False
    else:
        test_pass()
    return passed

################################################################################
# Test: test_version (CRITICAL - Ring Doorbell lesson!)
################################################################################
def test_version(tver):
    """ Test to verify the software version is as expected.

    This is CRITICAL: Always verify build ID in tests to avoid shipping
    untested software (Ring Doorbell lesson from Gene Schrader).

    tver - the software version string.
    """

    passed = True
    start_test('version')
    g_dut.send_line('version')
    pat_list = ['Version="' + tver + '"', g_prompt]
    rc, failed_pat = g_dut.get_pattern_list(pat_list)
    if rc != 0:
        test_fail('Did not find pattern "%s" rc=%d' % (failed_pat, rc))
        passed = False
    else:
        test_pass()
    return passed

################################################################################
# Test: test_help
################################################################################
def test_help():
    """ Test help command. """

    passed = True
    start_test('help')
    g_dut.send_line('help')
    # Should see module names with their commands (search in order they appear in output)
    pat_list = [r'i2c \(status, test\)', r'main \(status, version, reset\)', g_prompt]
    rc, failed_pat = g_dut.get_pattern_list(pat_list)
    if rc != 0:
        test_fail('Did not find pattern "%s" rc=%d' % (failed_pat, rc))
        passed = False
    else:
        test_pass()
    return passed

################################################################################
# Test: test_i2c_status
################################################################################
def test_i2c_status():
    """ Test I2C status command. """

    passed = True
    start_test('i2c_status')
    g_dut.send_line('i2c status')
    # Should see I2C status table with ID column and data rows
    pat_list = ['ID', r'\s+0\s+', g_prompt]
    rc, failed_pat = g_dut.get_pattern_list(pat_list)
    if rc != 0:
        test_fail('Did not find pattern "%s" rc=%d' % (failed_pat, rc))
        passed = False
    else:
        test_pass()
    return passed

################################################################################
# Test: test_i2c_reserve_release
################################################################################
def test_i2c_reserve_release():
    """ Test I2C bus reservation and release. """

    passed = True
    start_test('i2c_reserve_release', timeout=5)

    # Reserve I2C bus
    g_dut.send_line('i2c test reserve 0')
    rc, failed_pat = g_dut.get_pattern_list(['OK', g_prompt])
    if rc != 0:
        test_fail('Reserve failed: pattern "%s" rc=%d' % (failed_pat, rc))
        passed = False
    else:
        # Release I2C bus
        g_dut.send_line('i2c test release 0')
        rc, failed_pat = g_dut.get_pattern_list(['OK', g_prompt])
        if rc != 0:
            test_fail('Release failed: pattern "%s" rc=%d' % (failed_pat, rc))
            passed = False
        else:
            test_pass()
    return passed

################################################################################
# Test: test_i2c_write
################################################################################
def test_i2c_write():
    """ Test I2C write operation to SHT31-D sensor. """

    passed = True
    start_test('i2c_write', timeout=5)

    # Reserve I2C bus
    g_dut.send_line('i2c test reserve 0')
    rc, failed_pat = g_dut.get_pattern_list(['OK', g_prompt])
    if rc != 0:
        test_fail('Reserve failed: pattern "%s" rc=%d' % (failed_pat, rc))
        passed = False
    else:
        # Write measurement command to sensor (address 0x44, command 0x2c 0x06)
        g_dut.send_line('i2c test write 0 44 2c 06')
        rc, failed_pat = g_dut.get_pattern_list(['OK', g_prompt])
        if rc != 0:
            test_fail('Write failed: pattern "%s" rc=%d' % (failed_pat, rc))
            passed = False
        else:
            test_pass()

        # Release I2C bus
        g_dut.send_line('i2c test release 0')
        g_dut.get_prompt()

    return passed

################################################################################
# Test: test_i2c_read
################################################################################
def test_i2c_read():
    """ Test I2C read operation from SHT31-D sensor. """

    passed = True
    start_test('i2c_read', timeout=5)

    # Reserve I2C bus
    g_dut.send_line('i2c test reserve 0')
    rc, failed_pat = g_dut.get_pattern_list(['OK', g_prompt])
    if rc != 0:
        test_fail('Reserve failed: pattern "%s" rc=%d' % (failed_pat, rc))
        passed = False
    else:
        # Write measurement command to sensor
        g_dut.send_line('i2c test write 0 44 2c 06')
        rc, failed_pat = g_dut.get_pattern_list(['OK', g_prompt])
        if rc != 0:
            test_fail('Write failed: pattern "%s" rc=%d' % (failed_pat, rc))
            passed = False
        else:
            # Wait for measurement (20ms minimum for SHT31-D)
            time.sleep(0.05)

            # Read 6 bytes from sensor
            g_dut.send_line('i2c test read 0 44 6')
            rc, failed_pat = g_dut.get_pattern_list(['OK', g_prompt])
            if rc != 0:
                test_fail('Read failed: pattern "%s" rc=%d' % (failed_pat, rc))
                passed = False
            else:
                test_pass()

        # Release I2C bus
        g_dut.send_line('i2c test release 0')
        g_dut.get_prompt()

    return passed

################################################################################
# Test: test_tmphm_measurement
################################################################################
def test_tmphm_measurement():
    """ Test TMPHM sensor measurement.

    The TMPHM module runs in background, taking measurements every second.
    We need to wait for a measurement to be available before querying.
    """

    passed = True
    start_test('tmphm_measurement', timeout=10)

    # Wait for background measurement to complete (measurements happen every second)
    time.sleep(1.5)

    # Query last measurement
    g_dut.send_line('tmphm test lastmeas 0')
    # Should see temperature and humidity readings
    pat_list = [r'Temp=.*C', r'Hum=.*%', r'age=.*ms', g_prompt]
    rc, failed_pat = g_dut.get_pattern_list(pat_list)
    if rc != 0:
        test_fail('Did not find pattern "%s" rc=%d' % (failed_pat, rc))
        passed = False
    else:
        test_pass()
    return passed

################################################################################
# Test: test_tmphm_crc8
################################################################################
def test_tmphm_crc8():
    """ Test TMPHM CRC8 calculation. """

    passed = True
    start_test('tmphm_crc8', timeout=5)

    # Test CRC8 with known values (from SHT31-D datasheet example)
    # Data: 0xBE 0xEF, CRC should be 0x92
    g_dut.send_line('tmphm test crc8 0xBE 0xEF')
    pat_list = [r'crc8: 0x92', g_prompt]
    rc, failed_pat = g_dut.get_pattern_list(pat_list)
    if rc != 0:
        test_fail('Did not find pattern "%s" rc=%d' % (failed_pat, rc))
        passed = False
    else:
        test_pass()
    return passed

################################################################################
# Test: test_tmphm_status
################################################################################
def test_tmphm_status():
    """ Test TMPHM status command. """

    passed = True
    start_test('tmphm_status', timeout=5)

    g_dut.send_line('tmphm status')
    # Should see instance info
    pat_list = ['ID', r'\s+0\s+', g_prompt]
    rc, failed_pat = g_dut.get_pattern_list(pat_list)
    if rc != 0:
        test_fail('Did not find pattern "%s" rc=%d' % (failed_pat, rc))
        passed = False
    else:
        test_pass()
    return passed

################################################################################
# Test: test_fault_status
################################################################################
def test_fault_status():
    """ Test fault status command. """

    passed = True
    start_test('fault_status', timeout=5)

    g_dut.send_line('fault status')
    # Should see stack info and MPU status
    pat_list = ['Stack.*bytes', 'MPU', g_prompt]
    rc, failed_pat = g_dut.get_pattern_list(pat_list)
    if rc != 0:
        test_fail('Did not find pattern "%s" rc=%d' % (failed_pat, rc))
        passed = False
    else:
        test_pass()
    return passed

################################################################################
# Test: test_lwl_enable_dump
################################################################################
def test_lwl_enable_dump():
    """ Test LWL enable and dump commands. """

    passed = True
    start_test('lwl_enable_dump', timeout=5)

    # Enable LWL (should already be enabled, but confirm)
    g_dut.send_line('lwl enable 1')
    rc, failed_pat = g_dut.get_pattern_list(['OK', g_prompt])
    if rc != 0:
        test_fail('Enable failed: pattern "%s" rc=%d' % (failed_pat, rc))
        passed = False
    else:
        # Dump LWL buffer
        g_dut.send_line('lwl dump')
        rc, failed_pat = g_dut.get_pattern_list(['LWL', g_prompt])
        if rc != 0:
            test_fail('Dump failed: pattern "%s" rc=%d' % (failed_pat, rc))
            passed = False
        else:
            test_pass()
    return passed

################################################################################
# Main test runner
################################################################################
def run_tests(tver):
    """ Run all HIL tests. """

    all_passed = True

    print('\n' + '='*60)
    print('Badweh Development HIL Test Suite')
    print('Day 6: Static Analysis and HIL Testing')
    print('='*60 + '\n')

    # Reset board before testing
    print('Resetting board before tests...')
    g_dut.set_timeout(10)  # Set timeout before reset (10 seconds should be enough)
    rc, failed_pat = g_dut.do_reset()  # Check return value
    if rc != 0:
        print(f'ERROR: Reset failed - pattern "{failed_pat}" not found')
    time.sleep(0.5)

    # Active tests: reset, version, help, and i2c status
    all_passed &= test_version(tver)  # CRITICAL: Ring Doorbell lesson!
    all_passed &= test_help()
    all_passed &= test_i2c_status()

    # COMMENTED OUT - not testing these right now:
    # all_passed &= test_console_prompt()
    # all_passed &= test_i2c_reserve_release()
    # all_passed &= test_i2c_write()
    # all_passed &= test_i2c_read()
    all_passed &= test_tmphm_measurement()
    all_passed &= test_tmphm_crc8()
    all_passed &= test_tmphm_status()
    # all_passed &= test_fault_status()
    # all_passed &= test_lwl_enable_dump()

    print('\n' + '='*60)
    if all_passed:
        print('All tests PASSED')
    else:
        print('Some tests FAILED - check results above')
    print('='*60 + '\n')

    return all_passed

################################################################################
# Entry point
################################################################################
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Badweh Development HIL Tests')
    parser.add_argument('--dut-serial', default='COM4',
                        help='Serial port for DUT (default: COM4)')
    parser.add_argument('--tver', default='v1.0.0',
                        help='Test version string (default: v1.0.0)')
    parser.add_argument('--jfile', default='badweh-test-results.xml',
                        help='JUnit XML output file (default: badweh-test-results.xml)')
    parser.add_argument('--verbose', action='store_true',
                        help='Enable verbose logging')

    args = parser.parse_args()

    if args.verbose:
        _log.setLevel(logging.DEBUG)

    print('Badweh HIL Testing')
    print('  DUT Serial: %s' % args.dut_serial)
    print('  Test Version: %s' % args.tver)
    print('  JUnit File: %s' % args.jfile)

    try:
        testing_init(args.dut_serial)
        all_passed = run_tests(args.tver)

        # Generate JUnit XML
        test_suite = jux.TestSuite('Badweh HIL Tests', g_test_junits)
        with open(args.jfile, 'w') as f:
            jux.TestSuite.to_file(f, [test_suite], prettyprint=True)

        print('\nJUnit XML written to: %s' % args.jfile)

        # Return appropriate exit code for CI/CD
        sys.exit(0 if all_passed else 1)

    except KeyboardInterrupt:
        print('\nInterrupted by user')
        sys.exit(1)

    except Exception as e:
        print('\nFatal error: %s' % str(e))
        import traceback
        traceback.print_exc()
        sys.exit(1)

    finally:
        if g_dut is not None:
            g_dut.terminate()
