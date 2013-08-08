
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include "imagekit.h"

#define BILINEAR(A, B, C, D, Lx, Ly)\
(\
    (A * (1 - Lx) * (1 - Ly))    +\
    (B * (    Lx) * (1 - Ly))    +\
    (C * (    Ly) * (1 - Lx))    +\
    (D * (    Lx) * (    Ly))\
)

API
ImageKit_Image *
ImageKit_Image_ScaleBilinear(
    ImageKit_Image *self,
    DIMENSION width,
    DIMENSION height
)
{
    ImageKit_Image *output;
    double x_scale, y_scale;
    double Lx, Ly;
    REAL *A, *B, *C, *D;
    REAL *ptr_out;
    
    DIMENSION x_in, y_in, x_out, y_out, c;
    
    output = ImageKit_Image_New(
        width,
        height,
        self->channels,
        self->scale,
        self->colorspace,
        self->colorspace_format
    );
    
    if (!output) {
        return NULL;
    }
    
    x_scale = (double)(self->width - 1) / (double)width;
    y_scale = (double)(self->height - 1) / (double)height;
    ptr_out = output->data;
    
    for (y_out = 0; y_out < height; y_out++) {
        y_in = (DIMENSION) (y_out * y_scale);
        Ly = (double)(y_out * y_scale - y_in);
        
        for (x_out = 0; x_out < width; x_out++) {
            x_in = (DIMENSION) (x_out * x_scale);
            Lx = (double)(x_out * x_scale - x_in);
            
            A = &self->data[PIXEL_INDEX(self, x_in,   y_in)];
            B = &self->data[PIXEL_INDEX(self, x_in+1, y_in)];
            C = &self->data[PIXEL_INDEX(self, x_in,   y_in+1)];
            D = &self->data[PIXEL_INDEX(self, x_in+1, y_in+1)];
            
            for (c = 0; c < self->channels; c++) {
                *ptr_out++ = BILINEAR((*A++), (*B++), (*C++), (*D++), Lx, Ly);
            }
        }
    }
    
    return output;
}
