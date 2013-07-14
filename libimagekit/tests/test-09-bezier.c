
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "imagekit.h"
#include "tests/framework.h"

/*
typedef struct ImageKit_Bezier {
    DIMENSION data_items;
    REAL *coords;
} ImageKit_Bezier;

API
ImageKit_Bezier *
ImageKit_Bezier_New(uint32_t samples, uint32_t *xy, size_t xy_items);

API
void
ImageKit_Bezier_Delete(ImageKit_Bezier *self);
*/

int main(void)
{
    int status, i;
    ImageKit_Image *buf;
    REAL fill[4] = {0.0, 0.0, 0.0, 255.0};
    uint32_t xy[] = {
        40, 50,
        50, 60,
        60, 0,
        70, 60,
        80, 50,
    };
    
    ImageKit_Bezier *bezier;
    ImageKit_Coords *coords;
    
    buf = ImageKit_Image_FromPNG("/Users/cleure/Downloads/smw-1x.png", -1);
    assert(buf != NULL);
    
    bezier = ImageKit_Bezier_New(100, (uint32_t *)&xy, sizeof(xy)/sizeof(xy[0])/2);
    coords = ImageKit_Coords_New(bezier->data_items);
    
    for (i = 0; i < bezier->data_items; i++) {
        ImageKit_Coords_Append(coords, bezier->coords[i*2], bezier->coords[i*2+1]);
    }
    
    ImageKit_Image_FillCoords(buf, coords, (REAL *)&fill);
    
        /*    
    ImageKit_Image_Fill(out, (REAL *)&fill);
    
    status = ImageKit_Image_BlitRect(out, &dst_rect, buf, &src_rect);
    assert(status == 0);
    
    status = ImageKit_Image_BlitCoords(out, 96, 64, buf, coords);
    assert(status == 0);
    
    status = ImageKit_Image_SavePNG(out, "output.png");
    assert(status == 0);
    
    ImageKit_Coords_Delete(coords);
    ImageKit_Image_Delete(out);
    */
    
    status = ImageKit_Image_SavePNG(buf, "output.png");
    assert(status == 0);
    
    ImageKit_Coords_Delete(coords);
    ImageKit_Bezier_Delete(bezier);
    ImageKit_Image_Delete(buf);

    return 0;
}
