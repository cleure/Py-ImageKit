
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "imagekit.h"

#define ADD_POINT(_x, _y)\
    ptr = &(self->data[PIXEL_INDEX(self, ((_x) % self->width), ((_y) % self->height))]);\
    for (c = 0; c < self->channels; c++) {\
        ptr[c] = color[c];\
    }

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
        ADD_POINT(x0, y0);
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

API
int
ImageKit_Image_DrawBresenhamCircle(
    ImageKit_Image *self,
    DIMENSION midpoint_x,
    DIMENSION midpoint_y,
    DIMENSION radius,
    REAL *color
)
{
    int32_t f, dx, dy, sx, sy, x, y, r;
    int32_t c;
    REAL *ptr;

    r = radius;
    sx = midpoint_x;
    sy = midpoint_y;

    f = 1 - r;
    dx = 1;
    dy = -2 * r;
    x = 0;
    y = r;
    
    ADD_POINT(sx,     sy + r);
    ADD_POINT(sx,     sy - r);
    ADD_POINT(sx + r, sy);
    ADD_POINT(sx - r, sy);
    
    while (x < y) {
        if (f >= 0) {
            y -= 1;
            dy += 2;
            f += dy;
        }
        
        x += 1;
        dx += 2;
        f += dx;
        
        ADD_POINT(sx + x, sy + y);
        ADD_POINT(sx - x, sy + y);
        ADD_POINT(sx + x, sy - y);
        ADD_POINT(sx - x, sy - y);
        ADD_POINT(sx + y, sy + x);
        ADD_POINT(sx - y, sy + x);
        ADD_POINT(sx + y, sy - x);
        ADD_POINT(sx - y, sy - x);
    }

    return 1;
}

API
int
ImageKit_Image_DrawBresenhamEllipse(
    ImageKit_Image *self,
    DIMENSION _x0,
    DIMENSION _y0,
    DIMENSION _x1,
    DIMENSION _y1,
    REAL *color
)
{
    int32_t a, b, b1, x0, y0, x1, y1, c;
    int64_t dx, dy, err, e2;
    
    REAL *ptr;

    x0 = _x0;
    x1 = _x1;
    y0 = _y0;
    y1 = _y1;

    a = abs(x1 - x0);
    b = abs(y1 - y0);
    b1 = b & 1;
    
    dx = 4 * ( 1 - a) * b * b;
    dy = 4 * (b1 + 1) * a * a;
    err = dx + dy + b1 * a * a;
    
    if (x0 > x1) {
        x0 = x1;
        x1 += a;
    }
    
    if (y0 > y1) {
        y0 = y1;
    }
    
    y0 += (b + 1) / 2;
    y1 = y0 - b1;
    a *= 8 * a;
    b1 = 8 * b * b;

    do {
       ADD_POINT(x1, y0);
       ADD_POINT(x0, y0);
       ADD_POINT(x0, y1);
       ADD_POINT(x1, y1);
       
       e2 = 2 * err;
       if (e2 <= dy) {
            // y step
            y0++;
            y1--;
            err += dy += a;
        }
        
        if (e2 >= dx || 2 * err > dy) {
            // x step
            x0++;
            x1--;
            err += dx += b1;
        }
    } while (x0 <= x1);
    
    while (y0 - y1 < b) {
        ADD_POINT(x0 - 1, y0);
        ADD_POINT(x1 + 1, y0);
        ADD_POINT(x0 - 1, y1);
        ADD_POINT(x1 + 1, y1);
        
        y0++;
        y1--;
    }

    return 1;
}
