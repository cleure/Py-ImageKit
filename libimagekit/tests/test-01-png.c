
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "imagekit.h"

int main(void)
{
    size_t i, l;
    ImageKit_Image *buf;
    REAL min[4], max[4];
    
    buf = ImageKit_Image_FromPNG("/Users/cleure/Downloads/skyvase48.png");
    assert(buf != NULL);
    
    ImageKit_Image_SavePNG(buf, "output.png");
    ImageKit_Image_Delete(buf);
    
    assert(1 == 0);

    return 0;
}
