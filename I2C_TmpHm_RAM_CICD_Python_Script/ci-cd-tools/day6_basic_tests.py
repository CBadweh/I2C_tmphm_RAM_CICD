import sys
import signal
import time
import pexpect
import pexpect.popen_spawn as popen_spawn

SERIAL_PORT = 'COM4'        # Change to your COM port
BAUD_RATE = 115200          # Match your board settings
EXPECTED_VERSION = 'v1.0.0' # Firmware version to verify


def main():
    print("=" * 60)
    print("  Ultra-Simplified HIL Test: Version Command Only")
    print("=" * 60)
    print(f"Serial Port: {SERIAL_PORT}")
    print(f"Expected Version: {EXPECTED_VERSION}")
    print("=" * 60)

    console = None
    try:
        # Open the serial connection through plink
        print(f"Connecting to {SERIAL_PORT} at {BAUD_RATE} baud...")
        console = popen_spawn.PopenSpawn(f'plink -serial {SERIAL_PORT} -sercfg {BAUD_RATE}')
        console.timeout = 3      # Seconds to wait for responses
        print("Connected!\n")

        # Send the version command exactly once
        print(f'Sending command: "main version"')
        console.sendline('main version')

        # Wait for Version="<expected>" in the console output
        version_pattern = fr'Version="{EXPECTED_VERSION}"'
        print(f'Waiting for pattern: "{version_pattern}"')
        result = console.expect([version_pattern, pexpect.TIMEOUT, pexpect.EOF])

        if result == 0:
            print("  -> Version pattern found.")
            # Version string matched; now wait for the prompt ">"
            prompt_result = console.expect([r'>', pexpect.TIMEOUT, pexpect.EOF])
            if prompt_result == 0:
                print('  -> Prompt found!')
                print('\nRunning inline I2C read smoke test (reserve/write/read/release)')

                # Reserve bus
                reserve_ok = False
                for attempt in range(6):
                    console.sendline('i2c test reserve 0')
                    rc = console.expect([r'Return code (-?\d+)', pexpect.TIMEOUT, pexpect.EOF])
                    if rc == 0:
                        code = int(console.match.group(1))
                        console.expect([r'>', pexpect.TIMEOUT, pexpect.EOF])
                        if code == 0:
                            reserve_ok = True
                            break
                        if code == -10 and attempt < 5:
                            time.sleep(0.1)
                            continue
                    break

                if not reserve_ok:
                    print('  -> Reserve failed')
                    test_passed = False
                else:
                    # Write command
                    write_ok = False
                    for attempt in range(3):
                        console.sendline('i2c test write 0 0x44 0x2c 0x06')
                        rc = console.expect([r'Return code (-?\d+)', pexpect.TIMEOUT, pexpect.EOF])
                        if rc == 0:
                            code = int(console.match.group(1))
                            console.expect([r'>', pexpect.TIMEOUT, pexpect.EOF])
                            if code == 0:
                                write_ok = True
                                break
                            if code == -10 and attempt < 2:
                                time.sleep(0.05)
                                continue
                        break

                    if not write_ok:
                        print('  -> Write failed')
                        test_passed = False
                    else:
                        time.sleep(0.05)

                        # Read command
                        read_ok = False
                        for attempt in range(3):
                            console.sendline('i2c test read 0 0x44 6')
                            rc = console.expect([r'Return code (-?\d+)', pexpect.TIMEOUT, pexpect.EOF])
                            if rc == 0:
                                code = int(console.match.group(1))
                                console.expect([r'>', pexpect.TIMEOUT, pexpect.EOF])
                                if code == 0:
                                    read_ok = True
                                    break
                                if code == -10 and attempt < 2:
                                    time.sleep(0.05)
                                    continue
                            break

                        if not read_ok:
                            print('  -> Read failed')
                            test_passed = False
                        else:
                            # Release command
                            release_ok = False
                            for attempt in range(3):
                                console.sendline('i2c test release 0')
                                rc = console.expect([r'Return code (-?\d+)', pexpect.TIMEOUT, pexpect.EOF])
                                if rc == 0:
                                    code = int(console.match.group(1))
                                    console.expect([r'>', pexpect.TIMEOUT, pexpect.EOF])
                                    if code == 0:
                                        release_ok = True
                                        break
                                    if code == -10 and attempt < 2:
                                        time.sleep(0.05)
                                        continue
                                break

                            if not release_ok:
                                print('  -> Release failed')
                                test_passed = False
                            else:
                                print('  -> I2C read sequence passed')
                                test_passed = True
            else:
                print("  -> Prompt missing or connection closed.")
                test_passed = False
        else:
            # Either timed out or console closed unexpectedly
            print("  -> Version pattern not detected.")
            test_passed = False

        # Exit with success (0) or failure (1) for scripts/CI to read
        print("=" * 60)
        if test_passed:
            print("  OVERALL RESULT: PASS")
            sys.exit(0)
        else:
            print("  OVERALL RESULT: FAIL")
            sys.exit(1)

    except KeyboardInterrupt:
        # Allow Ctrl+C to exit cleanly with non-zero status
        sys.exit(1)

    except Exception as error:
        # Any other unexpected error -> fail
        sys.exit(1)

    finally:
        # Always terminate the plink process if it was created
        if console:
            try:
                console.kill(signal.SIGTERM)
            except Exception:
                pass


if __name__ == '__main__':
    main()