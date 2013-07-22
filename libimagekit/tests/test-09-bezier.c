
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "imagekit.h"
#include "tests/framework.h"

void drawAA(ImageKit_Image *image, DIMENSION x, DIMENSION y)
{
    #define set(color, _x, _y)\
        image->data[PIXEL_INDEX(image, (_x) % image->width, (_y) % image->height)] = (image->data[PIXEL_INDEX(image, (_x) % image->width, (_y) % image->height)] + color) / 2;\
        image->data[PIXEL_INDEX(image, (_x) % image->width, (_y) % image->height)+1] = (image->data[PIXEL_INDEX(image, (_x) % image->width, (_y) % image->height)+1] + color) / 2;\
        image->data[PIXEL_INDEX(image, (_x) % image->width, (_y) % image->height)+2] = (image->data[PIXEL_INDEX(image, (_x) % image->width, (_y) % image->height)+2] + color) / 2;
    
    set(0, x, y);
    set(32, x+1, y);
    set(32, x-1, y);
    set(32, x, y+1);
    set(32, x, y-1);
}

int main(void)
{
    int status, i;
    ImageKit_Image *buf;
    int samples = 512;
    
    REAL xy[] = {
        0.0, 0.0,
        0.5, 0.0,
        0.5, 1.0,
        1.0, 1.0,
    };
    
    ImageKit_Curves *bezier;
    
    buf = ImageKit_Image_FromPNG("/Users/cleure/Downloads/smw-1x.png", -1);
    assert(buf != NULL);
    
    bezier = ImageKit_Curves_FromBezier(samples, (REAL *)&xy, sizeof(xy)/sizeof(xy[0])/2);
    assert(bezier != NULL);
    assert(bezier->coords != NULL);
    assert(bezier->data_items == samples);
    
    for (i = 0; i < bezier->data_items; i++) {
        drawAA(
            buf,
            bezier->coords[i*2] * (buf->width - 1),
            bezier->coords[i*2+1] * (buf->height - 1)
        );
    }
    
    status = ImageKit_Image_SavePNG(buf, "output.png");
    assert(status > 0);
    
    ImageKit_Curves_Delete(bezier);
    ImageKit_Image_Delete(buf);

    return 0;
}
