
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <math.h>

#include "imagekit.h"

PRIVATE
INLINE
void
get_result_convolved(
    ImageKit_Image *self,
    REAL *result,
    REAL *kernel,
    int32_t mid,
    int32_t x,
    int32_t y
)
{
    int32_t c, sx, sy, ex, ey, i;
    
    i = 0;
    ex = x + mid;
    ey = y + mid;
    sy = y - mid;
    
    result[0] = 0;
    result[1] = 0;
    result[2] = 0;
    result[3] = 0;
    
    /* Get box */
    while (sy <= ey) {
        sx = x - mid;
        
        while (sx <= ex) {
            if (sx < 0 || sy < 0 || sx >= self->width || sy >= self->height) {
                for (c = 0; c < self->channels; c++) {
                    result[c] += self->data[PIXEL_INDEX(self, x, y) + c] * kernel[i];
                }
            } else {
                for (c = 0; c < self->channels; c++) {
                    result[c] += self->data[PIXEL_INDEX(self, sx, sy) + c] * kernel[i];
                }
            }
            
            i++;
            sx++;
        }
        
        sy++;
    }
}

API
int
ImageKit_Image_ApplyCVKernel(
    ImageKit_Image *self,
    REAL *kernel,
    DIMENSION kernel_size,
    REAL factor,
    REAL bias,
    ImageKit_Coords *coords
)
{
    REAL *csfmt;
    REAL result[4];
    REAL min[4];
    REAL max[4];
    
    REAL *output = NULL;
    REAL *ptr_out;
    DIMENSION *coord_ptr;

    int32_t x, y;
    int32_t c, mid, i;
    
    /* Check errors */
    if (kernel_size < 3) {
        ImageKit_SetError(ImageKit_ValueError, "Minumum size for kernel is 3x3");
        return -1;
    }
    
    if (!(kernel_size % 2)) {
        ImageKit_SetError(ImageKit_ValueError,
            "Kernel must have an odd number of elements (ex: 3x3, 5x5)");
        return -1;
    }
    
    csfmt = (REAL *)&COLORSPACE_FORMAT_MINMAX[self->colorspace_format];
    
    /* Get min/max */
    if (self->scale <= 0.0) {
        for (c = 0; c < self->channels; c++) {
            min[c] = (REAL)csfmt[c];
            max[c] = (REAL)csfmt[c+4];
        }
    } else {
        for (c = 0; c < self->channels; c++) {
            min[c] = (REAL)0.0;
            max[c] = (REAL)self->scale;
        }
    }
    
    output = malloc(
        sizeof(*output) * self->width * self->height * self->channels
    );
    
    if (!output) {
        ImageKit_SetError(ImageKit_MemoryError, "Failed to allocate memory");
        return -1;
    }
    
    /*
get_result_convolved(
    ImageKit_Image *self,
    REAL *result,
    REAL *matrix,
    int32_t mid,
    int32_t x,
    int32_t y
)
    */
    
    ptr_out = output;
    mid = kernel_size / 2;
    
    if (coords == NULL) {
        for (y = 0; y < self->height; y++) {
            for (x = 0; x < self->width; x++) {
            
                /* Get result */
                get_result_convolved(
                    self,
                    (REAL *)&result,
                    kernel,
                    mid,
                    x,
                    y);
                
                for (c = 0; c < self->channels; c++) {
                    /* Apply factor / bias */
                    result[c] = factor * result[c] + bias;
                    
                    /* Clamp */
                    if (result[c] > max[c]) {
                        result[c] = max[c];
                    } else if (result[c] < min[c]) {
                        result[c] = min[c];
                    }
                    
                    /* Output */
                    *ptr_out++ = result[c];
                }
            }
        }
    } else {
        coord_ptr = coords->coords;
        memcpy(output, self->data, self->data_size);
        
        for (i = 0; i < coords->data_index; i++) {
            x = *coord_ptr++;
            y = *coord_ptr++;
            
            /* Get result */
            get_result_convolved(
                self,
                (REAL *)&result,
                kernel,
                mid,
                x,
                y);
            
            for (c = 0; c < self->channels; c++) {
                /* Apply factor / bias */
                result[c] = factor * result[c] + bias;
                    
                /* Clamp */
                if (result[c] > max[c]) {
                    result[c] = max[c];
                } else if (result[c] < min[c]) {
                    result[c] = min[c];
                }
                    
                /* Output */
                output[self->pitch * y + x * self->channels + c] = result[c];
            }
            
        }
    }
    
    free(self->data);
    self->data = output;
    return 1;
}
