
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include "imagekit.h"

/* RGB */
PRIVATE
int
rgb_to_hsv(ImageKit_Image *self)
{
    return -1;
}

PRIVATE
int
rgb_to_mono(ImageKit_Image *self)
{
    return -1;
}

/* HSV */
PRIVATE
int
hsv_to_rgb(ImageKit_Image *self, int colorspace_format, REAL scale)
{
    return -1;
}

PRIVATE
int
hsv_to_mono(ImageKit_Image *self)
{
    return -1;
}

/* Mono */
PRIVATE
int
mono_to_rgb(ImageKit_Image *self, int colorspace_format, REAL scale)
{
    return -1;
}

PRIVATE
int
mono_to_hsv(ImageKit_Image *self)
{
    return -1;
}

API
int
ImageKit_Image_toHSV(ImageKit_Image *self)
{
    return -1;
}

API
int
ImageKit_Image_toRGB(ImageKit_Image *self, int colorspace_format, REAL scale)
{
    return -1;
}

API
int
ImageKit_Image_toMono(ImageKit_Image *self)
{
    return -1;
}
