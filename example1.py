
import os, sys, imagekit

def main():
    if len(sys.argv) < 3:
        print('Prints the pixel at x/y coordinate')
        print('Usage: %s <x> <y>' % (sys.argv[0]))
        sys.exit(1)

    try:
        x = int(sys.argv[1])
        y = int(sys.argv[2])
    except:
        print('X/Y must be integer')
        sys.exit(1)
    
    input = imagekit.Image.fromPNG('images/image01.png')
    
    if not x < input.width:
        print('X coordinate exceeds width of image (%d)' % (input.width))
        sys.exit(1)
        
    if not y < input.height:
        print('Y coordinate exceeds height of image (%d)' % (input.height))
        sys.exit(1)
    
    pixel = input.get_pixel(x, y)
    print('The pixel at %dx%d is %s' % (x, y, str(pixel)))

    sys.exit(0)
    
if __name__ == '__main__':
    main()
