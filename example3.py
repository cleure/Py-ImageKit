
import os, sys, imagekit

def main():

    in_file = 'images/image02.png'
    out_file = 'median.png'

    print('Applying median filter to %s' % (in_file))

    input = imagekit.Image.fromPNG()
    filter = imagekit.Filter_CVKernel(median_size=3, method='median')
    filter.apply(input).savePNG(out_file)
    
    print('Output saved to %s' % (out_file))

    sys.exit(0)
    
if __name__ == '__main__':
    main()
