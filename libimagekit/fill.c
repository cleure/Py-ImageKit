
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include "imagekit.h"

API
int
ImageKit_Image_Fill(ImageKit_Image *self, REAL *color)
{
    REAL *ptr;
    size_t i, l;
    
    l = self->width * self->height * self->channels;
    ptr = self->data;
    
    for (i = 0; i < l; i++) {
        *ptr++ = color[i % self->channels];
    }
    
    return 1;
}

API
int
ImageKit_Image_FillCoords(ImageKit_Image *self, ImageKit_Coords *coords, REAL *color)
{
    DIMENSION *coord_ptr;
    DIMENSION x, y;
    size_t i, c, idx;
    
    coord_ptr = coords->coords;
    for (i = 0; i < coords->data_index; i++) {
        x = *coord_ptr++;
        y = *coord_ptr++;
        idx = PIXEL_INDEX(self, x, y);
        
        for (c = 0; c < self->channels; c++) {
            self->data[idx++] = color[c];
        }
    }
    
    return 1;
}
