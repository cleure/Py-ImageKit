
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "imagekit.h"

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
