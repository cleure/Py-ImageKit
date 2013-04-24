
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
