
from imagekit import *

def main():
    b = ImageBuffer.fromPNG('images/image01.png')
    for x in xrange(b.width):
        line = b.vtline_out(x)
        line.sort()
        b.vtline_in(x, line)
    
    b.savePNG('output.png')

if __name__ == '__main__':
    main()
