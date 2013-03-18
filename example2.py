
import os, sys, imagekit

def main():
    if len(sys.argv) < 3:
        print('Scale image with nearest and bilinear scaling algorithms')
        print('Usage: %s <width> <height>' % (sys.argv[0]))
        sys.exit(1)

    try:
        w = int(sys.argv[1])
        h = int(sys.argv[2])
    except:
        print('X/Y must be integer')
        sys.exit(1)
    
    nearest_out = 'nearest.png'
    linear_out = 'linear.png'
    
    input = imagekit.Image.fromPNG('images/image01.png')
    nearest = imagekit.Scale_NearestNeighbor(input)
    linear = imagekit.Scale_Bilinear(input)
    
    nearest.apply(w, h).savePNG(nearest_out)
    linear.apply(w, h).savePNG(linear_out)
    
    print('Nearest saved to %s' % (nearest_out))
    print('Bilinear saved to %s' % (linear_out))

    sys.exit(0)
    
if __name__ == '__main__':
    main()
