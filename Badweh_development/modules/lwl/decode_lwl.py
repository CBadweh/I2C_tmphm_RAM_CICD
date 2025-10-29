# Save this as decode_lwl.py
import re

# Copy your hex data here (from the console output)
hex_data = """
0000: 01 00 0d f0 00 04 00 00 f0 03 00 00 e1 00 00 00
0010: 01 00 03 00 00 44 02 00 04 01 05 06 44 02 07 02
...
"""

# Extract just the hex bytes
hex_bytes = re.findall(r'[0-9a-f]{2}', hex_data)
data = bytearray.fromhex(''.join(hex_bytes))

# Decode (simplified for now - just shows IDs)
print("LWL Log Entries:")
idx = 16  # Skip header (first 16 bytes)
while idx < len(data):
    id = data[idx]
    print(f"ID {id} at offset {idx}")
    idx += 1