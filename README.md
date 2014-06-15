Py-ImageKit
===========

ImageKit is a lightweight low level image manipulation library, which I use for quickly prototyping new image processing filters and computer vision algorithms. It's currently supported on Linux and OS X, for Python 2.5+ and 3.x.

Dependencies
============
ImageKit requires libpng and libjpeg. The implementation doesn't necessarily matter (it will work fine with both libjpeg and libjpeg-turbo). GIF support may be added in the future.

Installing
==========
To install, simply run 'sudo python setup.py install'

TODO
====
On OS X, libjpeg-config is not necessarily available, if installed through MacPorts. I need to find a more permanent workaround to this problem.

Examples
========

Rank Filter:

    from imagekit import *

    def main():
        b = ImageBuffer.from_png('images/image02.png')
        b.apply_rankfilter(3, 0.5) # Neighborhood size, Rank (0.0 - 1.0)
        b.save_png('output.png')

    if __name__ == '__main__':
        main()

Rotate 90 Degress:

    from imagekit import *

    def main():
        input = ImageBuffer.from_png('images/image01.png')
        output = ImageBuffer(   width=input.height,
                                height=input.width,
                                channels=input.channels)
        
        # Rotate image by 90 degrees
        for y in range(input.height):
            line = input.hzline_out(y)
            output.vtline_in(input.height - y - 1, line)
        
        output.save_png('example1-output.png')

    if __name__ == '__main__':
        main()

Generate HSV Color Pattern:

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
