
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
ImageKit_Image_FillRect(ImageKit_Image *self, REAL *color, ImageKit_Rect *rect)
{
    REAL *ptr;
    size_t xs, ys, xe, ye, pitch;
    
    ys = 0;
    xe = rect->w * self->channels;
    ye = rect->h;
    
    ptr = &(self->data[PIXEL_INDEX(self, rect->x, rect->y)]);
    pitch = (self->width - rect->w) * self->channels;
    
    for (; ys < ye; ys++) {
        for (xs = 0; xs < xe; xs++) {
            *ptr++ = color[xs % self->channels];
        }
        
        ptr += pitch;
    }
    
    return 1;
}

API
int
ImageKit_Image_FillChannel(ImageKit_Image *self, REAL color, DIMENSION channel)
{
    REAL *ptr;
    size_t i, l;
    
    l = self->width * self->height;
    ptr = (REAL *)&self->data[channel];
    
    for (i = 0; i < l; i++) {
        *ptr = color;
        ptr += self->channels;
    }
    
    return 1;
}

API
int
ImageKit_Image_FillCoords(ImageKit_Image *self, REAL *color, ImageKit_Coords *coords)
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
