"""
Simplified LWL Formatter - Core Logic Only, no fault data parsing
Demonstrates the two-phase process:
  Phase 1: Parse source code to extract LWL metadata
  Phase 2: Decode raw hex data using metadata
"""

import argparse
import os
import re

################################################################################
# PHASE 1: SOURCE CODE PARSING - Extract metadata from .c files
################################################################################

# ============================================================================
# PHASE 1: SOURCE CODE PARSING
# ============================================================================

# Step 1.5: LwlMsg() - Metadata Object
class LwlMsg:
    """Metadata for a single LWL statement"""
    def __init__(self, id, fmt, arg_bytes, file_path, line_num, lwl_statement):
        # Example: id=3, fmt="test 3 %d %d", arg_bytes="12", file_path="lwl.c", line_num=259
        self.id = id                        # LWL message ID (e.g., 3)
        self.fmt = fmt                      # Format string (e.g., "test 3 %d %d")
        self.arg_bytes = arg_bytes          # String like "12" for LWL_1() and LWL_2()
                                            # Each digit represents byte length of argument
        self.file_path = file_path          # Source file path (e.g., "lwl.c")
        self.line_num = line_num            # Line number in source file (e.g., 259)
        self.lwl_statement = lwl_statement  # Full LWL(...) statement text
        # Calculate total argument bytes: int('1') + int('2') = 3
        self.num_arg_bytes = sum(int(d) for d in self.arg_bytes) if arg_bytes else 0

class LwlMsgSet:
    """Collection of all LWL messages (metadata database)"""
    def __init__(self):
        self.lwl_msgs = {}  # Dictionary: id -> LwlMsg
        self.max_msg_len = 0

    # Step 1.4: LwlMsgSet.add_lwl_msg() - Store Metadata
    def add_lwl_msg(self, id, fmt, arg_bytes, lwl_statement, file_path, line_num):
        # Check for duplicate IDs (same ID used in multiple LWL statements)
        if id in self.lwl_msgs:
            print(f'ERROR: Duplicate LWL ID {id} at {file_path}:{line_num}')
            return False

        # Create LwlMsg object and store it in dictionary
        # Example: Creates LwlMsg(id=3, fmt="test 3 %d %d", arg_bytes="12", ...)
        self.lwl_msgs[id] = LwlMsg(id, fmt, arg_bytes, file_path, line_num, lwl_statement)

        # Track max message length for optimization (used in Phase 2 synchronization)
        # msg_len = 1 (ID byte) + sum of arg_bytes (e.g., 1 + 1 + 2 = 4 bytes total)
        msg_len = 1 + sum(int(d) for d in arg_bytes) if arg_bytes else 1
        if msg_len > self.max_msg_len:
            self.max_msg_len = msg_len
        return True

    def get_metadata(self, id):
        return self.lwl_msgs.get(id, None)

class SourceParser:
    """Scans source code to find LWL statements"""

    # Step 1.2: SourceParser.parse_source_dir()
    def parse_source_dir(self, dir_path, lwl_msg_set):
        # Search for lwl.c in directory tree and stop after finding it
        for root_dir_path, dir_names, file_names in os.walk(dir_path):
            if 'lwl.c' in file_names:
                file_path = os.path.join(root_dir_path, 'lwl.c')
                self.parse_source_file(file_path, lwl_msg_set)
                return  # Exit early since we only need lwl.c

    # Step 1.3: SourceParser.parse_source_file() - THE HEART OF PHASE 1
    # This is where your LWL("test 3 %d %d", 3, LWL_1(10), LWL_2(1000)); gets parsed
    def parse_source_file(self, file_path, lwl_msg_set):
        # Setup regex patterns for parsing different parts of LWL statements
        lwl_base_id_pat = re.compile(r'^\s*#define\s+LWL_BASE_ID\s+(\d+)')  # Find: #define LWL_BASE_ID 1
        detect_lwl_pat = re.compile(r'^\s*LWL\(')                           # Detect LWL statement start
        parse_fmt_pat = re.compile(r'\s*LWL\("([^"]*)"')                    # Extract format string
        parse_num_arg_bytes_pat = re.compile(r'\s*,\s*([0-9]+)')            # Extract num_arg_bytes
        parse_arg_pat = re.compile(r',\s*LWL_([\d])\(')                     # Extract arg lengths from LWL_1(), LWL_2(), etc.

        lwl_base_id = None      # Will store the base ID (e.g., 1)
        lwl_id_offset = -1      # Offset counter for calculating message IDs
        lwl_statement = None    # Accumulates multi-line LWL statements

        with open(file_path) as f:
            line_num = 0
            for line in f:
                line_num += 1
                line = line.strip()
                if not line:
                    continue

                # ═══ STEP 1: Find: #define LWL_BASE_ID 1 ═══
                m = re.match(lwl_base_id_pat, line)
                if m:
                    lwl_base_id = int(m.group(1))  # ← lwl_base_id = 1
                    lwl_line_num = line_num
                    continue

                # ═══ STEP 2: Detect LWL statement start ═══
                # Handle multi-line LWL statements
                if lwl_statement:
                    lwl_statement += line  # Continue accumulating multi-line statement
                else:
                    if re.match(detect_lwl_pat, line):
                        lwl_statement = line  # ← "LWL("test 3 %d %d", 3, LWL_1(10), LWL_2(1000));"

                # Check if statement is complete (ends with ');')
                if (not lwl_statement) or (lwl_statement[-2:] != ');'):
                    continue

                # ═══ STEP 3: Parse complete statement ═══
                # Increment offset for this LWL statement (0-indexed, so first is offset 0)
                lwl_id_offset += 1  # ← offset = 2 (third statement, 0-indexed)

                # Extract format string: "test 3 %d %d"
                m = re.match(parse_fmt_pat, lwl_statement)
                if not m:
                    print(f'ERROR: Cannot parse LWL fmt at {file_path}:{line_num}')
                    lwl_statement = None
                    continue
                lwl_fmt = m.group(1)  # ← "test 3 %d %d"
                lwl_remain = lwl_statement[m.end():]  # Remaining part after format string

                # Extract num_arg_bytes: 3
                m = re.match(parse_num_arg_bytes_pat, lwl_remain)
                if not m:
                    print(f'ERROR: Cannot parse num_arg_bytes at {file_path}:{line_num}')
                    lwl_statement = None
                    continue
                lwl_num_arg_bytes = int(m.group(1))  # ← 3
                lwl_remain = lwl_remain[m.end():]  # Remaining part after num_arg_bytes

                # Extract arg lengths from LWL_1(), LWL_2(), etc.
                # Example: LWL_1(10) → "1", LWL_2(1000) → "2", result: "12"
                lwl_arg_lengths = ''
                while True:
                    m = re.search(parse_arg_pat, lwl_remain)
                    if m:
                        lwl_arg_lengths += m.group(1)  # ← "1" then "2" → "12"
                        lwl_remain = lwl_remain[m.end():]
                    else:
                        break

                # Verify consistency: sum of arg lengths should equal num_arg_bytes
                if sum(int(d) for d in lwl_arg_lengths) != lwl_num_arg_bytes:
                    print(f'ERROR: Inconsistent num_arg_bytes at {file_path}:{line_num}')

                # ═══ STEP 4: Create and store metadata ═══
                if lwl_base_id is not None:
                    lwl_msg_set.add_lwl_msg(
                        lwl_base_id + lwl_id_offset,  # ← ID = 1 + 2 = 3
                        lwl_fmt,                       # ← "test 3 %d %d"
                        lwl_arg_lengths,               # ← "12"
                        lwl_statement,                 # ← Full statement text
                        file_path,                     # ← lwl.c
                        lwl_line_num                   # ← 259
                    )

                lwl_statement = None  # Reset for next statement

################################################################################
# PHASE 2: RAW DATA DECODING - Read and decode hex dump
################################################################################

class Data:
    """Container for raw data from hex dump file"""

    MOD_MAGIC_LWL = 0xf00d0001

    def __init__(self):
        self.data_array = bytearray()
        self.data_len = 0

    # Step 2.2: Data.read_data_file() - Load Raw Hex
    def read_data_file(self, file_path):
        """Read hex file format: 'offset: hexdata'
        Example: Parse lines like "0000: 030a03e8" and convert to bytes
        "03 0a 03e8" → [0x03, 0x0a, 0x03, 0xe8]
        """
        line_pat = re.compile(r'([0-9a-fA-F]+):\s*([0-9a-fA-F]+)')  # Pattern: "offset: hexdata"

        with open(file_path) as f:
            for line in f:
                m = re.search(line_pat, line)
                if m:
                    data_hex = m.group(2)  # Extract hex data part (ignore offset)
                    # Convert hex string to bytes and append to data_array
                    self.data_array.extend(bytearray.fromhex(data_hex))
                    # Example: "030a03e8" → [0x03, 0x0a, 0x03, 0xe8]

        self.data_len = len(self.data_array)
        return True

    # Step 2.4: Data.get_data() - Extract Linear Data
    def get_data(self, idx, num_bytes):
        """Extract multi-byte value (little-endian)
        Example: [0x03, 0x0a] at idx with num_bytes=2
        → value = 0x0a03 (little-endian: least significant byte first)
        """
        value = 0
        shift = 0
        for i in range(num_bytes):
            value = value + (self.data_array[idx + i] << shift)  # Little-endian: shift increases
            shift += 8
        return value

    # Step 2.6: Data.get_data_circ() - Extract from Circular Buffer
    def get_data_circ(self, idx, num_bytes, buf_start_idx, buf_len, put_idx, first_get):
        """Extract value from circular buffer (big-endian)
        Example for "test 3" arg 2 (2 bytes):
          Loop 1: value = 0x03
          Loop 2: value = (0x03 << 8) + 0xe8 = 0x03e8 = 1000
        """
        # Check if we've reached the end (wrapped to put_idx)
        if idx == put_idx and not first_get:
            raise EOFError

        value = 0
        for i in range(num_bytes):
            # Check again after each byte
            if idx == put_idx and not first_get:
                raise EOFError
            # Read byte (big-endian: most significant byte first)
            value = (value << 8) + self.data_array[buf_start_idx + idx]
            # Advance index with wraparound
            idx = (idx + 1) % buf_len  # Wrap to beginning when idx >= buf_len

        return value, idx

class LwlPrinter:
    """Decodes and prints LWL messages from raw data"""

    def __init__(self, data, lwl_msg_set):
        self.data = data
        self.lwl_msg_set = lwl_msg_set

    # Step 2.5: LwlPrinter.pretty_print() - THE HEART OF PHASE 2
    def pretty_print(self, section_offset, section_len):
        """Main decoding loop - decodes LWL messages from circular buffer"""
        print('=' * 80)
        print('LWL Messages')
        print('=' * 80)

        # Parse LWL section header:
        # Offset 0:  magic (4 bytes) - already checked (0xf00d0001)
        # Offset 4:  num_section_bytes (4 bytes) - total section size
        # Offset 8:  buf_size (4 bytes) - circular buffer size
        # Offset 12: put_idx (4 bytes) - current write position in buffer
        # Offset 16: circular buffer starts - actual message data begins here

        buf_len = self.data.get_data(section_offset + 8, 4)      # ← 1008 (example)
        put_idx = self.data.get_data(section_offset + 12, 4)     # ← Current position
        buf_start_idx = section_offset + 16                       # ← Buffer start

        print(f'Buffer size: {buf_len}, Put index: {put_idx}')
        print('=' * 80)

        # Find optimal starting point (handles partial overwrites in circular buffer)
        idx = self.get_optimal_start_idx(buf_start_idx, buf_len, put_idx)
        first_get = True
        skipped_data = []  # Track invalid IDs we skip

        try:
            while True:
                # ═══ STEP A: Get Message ID ═══
                while True:
                    id_idx = idx
                    id, idx = self.data.get_data_circ(
                        idx, 1, buf_start_idx, buf_len, put_idx, first_get)
                    # ▲ Reads: 0x03 (your "test 3" ID)
                    first_get = False

                    # ═══ STEP B: Lookup Metadata ═══
                    msg_meta = self.lwl_msg_set.get_metadata(id)
                    # ▲ Returns: LwlMsg(id=3, fmt="test 3 %d %d", arg_bytes="12", ...)
                    if msg_meta is None:
                        # Invalid ID, skip byte and try again
                        skipped_data.append(id)
                        continue
                    break  # Found valid metadata, proceed to extract arguments

                # Print any skipped invalid IDs
                if skipped_data:
                    print(f'Skipped: {" ".join([f"{x:02x}" for x in skipped_data])}')
                    skipped_data = []

                # ═══ STEP C: Extract Arguments ═══
                arg_values = []
                for arg_bytes in msg_meta.arg_bytes:  # ← "12" → ['1', '2']
                    arg_bytes = int(arg_bytes)         # ← 1, then 2
                    arg_value, idx = self.data.get_data_circ(
                        idx, arg_bytes, buf_start_idx, buf_len,
                        put_idx, first_get)
                    # First loop:  reads 1 byte  → 0x0a (10)
                    # Second loop: reads 2 bytes → 0x03e8 (1000, big-endian)
                    arg_values.append(arg_value)
                # arg_values = [10, 1000]

                # ═══ STEP D: Format and Print ═══
                # Format string with arguments: "test 3 %d %d" % (10, 1000) → "test 3 10 1000"
                print(msg_meta.fmt % tuple(arg_values))

        except EOFError:
            pass  # End of buffer reached

    # Step 2.7: LwlPrinter.get_optimal_start_idx() - Synchronization
    def get_optimal_start_idx(self, buf_start_idx, buf_len, put_idx):
        """Find the best starting point in circular buffer.
        
        Problem: put_idx might point into middle of a message
                 (if circular buffer wrapped and overwrote partial message)
        
        Solution: Try starting at put_idx, put_idx+1, put_idx+2, ...
                  up to put_idx + max_msg_len, and see which offset
                  produces the fewest invalid IDs.
        """
        optimal_invalid_ids = buf_len  # Start with worst case
        optimal_start_idx = put_idx

        # Try different starting offsets (0 to max_msg_len)
        for offset in range(self.lwl_msg_set.max_msg_len + 1):
            start_idx = (put_idx + offset) % buf_len
            idx = start_idx
            first_get = (offset == 0)
            invalid_id_ctr = 0

            try:
                # Count invalid IDs when starting from this offset
                while True:
                    id, idx = self.data.get_data_circ(
                        idx, 1, buf_start_idx, buf_len, put_idx, first_get)
                    first_get = False

                    msg_meta = self.lwl_msg_set.get_metadata(id)
                    if msg_meta is None:
                        invalid_id_ctr += 1  # Bad ID
                    else:
                        # Valid ID - skip its argument bytes
                        for arg_bytes in msg_meta.arg_bytes:
                            idx = (idx + int(arg_bytes)) % buf_len
            except EOFError:
                pass  # Reached end of buffer

            # Keep track of best offset (fewest invalid IDs)
            if invalid_id_ctr < optimal_invalid_ids:
                optimal_invalid_ids = invalid_id_ctr
                optimal_start_idx = start_idx

        return optimal_start_idx

    # Step 2.3: LwlPrinter.process_data() - Find LWL Section
    def process_data(self):
        """Find and decode LWL section
        Scans raw data for magic number (0xf00d0001) and dispatches to decoder
        """
        idx = 0

        while idx < self.data.data_len:
            if idx + 8 > self.data.data_len:
                break

            # Read magic number (4 bytes) to determine section type
            magic = self.data.get_data(idx, 4)
            # Read section length (4 bytes) to know how much data to process
            section_len = self.data.get_data(idx + 4, 4)

            # If this is an LWL section, decode it
            if magic == Data.MOD_MAGIC_LWL:  # 0xf00d0001
                self.pretty_print(idx, section_len)  # ← Decode LWL data

            # Move to next section
            idx += section_len

################################################################################
# MAIN PROGRAM
################################################################################

def main():
    parser = argparse.ArgumentParser(description='Simplified LWL formatter')
    parser.add_argument('-f', help='Raw data file (hex dump)')
    parser.add_argument('-d', action='append', nargs='+',
                        help='Source code directory (can specify multiple)')
    args = parser.parse_args()

    # ═══════════════════════════════════════════════════════════════
    # PHASE 1: Parse source code to build metadata database
    # ═══════════════════════════════════════════════════════════════
    # Step 1.1: main() starts parsing
    # Entry point for Phase 1: iterate through directories and parse source files
    print('=' * 80)
    print('PHASE 1: Parsing source code for LWL statements')
    print('=' * 80)

    lwl_msg_set = LwlMsgSet()      # Create metadata database
    source_parser = SourceParser() # Create parser instance

    if not args.d:
        print('ERROR: No source directories specified')
        return

    # Process each directory specified via -d argument
    for dir_list in args.d:
        for dir_path in dir_list:
            print(f'Scanning directory: {dir_path}')
            # This calls parse_source_dir() which processes each .c file
            source_parser.parse_source_dir(dir_path, lwl_msg_set)  # ← Entry point for Phase 1

    print(f'Found {len(lwl_msg_set.lwl_msgs)} LWL statements')
    print()

    # Show parsed metadata
    print('Metadata extracted:')
    for id in sorted(lwl_msg_set.lwl_msgs.keys()):
        msg = lwl_msg_set.lwl_msgs[id]
        print(f'  ID {id}: fmt="{msg.fmt}", arg_bytes="{msg.arg_bytes}", '
              f'file={msg.file_path}:{msg.line_num}')
    print()

    # ═══════════════════════════════════════════════════════════════
    # PHASE 2: Decode raw data using metadata
    # ═══════════════════════════════════════════════════════════════
    # Step 2.1: main() reads raw data
    # After Phase 1 completes, read raw hex data file and decode using metadata
    print('=' * 80)
    print('PHASE 2: Decoding raw data')
    print('=' * 80)

    if args.f is None:
        print('No data file provided (-f option)')
        return

    data = Data()
    if not data.read_data_file(args.f):  # ← Read raw1.txt
        print(f'ERROR: Failed to read {args.f}')
        return

    print(f'Loaded {data.data_len} bytes from {args.f}')
    print()

    lwl_printer = LwlPrinter(data, lwl_msg_set)
    lwl_printer.process_data()  # ← Process it (finds LWL section and decodes)

if __name__ == '__main__':
    main()
