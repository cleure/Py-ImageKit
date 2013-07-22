
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
    ImageKit_PointFilter *filter;
    ImageKit_Curves *bezier, *bezier2;
    
    uint32_t samples = 512;
    REAL xy[] = {
        0.0, 0.0,
        0.3, 0.0,
        0.7, 1.0,
        1.0, 1.0,
    };
    
    buf = ImageKit_Image_FromPNG("/Users/cleure/Downloads/smw-1x.png", -1);
    assert(buf != NULL);
    
    bezier = ImageKit_Curves_FromBezier(samples, (REAL *)&xy, sizeof(xy)/sizeof(xy[0])/2);
    assert(bezier != NULL);
    
    bezier2 = ImageKit_Curves_FromBezier(samples / 2, (REAL *)&xy, sizeof(xy)/sizeof(xy[0])/2);
    assert(bezier != NULL);

    /* Test invlid inputs */
    filter = ImageKit_PointFilter_FromCurves(NULL, NULL, NULL, NULL);
    assert(filter == NULL);
    
    filter = ImageKit_PointFilter_FromCurves(bezier, bezier2, bezier, bezier2);
    assert(filter == NULL);
    
    /* Test valid inputs */
    filter = ImageKit_PointFilter_FromCurves(bezier, bezier, bezier, NULL);
    assert(filter != NULL);
    
    status = ImageKit_PointFilter_Apply(filter, buf);
    assert(status > 0);
    
    status = ImageKit_Image_SavePNG(buf, "output.png");
    assert(status > 0);
    
    ImageKit_Curves_Delete(bezier);
    ImageKit_Curves_Delete(bezier2);
    ImageKit_PointFilter_Delete(filter);
    ImageKit_Image_Delete(buf);

    return 0;
}
