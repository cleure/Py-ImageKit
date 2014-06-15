
from imagekit import *

def main():
    b = ImageBuffer.from_png('images/image02.png')
    b.apply_rankfilter(3, 0.5)
    b.save_png('example4-output.png')

if __name__ == '__main__':
    main()
