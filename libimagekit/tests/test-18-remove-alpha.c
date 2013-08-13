
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "imagekit.h"
#include "tests/framework.h"

int main(void)
{
    int status;
    ImageKit_Image *buf;
    
    REAL kernel[] = {
        -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1
    };
    
    buf = ImageKit_Image_FromPNG("/Users/cleure/Downloads/smw-1x.png", -1);
    assert(buf != NULL);
    assert(buf->channels == 4);
    
    status = ImageKit_Image_RemoveAlpha(buf);
    assert(status > 0);
    assert(buf->channels == 3);
    
    printf("%f %f %f %f\n",
        buf->data[0],
        buf->data[1],
        buf->data[2],
        buf->data[3]
    );
    
    ImageKit_Image_ApplyCVKernel(
        buf,
        (REAL *)&kernel,
        3,
        1.0,
        0.0,
        0,
        NULL
    );
    
    status = ImageKit_Image_SavePNG(buf, "output.png");
    assert(status > 0);
    
    ImageKit_Image_Delete(buf);

    return 0;
}
