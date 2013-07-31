
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

#include "imagekit.h"
#include "tests/framework.h"

/*
API
int
ImageKit_Image_ApplyRankFilter(
    ImageKit_Image *self,
    DIMENSION matrix_size,
    REAL rank,
    ImageKit_Coords *coords
);

*/

int main(void)
{
    int status;
    ImageKit_Image *buf;
    ImageKit_Coords *coords;
    ImageKit_Rect rect = {32, 32, 128, 128};
    
    buf = ImageKit_Image_FromPNG("/Users/cleure/Downloads/Seattle Skyline.png", -1);
    assert(buf != NULL);
    
    coords = ImageKit_Coords_FromRect(&rect);
    
    ImageKit_Image_ApplyRankFilter(buf, 3, 1.0, coords);
    ImageKit_Image_ApplyRankFilter(buf, 3, 1.0, coords);
    
    status = ImageKit_Image_SavePNG(buf, "output.png");
    assert(status > 0);
    
    ImageKit_Coords_Delete(coords);
    ImageKit_Image_Delete(buf);
    
    return 0;
}
