
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
* @param    ImageKit_Image *self
* @param    REAL *matrix
* @return   <= 0 on error
**/
API
int
ImageKit_Image_ApplyMatrix(ImageKit_Image *self, REAL *matrix)
{
    size_t i, l;
    REAL *ptr;
    
    l = self->width * self->height * self->channels;
    ptr = self->data;
    
    for (i = 0; i < l; i++) {
        *ptr++ *= matrix[i % self->channels];
    }
    
    return 1;
}
