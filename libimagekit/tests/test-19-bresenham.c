
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
    
    DIMENSION xy0[] = {16, 16};
    DIMENSION xy1[] = {512, 64};
    REAL color[] = {0, 0, 0, 255};
    
    buf = ImageKit_Image_FromPNG("/Users/cleure/Downloads/smw-1x.png", -1);
    assert(buf != NULL);
    

    status = ImageKit_Image_DrawBresenhamLine(buf, xy0[0], xy0[1], xy1[0], xy1[1], (REAL *)&color);
    assert(status > 0);
    
    status = ImageKit_Image_SavePNG(buf, "output.png");
    assert(status > 0);
    
    ImageKit_Image_Delete(buf);

    return 0;
}
