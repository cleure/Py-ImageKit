
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "imagekit.h"

PRIVATE
uint32_t *
PascalsTriangle_GetRow(uint32_t n)
{
    /* Gets Nth row of Pascals Triagle */

    size_t i, l;
    int32_t x, numerator;
    uint32_t *row;
    
    row = malloc(sizeof(*row) * (n + 1));
    if (!row) {
        return NULL;
    }
    
    row[0] = 1;
    x = 1;
    numerator = n;
    
    // Floor division
    l = (n / 2) + 1;
    
    for (i = 1; i < l; i++) {
        x *= numerator--;
        x /= i;
        row[i] = x;
    }
    
    x = n / 2;
    if ((n & 1) == 0) {
        x--;
    }
    
    while (x >= 0) row[i++] = row[x--];
    return row;
}

API
ImageKit_Curves *
ImageKit_Curves_FromBezier(uint32_t samples, REAL *xy, size_t xy_items)
{
    /* Uses Pascals Triangle for close approximation */

    ImageKit_Curves *bezier = NULL;
    REAL *ptr_out;
    REAL t, coef, sumx, sumy;
    uint32_t *pascal_row;
    size_t a, b;

    bezier = malloc(sizeof(*bezier));
    if (!bezier) {
        return NULL;
    }

    bezier->coords = malloc(sizeof(*(bezier->coords)) * samples * 2);
    if (!bezier->coords) {
        free(bezier);
        return NULL;
    }

    pascal_row = PascalsTriangle_GetRow((uint32_t)xy_items - 1);
    if (!pascal_row) {
        free(bezier->coords);
        free(bezier);
        return NULL;
    }
    
    bezier->data_items = samples;
    ptr_out = bezier->coords;
    
    for (a = 0; a < samples; a++) {
        t = (1.0 / (samples - 1)) * a;
        sumx = 0;
        sumy = 0;
        
        for (b = 0; b < xy_items; b++) {
            coef =      pascal_row[b]   *
                        pow(t, b)       *
                        pow(1 - t, xy_items - (b + 1));
            
            sumx += coef * xy[b*2];
            sumy += coef * xy[b*2+1];
        }
        
        *ptr_out++ = sumx;
        *ptr_out++ = sumy;
    }

    free(pascal_row);
    return bezier;
}

API
void
ImageKit_Curves_Delete(ImageKit_Curves *self)
{
    if (self) {
        free(self->coords);
        free(self);
    }
}
