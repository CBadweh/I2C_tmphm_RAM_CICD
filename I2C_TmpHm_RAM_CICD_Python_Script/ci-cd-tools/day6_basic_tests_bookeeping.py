import argparse # used to parse the command line arguments
import logging # used to log the messages
import signal # used to kill the process
import sys # used to exit the program
import time # used to sleep for a short time

import junit_xml as jux # used to create the JUnit XML file
import pexpect # used to send and receive data from the serial port
import pexpect.popen_spawn as popen_spawn # used to connect to the serial port
import psutil # used to get the process information


PROMPT_PATTERN = r'>\s?'
OK_PATTERN = r'(OK|Return code 0)'

_log = logging.getLogger('day6-hil-abstraction') 
_handler = logging.StreamHandler(sys.stdout)
_log.addHandler(_handler)
_log.setLevel(logging.INFO)

g_dut = None
g_testcases = []
g_current_case = None


###############################################################################
# Console helper class (BadwehDev)
###############################################################################
class BadwehDev:
    """Lightweight console wrapper for the simplified HIL runner."""

    def __init__(self, serial_port, baud_rate):
        _log.info('Connecting to %s @ %d', serial_port, baud_rate)
        self.console = popen_spawn.PopenSpawn(f'plink -serial {serial_port} -sercfg {baud_rate}')
        self.console.timeout = 3

    def set_timeout(self, timeout):
        self.console.timeout = timeout

    def flush_input(self):
        """Drain any buffered console output before starting a command."""
        while True:
            rc = self.console.expect([pexpect.TIMEOUT, pexpect.EOF, '.*'])
            if rc != 2 or not self.console.after:
                break

    def send_line(self, line):
        _log.debug('CMD: %s', line)
        self.console.sendline(line)

    def expect_patterns(self, patterns):
        """Wait for each pattern in order; return (True, None) on success."""
        for pat in patterns:
            rc = self.console.expect([pat, pexpect.TIMEOUT, pexpect.EOF])
            if rc != 0:
                _log.debug('Pattern "%s" not found. before="%s" after="%s"', pat, self.console.before, self.console.after)
                return False, pat
            _log.debug('Matched "%s" with before="%s" after="%s"', pat, self.console.before, self.console.after)
        return True, None

    def get_prompt(self):
        ok, failed = self.expect_patterns([PROMPT_PATTERN])
        return 0 if ok else failed

    def do_reset(self):
        self.flush_input()
        self.send_line('main reset')
        return self.expect_patterns([r'\[READY\].*Entering super loop', PROMPT_PATTERN])

    def terminate(self):
        if self.console is None:
            return
        try:
            self.console.kill(signal.SIGTERM)
        except Exception:
            pass


def expect_return_code_zero(cmd, retries=3, delay=0.05):
    """Send a command and wait for Return code 0, retrying on busy (-10)."""
    for attempt in range(retries):
        g_dut.send_line(cmd)
        idx = g_dut.console.expect([r'Return code (-?\d+)', pexpect.TIMEOUT, pexpect.EOF])
        if idx == 0:
            code = int(g_dut.console.match.group(1))
            _log.debug('"%s" returned %d', cmd, code)
            g_dut.expect_patterns([PROMPT_PATTERN])
            if code == 0:
                return True, code
            if code == -10 and attempt < retries - 1:
                time.sleep(delay)
                continue
            return False, code
        _log.debug('Timeout/EOF waiting for return code on "%s"', cmd)
        return False, None
    return False, None


###############################################################################
# Test scaffolding
###############################################################################
def testing_init(serial_port, baud_rate):
    """Kill stale plink.exe instances, then connect."""
    global g_dut
    for proc in psutil.process_iter():
        if proc.name().lower() == 'plink.exe' and serial_port.upper() in [arg.upper() for arg in proc.cmdline()]:
            proc.kill()
    g_dut = BadwehDev(serial_port, baud_rate)


def start_test(name, timeout=3):
    global g_current_case
    g_current_case = jux.TestCase(name)
    g_testcases.append(g_current_case)
    g_dut.set_timeout(timeout)
    g_dut.flush_input()
    print(f'  - {name} ...', end=' ', flush=True)


def test_pass():
    print('PASS')
    g_dut.flush_input()


def test_fail(reason):
    print(f'FAIL ({reason})')
    g_current_case.add_failure_info(reason)
    g_dut.do_reset()


###############################################################################
# Tests
###############################################################################
def test_version(expected_version):
    start_test('version')
    g_dut.send_line('main version')
    ok, missing = g_dut.expect_patterns([fr'Version="{expected_version}"', PROMPT_PATTERN])
    if ok:
        test_pass()
        return True
    test_fail(f'Missing pattern: {missing}')
    return False


def test_console_prompt():
    start_test('console_prompt')
    g_dut.send_line('')
    if g_dut.get_prompt() == 0:
        test_pass()
        return True
    test_fail('Prompt missing')
    return False


def test_i2c_status():
    start_test('i2c_status')
    g_dut.send_line('i2c status')
    ok, missing = g_dut.expect_patterns([r'ID vrd', r'0x40005c00', PROMPT_PATTERN])
    if ok:
        test_pass()
        return True
    test_fail(f'Missing pattern: {missing}')
    return False


def test_i2c_read():
    start_test('i2c_read', timeout=5)

    ok, rc_val = expect_return_code_zero('i2c test reserve 0', retries=6, delay=0.1)
    if not ok:
        test_fail(f'Reserve rc={rc_val}')
        return False

    ok, rc_val = expect_return_code_zero('i2c test write 0 0x44 0x2c 0x06', retries=3, delay=0.05)
    if not ok:
        test_fail(f'Write rc={rc_val}')
        return False

    time.sleep(0.05)
    ok, rc_val = expect_return_code_zero('i2c test read 0 0x44 6', retries=3, delay=0.05)
    if not ok:
        test_fail(f'Read rc={rc_val}')
        return False

    ok, rc_val = expect_return_code_zero('i2c test release 0', retries=3, delay=0.05)
    if not ok:
        test_fail(f'Release rc={rc_val}')
        return False

    test_pass()
    return True


def test_tmphm_measurement():
    start_test('tmphm_measurement', timeout=5)
    for attempt in range(5):
        g_dut.send_line('tmphm test lastmeas 0')
        idx = g_dut.console.expect([r'Temp=.*C.*Hum=.*%', r'tmphm_get_last_meas fails.*', pexpect.TIMEOUT])
        if idx == 0:
            g_dut.expect_patterns([PROMPT_PATTERN])
            test_pass()
            return True
        if idx == 1:
            _log.debug('tmphm measurement not ready, retrying...')
            g_dut.expect_patterns([PROMPT_PATTERN])
            time.sleep(0.5)
            continue
        test_fail('Timeout waiting for TMPHM measurement')
        return False

    test_fail('No TMPHM measurement ready')
    return False


def test_fault_status():
    start_test('fault_status', timeout=5)
    g_dut.send_line('fault status')
    ok, missing = g_dut.expect_patterns(['Stack.*bytes', PROMPT_PATTERN])
    if ok:
        test_pass()
        return True
    test_fail(f'Missing pattern: {missing}')
    return False


###############################################################################
# Runner
###############################################################################
def run_tests(expected_version):
    print('\n=== Day 6 Simplified HIL Suite ===')
    g_dut.do_reset()
    time.sleep(0.5)

    results = [
        test_console_prompt(),   # >
        test_version(expected_version), # main version
        test_i2c_status(),        # i2c status   
        test_i2c_read(),          # Smoke test, > i2c test reserve 0, > i2c test write 0 0x44 0x2c 0x06, > i2c test read 0 0x44 6, > i2c test release 0, >
        test_tmphm_measurement(), # > tmphm test lastmeas 0
        test_fault_status(),      # > fault status
    ]

    passed = all(results)
    print('=== RESULT: {} ===\n'.format('PASS' if passed else 'FAIL'))
    return passed


def main():
    parser = argparse.ArgumentParser(description='Ultra-simplified HIL with logging scaffolding')
    parser.add_argument('--dut-serial', default='COM4', help='Serial port (default: COM4)')
    parser.add_argument('--baud', type=int, default=115200, help='Baud rate (default: 115200)')
    parser.add_argument('--tver', default='v1.0.0', help='Expected firmware version')
    parser.add_argument('--jfile', default='day6-hil-abstraction-results.xml', help='JUnit output file')
    parser.add_argument('--verbose', action='store_true', help='Enable debug logging')
    args = parser.parse_args()

    if args.verbose:
        _log.setLevel(logging.DEBUG)

    print('Badweh HIL (Simplified)')
    print(f'  Serial : {args.dut_serial}')
    print(f'  Version: {args.tver}')
    print(f'  JUnit  : {args.jfile}\n')

    try:
        testing_init(args.dut_serial, args.baud) # kill stale plink.exe instances, then connect_console()
        g_dut.flush_input() # drain any buffered console output before starting a command
        all_passed = run_tests(args.tver) # return True or False, test_version(expected_version), test_i2c_smoke()
        suite = jux.TestSuite('Day6 Simplified HIL', g_testcases)
        with open(args.jfile, 'w') as jfile:
            jux.TestSuite.to_file(jfile, [suite], prettyprint=True)
        sys.exit(0 if all_passed else 1)
    except KeyboardInterrupt:
        print('\nInterrupted by user')
        sys.exit(1)
    finally:
        if g_dut is not None:
            g_dut.terminate()


if __name__ == '__main__':
    main()