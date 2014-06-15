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
        b = ImageBuffer.fromPNG('images/image02.png')
        b.apply_rankfilter(3, 0.5) # Neighborhood size, Rank (0.0 - 1.0)
        b.savePNG('output.png')

    if __name__ == '__main__':
        main()

Rotatae 90 Degress:

    from imagekit import *

    def main():
        input = ImageBuffer.fromPNG('images/image01.png')
        output = ImageBuffer(   width=input.height,
                                height=input.width,
                                channels=input.channels)
        
        # Rotate image by 90 degrees
        for y in xrange(input.height):
            line = input.hzline_out(y)
            output.vtline_in(input.height - y - 1, line)
        
        output.savePNG('example1-output.png')

    if __name__ == '__main__':
        main()
