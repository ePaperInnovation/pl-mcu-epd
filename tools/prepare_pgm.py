# Produce images in a format compatible with Plastic Logic displays

# Copyright (C) 2013 Plastic Logic Limited
#
#     Guillaume Tucker <guillaume.tucker@plasticlogic.com>
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from __future__ import print_function

import sys
import os
import argparse
from PIL import Image

def pad(img, left_border):
    "Pad image with left border"
    w, h = img.size
    out_img = Image.new('L', ((w + left_border), h))
    out_img.paste(img, ((left_border, 0, (w + left_border), h)))
    return out_img

def interleave(img):
    "Reorder pixels for interleaved display (i.e. Type19 bracelet 720x120)"
    w, h = img.size
    w2, h2 = (w / 2), (h * 2)
    out_img = Image.new('L', (w2, h2))

    for i, pix in enumerate(img.getdata()):
        x = int(i / 2) % w2
        y = (2 * int(i / w)) + 1 - (i % 2)
        out_img.putpixel((x, y), pix)

    return out_img


def main(argv):
    parser = argparse.ArgumentParser(
        description="Produce images for Plastic Logic displays")
    parser.add_argument('input_file', help="path to the input image file")
    parser.add_argument('--output',
                        help="path to output image file, default is .pgm")
    parser.add_argument('--interleave', action='store_true',
                        help="reorder the pixels for interleaved display")
    parser.add_argument('--pad', type=int,
                        help="pad the image with pixels on the left border")
    args = parser.parse_args(argv[1:])

    print("Opening {}".format(args.input_file))
    img = Image.open(args.input_file).convert('L')

    if args.pad:
        print("Padding with {} pixels on the left border".format(args.pad))
        img = pad(img, args.pad)

    if args.interleave:
        print("Interleaving image")
        original_size = img.size
        img = interleave(img)
        print("Image size: {} -> {}".format(original_size, img.size))

    if args.output:
        output_name = args.output
    else:
        output_name = os.path.basename(args.input_file)
        output_name = os.path.splitext(output_name)[0]
        output_name = '.'.join([output_name, 'pgm'])

    print("Saving output image as {}".format(output_name))
    img.save(output_name)

    return True

if __name__ == '__main__':
    ret = main(sys.argv)
    sys.exit(0 if ret is True else 1)

