
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
    
    buf = ImageKit_Image_FromPNG("/Users/cleure/Downloads/smw-1x.png", -1);
    assert(buf != NULL);
    
    status = ImageKit_Image_SaveJPEG(buf, "output.jpg", 95);
    assert(status == 0);
    
    ImageKit_Image_Delete(buf);

    return 0;
}
