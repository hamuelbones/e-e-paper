
import argparse
import toml
from PIL import Image
import pathlib
import os
import numpy


def import_toml_data(filename: str):

    toml_path = pathlib.Path(filename)

    with open(toml_path, 'r') as f:
        parsed_data = toml.load(f)

    image_dir = toml_path.parent
    old_wd = os.getcwd()
    os.chdir(image_dir)

    for char_table in parsed_data['character']:
        print(char_table)
        image_name = char_table['image']
        with Image.open(image_name) as im:
            im.convert("1")

            image_array = numpy.asarray(im)
            char_table["width"] = im.width
            char_table["height"] = im.height

        image_yaff_part = ""
        for line in image_array:
            image_text = "".join(["." if px > 128 else "@" for px in line])
            image_yaff_part += "\t" + image_text + "\n"

        char_table["data"] = image_yaff_part

    os.chdir(old_wd)
    return parsed_data


def write_yaff_output(filename: str, data):

    with open(filename, "w") as f:
        for key in data.keys():
            if key != 'character':
                f.write(f"{key}: {data[key]}\n")
        f.write("\n")
        for character in data['character']:
            f.write("\n")
            f.write(f"0x{character['index']:02x}:\n")
            f.write(character["data"])
            f.write("\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=str, help="Input TOML file describing font information")
    parser.add_argument("output", type=str, help="Output yaff file.")
    args = parser.parse_args()

    data = import_toml_data(args.input)
    write_yaff_output(args.output, data)
