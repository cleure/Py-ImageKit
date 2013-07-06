
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "imagekit.h"

int main(void)
{
    int status;
    ImageKit_Image *buf;
    
    buf = ImageKit_Image_FromJPEG("/Users/cleure/Downloads/xVkohgV.jpg", -1);
    assert(buf != NULL);
    
    status = ImageKit_Image_SavePNG(buf, "output.png");
    assert(status != 0);
    
    ImageKit_Image_Delete(buf);

    return 0;
}