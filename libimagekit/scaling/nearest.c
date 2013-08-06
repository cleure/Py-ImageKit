
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include "imagekit.h"

API
ImageKit_Image *
ImageKit_Image_ScaleNearest(
    ImageKit_Image *self,
    DIMENSION width,
    DIMENSION height
)
{
    ImageKit_Image *output;
    double x_scale, y_scale;
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
        
        for (x_out = 0; x_out < width; x_out++) {
            x_in = (DIMENSION) (x_out * x_scale);
            
            for (c = 0; c < self->channels; c++) {
                *ptr_out++ = self->data[PIXEL_INDEX(self, x_in, y_in) + c];
            }
        }
    }
    
    return output;
}
