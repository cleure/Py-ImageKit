
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include "imagekit.h"

API
ImageKit_Histogram *
ImageKit_Histogram_New(uint16_t channels, uint16_t samples)
{
    size_t mem_size;
    ImageKit_Histogram *hist;
    
    hist = malloc(sizeof(*hist));
    if (!hist) {
        ImageKit_SetError(ImageKit_MemoryError, "Failed to allocate memory");
        return NULL;
    }
    
    mem_size = sizeof(*(hist->a)) * 4 * samples + 4;
    hist->a = malloc(mem_size);
    if (!hist->a) {
        free(hist);
        ImageKit_SetError(ImageKit_MemoryError, "Failed to allocate memory");
        return NULL;
    }
    
    /* hist->b, hist->c, and hist->d are all pointers to offsets of hist->a */
    hist->b = (uint32_t *)&(hist->a[samples]);
    hist->c = (uint32_t *)&(hist->a[samples*2]);
    hist->d = (uint32_t *)&(hist->a[samples*3]);
    
    hist->samples = samples;
    hist->channels = channels;
    
    /* Zero out memory */
    memset(hist->a, 0, mem_size);
    
    return hist;
}

API
ImageKit_Histogram *
ImageKit_Histogram_FromImage(ImageKit_Image *self, uint16_t samples)
{
    size_t i, l, c, idx;
    ImageKit_Histogram *hist;
    REAL *ptr_in;
    REAL *csfmt;
    REAL scales[4];
    
    csfmt = (REAL *)&COLORSPACE_FORMAT_MINMAX[self->colorspace_format];
    
    if (self->scale > 0) {
        scales[0] = (REAL)(samples - 1) / self->scale;
        scales[1] = (REAL)(samples - 1) / self->scale;
        scales[2] = (REAL)(samples - 1) / self->scale;
        scales[3] = (REAL)(samples - 1) / self->scale;
    } else {
        scales[0] = (REAL)(samples - 1) / csfmt[4];
        scales[1] = (REAL)(samples - 1) / csfmt[5];
        scales[2] = (REAL)(samples - 1) / csfmt[6];
        scales[3] = (REAL)(samples - 1) / csfmt[7];
    }
    
    hist = ImageKit_Histogram_New(self->channels, samples);
    if (!hist) {
        return NULL;
    }
    
    l = self->width * self->height;
    ptr_in = self->data;
    
    for (i = 0; i < l; i++) {
        for (c = 0; c < self->channels; c++) {
            idx = (size_t)((*ptr_in) * scales[c]);
            
            hist->a[samples*c+idx]++;
            ptr_in++;
        }
    }
    
    return hist;
}

API
void
ImageKit_Histogram_Delete(ImageKit_Histogram *self)
{
    if (self) {
        free(self->a);
        free(self);
    }
}
