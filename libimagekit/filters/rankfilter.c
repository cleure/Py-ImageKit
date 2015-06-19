
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include "imagekit.h"

#define QUICK_SELECT_TYPE double
#define QUICK_SELECT_SYMBOL(sym) sym##_real
#define QUICK_SELECT_LINKAGE PRIVATE INLINE
    #include "quickselect.h"
#undef QUICK_SELECT_TYPE
#undef QUICK_SELECT_SYMBOL
#undef QUICK_SELECT_LINKAGE

PRIVATE
INLINE
void
fill_matrix_box(
    ImageKit_Image *self,
    REAL *rank_matrix,
    int32_t matrix_elements,
    int32_t mid,
    int32_t x,
    int32_t y
)
{
    int32_t i,
            c,
            sx,
            sy,
            ex,
            ey;
    
    i = 0;
    ex = x + mid;
    ey = y + mid;
    sy = y - mid;
    
    /* Get box */
    while (sy <= ey) {
        sx = x - mid;
        
        while (sx <= ex) {
        
            /* Fill Matrix, checking bounds */
            if (sx < 0 || sy < 0 || sx >= self->width || sy >= self->height) {
                for (c = 0; c < self->channels; c++) {
                    rank_matrix[matrix_elements*c+i] = self->data[PIXEL_INDEX(self, x, y) + c];
                }
            } else {
                for (c = 0; c < self->channels; c++) {
                    rank_matrix[matrix_elements*c+i] = self->data[PIXEL_INDEX(self, sx, sy) + c];
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
ImageKit_Image_ApplyRankFilter(
    ImageKit_Image *self,
    DIMENSION matrix_size,
    REAL rank,
    ImageKit_Coords *coords
)
{
    size_t x, y;

    REAL *csfmt;
    REAL min[4];
    REAL max[4];
    REAL *rank_matrix;
    REAL *matrix_ptr;
    REAL *output;
    REAL *ptr_out;
    DIMENSION *coord_ptr;
    
    int32_t matrix_mid, matrix_elements;
    int32_t c, mid, i;
    
    /*
    
    FIXME: int32_t preserve_alpha option, for processing alpha channels
    
    */
    
    if (matrix_size < 3) {
        ImageKit_SetError(ImageKit_ValueError, "Matrix size must be a minimum of 3");
        return -1;
    }
    
    if ((matrix_size % 2) == 0) {
        ImageKit_SetError(ImageKit_ValueError, "Matrix size must be odd value");
        return -1;
    }
    
    if (rank < 0.0 || rank > 1.0) {
        ImageKit_SetError(ImageKit_ValueError, "Rank must be between 0.0 and 1.0");
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
    
    rank_matrix = malloc(
        sizeof(*rank_matrix) * matrix_size * matrix_size * self->channels
    );
    
    if (!rank_matrix) {
        ImageKit_SetError(ImageKit_MemoryError, "Failed to allocate memory");
        return -1;
    }
    
    output = malloc(
        sizeof(*output) * self->width * self->height * self->channels
    );
    
    if (!output) {
        free(rank_matrix);
        ImageKit_SetError(ImageKit_MemoryError, "Failed to allocate memory");
        return -1;
    }
    
    mid = matrix_size / 2;
    matrix_elements = matrix_size * matrix_size;
    matrix_mid = (int32_t)(matrix_elements * rank);
    
    /* Clamp Rank */
    if (matrix_mid > (matrix_elements) - self->channels) {
        matrix_mid = (matrix_elements) - self->channels;
    }
    
    if (coords == NULL) {
        ptr_out = output;
        for (y = 0; y < self->height; y++) {
            for (x = 0; x < self->width; x++) {
                /* Fill rank matrix */
                fill_matrix_box(self, rank_matrix, matrix_elements, mid, (int32_t)x, (int32_t)y);
                
                /* Output */
                for (c = 0; c < self->channels; c++) {
                    /* Get ptr offset */
                    matrix_ptr = (rank_matrix+(c*matrix_elements));
                    
                    /* QuickSelect Algorithm */
                    matrix_ptr[0] =
                        quick_select_real(matrix_ptr, 0, matrix_elements-1, matrix_mid);
                
                    /* Clamp */
                    if (matrix_ptr[0] > max[c]) {
                        matrix_ptr[0] = max[c];
                    } else if (matrix_ptr[0] < min[c]) {
                        matrix_ptr[0] = min[c];
                    }
                    
                    /* Output */
                    *ptr_out++ = matrix_ptr[0];
                }
            }
        }
    } else {
        coord_ptr = coords->coords;
        memcpy(output, self->data, self->data_size);
        
        for (i = 0; i < coords->data_index; i++) {
            x = *coord_ptr++;
            y = *coord_ptr++;
            
            /* Fill rank matrix */
            fill_matrix_box(self, rank_matrix, matrix_elements, mid, (int32_t)x, (int32_t)y);
            
            for (c = 0; c < self->channels; c++) {
                /* Get ptr offset */
                matrix_ptr = (rank_matrix+(c*matrix_elements));
                    
                /* QuickSelect Algorithm */
                matrix_ptr[0] =
                    quick_select_real(matrix_ptr, 0, matrix_elements-1, matrix_mid);
                
                /* Clamp */
                if (matrix_ptr[0] > max[c]) {
                    matrix_ptr[0] = max[c];
                } else if (matrix_ptr[0] < min[c]) {
                    matrix_ptr[0] = min[c];
                }
                
                /* Output */
                output[(self->pitch * y) + (x * self->channels) + c] = matrix_ptr[0];
            }
        }
    }

    free(self->data);
    self->data = output;
    free(rank_matrix);
    
    return 1;
}
