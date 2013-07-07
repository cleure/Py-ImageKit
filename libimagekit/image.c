
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include "imagekit.h"

/*

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
};

*/

/*
    
enum {
    ImageKit_NotImplementedError,
    ImageKit_StandardError,
    ImageKit_ValueError,
    ImageKit_TypeError,
    ImageKit_OSError,
    ImageKit_IOError,
    ImageKit_MemoryError,
    IMAGEKIT_NUM_ERRORS
} IMAGEKIT_ERROR;
    
*/

/* Colorspace Formats (Min/Max) */
const REAL COLORSPACE_FORMAT_MINMAX[COLORSPACE_FORMAT_SIZE][8] = {
    {0.0, 0.0, 0.0, 0.0,    31.0,      31.0,       31.0,     0.0},
    {0.0, 0.0, 0.0, 0.0,    31.0,      63.0,       31.0,     0.0},
    {0.0, 0.0, 0.0, 0.0,   255.0,     255.0,      255.0,   255.0},
    {0.0, 0.0, 0.0, 0.0,  1023.0,    1023.0,     1023.0,     0.0},
    {0.0, 0.0, 0.0, 0.0, 65535.0,   65535.0,    65535.0, 65535.0},
    {0.0, 0.0, 0.0, 0.0,   360.0,       1.0,        1.0,     1.0},
    {0.0, 0.0, 0.0, 0.0,     1.0,       0.0,        0.0,     1.0},
};

/**
* Create a new ImageKit_Image instance.
*
* @param    DIMENSION width
* @param    DIMENSION height
* @param    DIMENSION channels
* @param    REAL scale
* @param    int colorspace
* @param    int colorspace_format
*
* @return   NULL on error
**/
API
ImageKit_Image *
ImageKit_Image_New(
    DIMENSION width,
    DIMENSION height,
    DIMENSION channels,
    REAL scale,
    int colorspace,
    int colorspace_format
)
{
    ImageKit_Image *self;
    REAL *data;
    size_t data_size, data_items;
    
    data_items = width * height * channels;
    data_size = data_items * sizeof(*data);
    
    if (channels > 4) {
        ImageKit_SetError(ImageKit_ValueError, "Argument 'channels' cannot exceed value of 4");
        return NULL;
    }
    
    /* Check colorspace format is valid */
    if (colorspace == CS(RGB)) {
        switch (colorspace_format) {
            case CS_FMT(RGB15):
            case CS_FMT(RGB16):
            case CS_FMT(RGB24):
            case CS_FMT(RGB30):
            case CS_FMT(RGB48):
                break;
            default:
                ImageKit_SetError(ImageKit_ValueError, "RGB Colorspace must use RGB format");
                return NULL;
                break;
        }
    } else if (colorspace == CS(HSV) && colorspace_format != CS_FMT(HSV_NATURAL)) {
        ImageKit_SetError(ImageKit_ValueError, "HSV Colorspace must use HSV format");
        return NULL;
    }
    
    /* If colorspace / format invalid, default to RGB24 */
    if (    colorspace < 0 ||
            colorspace > CS(SIZE) ||
            colorspace_format < 0 ||
            colorspace > CS_FMT(SIZE)) {
        
        colorspace = CS(RGB);
        colorspace_format = CS_FMT(RGB24);
    }

    self = malloc(sizeof(*self));
    if (!self) {
        ImageKit_SetError(ImageKit_MemoryError, "Failed to allocate memory");
        return NULL;
    }
    
    data = malloc(sizeof(*data) * width * height * channels);
    if (!data) {
        free(self);
        ImageKit_SetError(ImageKit_MemoryError, "Failed to allocate memory");
        return NULL;
    }
    
    memset(data, 0, data_size);
    
    self->data = data;
    self->data_size = data_size;
    self->data_items = data_items;
    self->width = width;
    self->height = height;
    self->channels = channels;
    self->pitch = width * channels;
    self->colorspace = colorspace;
    self->colorspace_format = colorspace_format;
    self->scale = scale;
    
    /* Set channel scales */
    if (self->scale <= 0.0) {
        self->channel_scales[0] = (REAL)1.0;
        self->channel_scales[1] = (REAL)1.0;
        self->channel_scales[2] = (REAL)1.0;
        self->channel_scales[3] = (REAL)1.0;
    } else {
        self->channel_scales[0] =
            self->scale / (REAL)COLORSPACE_FORMAT_MINMAX[self->colorspace_format][4];
        self->channel_scales[1] =
            self->scale / (REAL)COLORSPACE_FORMAT_MINMAX[self->colorspace_format][5];
        self->channel_scales[2] =
            self->scale / (REAL)COLORSPACE_FORMAT_MINMAX[self->colorspace_format][6];
        self->channel_scales[3] =
            self->scale / (REAL)COLORSPACE_FORMAT_MINMAX[self->colorspace_format][7];
    }
    
    return self;
}

/**
* Delete ImageKit_Image instance.
*
* @param    ImageKit_Image *self
* @return   void
**/
API
void
ImageKit_Image_Delete(ImageKit_Image *self)
{
    if (!self) {
        return;
    }
    
    free(self->data);
    free(self);
}

/**
* Clone ImageKit_Image instance.
*
* @param    ImageKit_Image *self
* @return   NULL on error
**/
API
ImageKit_Image *
ImageKit_Image_Clone(ImageKit_Image *self)
{
    ImageKit_Image *clone;
    
    clone = ImageKit_Image_New(
        self->width,
        self->height,
        self->channels,
        self->scale,
        self->colorspace,
        self->colorspace_format
    );
    
    if (!clone) {
        return NULL;
    }
    
    memcpy(clone->data, self->data, clone->data_size);
    return clone;
}

API
void
ImageKit_Image_ChannelRanges(ImageKit_Image *self, REAL *min, REAL *max)
{
    size_t i;
    REAL *csfmt;
    
    csfmt = (REAL *)&COLORSPACE_FORMAT_MINMAX[self->colorspace_format];
    
    for (i = 0; i < self->channels; i++) {
        min[i] = csfmt[i];
        max[i] = csfmt[i+4];
    }
    
    for (; i < 4; i++) {
        min[i] = 0.0;
        max[i] = 0.0;
    }
}
