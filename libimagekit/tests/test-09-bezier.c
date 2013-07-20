
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "imagekit.h"
#include "tests/framework.h"

int main(void)
{
    int status, i;
    ImageKit_Image *buf;
    
    REAL fill[4] = {0.0, 0.0, 0.0, 255.0};
    int samples = 400;
    
    REAL xy[] = {
        0.0, 0.0,
        0.3, 0.0,
        0.7, 1.0,
        1.0, 1.0,
    };
    
    ImageKit_Curves *bezier;
    ImageKit_Coords *coords;
    
    buf = ImageKit_Image_FromPNG("/Users/cleure/Downloads/smw-1x.png", -1);
    assert(buf != NULL);
    
    bezier = ImageKit_Curves_FromBezier(samples, (REAL *)&xy, sizeof(xy)/sizeof(xy[0])/2);
    assert(bezier != NULL);
    assert(bezier->coords != NULL);
    assert(bezier->data_items == samples);
    
    coords = ImageKit_Coords_New(bezier->data_items);
    
    for (i = 0; i < bezier->data_items; i++) {
        ImageKit_Coords_Append(coords,
            bezier->coords[i*2] * (buf->width - 1),
            bezier->coords[i*2+1] * (buf->height - 1));
    }
    
    ImageKit_Image_FillCoords(buf, coords, (REAL *)&fill);
    
    status = ImageKit_Image_SavePNG(buf, "output.png");
    assert(status == 0);
    
    ImageKit_Coords_Delete(coords);
    ImageKit_Curves_Delete(bezier);
    ImageKit_Image_Delete(buf);

    return 0;
}
