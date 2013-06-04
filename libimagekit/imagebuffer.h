#pragma once

#include "imagekit.h"

typedef struct ImageBuffer {
    REAL scale;
    REAL channel_scales[4];
    
    int colorspace;
    int colorspace_format;
    
    DIMENSION width;
    DIMENSION height;
    DIMENSION channels;
    DIMENSION pitch;
    size_t data_size;
    size_t data_items;
    
    REAL *data;
};

API
ImageBuffer *
ImageBuffer_New(
    DIMENSION width,
    DIMENSION height,
    DIMENSION channels,
    REAL scale,
    int colorspace,
    int colorspace_format
);

API
void
ImageBuffer_Delete(
    ImageBuffer *self
);

API
ImageBuffer *
ImageBuffer_Clone(
    ImageBuffer *self
);

/*

ImageBuffer_New()
ImageBuffer_Delete()
ImageBuffer_Clone()

*/
