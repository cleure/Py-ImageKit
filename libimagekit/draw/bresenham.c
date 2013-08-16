
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "imagekit.h"

API
int
ImageKit_Image_DrawBresenhamLine(
    ImageKit_Image *self,
    DIMENSION _x0,
    DIMENSION _y0,
    DIMENSION _x1,
    DIMENSION _y1,
    REAL *color
)
{
    int32_t dx, dy;
    int32_t sx, sy;
    int32_t x0, y0;
    int32_t x1, y1;
    int32_t err, e2;
    int32_t res1, res2;
    int32_t c;
    
    REAL *ptr;
    
    x0 = _x0;
    x1 = _x1;
    y0 = _y0;
    y1 = _y1;
    
    dx = abs(x1 - x0);
    dy = abs(y1 - y0);
    
    sx = (x0 < x1) ? 1 : -1;
    sy = (y0 < y1) ? 1 : -1;
    
    err = dx - dy;
    while (1) {
        ptr = &(self->data[PIXEL_INDEX(self, (x0 % self->width), (y0 % self->height))]);
        for (c = 0; c < self->channels; c++) {
            ptr[c] = color[c];
        }

        if (x0 == x1 && y0 == y1) {
            break;
        }
        
        e2 = 2 * err;
        
        /*
        Optimized for minimal branching. Same as:
            if (e2 > -dy) {
                err = err - dy;
                x0 = x0 + sx;
            }
            
            if (e2 < dx) {
                err = err + dx;
                y0 = y0 + sy;
            }
        */
        res1 = e2 > -dy;
        res2 = e2 < dx;
        
        err = (err - (dy * res1)) + (dx * res2);
        x0 = x0 + (sx * res1);
        y0 = y0 + (sy * res2);
    }
    
    return 1;
}
