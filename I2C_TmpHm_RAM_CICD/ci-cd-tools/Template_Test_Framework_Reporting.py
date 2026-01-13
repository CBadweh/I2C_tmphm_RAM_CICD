"""
Template 2: Test Framework & Reporting
Merges: start_test/test_pass/test_fail + JUnit XML output

Use this when: Need structured, CI/CD-compatible test reporting
"""

import junit_xml as jux

# Global test cases list for JUnit XML output
test_cases = []

# Global variables for test framework
g_test_name = None
g_test_junit = None
g_console = None


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


def test_pass():
    """Handle a test pass, recording info."""
    global g_test_name
    
    print(f'Test "{g_test_name}" passes')
    # Test passes if no failure info is added to g_test_junit


def test_fail(failure_info):
    """Handle a test failure, recording info.
    
    Args:
        failure_info: String describing how it failed
    """
    global g_test_name, g_test_junit
    
    print(f'Test "{g_test_name}" fails: {failure_info}')
    if g_test_junit:
        g_test_junit.add_failure_info(failure_info)


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


# ==================
# USAGE EXAMPLE
# ==================
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


if __name__ == '__main__':
    # Example: Assuming console is already connected
    # console = connect_serial('COM4', 115200)
    
    # Run tests
    # version_passed = test_version(console, 'v1.0.0')
    # help_passed = test_help(console)
    
    # Write JUnit XML at the end
    write_junit_xml('test-results.xml')
