
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "imagekit.h"
#include "tests/framework.h"

/*
ImageKit_Image_BlitRect(
    ImageKit_Image *dst,
    ImageKit_Image *src,
    ImageKit_Rect *dst_rect,
    ImageKit_Rect *src_rect
*/

int main(void)
{
    int status, i;
    ImageKit_Image *buf, *out;
    ImageKit_Rect src_rect = {0, 0, 64, 64};
    ImageKit_Rect dst_rect = {230, 0, 64, 64};
    REAL fill[4] = {0.0, 0.0, 0.0, 255.0};
    
    ImageKit_Coords *coords;
    coords = ImageKit_Coords_New(512);
    
    for (i = 0; i < 32; i++) {
        ImageKit_Coords_Append(coords, 64+i, 64+i);
        ImageKit_Coords_Append(coords, 64+i, 64-i);
        ImageKit_Coords_Append(coords, 64-i, 64+i);
        ImageKit_Coords_Append(coords, 64-i, 64-i);
    }
    
    DebugMarker marker = {"Debug Point", 0};
    
    DebugMarker_Inc(&marker);
    buf = ImageKit_Image_FromPNG("/Users/cleure/Downloads/smw-1x.png", -1);
    assert(buf != NULL);
    
    DebugMarker_Inc(&marker);
    //out = ImageKit_Image_Clone(buf);
    out = ImageKit_Image_New(
        buf->width,
        buf->height,
        3,
        -1,
        buf->colorspace,
        buf->colorspace_format
    );
    assert(out != NULL);
    
    DebugMarker_Inc(&marker);
    ImageKit_Image_Fill(out, (REAL *)&fill);
    
    DebugMarker_Inc(&marker);
    ImageKit_Image_BlitRect(out, &dst_rect, buf, &src_rect);
    
    ImageKit_Image_BlitCoords(out, 96, 64, buf, coords);
    
    DebugMarker_Inc(&marker);
    status = ImageKit_Image_SavePNG(out, "output.png");
    assert(status == 0);
    
    DebugMarker_Inc(&marker);
    ImageKit_Coords_Delete(coords);
    ImageKit_Image_Delete(out);
    
    DebugMarker_Inc(&marker);
    ImageKit_Image_Delete(buf);
    
    DebugMarker_Inc(&marker);

    return 0;
}
