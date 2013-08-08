
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

#include "imagekit.h"
#include "tests/framework.h"

#define BILINEAR(A, B, C, D, Lx, Ly)\
(\
    (A * (1 - Lx) * (1 - Ly))    +\
    (B * (    Lx) * (1 - Ly))    +\
    (C * (    Ly) * (1 - Lx))    +\
    (D * (    Lx) * (    Ly))\
)

int main(void)
{
    int status, i, x, y;
    ImageKit_Image *input, *output;
    REAL horz_xy[] = {
        0.05, 0.05,
        0.3,  0.02,
        0.7,  0.02,
        0.95, 0.05,
    };
    
    REAL vert_xy[] = {
        0.05, 0.05,
        0.04, 0.3,
        0.04, 0.7,
        0.05, 0.95,
    };
    
    ImageKit_Curves *curves_horz = NULL;
    ImageKit_Curves *curves_vert = NULL;
    
    input = ImageKit_Image_FromPNG("/Users/cleure/Downloads/smw-1x.png", -1);
    assert(input != NULL);
    
    output = ImageKit_Image_ScaleBilinear(input, (DIMENSION)input->width*2.3, input->height*2);
    assert(output != NULL);
    
    REAL kernel[] = {
        -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1
    };
    
    ImageKit_Image_ApplyCVKernel(
        output,
        (REAL *)&kernel,
        3,
        1.0,
        0.0,
        1,
        NULL
    );
    
    curves_horz = ImageKit_Curves_FromBezier(output->width*2, (REAL *)&horz_xy, sizeof(horz_xy)/sizeof(horz_xy[0])/2);
    assert(curves_horz != NULL);
    
    curves_vert = ImageKit_Curves_FromBezier(output->height*2, (REAL *)&vert_xy, sizeof(vert_xy)/sizeof(vert_xy[0])/2);
    assert(curves_vert != NULL);
    
    for (i = 0; i < curves_horz->data_items; i++) {
        x = curves_horz->coords[i*2] * (output->width - 1);
        y = curves_horz->coords[i*2+1] * (output->height - 1);
        
        while (y > 0) {
            output->data[PIXEL_INDEX(output, x, y)] = 0;
            output->data[PIXEL_INDEX(output, x, y)+1] = 0;
            output->data[PIXEL_INDEX(output, x, y)+2] = 0;
        
            output->data[PIXEL_INDEX(output, x, output->height - y)] = 0;
            output->data[PIXEL_INDEX(output, x, output->height - y)+1] = 0;
            output->data[PIXEL_INDEX(output, x, output->height - y)+2] = 0;
        
            y--;
        }
        
        y = curves_horz->coords[i*2+1] * (output->height - 1);
        
        if (x < output->width / 2) {
            while (x > 0)  {
                output->data[PIXEL_INDEX(output, x, y)] = 0;
                output->data[PIXEL_INDEX(output, x, y)+1] = 0;
                output->data[PIXEL_INDEX(output, x, y)+2] = 0;
        
                output->data[PIXEL_INDEX(output, x, output->height - y)] = 0;
                output->data[PIXEL_INDEX(output, x, output->height - y)+1] = 0;
                output->data[PIXEL_INDEX(output, x, output->height - y)+2] = 0;
                x--;
            }
        } else {
            while (x < output->width)  {
                output->data[PIXEL_INDEX(output, x, y)] = 0;
                output->data[PIXEL_INDEX(output, x, y)+1] = 0;
                output->data[PIXEL_INDEX(output, x, y)+2] = 0;
        
                output->data[PIXEL_INDEX(output, x, output->height - y)] = 0;
                output->data[PIXEL_INDEX(output, x, output->height - y)+1] = 0;
                output->data[PIXEL_INDEX(output, x, output->height - y)+2] = 0;
                x++;
            }
        }
    }
    
    for (i = 0; i < curves_vert->data_items; i++) {
        x = curves_vert->coords[i*2] * (output->width - 1);
        y = curves_vert->coords[i*2+1] * (output->height - 1);
        
        while (x > 0) {
            output->data[PIXEL_INDEX(output, x, y)] = 0;
            output->data[PIXEL_INDEX(output, x, y)+1] = 0;
            output->data[PIXEL_INDEX(output, x, y)+2] = 0;
        
            output->data[PIXEL_INDEX(output, output->width - x, y)] = 0;
            output->data[PIXEL_INDEX(output, output->width - x, y)+1] = 0;
            output->data[PIXEL_INDEX(output, output->width - x, y)+2] = 0;
            x--;
        }
        
        x = curves_vert->coords[i*2] * (output->width - 1);
        
        if (y < output->height / 2) {
            while (y > 0)  {
                output->data[PIXEL_INDEX(output, x, y)] = 0;
                output->data[PIXEL_INDEX(output, x, y)+1] = 0;
                output->data[PIXEL_INDEX(output, x, y)+2] = 0;
        
                output->data[PIXEL_INDEX(output, output->width - x, y)] = 0;
                output->data[PIXEL_INDEX(output, output->width - x, y)+1] = 0;
                output->data[PIXEL_INDEX(output, output->width - x, y)+2] = 0;
                y--;
            }
        } else {
            while (y < output->height)  {
                output->data[PIXEL_INDEX(output, x, y)] = 0;
                output->data[PIXEL_INDEX(output, x, y)+1] = 0;
                output->data[PIXEL_INDEX(output, x, y)+2] = 0;
        
                output->data[PIXEL_INDEX(output, output->width - x, y)] = 0;
                output->data[PIXEL_INDEX(output, output->width - x, y)+1] = 0;
                output->data[PIXEL_INDEX(output, output->width - x, y)+2] = 0;
                y++;
            }
        }
    }
    
    status = ImageKit_Image_SavePNG(output, "output.png");
    assert(status > 0);
    
    ImageKit_Curves_Delete(curves_horz);
    ImageKit_Curves_Delete(curves_vert);
    ImageKit_Image_Delete(input);
    ImageKit_Image_Delete(output);
    
    return 0;
}
