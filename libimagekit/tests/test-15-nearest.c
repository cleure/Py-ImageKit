
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

#include "imagekit.h"
#include "tests/framework.h"

int main(void)
{
    int status;
    ImageKit_Image *input, *output;
    
    input = ImageKit_Image_FromPNG("/Users/cleure/Downloads/smw-1x.png", -1);
    assert(input != NULL);
    
    output = ImageKit_Image_ScaleNearest(input, (DIMENSION)input->width*2.3, input->height*2);
    assert(output != NULL);
    
    status = ImageKit_Image_SavePNG(output, "output.png");
    assert(status > 0);
    
    ImageKit_Image_Delete(input);
    ImageKit_Image_Delete(output);
    
    return 0;
}
