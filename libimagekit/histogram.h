#pragma once

typedef struct ImageKit_Histogram {
    uint16_t channels;
    uint16_t samples;
    
    uint32_t *a;
    uint32_t *b;
    uint32_t *c;
    uint32_t *d;
} ImageKit_Histogram;

API
ImageKit_Histogram *
ImageKit_Histogram_New(uint16_t channels, uint16_t samples);

API
ImageKit_Histogram *
ImageKit_Histogram_FromImage(ImageKit_Image *self, uint16_t samples);

API
void
ImageKit_Histogram_Delete(ImageKit_Histogram *self);
