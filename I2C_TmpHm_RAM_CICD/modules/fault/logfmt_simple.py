#!/usr/bin/env python3
"""
Simplified fault data parser for fault_simple.c

Usage:
    python logfmt_simple.py raw.txt

Where raw.txt is the hex dump exported from STM32CubeIDE Memory Browser
starting at address 0x08004000
"""

import sys
import struct

def parse_hex_dump(filename):
    """Parse hex dump file (space-separated hex bytes) into binary data"""
    data = bytearray()

    with open(filename, 'r') as f:
        for line in f:
            # Remove line numbers and split by spaces
            hex_bytes = line.strip().split()
            for hex_byte in hex_bytes:
                # Skip line numbers (they have '→' or are numeric with →)
                if '→' in hex_byte or len(hex_byte) != 2:
                    continue
                try:
                    data.append(int(hex_byte, 16))
                except ValueError:
                    continue

    return bytes(data)

def decode_fault_data(data):
    """Decode fault_data structure (80 bytes)"""

    if len(data) < 88:  # Structure is actually 88 bytes due to padding
        print(f"ERROR: Not enough data ({len(data)} bytes, expected at least 88)")
        return

    # Unpack fault data structure (22 fields × 4 bytes = 88 bytes)
    # Note: Your structure has 88 bytes instead of 80 due to compiler padding
    fault_data = struct.unpack('<22I', data[0:88])

    print("=" * 80)
    print("Fault data")
    print("=" * 80)

    field_names = [
        ("magic", 0),
        ("num_section_bytes", 1),
        ("fault_type", 2),
        ("fault_param", 3),
        ("excpt_stk_r0", 4),
        ("excpt_stk_r1", 5),
        ("excpt_stk_r2", 6),
        ("excpt_stk_r3", 7),
        ("excpt_stk_r12", 8),
        ("excpt_stk_lr", 9),
        ("excpt_stk_rtn_addr", 10),
        ("excpt_stk_xpsr", 11),
        ("sp", 12),
        ("lr", 13),
        ("ipsr", 14),
        ("icsr", 15),
        ("shcsr", 16),
        ("cfsr", 17),
        ("hfsr", 18),
        ("mmfar", 19),
        ("bfar", 20),
        ("tick_ms", 21),
    ]

    for name, idx in field_names:
        value = fault_data[idx]
        print(f"{name:>18s}: 0x{value:08x} ({value})")

    # Decode fault type
    print("\n" + "=" * 80)
    print("Fault Analysis")
    print("=" * 80)

    fault_type = fault_data[2]
    fault_param = fault_data[3]

    fault_type_names = {
        1: "EXCEPTION",
        2: "WATCHDOG",
        3: "ASSERT"
    }
    print(f"\nFault Type: {fault_type_names.get(fault_type, 'UNKNOWN')} ({fault_type})")

    # Decode exception number
    exception_names = {
        3: "HardFault",
        4: "MemManage",
        5: "BusFault",
        6: "UsageFault"
    }
    if fault_type == 1:  # EXCEPTION
        print(f"Exception: {exception_names.get(fault_param, 'Unknown')} (#{fault_param})")

    # Decode PC (where fault occurred)
    pc = fault_data[10]
    print(f"\nFault occurred at PC: 0x{pc:08x}")
    print(f"  This is the address of the instruction that caused the fault")

    # Decode LR (EXC_RETURN)
    lr = fault_data[13]
    exc_return_meanings = {
        0xFFFFFFF1: "Return to Handler mode, use MSP",
        0xFFFFFFF9: "Return to Thread mode, use MSP",
        0xFFFFFFFD: "Return to Thread mode, use PSP"
    }
    if lr in exc_return_meanings:
        print(f"\nEXC_RETURN (LR): {exc_return_meanings[lr]}")

    # Decode CFSR (Configurable Fault Status Register)
    cfsr = fault_data[17]
    mmfsr = cfsr & 0xFF
    bfsr = (cfsr >> 8) & 0xFF
    ufsr = (cfsr >> 16) & 0xFFFF

    print(f"\nCFSR Breakdown:")
    print(f"  MMFSR (MemManage): 0x{mmfsr:02x}")
    if mmfsr & 0x01: print(f"    - IACCVIOL: Instruction access violation")
    if mmfsr & 0x02: print(f"    - DACCVIOL: Data access violation")
    if mmfsr & 0x08: print(f"    - MUNSTKERR: MemManage fault on unstacking")
    if mmfsr & 0x10: print(f"    - MSTKERR: MemManage fault on stacking")
    if mmfsr & 0x80: print(f"    - MMARVALID: MMFAR valid")

    print(f"  BFSR (BusFault): 0x{bfsr:02x}")
    if bfsr & 0x01: print(f"    - IBUSERR: Instruction bus error")
    if bfsr & 0x02: print(f"    - PRECISERR: Precise data bus error")
    if bfsr & 0x04: print(f"    - IMPRECISERR: Imprecise data bus error")
    if bfsr & 0x08: print(f"    - UNSTKERR: Bus fault on unstacking")
    if bfsr & 0x10: print(f"    - STKERR: Bus fault on stacking")
    if bfsr & 0x80: print(f"    - BFARVALID: BFAR valid")

    print(f"  UFSR (UsageFault): 0x{ufsr:04x}")
    if ufsr & 0x0001: print(f"    - UNDEFINSTR: Undefined instruction")
    if ufsr & 0x0002: print(f"    - INVSTATE: Invalid state")
    if ufsr & 0x0004: print(f"    - INVPC: Invalid PC")
    if ufsr & 0x0008: print(f"    - NOCP: No coprocessor")
    if ufsr & 0x0100: print(f"    - UNALIGNED: Unaligned access")
    if ufsr & 0x0200: print(f"    - DIVBYZERO: Divide by zero")

    # Decode HFSR (HardFault Status Register)
    hfsr = fault_data[18]
    print(f"\nHFSR (HardFault Status): 0x{hfsr:08x}")
    if hfsr & 0x40000000:
        print(f"  - FORCED: Hard fault escalated from configurable fault")
    if hfsr & 0x80000000:
        print(f"  - DEBUGEVT: Debug event")

    # Decode fault addresses
    mmfar = fault_data[19]
    bfar = fault_data[20]

    if mmfsr & 0x80:  # MMARVALID
        print(f"\nMemManage Fault Address (MMFAR): 0x{mmfar:08x}")

    if bfsr & 0x80:  # BFARVALID
        print(f"\nBus Fault Address (BFAR): 0x{bfar:08x}")
        if bfar == 0xffffffff:
            print(f"  [OK] This is our test address from fault_simple.c!")

    # Check for test values in registers
    r2 = fault_data[6]
    r3 = fault_data[7]

    print(f"\nRegister Analysis:")
    if r2 == 0xbad:
        print(f"  [OK] R2 contains 0xBAD (our test value)")
    if r3 == 0xffffffff:
        print(f"  [OK] R3 contains 0xFFFFFFFF (our bad address)")

    # Timestamp
    tick_ms = fault_data[21]
    print(f"\nFault occurred at: {tick_ms} ms after boot")

def decode_lwl_data(data):
    """Decode LWL buffer data"""

    # LWL data starts after fault data (88 bytes due to padding)
    lwl_offset = 88

    if len(data) < lwl_offset + 16:
        print("\nNo LWL data found")
        return

    # Check for LWL magic
    lwl_magic = struct.unpack('<I', data[lwl_offset:lwl_offset+4])[0]

    if lwl_magic != 0xf00d0001:
        print(f"\nNo LWL data found (magic = 0x{lwl_magic:08x})")
        return

    lwl_size = struct.unpack('<I', data[lwl_offset+4:lwl_offset+8])[0]
    lwl_buf_size = struct.unpack('<I', data[lwl_offset+8:lwl_offset+12])[0]
    lwl_put_idx = struct.unpack('<I', data[lwl_offset+12:lwl_offset+16])[0]

    print("\n" + "=" * 80)
    print("LWL (Last Words Log)")
    print("=" * 80)
    print(f"\nLWL Magic: 0x{lwl_magic:08x} [OK]")
    print(f"Section Size: {lwl_size} bytes")
    print(f"Buffer Size: {lwl_buf_size} bytes")
    print(f"Put Index: {lwl_put_idx}")

    # Extract buffer contents
    lwl_buf_offset = lwl_offset + 16
    if len(data) >= lwl_buf_offset + lwl_buf_size:
        lwl_buffer = data[lwl_buf_offset:lwl_buf_offset + lwl_buf_size]

        # Show first non-zero entries
        print(f"\nBuffer Contents (first 32 bytes):")
        for i in range(min(32, lwl_buf_size)):
            if i % 16 == 0:
                print(f"\n  {i:04x}: ", end="")
            print(f"{lwl_buffer[i]:02x} ", end="")
        print()

        # Find non-zero entries
        non_zero = [i for i in range(min(lwl_put_idx, lwl_buf_size)) if lwl_buffer[i] != 0]
        if non_zero:
            print(f"\nNon-zero entries found at indices: {non_zero}")
            print(f"Values: {[f'0x{lwl_buffer[i]:02x}' for i in non_zero]}")

            # Decode known test values
            if lwl_buffer[0] == 0x10 and lwl_buffer[1] == 0x20 and lwl_buffer[2] == 0x30:
                print("\n[OK] Found test LWL entries: 0x10, 0x20, 0x30")
                print("  These were recorded by lwl_record() in main()")

def main():
    if len(sys.argv) < 2:
        print("Usage: python logfmt_simple.py raw.txt")
        print("\nWhere raw.txt is the hex dump from STM32CubeIDE Memory Browser")
        print("Export memory from address 0x08004000 (at least 1120 bytes)")
        sys.exit(1)

    filename = sys.argv[1]

    print(f"INFO: Parsing fault data from {filename}")

    # Parse hex dump
    data = parse_hex_dump(filename)
    print(f"INFO: Parsed {len(data)} bytes from hex dump\n")

    # Decode fault data
    decode_fault_data(data)

    # Decode LWL data
    decode_lwl_data(data)

    print("\n" + "=" * 80)

if __name__ == '__main__':
    main()
