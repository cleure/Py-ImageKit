#pragma once

API
int
ImageKit_GetConversionScales(
    REAL scale_out,
    int colorspace_format_out,
    REAL scale_in,
    int colorspace_format_in,
    REAL *scales);

API
int
ImageKit_Image_toHSV(ImageKit_Image *self);

API
int
ImageKit_Image_toRGB(ImageKit_Image *self, int colorspace_format, REAL scale);

API
int
ImageKit_Image_toMono(ImageKit_Image *self);
