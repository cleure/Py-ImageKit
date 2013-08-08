
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
    int32_t channels,
    int32_t mid,
    int32_t x,
    int32_t y
)
{
    int32_t c, sx, sy, ex, ey;
    
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
                for (c = 0; c < channels; c++) {
                    result[c] += self->data[PIXEL_INDEX(self, x, y) + c] * (*kernel);
                }
            } else {
                for (c = 0; c < channels; c++) {
                    result[c] += self->data[PIXEL_INDEX(self, sx, sy) + c] * (*kernel);
                }
            }
            
            kernel++;
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
    int32_t preserve_alpha,
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
    int32_t channels;
    int32_t has_alpha;
    int32_t process_alpha_separately = 0;
    int32_t loop_mask;
    
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
    
    channels = self->channels;
    
    /* Process Alpha Separately? */
    has_alpha = (self->channels == 2 || self->channels == 4);
    if (has_alpha && preserve_alpha) {
        process_alpha_separately = 1;
    }
    
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
    
    ptr_out = output;
    mid = kernel_size / 2;
    
    /*
    
    0b11 = (coords, process_alpha_separately)
    
    0x0 == (coords 0, process_alpha_separately 0)
    0x1 == (coords 0, process_alpha_separately 1)
    0x2 == (coords 1, process_alpha_separately 0)
    0x3 == (coords 1, process_alpha_separately 1)
    
    */
    loop_mask = (coords != NULL) << 1 |
                (process_alpha_separately == 1);
    
    #define GET_RESULT_CONVOLUTED()\
                get_result_convolved(\
                    self,\
                    (REAL *)&result,\
                    kernel,\
                    channels,\
                    mid,\
                    x,\
                    y);
    
    #define FACTOR_AND_CLAMP_RESULT()\
                    result[c] = factor * result[c] + bias;\
                    \
                    if (result[c] > max[c]) {\
                        result[c] = max[c];\
                    } else if (result[c] < min[c]) {\
                        result[c] = min[c];\
                    }
    
    if (loop_mask == 0x0) {
        // (coords 0, process_alpha_separately 0)
        
        for (y = 0; y < self->height; y++) {
            for (x = 0; x < self->width; x++) {
            
                /* Get result */
                GET_RESULT_CONVOLUTED();
                
                for (c = 0; c < channels; c++) {
                    /* Factor / Bias / Clamp */
                    FACTOR_AND_CLAMP_RESULT();
                    
                    /* Output */
                    *ptr_out++ = result[c];
                }
            }
        }
    } else if (loop_mask == 0x1) {
        // (coords 0, process_alpha_separately 1)
        
        channels--;
        for (y = 0; y < self->height; y++) {
            for (x = 0; x < self->width; x++) {
            
                /* Get result */
                GET_RESULT_CONVOLUTED();
                
                for (c = 0; c < channels; c++) {
                    /* Factor / Bias / Clamp */
                    FACTOR_AND_CLAMP_RESULT();
                    
                    /* Output */
                    *ptr_out++ = result[c];
                }
                
                *ptr_out++ = self->data[PIXEL_INDEX(self, x, y) + channels];
            }
        }
    } else if (loop_mask == 0x2) {
        // (coords 1, process_alpha_separately 0)
        
        coord_ptr = coords->coords;
        memcpy(output, self->data, self->data_size);
        
        for (i = 0; i < coords->data_index; i++) {
            x = *coord_ptr++;
            y = *coord_ptr++;
            
            /* Get result */
            GET_RESULT_CONVOLUTED();
            
            for (c = 0; c < channels; c++) {
                /* Factor / Bias / Clamp */
                FACTOR_AND_CLAMP_RESULT();
                
                /* Output */
                output[self->pitch * y + x * self->channels + c] = result[c];
            }
        }
    } else if (loop_mask == 0x3) {
        // (coords 1, process_alpha_separately 1)
        
        channels--;
        coord_ptr = coords->coords;
        memcpy(output, self->data, self->data_size);
        
        for (i = 0; i < coords->data_index; i++) {
            x = *coord_ptr++;
            y = *coord_ptr++;
            
            /* Get result */
            GET_RESULT_CONVOLUTED();
            
            for (c = 0; c < channels; c++) {
                /* Factor / Bias / Clamp */
                FACTOR_AND_CLAMP_RESULT();
                
                /* Output */
                output[self->pitch * y + x * self->channels + c] = result[c];
            }
            
            output[self->pitch * y + x * self->channels + channels] =
                self->data[PIXEL_INDEX(self, x, y) + channels];
        }
    }
    
    free(self->data);
    self->data = output;
    return 1;
}
