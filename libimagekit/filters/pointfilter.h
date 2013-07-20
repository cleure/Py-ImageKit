#pragma once

/* Point filter values should be between 0.0 and 1.0 */

typedef struct ImageKit_PointFilter {
    uint16_t channels;
    uint32_t samples;
    
    REAL *a;
    REAL *b;
    REAL *c;
    REAL *d;
} ImageKit_PointFilter;

API
ImageKit_PointFilter *
ImageKit_PointFilter_New(uint16_t channels, uint32_t samples);

API
int
ImageKit_PointFilter_Apply(ImageKit_PointFilter *self, ImageKit_Image *image);

API
void
ImageKit_PointFilter_Delete(ImageKit_PointFilter *self);
