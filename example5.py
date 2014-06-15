
import random
from imagekit import *

def main():
    b = ImageBuffer(640, 480, 3, 360.0, COLORSPACE_HSV, COLORSPACE_FORMAT_HSV_NATURAL)
    
    hue_scale = 360.0 / b.width
    for y in range(b.height):
        for x in range(b.width):
            b.set_pixel(x, y, (x * hue_scale, 1.0, 1.0))

    b.save_png('example5-output.png')

if __name__ == '__main__':
    main()
