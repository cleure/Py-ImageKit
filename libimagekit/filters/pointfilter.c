
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <math.h>

#include "imagekit.h"

API
ImageKit_PointFilter *
ImageKit_PointFilter_New(uint16_t channels, uint32_t samples)
{
    size_t mem_size;
    ImageKit_PointFilter *filter;
    
    filter = malloc(sizeof(*filter));
    if (!filter) {
        ImageKit_SetError(ImageKit_MemoryError, "Failed to allocate memory");
        return NULL;
    }
    
    mem_size = sizeof(*(filter->a)) * 4 * samples + 4;
    filter->a = malloc(mem_size);
    if (!filter->a) {
        free(filter);
        ImageKit_SetError(ImageKit_MemoryError, "Failed to allocate memory");
        return NULL;
    }
    
    /* b/c/d are all pointers to offsets of filter->a */
    filter->b = (REAL *)&(filter->a[samples]);
    filter->c = (REAL *)&(filter->a[samples*2]);
    filter->d = (REAL *)&(filter->a[samples*3]);
    
    filter->samples = samples;
    filter->channels = channels;
    
    /* Zero out memory */
    memset(filter->a, 0, mem_size);
    
    return filter;
}

API
int
ImageKit_PointFilter_Apply(ImageKit_PointFilter *self, ImageKit_Image *image)
{
    size_t i, l;
    uint32_t lk, channel;
    REAL *ptr;
    REAL *filter[4] = {self->a, self->b, self->c, self->d};
    REAL lk_scales[4];
    REAL ch_scales[4];
    REAL *csfmt;
    
    csfmt = (REAL *)&COLORSPACE_FORMAT_MINMAX[image->colorspace_format];
    
    /* Lookup scales */
    lk_scales[0] = (REAL)(self->samples - 1) / (csfmt[4] * image->channel_scales[0]);
    lk_scales[1] = (REAL)(self->samples - 1) / (csfmt[5] * image->channel_scales[1]);
    lk_scales[2] = (REAL)(self->samples - 1) / (csfmt[6] * image->channel_scales[2]);
    lk_scales[3] = (REAL)(self->samples - 1) / (csfmt[7] * image->channel_scales[3]);
    
    /* Channel scales */
    ch_scales[0] = csfmt[4] * (image->channel_scales[0]);
    ch_scales[1] = csfmt[5] * (image->channel_scales[1]);
    ch_scales[2] = csfmt[6] * (image->channel_scales[2]);
    ch_scales[3] = csfmt[7] * (image->channel_scales[3]);
    
    l = image->width * image->height * image->channels;
    ptr = image->data;
    
    for (i = 0; i < l; i++) {
        channel = i % image->channels;
        lk = (uint32_t)round((*ptr) * lk_scales[channel]);
        *ptr = filter[channel][lk] * ch_scales[channel];
        ptr++;
    }
    
    return 0;
}

API
void
ImageKit_PointFilter_Delete(ImageKit_PointFilter *self)
{
    if (self) {
        free(self->a);
        free(self);
    }
}