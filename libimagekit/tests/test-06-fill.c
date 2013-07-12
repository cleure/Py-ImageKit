
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "imagekit.h"
#include "tests/framework.h"

int main(void)
{
    int status;
    REAL color[] = {64, 255, 64, 255};
    REAL colorB[] = {255, 0, 0, 255};
    ImageKit_Image *buf;
    ImageKit_Coords *coords;
    
    coords = ImageKit_Coords_New(64);
    ImageKit_Coords_Append(coords, 16, 16);
    ImageKit_Coords_Append(coords, 17, 17);
    ImageKit_Coords_Append(coords, 18, 18);
    ImageKit_Coords_Append(coords, 19, 19);
    ImageKit_Coords_Append(coords, 20, 20);
    ImageKit_Coords_Append(coords, 21, 21);
    ImageKit_Coords_Append(coords, 22, 22);
    ImageKit_Coords_Append(coords, 23, 23);
    
    buf = ImageKit_Image_FromPNG("/Users/cleure/Downloads/smw-1x.png", -1);
    assert(buf != NULL);
        
    ImageKit_Image_Fill(buf, (REAL *)&color);
    ImageKit_Image_FillCoords(buf, coords, (REAL *)&colorB);
    
    status = ImageKit_Image_SavePNG(buf, "output.png");
    assert(status == 0);
    
    ImageKit_Image_Delete(buf);
    ImageKit_Coords_Delete(coords);

    return 0;
}
