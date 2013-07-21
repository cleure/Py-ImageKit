#pragma once

/* Point filter using lookup table. Values should be between 0.0 and 1.0 */

typedef struct ImageKit_PointFilter {
    uint32_t samples;
    
    REAL *a;
    REAL *b;
    REAL *c;
    REAL *d;
} ImageKit_PointFilter;

/*

TODO:
    PointFilter_FromCurves()
    PointFilter_Clear()

*/

API
ImageKit_PointFilter *
ImageKit_PointFilter_New(uint32_t samples);

API
ImageKit_PointFilter *
ImageKit_PointFilter_FromCurves(
    ImageKit_Curves *curves_a,
    ImageKit_Curves *curves_b,
    ImageKit_Curves *curves_c,
    ImageKit_Curves *curves_d
);

API
int
ImageKit_PointFilter_Apply(ImageKit_PointFilter *self, ImageKit_Image *image);

API
void
ImageKit_PointFilter_Delete(ImageKit_PointFilter *self);
