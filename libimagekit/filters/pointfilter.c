
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <math.h>

#include "imagekit.h"

API
ImageKit_PointFilter *
ImageKit_PointFilter_New(uint32_t samples)
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
    
    /* Zero out memory */
    memset(filter->a, 0, mem_size);
    
    return filter;
}

API
ImageKit_PointFilter *
ImageKit_PointFilter_FromCurves(
    ImageKit_Curves *curves_a,
    ImageKit_Curves *curves_b,
    ImageKit_Curves *curves_c,
    ImageKit_Curves *curves_d
)
{
    size_t i, c, samples;
    ImageKit_Curves *curves[4] = {curves_a, curves_b, curves_c, curves_d};
    ImageKit_PointFilter *self;
    
    REAL *filters[4];
    uint32_t valid_min, valid_max;
    
    /* Not all channels are required to have arguments, BUT ALL channels that do
       MUST have the same number of samples. */
    valid_min = 0;
    valid_max = 0;
    for (c = 0; c < 4; c++) {
        if (curves[c] != NULL) {
            if (valid_min == 0 || curves[c]->data_items < valid_min) {
                valid_min = curves[c]->data_items;
            }
            
            if (valid_max == 0 || curves[c]->data_items > valid_max) {
                valid_max = curves[c]->data_items;
            }
        }
    }
    
    if (valid_min != valid_max) {
        ImageKit_SetError(
            ImageKit_ArgumentError,
            "Number of samples must be the same for each [Curves] arguemnt");
        return NULL;
    }
    
    if (valid_min == 0) {
        ImageKit_SetError(
            ImageKit_ArgumentError,
            "At least one argument is required to be non-NULL");
        return NULL;
    }
    
    samples = valid_max;
    self = ImageKit_PointFilter_New(samples);
    if (self == NULL) {
        return NULL;
    }
    
    filters[0] = self->a;
    filters[1] = self->b;
    filters[2] = self->c;
    filters[3] = self->d;
    
    for (c = 0; c < 4; c++) {
        if (curves[c] == NULL) {
            for (i = 0; i < samples; i++) {
                filters[c][i] = (REAL)i / (REAL)samples;
            }
        } else {
            for (i = 0; i < samples; i++) {
                filters[c][i] = curves[c]->coords[i*2+1];
            }
        }
    }
    
    return self;
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
        *ptr++ = filter[channel][lk] * ch_scales[channel];
    }
    
    return 1;
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
