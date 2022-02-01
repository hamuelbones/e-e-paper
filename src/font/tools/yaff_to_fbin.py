
# TODO refactor this out
from yaff_to_header import import_yaff

import argparse
import struct

# Converts a yaff file to a custom header format.

# Format: (all numbers little-endian)

# Header section
# 4 bytes magic "FBIN"
# 2 bytes: base character code
# 2 bytes: number of character codes

# Character information section, an array of codes of length "num_codes"
# 2 bytes: x size
# 2 bytes: y size
# 4 bytes: offset in data where character data starts

# Data section
# Just contains character data. Binary on/off values, with rows padded to the nearest byte.

def yaffstr_to_bytes(dots_and_ats: str):
    bit_str = dots_and_ats.replace(".", "0").replace("@", "1")
    # Pad to 8
    bit_str += "0" * ((8 - len(bit_str)) % 8)
    byte_array = int(bit_str, 2).to_bytes(length=len(bit_str)//8, byteorder='big')
    return byte_array


def export_fbin(filename, data):
    with open(filename, 'wb') as f:

        base = min(data.keys())
        length = max(data.keys())-base+1

        header_section = b"FBIN" + struct.pack("<HH", base, length)

        character_info_section = b""
        data_section = b""

        for i in range(base, base+length):
            if i not in data.keys():
                x = 0
                y = 0
                offset = 0
            else:
                x = data[i]['width']
                y = data[i]['height']
                offset = len(data_section)
                for row in data[i]['data']:
                    data_section += yaffstr_to_bytes(row)

            character_info_section += struct.pack("<HHI", x, y, offset)

        f.write(header_section)
        f.write(character_info_section)
        f.write(data_section)



if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=str, help="Input .yaff file to convert.")
    parser.add_argument("output", type=str, help="Output font header file.")
    args = parser.parse_args()

    font_metadata, font_data = import_yaff(args.input)
    export_fbin(args.output, font_data)