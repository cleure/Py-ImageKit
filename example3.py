
"""

Tool that outputs the NES Color Palette to a PNG file.

"""

import math
from imagekit import *

# NES Color Palette in RGB-24
palette = [
    (124.0, 124.0, 124.0),
    (0.0, 0.0, 252.0),
    (0.0, 0.0, 188.0),
    (68.0, 40.0, 188.0),
    (148.0, 0.0, 132.0),
    (168.0, 0.0, 32.0),
    (168.0, 16.0, 0.0),
    (136.0, 20.0, 0.0),
    (80.0, 48.0, 0.0),
    (0.0, 120.0, 0.0),
    (0.0, 104.0, 0.0),
    (0.0, 88.0, 0.0),
    (0.0, 64.0, 88.0),
    (0.0, 0.0, 0.0),
    (0.0, 0.0, 0.0),
    (0.0, 0.0, 0.0),
    (188.0, 188.0, 188.0),
    (0.0, 120.0, 248.0),
    (0.0, 88.0, 248.0),
    (104.0, 68.0, 252.0),
    (216.0, 0.0, 204.0),
    (228.0, 0.0, 88.0),
    (248.0, 56.0, 0.0),
    (228.0, 92.0, 16.0),
    (172.0, 124.0, 0.0),
    (0.0, 184.0, 0.0),
    (0.0, 168.0, 0.0),
    (0.0, 168.0, 68.0),
    (0.0, 136.0, 136.0),
    (0.0, 0.0, 0.0),
    (0.0, 0.0, 0.0),
    (0.0, 0.0, 0.0),
    (248.0, 248.0, 248.0),
    (60.0, 188.0, 252.0),
    (104.0, 136.0, 252.0),
    (152.0, 120.0, 248.0),
    (248.0, 120.0, 248.0),
    (248.0, 88.0, 152.0),
    (248.0, 120.0, 88.0),
    (252.0, 160.0, 68.0),
    (248.0, 184.0, 0.0),
    (184.0, 248.0, 24.0),
    (88.0, 216.0, 84.0),
    (88.0, 248.0, 152.0),
    (0.0, 232.0, 216.0),
    (120.0, 120.0, 120.0),
    (0.0, 0.0, 0.0),
    (0.0, 0.0, 0.0),
    (252.0, 252.0, 252.0),
    (164.0, 228.0, 252.0),
    (184.0, 184.0, 248.0),
    (216.0, 184.0, 248.0),
    (248.0, 184.0, 248.0),
    (248.0, 164.0, 192.0),
    (240.0, 208.0, 176.0),
    (252.0, 224.0, 168.0),
    (248.0, 216.0, 120.0),
    (216.0, 248.0, 120.0),
    (184.0, 248.0, 184.0),
    (184.0, 248.0, 216.0),
    (0.0, 252.0, 252.0),
    (248.0, 216.0, 248.0),
    (0.0, 0.0, 0.0),
    (0.0, 0.0, 0.0),
]

def chunks(array, n):
    if len(array) % n:
        raise ValueError('array must be divisible by n')
    
    for i in xrange(len(array) / n):
        yield array[i*n:i*n+n]

def main():
    width = 16
    height = int(math.ceil(float(len(palette)) / width))
    
    b = ImageBuffer(width, height, 3)
    for i in xrange(len(palette)):
        y = i / width
        x = i % width
        b.set_pixel(x, y, palette[i])

    b.scale_nearest(b.width*32, b.height*32)
    b.savePNG('output2.png')

if __name__ == '__main__':
    main()