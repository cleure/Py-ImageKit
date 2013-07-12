
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

#include "imagekit.h"
#include "tests/framework.h"

int main(void)
{
    ImageKit_Image *buf;
    
    buf = ImageKit_Image_FromJPEG("/Users/cleure/Downloads/xVkohgV.jpg", -1);
    assert(buf != NULL);
    
    ImageKit_Image_toMono(buf);

    ImageKit_Image_SavePNG(buf, "output.png");
    ImageKit_Image_Delete(buf);

    return 0;
}
