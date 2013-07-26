
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "imagekit.h"

/**
* Apply matrix (point filter). Matrix must be the same size as the number of
* channels the image has. Each channel in the image will be multiplied by its
* corresponding value in the matrix array. Eg:
*
*   RGB[255, 255, 255] * matrix[1.0, 0.5, 0.5] = RGB[255, 127, 127]
*
* The coords parameters is optional. If supplied, then only the pixels described
* by "ImageKit_Coords *coords" will be filtered.
*
* @param    ImageKit_Image *self
* @param    REAL *matrix
* @param    ImageKit_Coords *coords
* @return   <= 0 on error
**/
API
int
ImageKit_Image_ApplyMatrix(ImageKit_Image *self, REAL *matrix, ImageKit_Coords *coords)
{
    size_t i, l, c;
    DIMENSION x, y;
    DIMENSION *ptr_xy;
    REAL *ptr;
    
    ptr = self->data;
    
    if (coords == NULL) {
        l = self->width * self->height * self->channels;
        for (i = 0; i < l; i++) {
            *ptr++ *= matrix[i % self->channels];
        }
    } else {
        ptr_xy = coords->coords;
        for (i = 0; i < coords->data_index; i++) {
            x = (*ptr_xy++) % self->width;
            y = (*ptr_xy++) % self->height;
            
            for (c = 0; c < self->channels; c++) {
                ptr[PIXEL_INDEX(self, x, y) + c] *= matrix[c];
            }
        }
    }
    
    return 1;
}
