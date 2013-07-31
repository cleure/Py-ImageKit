
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

#include "imagekit.h"
#include "tests/framework.h"

int main(void)
{
    int status;
    ImageKit_Image *buf;
    ImageKit_Coords *coords;
    ImageKit_Rect rect = {340, 64, 128, 128};
    REAL kernel1[] = {
        1.0, 1.0, 1.0,
        1.0, 1.0, 1.0,
        1.0, 1.0, 1.0
    };
    
    REAL kernel2[] = {
        -1, -2, -1,
         0,  0,  0,
         1,  2,  1
    };
    
    buf = ImageKit_Image_FromPNG("/Users/cleure/Downloads/Seattle Skyline.png", -1);
    assert(buf != NULL);
    
    coords = ImageKit_Coords_FromRect(&rect);
    
    status = ImageKit_Image_ApplyCVKernel(
        buf,
        (REAL *)&kernel1,
        3,
        1.0/9.0,
        0.0,
        NULL
    );
    
    assert(status > 0);
    
    status = ImageKit_Image_ApplyCVKernel(
        buf,
        (REAL *)&kernel2,
        3,
        1.0,
        0.0,
        coords
    );
    
    assert(status > 0);
    
    status = ImageKit_Image_SavePNG(buf, "output.png");
    assert(status > 0);
    
    ImageKit_Coords_Delete(coords);
    ImageKit_Image_Delete(buf);
    
    return 0;
}
