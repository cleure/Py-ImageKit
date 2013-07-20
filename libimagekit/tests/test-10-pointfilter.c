
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "imagekit.h"
#include "tests/framework.h"

int main(void)
{
    int status, i;
    ImageKit_Image *buf;
    ImageKit_PointFilter *filter;
    uint32_t samples = 512;
    REAL value;
    
    buf = ImageKit_Image_FromPNG("/Users/cleure/Downloads/smw-1x.png", -1);
    assert(buf != NULL);
    
    filter = ImageKit_PointFilter_New(buf->channels, samples);
    assert(filter != NULL);
    
    for (i = 0; i < samples; i++) {
        value = (REAL)i / (REAL)samples;
        filter->a[i] = value - (value / 4);
        filter->b[i] = value - (value / 4);
        filter->c[i] = value - (value / 4);
        filter->d[i] = value - (value / 4);
    }
    
    status = ImageKit_PointFilter_Apply(filter, buf);
    assert(status == 0);
    
    status = ImageKit_Image_SavePNG(buf, "output.png");
    assert(status == 0);
    
    ImageKit_PointFilter_Delete(filter);
    ImageKit_Image_Delete(buf);

    return 0;
}
