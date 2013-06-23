#pragma once

typedef struct ImageKit_Image {
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
} ImageKit_Image;

API
ImageKit_Image *
ImageKit_Image_New(
    DIMENSION width,
    DIMENSION height,
    DIMENSION channels,
    REAL scale,
    int colorspace,
    int colorspace_format
);

API
void
ImageKit_Image_Delete(
    ImageKit_Image *self
);

API
ImageKit_Image *
ImageKit_Image_Clone(
    ImageKit_Image *self
);

API
void
ImageKit_Image_ChannelRanges(
    ImageKit_Image *self,
    REAL *min,
    REAL *max
);

/*

ImageBuffer_New()
ImageBuffer_Delete()
ImageBuffer_Clone()

*/
