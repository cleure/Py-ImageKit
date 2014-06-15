
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
