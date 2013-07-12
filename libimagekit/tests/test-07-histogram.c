
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "imagekit.h"
#include "tests/framework.h"

int main(void)
{
    ImageKit_Image *buf;
    ImageKit_Histogram *hist;
    
    buf = ImageKit_Image_FromPNG("/Users/cleure/Downloads/smw-1x.png", -1);
    assert(buf != NULL);
    
    hist = ImageKit_Histogram_FromImage(buf, 16);
    assert(hist != NULL);
    
    // Alpha should be 255 for every pixel
    assert(hist->d[15] == buf->width * buf->height);
    
    ImageKit_Histogram_Delete(hist);
    ImageKit_Image_Delete(buf);

    return 0;
}
