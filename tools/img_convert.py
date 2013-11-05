from __future__ import print_function
import sys
import os
from PIL import Image

def dump_hex(img):
    w, h = img.size
    for i, pix in enumerate(img.getdata()):
        x = i % w
        end = '\n' if (x == (w - 1)) else ' '
        print('{0:02}'.format(pix), end=end)

def scramble_img(img):
    print("Scrambling...")
    w, h = img.size
    w2, h2 = (w / 2), (h * 2)

    print("Image size: {0} -> {1}".format((w, h), (w2, h2)))
    out_img = Image.new('L', (w2, h2))

    for i, pix in enumerate(img.getdata()):
        x = int(i / 2) % w2
        y = (2 * int(i / w)) + 1 - (i % 2)
        out_img.putpixel((x, y), pix)

    return out_img

def main(argv):
    if len(argv) < 2:
        print("No file name provided")
        return False

    file_name = argv[1]
    scramble = True if (len(argv) > 2 and argv[2] == 'scramble') else False
    

    print("Opening {0}".format(file_name))
    img = Image.open(file_name).convert('L')

    if scramble:
    	out_img = scramble_img(img)
    else:
        out_img = img
        
    if False: # pad to 400x320 for the .pgm version
        img_padded = Image.new('L', (800, 120))
        w, h = img.size
        img_padded.paste(img, (0, 0, w, h))
        out_img_padded = scramble(img_padded)
    else:
        out_img_padded = out_img

    base_name = os.path.basename(file_name)
    if scramble:
        scrambled_name = '_'.join([os.path.splitext(base_name)[0], 's'])
    else:
        scrambled_name = '_'.join([os.path.splitext(base_name)[0], 'n'])

    for ext, img in [('pgm', out_img_padded)]:
        output_name = '.'.join([scrambled_name, ext])
        print("Saving as {0}".format(output_name))
        img.save(output_name)

    return True

if __name__ == '__main__':
    ret = main(sys.argv)
    sys.exit(0 if ret is True else 1)

