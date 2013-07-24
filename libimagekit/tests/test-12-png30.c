
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <png.h>

#include "imagekit.h"
#include "tests/framework.h"

int main(void)
{
    int status;
    ImageKit_Image *buf;
    REAL fill[4] = {0.7, 0.2, 0.5, 1.0};
    
    buf = ImageKit_Image_New(
        640, 480, 3, 1.0,
        CS(RGB), CS_FMT(RGB30)
    );
    
    assert(buf != NULL);
    ImageKit_Image_Fill(buf, (REAL *)&fill);
    
    status = ImageKit_Image_SavePNG(buf, "output.png");
    assert(status > 0);
    
    ImageKit_Image_Delete(buf);
    
    return 0;
}
