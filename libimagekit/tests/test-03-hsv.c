
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

#include "imagekit.h"

int main(void)
{
    REAL input[3], output[3];
    ImageKit_Image *buf;
    
    buf = ImageKit_Image_FromJPEG("/Users/cleure/Downloads/xVkohgV.jpg", -1);
    assert(buf != NULL);
    
    input[0] = buf->data[0];
    input[1] = buf->data[1];
    input[2] = buf->data[2];
    
    ImageKit_Image_toHSV(buf);
    ImageKit_Image_toRGB(buf, -1, -1);
    
    output[0] = buf->data[0];
    output[1] = buf->data[1];
    output[2] = buf->data[2];
    
    assert(round(output[0]) == round(input[0]));
    assert(round(output[1]) == round(input[1]));
    assert(round(output[2]) == round(input[2]));
    
    ImageKit_Image_Delete(buf);

    return 0;
}
