
import os, sys, imagekit

filters = {
    'blur': {
        'matrix': [ [0.0, 0.2, 0.0],
                    [0.2, 0.0, 0.2],
                    [0.0, 0.2, 0.0]],
        'factor': 1.0,
        'bias': 0.0
    },
    'motion_blur': {
        'matrix': [ [1.0,  0.0,  0.0,  0.0,  0.0],
                    [0.0,  1.0,  0.0,  0.0,  0.0],
                    [0.0,  0.0,  1.0,  0.0,  0.0],
                    [0.0,  0.0,  0.0,  1.0,  0.0],
                    [0.0,  0.0,  0.0,  0.0,  1.0]],
        'factor': 0.18,
        'bias': 0.0
    },
    'find_edges': {
        'matrix': [ [0.0,  0.0,  0.0,  0.0,  0.0],
                    [0.0,  0.0,  0.0,  0.0,  0.0],
                    [0.0,  0.0,  2.0, -1.0, -1.0],
                    [0.0,  0.0,  0.0,  0.0,  0.0],
                    [0.0,  0.0,  0.0,  0.0,  0.0]],
        'factor': 1.0,
        'bias': 0.0
    },
    'sharpen': {
        'matrix': [ [-1.0, -1.0, -1.0, -1.0, -1.0],
                    [-1.0,  2.0,  2.0,  2.0, -1.0],
                    [-1.0,  2.0,  8.0, -1.0, -1.0],
                    [-1.0,  2.0,  2.0,  2.0, -1.0],
                    [-1.0, -1.0, -1.0, -1.0, -1.0]],
        'factor': 0.125,
        'bias': 0.0
    },
    'emboss': {
        'matrix': [ [1.0,  1.0,  0.0],
                    [1.0,  0.0, -1.0],
                    [0.0, -1.0, -1.0]],
        'factor': 1.0,
        'bias': 96.0
    }
}

def main():

    """
class Filter_CVKernel(Filter):

    def __init__(self,
                avg_matrix=[],
                avg_factor=1.0,
                avg_bias=0.0,
                median_size=3,
                method="average"):
    """

    in_file = 'images/image02.png'
    out_file_fmt = 'example4_%s.png'
    input = imagekit.Image.fromPNG(in_file)
    
    for k, v in filters.items():
        print('Applying %s filter to %s' % (k, in_file))
    
        f = imagekit.Filter_CVKernel(
            avg_matrix=v['matrix'],
            avg_factor=v['factor'],
            avg_bias=v['bias']
        )
        
        output = f.apply(input)
        output_file = out_file_fmt % (k)
        output.savePNG(output_file)
        del output
        
        print('Output saved to %s\n' % (output_file))
    
    sys.exit(0)
    
if __name__ == '__main__':
    main()
