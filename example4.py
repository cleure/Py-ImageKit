
import copy
from imagekit import *

def main():
    b = ImageBuffer.fromPNG('images/image02.png')
    b.apply_rankfilter(3, 0.5)
    b.savePNG('output.png')

if __name__ == '__main__':
    main()
