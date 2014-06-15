
from imagekit import *

def scale_bilinear(input, width, height):
    output = ImageBuffer(width, height, input.channels)
    in_width = input.width
    in_height = input.height
    
    x_scale = float(in_width - 1) / width
    y_scale = float(in_height - 1) / height
    
    for y in range(height):
        y_in = int(y * y_scale)
        Ly = y * y_scale - y_in
        
        for x in range(width):
            x_in = int(x * x_scale)
            Lx = x * x_scale - x_in
            
            A = list(input.get_pixel(x_in, y_in))
            B = input.get_pixel(x_in+1, y_in)
            C = input.get_pixel(x_in, y_in+1)
            D = input.get_pixel(x_in+1, y_in+1)
            
            for i in range(len(A)):
                A[i] = (
                        (A[i] * (1 - Lx) * (1 - Ly))    +
                        (B[i] * (    Lx) * (1 - Ly))    +
                        (C[i] * (    Ly) * (1 - Lx))    +
                        (D[i] * (    Lx) * (    Ly))
                )
            
            output.set_pixel(x, y, A)
    return output

def main():
    b = ImageBuffer.from_png('images/image01.png')
    output = scale_bilinear(b, b.width*2, b.height*2)
    output.save_png('example2-output.png')

if __name__ == '__main__':
    main()
