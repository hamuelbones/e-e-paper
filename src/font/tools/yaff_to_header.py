
# Script to convert yaff files to bespoke C headers / fonts we can render.
# Fonts sourced from: https://github.com/robhagemans/hoard-of-bitfonts

import argparse


def import_yaff(filename):
    font_data = {}
    font_metadata = {}

    with open(filename, 'r') as f:
        # File format is almost YAML, but is different enough that yaml parsers don't like it.

        comment = ""
        in_yaff_header = True
        parsing_character = False
        character_data = []
        character = ""

        for line in f:
            line = line.strip()
            if in_yaff_header:
                if len(line) == 0:
                    in_yaff_header = 0
                else:
                    try:
                        key, val = line.split(": ")
                        font_metadata[key] = val
                    except ValueError:
                        continue
            elif not parsing_character:
                if len(line) >= 1 and line[0] == "#":
                    comment = line
                elif len(line) >= 2 and line[1] == "x" and line[-1] == ":":
                    character = int(line[2:4], base=16)
                    parsing_character = True
                elif len(line) >= 7 and line[0:4] == "u+00" and line[-1] == ":":
                    character = int(line[4:6], base=16)
                    parsing_character = True
            else:
                if len(line) == 0:
                    font_data[character] = {
                        "width": len(character_data[0]),
                        "height": len(character_data),
                        "comment": comment,
                        "data": character_data[:]
                    }
                    comment = ""
                    character_data = []
                    parsing_character = False
                else:
                    character_data.append(line)

    if parsing_character:
        font_data[character] = {
            "width": len(character_data[0]),
            "height": len(character_data),
            "comment": comment,
            "data": character_data[:]
        }

    return font_metadata, font_data


def yaffstr_to_bytes(dots_and_ats: str):
    print(dots_and_ats)
    bit_str = dots_and_ats.replace(".", "0").replace("@", "1")
    # Pad to 8
    bit_str += "0" * ((8 - len(bit_str)) % 8)
    print(bit_str)
    byte_array = int(bit_str, 2).to_bytes(length=len(bit_str)//8, byteorder='big')
    print(byte_array)
    return ", ".join([hex(x) for x in byte_array])


def export_header(filename, metadata, data):
    with open(filename, 'w') as f:

        f.write("/* Generated Font Header from script - yaff_to_header.py */\n")
        f.write("\n\n#include \"font.h\"\n\n")
        f.write("/* Font Information: \n")
        for key, value in metadata.items():
            f.write(f"   {key}: {value}\n")
        f.write("*/\n\n\n")

        f.write("/* Character data below... */\n")

        base_name = metadata["name"].replace(" ", "_")

        for index, char_data in data.items():
            f.write("\n")
            f.write(f"/* {char_data['comment']} */\n")
            f.write("\n")

            # Character data
            f.write(f"static const uint8_t ch_{base_name}_{index}_data[] = {{\n")
            for row in char_data['data']:
                f.write(f"        {yaffstr_to_bytes(row)},\n")
            f.write(f"}};\n")

            # Character structure
            f.write(f"const FONT_CHARACTER ch_{base_name}_{index} = {{\n")
            f.write(f"    .width = {char_data['width']},\n")
            f.write(f"    .height = {char_data['height']},\n")
            f.write(f"    .data = ch_{base_name}_{index}_data,")
            f.write(f"}};\n")

        # table
        base = min(data.keys())
        length = max(data.keys())-base+1

        f.write(f"\n\nstatic const FONT_CHARACTER * tab_{base_name}[] = {{\n")
        for i in range(base, base+length):
            if i in data.keys():
                f.write(f"    &ch_{base_name}_{i},\n")
            else:
                f.write(f"    NULL,\n")
        f.write(f"}};\n\n")

        f.write(f"const FONT_TABLE font_{base_name} = {{\n")
        f.write(f"    .code_base = {base},\n")
        f.write(f"    .code_length = {length},\n")
        f.write(f"    .characters = tab_{base_name},\n")
        f.write(f"}};\n\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=str, help="Input .yaff file to convert.")
    parser.add_argument("output", type=str, help="Output font header file.")
    args = parser.parse_args()

    font_metadata, font_data = import_yaff(args.input)
    export_header(args.output, font_metadata, font_data)
    print(font_metadata)
    print(font_data)
