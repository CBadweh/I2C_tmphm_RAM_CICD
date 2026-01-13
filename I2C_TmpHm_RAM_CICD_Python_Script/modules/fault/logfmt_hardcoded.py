"""
This program interprets and formats the output of the lwl and fault
modules. Note that the fault module outputs the lwl buffer as well as other
fault information.

For the fault module, that output is (hopefully) generated when the system
crashes, just before it restarts.

If there is a console connected at the time of the crash, you can possibly cut
the fault data (text) from there. Otherwise, if the fault data was successfully
written to flash, you can use the "fault data" command to print it out. In
either case, put the data in a file, and pass it to this program.

For the lwl module, this program can interpret and format the output of the "lwl
dump" command.

This program does the following:
- Searches through your source code to get metadata about the LWL statements
  and other fault data.
- Reads in the lwl or fault data (hexadecimal text).
- Outputs the data in an easy-to-read format.

This program is picky about the module data data format. The idea is if there is
corruption, you should fix it manually, as best you can do, and then run this
program again.
"""

import os
import re

g_swab = False

################################################################################

class Data:
    """
    This class represents the fault data being processed.
    """

    # Fault data section tokens, taken from module.h. These are mapped to
    # to section IDs.

    MOD_MAGIC_FAULT = 0xdead0001
    MOD_MAGIC_LWL = 0xf00d0001
    MOD_MAGIC_TRAILER = 0xc0da0001                  

    SECTION_TYPE_FAULT = 0
    SECTION_TYPE_LWL = 1
    SECTION_TYPE_TRAILER = 2

    magic_to_fault_type = {
        MOD_MAGIC_FAULT : SECTION_TYPE_FAULT,
        MOD_MAGIC_LWL : SECTION_TYPE_LWL,
        MOD_MAGIC_TRAILER : SECTION_TYPE_TRAILER,
        }

    def __init__(self):
        self.data_array = bytearray()
        self.data_len = 0

    # ========================================================================
    # PHASE 2.2: read_data_file
    # ========================================================================
    # Reads hex dump file (format: "offset: hexdata") and converts it
    # into a bytearray for processing.
    def read_data_file(self, file_path):
        """
        Read file containing fault data.

        Parameters:
            file_path : The file to read.

        Return:
            True if successful, otherwise False.

        The file lines have this format, where all data is hexadecimal text:
            <offset>: <raw-data>
        """

        line_pat = re.compile(r'([0-9a-fA-F]+):\s*([0-9a-fA-F]+)')
        expected_offset = 0
        with open(file_path, encoding='utf-8', errors='ignore') as f:
            for line in f:
                line = line.strip()

                m = re.search(line_pat, line)
                if m:
                    offset_hex = m.group(1)
                    data_hex = m.group(2)

                    if (len(offset_hex) % 2) == 0 and (len(data_hex) % 2) == 0:
                        offset = int(offset_hex, 16)
                        if offset == expected_offset:
                            self.data_array.extend(bytearray.fromhex(data_hex))
                            expected_offset += len(data_hex)//2

        self.data_len = len(self.data_array)

    # ========================================================================
    # PHASE 2.3: process_data - Find LWL section
    # ========================================================================
    # Processes the data array by identifying sections via magic numbers.
    # Routes to appropriate printer based on section type (FAULT, LWL, TRAILER).
    def process_data(self):
        """
        Process fault data - HARDCODED VERSION for exactly 3 sections.
        
        Assumes structure: FAULT → LWL → TRAILER
        """
        
        idx = 0
        
        # ========================================================================
        # SECTION 1: FAULT
        # ========================================================================
        magic = self.get_data(idx, 4)               # 3735879681: 0xDEAD0001: MOD_MAGIC_FAULT
        if magic not in self.magic_to_fault_type:
            g_swab = not g_swab
            magic = self.get_data(idx, 4)
        
        section_len = self.get_data(idx + 4, 4)     # 88 bytes
        if idx + section_len <= self.data_len:
            # Inline implementation of g_fault_data.pretty_print(idx, section_len)
            section_offset = idx  # parameter name from original method
            
            print('=' * 80)
            print('Fault data')
            print('=' * 80)
            for field in g_fault_data.fields:
                if (field.name.startswith('pad') or
                    field.name in ('magic', 'num_section_bytes')):
                    continue
                
                value = g_data.get_data(section_offset + field.offset,
                                        field.num_bytes)
                print('%*s: 0x%08x (%d)' %
                      (g_fault_data.max_name_len, field.name, value, value))
            # End of inline pretty_print
            idx += section_len
        
        # ========================================================================
        # SECTION 2: LWL
        # ========================================================================
        magic = self.get_data(idx, 4) # 4027383809: 0xF00D0001: MOD_MAGIC_LWL
        if magic not in self.magic_to_fault_type:
            g_swab = not g_swab
            magic = self.get_data(idx, 4)
        
        section_len = self.get_data(idx + 4, 4) # 1024 bytes
        if idx + section_len <= self.data_len:
            # Inline implementation of lwl_printer.pretty_print(idx, section_len)
            section_offset = idx  # parameter name from original method
            
            # Following the section header (magic and length) 
            print('=' * 80)
            print('LWL')
            print('=' * 80)

            # The section layout is as follows:
            #
            # Offset size name
            # ------ ---- ----
            #    0     4  magic
            #    4     4  num_section_bytes
            #    8     4  buf_size
            #   12     4  put_idx
            #   16     N  buf

            if section_len >= 20:
                buf_len = g_data.get_data(section_offset + 8, 4)
                put_idx = g_data.get_data(section_offset + 12, 4)
                buf_start_idx = section_offset + 16

                if buf_len + 16 == section_len and put_idx < buf_len:
                    # Inline implementation of get_optimal_start_idx(buf_start_idx, buf_len, put_idx)
                    optimal_invalid_ids = buf_len
                    optimal_start_idx = None

                    for offset in range(g_lwl_msg_set.max_msg_len + 1):
                        start_idx = put_idx + offset
                        if start_idx >= buf_len:
                            start_idx -= buf_len
                        test_idx = start_idx
                        first_get_test = offset == 0
                        invalid_id_ctr = 0
                        bytes_left = 0

                        while (True):
                            # Try to get ID.
                            try:
                                id, test_idx = g_data.get_data_circ(
                                    test_idx, 1, buf_start_idx, buf_len, put_idx, first_get_test)
                            except EOFError:
                                break

                            first_get_test = False
                            msg_meta_test = g_lwl_msg_set.get_metadata(id)
                            if msg_meta_test == None:
                                invalid_id_ctr += 1
                                continue

                            # The ID is valid. Try to get the argument bytes.
                            bytes_left, test_idx = g_data.get_bytes_left_circ(
                                test_idx, msg_meta_test.num_arg_bytes, buf_start_idx,
                                buf_len, put_idx, first_get_test)
                            if bytes_left < msg_meta_test.num_arg_bytes:
                                # Add byte for ID.
                                bytes_left += 1
                                break

                        if invalid_id_ctr < optimal_invalid_ids:
                            optimal_invalid_ids = invalid_id_ctr
                            optimal_start_idx = start_idx

                    circ_buf_idx = optimal_start_idx  # result of get_optimal_start_idx
                    first_get = True

                    try:        
                        # ================================================================
                        # PHASE 2 DECODING LOOP: A → B → C → D
                        # ================================================================
                        # This loop processes each LWL message in the circular buffer:
                        # A: Read Message ID (1 byte)
                        # B: Lookup Metadata (find format string and argument sizes)
                        # C: Extract Arguments (read argument bytes based on metadata)
                        # D: Format & Print (fmt % tuple(arg_values))
                        while True:
                            # First we try to get the message ID. If we are not synced-up,
                            # we might have to try several bytes to get a valid message ID.
                            # Even if we get a valid message ID, it might be some random
                            # data that was a message ID.

                            id_idx = None
                            msg_meta = None

                            while True:
                                id_idx = circ_buf_idx;

                                # --------------------------------------------------------
                                # STEP A: Read Message ID
                                # --------------------------------------------------------
                                id, circ_buf_idx = g_data.get_data_circ(
                                    circ_buf_idx, 1, buf_start_idx, buf_len, put_idx, first_get)
                                first_get = False

                                # --------------------------------------------------------
                                # STEP B: Lookup Metadata
                                # --------------------------------------------------------
                                msg_meta = g_lwl_msg_set.get_metadata(id)
                                if msg_meta == None:
                                    id_idx = None
                                    continue
                                break

                            # ------------------------------------------------------------
                            # STEP C: Extract Arguments
                            # ------------------------------------------------------------
                            arg_values = []
                            for arg_bytes in msg_meta.arg_bytes:
                                arg_bytes = int(arg_bytes)
                                arg_value, circ_buf_idx = g_data.get_data_circ(
                                    circ_buf_idx, 
                                    arg_bytes,
                                    buf_start_idx, 
                                    buf_len,
                                    put_idx, 
                                    first_get)
                                arg_values.append(arg_value)
                            
                            # ------------------------------------------------------------
                            # STEP D: Format & Print (fmt % tuple(arg_values))
                            # ------------------------------------------------------------
                            print(msg_meta.fmt % tuple(arg_values))

                    except EOFError:
                        pass
            # End of inline pretty_print
            idx += section_len
        
        # ========================================================================
        # SECTION 3: TRAILER
        # ========================================================================
        magic = self.get_data(idx, 4) # 3235512321: 0xC0DA0001: MOD_MAGIC_TRAILER
        if magic not in self.magic_to_fault_type:
            g_swab = not g_swab
            magic = self.get_data(idx, 4)
        
        section_len = self.get_data(idx + 4, 4) # 8 bytes
        if idx + section_len <= self.data_len:
            print('=' * 80)
            print("End of fault data")
            print('=' * 80)
            idx += section_len

    # ========================================================================
    # PHASE 2.4: get_data
    # ========================================================================
    # Reads a multi-byte value from the data array at a given index.
    # Handles endianness conversion (little/big endian).
    # Used for reading section headers, buffer sizes, etc.
    def get_data(self, idx, num_bytes):
        """
        Return value from data array as an int.

        Parameters:
            idx (int)              : The starting point for the data
            num_bytes (int)        : Number of bytes in value

        Raises:
            EOFError if out of data.

        Returns the value.

        Note that little endian encoding is assumed, but if g_swab is True, we
        use big endian.
        """

        value = 0
        shift = 0

        for loop_counter in range(num_bytes):
            if idx >= self.data_len:
                raise EOFError
            if g_swab:
                value = (value << 8) + self.data_array[idx]
            else:
                value = value + (self.data_array[idx] << shift)
                shift += 8
            idx = idx + 1

        return value

    # ========================================================================
    # PHASE 2.6: get_data_circ
    # ========================================================================
    # Reads a multi-byte value from a circular buffer, handling wraparound.
    # Used in the LWL decoding loop to read message IDs and arguments.
    def get_data_circ(self, idx, num_bytes, buf_start_idx, buf_len,
                      put_idx, first_get):
        """
        Return value from a circular buffer (within the data array) as an int.

        Parameters:
            idx (int)           : Current (relative) idx into buffer.
            num_bytes (int)     : Number of bytes in value.
            buf_start_idx (int) : Starting index of buffer in data array.
            buf_len (int)       : Number of bytes in the buffer.
            put_idx (int)       : Buffer put_idx value.
            first_get (boolean) : True if getting first data from the buffer.

        Raises:
            EOFError if out of data

        Note that big endian encoding is assumed.
        """

        if idx >= buf_len:
            raise IndexError
        if put_idx >= buf_len:
            raise IndexError

        value = 0
        for loop_counter in range(num_bytes):
            if idx == put_idx and not first_get:
                raise EOFError
            value = (value << 8) + self.data_array[buf_start_idx + idx]
            idx = idx + 1
            if idx >= buf_len:
                idx = 0
        return value, idx

    def get_bytes_left_circ(self, idx,num_to_check_for, buf_start_idx, buf_len,
                            put_idx, first_get):
        """
        Check if there are at least N bytes left in buffer.

        Parameters:
            idx (int)              : Current (relative) idx into buffer.
            num_to_check_for (int) : Number of bytes to check for.
            buf_start_idx (int)    : Starting index of buffer in data array.
            buf_len (int)          : Number of bytes in the buffer.
            put_idx (int)          : Buffer put_idx value.
            first_get (boolean)    : True if getting first data from the buffer.
        """

        if idx >= buf_len:
            raise IndexError
        if put_idx >= buf_len:
            raise IndexError
        bytes_left = 0

        for loop_counter in range(num_to_check_for):
            if idx == put_idx and not first_get:
                break
            idx += 1
            if idx >= buf_len:
                idx = 0
            bytes_left += 1
        return bytes_left, idx

################################################################################

g_data = Data()

################################################################################

class FaultField:

    def __init__(self, name, offset, num_bytes):
        self.name = name
        self.offset = offset
        self.num_bytes = num_bytes

################################################################################

class FaultData:

    def __init__(self):
        self.fields = []
        self.max_name_len = 0

    def add_fault_field(self, name, offset, num_bytes):
        """
        Add a fault information field.

        Parameters:
            name (str)      : Name of field.
            offset (int)    : Offset of field in section data.
            num_bytes (int) : Number of bytes for field.
        """

        self.fields.append(FaultField(name, offset, num_bytes))
        if len(name) > self.max_name_len:
            self.max_name_len = len(name)

    def pretty_print(self, section_offset, section_len):
        """
        Pretty print the fault data.

        Parameters:
            section_offset (int) : Data array index of start of the section.
            section_len (int)    : Length of segment in bytes.
        """

        print('=' * 80)
        print('Fault data')
        print('=' * 80)
        for field in self.fields:
            if (field.name.startswith('pad') or
                field.name in ('magic', 'num_section_bytes')):
                continue
            
            value = g_data.get_data(section_offset + field.offset,
                                    field.num_bytes)
            print('%*s: 0x%08x (%d)' %
                  (self.max_name_len, field.name, value, value))

################################################################################

g_fault_data = FaultData()

################################################################################

################################################################################

# ============================================================================
# PHASE 1.5: LwlMsg Metadata Objects
# ============================================================================
# This class represents the metadata for a single LWL message.
# Objects of this class are created during Phase 1 (source code parsing)
# and stored in the g_lwl_msg_set dictionary for use during Phase 2 (decoding).
class LwlMsg:
    """
    This class represents the meta data for a single LWL messages.

    This data is extracted from source files, and is used to format the raw
    data.
    """

    def __init__(self, id, fmt, arg_bytes, file_path, line_num, lwl_statement):
        self.id = id
        self.fmt = fmt
        self.arg_bytes = arg_bytes
        self.file_path = file_path
        self.line_num = line_num
        self.lwl_statement = lwl_statement
        self.num_arg_bytes = 0
        for d in self.arg_bytes:
            self.num_arg_bytes += int(d)

################################################################################

class LwlMsgSet:
    """
    This class represents the set of meta data for all LWL messages.

    This data is extracted from source files, and is used to format the raw
    data. Error checking is done to ensure consistency of the message set.
    """

    def __init__(self):
        self.lwl_msgs = {}
        self.max_msg_len = 0

    # ========================================================================
    # PHASE 1.4: add_lwl_msg - Store Metadata
    # ========================================================================
    # This method stores LWL message metadata in the dictionary.
    # Called from parse_source_file() after parsing each LWL statement.
    def add_lwl_msg(self, id, fmt, arg_bytes, lwl_statement, file_path,
                    line_num):
        """
        Add a LWL message.

        Parameters:
            id (int)            : LWL statement ID (must be unique).
            fmt (str)           : LWL statement format string.
            arg_bytes (int)     : Number of argument bytes.
            lwl_statement (str) : Full LWL statement text.
            file_path (str)     : File name containing LWL statement.
            line_num (int)      : Line number of LWL statement.
        """

        if id not in self.lwl_msgs:
            self.lwl_msgs[id] = LwlMsg(id, fmt, arg_bytes, file_path,
                                       line_num, lwl_statement)
            msg_len = 1
            for len in arg_bytes:
                msg_len += int(len)
            if msg_len > self.max_msg_len:
                self.max_msg_len = msg_len

    def get_metadata(self, id):
        try:
            return self.lwl_msgs[id]
        except KeyError:
            return None

    def get_num_lwl_statements(self):
        return len(self.lwl_msgs);

################################################################################

g_lwl_msg_set = LwlMsgSet()

################################################################################

class LwlPrinter:

    # ========================================================================
    # PHASE 2.5: pretty_print
    # ========================================================================
    # Main function for decoding and printing LWL messages from the circular
    # buffer. Contains the decoding loop (steps A→B→C→D) that processes each
    # message in the buffer.
    def pretty_print(self, section_offset, section_len):
        """
        Pretty print the LWL messages.

        Parameters:
            section_offset (int) : Data array index of start of the section.
            section_len (int)    : Length of segment in bytes.

        Note that the "put index" is (was) the next location to be written, and
        thus is the oldest data in the circular buffer. This is where we
        start. But because the messages are variable length, the put index might
        point into the middle of a message. Thus, this function has to get in
        sync with the message boundary, using some ad-hoc process.
        """

        # Following the section header (magic and length) 
        print('=' * 80)
        print('LWL')
        print('=' * 80)

        # The section layout is as follows:
        #
        # Offset size name
        # ------ ---- ----
        #    0     4  magic
        #    4     4  num_section_bytes
        #    8     4  buf_size
        #   12     4  put_idx
        #   16     N  buf

        if section_len >= 20:
            buf_len = g_data.get_data(section_offset + 8, 4)
            put_idx = g_data.get_data(section_offset + 12, 4)
            buf_start_idx = section_offset + 16

            if buf_len + 16 == section_len and put_idx < buf_len:
                idx =  self.get_optimal_start_idx(buf_start_idx, buf_len, put_idx)
                first_get = True

                try:        
                    # ================================================================
                    # PHASE 2 DECODING LOOP: A → B → C → D
                    # ================================================================
                    # This loop processes each LWL message in the circular buffer:
                    # A: Read Message ID (1 byte)
                    # B: Lookup Metadata (find format string and argument sizes)
                    # C: Extract Arguments (read argument bytes based on metadata)
                    # D: Format & Print (fmt % tuple(arg_values))
                    while True:
                        # First we try to get the message ID. If we are not synced-up,
                        # we might have to try several bytes to get a valid message ID.
                        # Even if we get a valid message ID, it might be some random
                        # data that was a message ID.

                        id_idx = None
                        msg_meta = None

                        while True:
                            id_idx = idx;

                            # --------------------------------------------------------
                            # STEP A: Read Message ID
                            # --------------------------------------------------------
                            id, idx = g_data.get_data_circ(
                                idx, 1, buf_start_idx, buf_len, put_idx, first_get)
                            first_get = False

                            # --------------------------------------------------------
                            # STEP B: Lookup Metadata
                            # --------------------------------------------------------
                            msg_meta = g_lwl_msg_set.get_metadata(id)
                            if msg_meta == None:
                                id_idx = None
                                continue
                            break

                        # ------------------------------------------------------------
                        # STEP C: Extract Arguments
                        # ------------------------------------------------------------
                        arg_values = []
                        for arg_bytes in msg_meta.arg_bytes:
                            arg_bytes = int(arg_bytes)
                            arg_value, idx = g_data.get_data_circ(
                                idx, arg_bytes, buf_start_idx, buf_len,
                                put_idx, first_get)
                            arg_values.append(arg_value)
                        
                        # ------------------------------------------------------------
                        # STEP D: Format & Print (fmt % tuple(arg_values))
                        # ------------------------------------------------------------
                        print(msg_meta.fmt % tuple(arg_values))

                except EOFError:
                    pass

    # ========================================================================
    # PHASE 2.7: get_optimal_start_idx
    # ========================================================================
    # Finds the best starting position in the circular buffer to begin
    # decoding messages. Uses brute-force to find the offset that minimizes
    # invalid message IDs (indicating we're aligned at message boundaries).
    def get_optimal_start_idx(self, buf_start_idx, buf_len, put_idx):
        """
        Find the optimal starting index to decode log buffer.

        Parameters:
            buf_start_idx (int) : Start of LWL buffer in the fault data.
            buf_len (int)       : Length of LWL buffer.
            put_idx (int)       : LWL buffer put_idx.

        Return:
            The optimal starting index (>= put_idx)

        This is a brute-force solution where we start at the known "put_idx",
        add an offset to it, and using that starting point, determine how
        many invalid log message IDs are encountered.

        The start offset (relative to the put index) is the largest number
        of argument bytes for any LWL statement. This is for the case where
        just the ID for such an LWL statement got overwritten.

        We assume that the optimal starting point is the one that minimizes the
        number of invalid log IDs.  This is not guaranteed to be the truely
        optimal offset, but likely is.
        """

        optimal_invalid_ids = buf_len
        optimal_start_idx = None

        for offset in range(g_lwl_msg_set.max_msg_len + 1):
            start_idx = put_idx + offset
            if start_idx >= buf_len:
                start_idx -= buf_len
            idx = start_idx
            first_get = offset == 0
            invalid_id_ctr = 0
            bytes_left = 0

            while (True):
                # Try to get ID.
                try:
                    id, idx = g_data.get_data_circ(
                        idx, 1, buf_start_idx, buf_len, put_idx, first_get)
                except EOFError:
                    break

                first_get = False
                msg_meta = g_lwl_msg_set.get_metadata(id)
                if msg_meta == None:
                    invalid_id_ctr += 1
                    continue

                # The ID is valid. Try to get the argument bytes.
                bytes_left, idx = g_data.get_bytes_left_circ(
                    idx, msg_meta.num_arg_bytes, buf_start_idx,
                    buf_len, put_idx, first_get)
                if bytes_left < msg_meta.num_arg_bytes:
                    # Add byte for ID.
                    bytes_left += 1
                    break

            if invalid_id_ctr < optimal_invalid_ids:
                optimal_invalid_ids = invalid_id_ctr
                optimal_start_idx = start_idx

        return optimal_start_idx

################################################################################

lwl_printer = LwlPrinter()

################################################################################

class SourceParser:

    # ========================================================================
    # PHASE 1.2: parse_source_dir
    # ========================================================================
    # Walks through directory tree, finds all .c files, and calls
    # parse_source_file() for each one.
    def parse_source_dir(self, dir_path):
        for root_dir_path, dir_names, file_names in os.walk(dir_path):
            for file_name in file_names:
                file_path = os.path.join(root_dir_path, file_name)
                if file_path[-2:] == '.c':
                    self.parse_source_file(file_path)

    # ========================================================================
    # PHASE 1.3: parse_source_file
    # ========================================================================
    # Parses a single C source file to extract:
    # - LWL statement definitions (format strings, argument sizes)
    # - Fault data field definitions (from //@fault_data annotations)
    # Calls add_lwl_msg() to store each LWL message metadata.
    def parse_source_file(self, file_path):
        """
        Search through a source file (normally a .c or .h), finding meta data.
        There are lwl statements, and fault data descriptions.

        lwl statements:

            Each file using lwl needs to have a statement like this before any
            lwl statement:

                #define LWL_BASE_ID 10
                #define LWL_NUM 5

            Example statements:
                LWL("Trace fmt %d", 2, LWL_2(abc));
                LWL("Simple trace", 0)

            The message defintions are checked for syntax and consistency, and
            errors reported.

            Multi-line LWL statements complicate this task.  But we make
            simplifying assumptions to help:
            - All non-final lines end with a comma (optionally followed by
              whitespace)
            - The final line ends with ");" (optionally followed by whitespace)

        fault data descriptions:

            A snippet:

                struct fault_data
                {
                    uint32_t magic;              //@fault_data,magic,4
                    uint32_t num_section_bytes;  //@fault_data,num_section_bytes,4

                    uint32_t fault_type;         //@fault_data,fault_type,4
                    uint32_t fault_param;        //@fault_data,fault_param,4
                    uint32_t return_addr;        //@fault_data,return_addr,4

                    :
                    :

            We assume fault data defintions are always on a single line.

        """

        lwl_base_id = None
        fault_field_offset = 0

        parse_fault_data_pat = re.compile(r'//@fault_data,([^,]+),(\d+)')

        lwl_base_id_pat = re.compile(r'^\s*#define\s+LWL_BASE_ID\s+(\d+)')
        lwl_num_pat = re.compile(r'^\s*#define\s+LWL_NUM\s+(\d+)')
        detect_lwl_pat = re.compile(r'^\s*LWL\(')
        parse_fmt_pat = re.compile(r'\s*LWL\("([^"]*)"')
        parse_zero_pat = re.compile(r'\s*,\s*0')
        parse_arg_string_pat = re.compile(r'\s*,\s*"([^"]*)"')
        parse_num_arg_bytes_pat = re.compile(r'\s*,\s*([0-9]+)')
        parse_arg_pat = re.compile(r',\s*LWL_([\d])\(')
        with open(file_path, encoding='utf-8', errors='ignore') as f:
            line_num = 0
            lwl_id_offset = -1
            lwl_statement = None
            for line in f:
                line_num += 1
                line = line.strip()
                if not line:
                    continue

                # Start with fault data descriptions.
                m = re.search(parse_fault_data_pat, line)
                if m:
                    name = m.group(1)
                    num_bytes = int(m.group(2))
                    g_fault_data.add_fault_field(name, fault_field_offset,
                                                 num_bytes)
                    fault_field_offset += num_bytes
                    continue

                # Now for lwl statements.
                m = re.match(lwl_base_id_pat, line)
                if m:
                    lwl_base_id = int(m.group(1))
                    lwl_line_num = line_num
                if lwl_statement:
                   # Check for acceptable line ending.
                   if (line[-1] == ',') or (line[-2:] == ');'):
                        lwl_statement += line
                   else:
                        lwl_statement = None
                else:
                    # Check for start of statement.
                    if re.match(detect_lwl_pat, line):
                        # Check for acceptable line ending.
                        if (line[-2:] == ');') or (line[-1] == ','):
                            lwl_statement = line
                if (not lwl_statement) or (lwl_statement[-2:] != ');'):
                    continue

                # Got a complete statement. First get the format.
                lwl_id_offset += 1
                m = re.match(parse_fmt_pat, lwl_statement)
                if m:
                    lwl_fmt = m.group(1)
                    lwl_remain = lwl_statement[m.end():]

                    # Get num_arg_bytes
                    m = re.match(parse_num_arg_bytes_pat, lwl_remain)
                    if m:
                        lwl_num_arg_bytes = int(m.group(1))
                        lwl_remain = lwl_remain[m.end():]

                        # Get the arg lengths
                        lwl_arg_lengths = ''
                        while True:
                            m = re.search(parse_arg_pat, lwl_remain)
                            if m:
                                lwl_arg_lengths += m.group(1)
                                lwl_remain = lwl_remain[m.end():]
                            else:
                                break

                        if lwl_base_id is not None:
                            g_lwl_msg_set.add_lwl_msg(lwl_base_id +
                                                     int(lwl_id_offset),
                                                     lwl_fmt, lwl_arg_lengths,
                                                     lwl_statement, file_path,
                                                     lwl_line_num)

                lwl_statement = None

    def get_num_fmt_params(self, fmt):
        """ Returns the number of parameters required for a format string."""

        state = 'idle'
        num_params = 0
        for c in fmt:
            if state == 'idle':
                if c == "%":
                    state = 'got_percent'
                continue
            if state == 'got_percent':
                if c == "%":
                    state = 'idle'
                else:
                    num_params += 1
                    state = 'idle'
                continue
        return num_params

################################################################################

source_parser = SourceParser()

################################################################################


def main():
    # ========================================================================
    # PHASE 1.1: main start parsing
    # ========================================================================
    # Phase 1: Parse source code directories to extract metadata
    # (LWL statements and fault data field definitions)
    # Hardcoded source directory - assumes script is run from modules/fault/
    source_dir = 'C:\\Users\\Sheen\\Desktop\\Embedded_System\\gene_Baremetal_I2CTmphm_RAM_CICD\\I2C_TmpHm_RAM_CICD'  # Points to I2C_TmpHm_RAM_CICD (parent of modules/)
    source_parser.parse_source_dir(source_dir)
    
    # ========================================================================
    # PHASE 2.1: main reads raw data
    # ========================================================================
    # Phase 2: Read and decode the raw fault data file
    # Hardcoded input file - assumes script is run from modules/fault/
    data_file = 'C:\\Users\\Sheen\\Desktop\\Embedded_System\\gene_Baremetal_I2CTmphm_RAM_CICD\\I2C_TmpHm_RAM_CICD\\modules\\fault\\raw_hardcoded.txt'  # Located in modules/fault/
    g_data.read_data_file(data_file)
    
    # DEBUG: Print data_array contents
    print(f"Total data length: {g_data.data_len} bytes")
    print("\nFirst 64 bytes as hex:")
    for i in range(g_data.data_len):
        if i % 16 == 0:
            print(f"\n{i:04x}: ", end="")
        print(f"{g_data.data_array[i]:02x} ", end="")
    print("\n")

    # DEBUG: Print lwl_msgs dictionary contents
    print(f"Total LWL messages: {len(g_lwl_msg_set.lwl_msgs)}")
    print("\nLWL Messages Dictionary:")
    print("=" * 80)
    for msg_id, msg_meta in sorted(g_lwl_msg_set.lwl_msgs.items()):
        print(f"\nMessage ID: {msg_id:04d} (0x{msg_id:04x})")
        print(f"  fmt            : {msg_meta.fmt}")
        print(f"  arg_bytes      : {msg_meta.arg_bytes}")
        print(f"  num_arg_bytes  : {msg_meta.num_arg_bytes}")
        print(f"  file_path      : {msg_meta.file_path}")
        print(f"  line_num       : {msg_meta.line_num}")
        print(f"  lwl_statement  : {msg_meta.lwl_statement}")
    print("=" * 80)
    print()

    # DEBUG: Print fault_data.fields contents
    print(f"Total fault fields: {len(g_fault_data.fields)}")
    print("\nFault Fields List:")
    print("=" * 80)
    for i, field in enumerate(g_fault_data.fields):
        print(f"  [{i:2d}] name='{field.name}', offset={field.offset:4d} (0x{field.offset:04x}), num_bytes={field.num_bytes}")
    print("=" * 80)
    print()
 

    g_data.process_data()
    print('End of logfmt processing.')
        
if __name__ == '__main__':
    main()
