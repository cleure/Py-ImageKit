
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <math.h>
#include "imagekit.h"

/* RGB */
PRIVATE
int
rgb_to_hsv(ImageKit_Image *self, int colorspace_format, REAL scale)
{
    REAL scale_r, scale_g, scale_b, scale_a;
    REAL rgb[4];
    REAL hue, saturation;
    REAL mn, mx, df;
    REAL *ptr;
    REAL *csfmt;
    size_t i, l;
    
    csfmt = (REAL *)&COLORSPACE_FORMAT_MINMAX[self->colorspace_format];

    /* Get Scales */
    if (self->scale <= 0.0) {
        scale_r = (REAL)1.0 / (REAL)csfmt[4];
        scale_g = (REAL)1.0 / (REAL)csfmt[5];
        scale_b = (REAL)1.0 / (REAL)csfmt[6];
        scale_a = (REAL)1.0 / (REAL)csfmt[7];
    } else {
        scale_r = (REAL)1.0 / self->scale;
        scale_g = (REAL)1.0 / self->scale;
        scale_b = (REAL)1.0 / self->scale;
        scale_a = (REAL)1.0 / self->scale;
    }

    l = self->width * self->height;
    ptr = (REAL *)&(self->data[0]);

    for (i = 0; i < l; i++) {
        rgb[0] = (*(ptr  ) * scale_r);
        rgb[1] = (*(ptr+1) * scale_g);
        rgb[2] = (*(ptr+2) * scale_b);
        
        /* MIN / MAX */
        mn = rgb[0];
        mx = rgb[0];
        if (mn > rgb[1]) mn = rgb[1];
        if (mn > rgb[2]) mn = rgb[2];
        if (mx < rgb[1]) mx = rgb[1];
        if (mx < rgb[2]) mx = rgb[2];
        
        /* Diff */
        df = mx - mn;
        
        /* Hue */
        if (mx == mn)           hue =  (REAL)0.0;
        else if (mx == rgb[0])  hue = ((REAL)60.0 * ((rgb[1] - rgb[2]) / df) + 360);
        else if (mx == rgb[1])  hue = ((REAL)60.0 * ((rgb[2] - rgb[0]) / df) + 120);
        else if (mx == rgb[2])  hue = ((REAL)60.0 * ((rgb[0] - rgb[1]) / df) + 240);
        
        /* Clamp */
        if (hue >= 360) hue -= 360;
        
        /* Saturation */
        if (mx == 0) {
            saturation = (REAL)0.0;
        } else {
            saturation = df / mx;
        }
        
        *(ptr  ) = hue;
        *(ptr+1) = saturation;
        *(ptr+2) = mx;
        
        if (self->channels > 3) {
            *(ptr+3) *= scale_a;
        }
        
        ptr += self->channels;
    }

    self->colorspace = CS(HSV);
    self->colorspace_format = CS_FMT(HSV_NATURAL);
    self->scale = -1;
    
    /* HSV is never scaled */
    self->channel_scales[0] = (REAL)1.0;
    self->channel_scales[1] = (REAL)1.0;
    self->channel_scales[2] = (REAL)1.0;
    self->channel_scales[3] = (REAL)1.0;

    return 0;
}

PRIVATE
int
rgb_to_mono(ImageKit_Image *self, int colorspace_format, REAL scale)
{
    REAL *ptr_in, *ptr_out;
    REAL value;
    REAL scales[4];
    REAL *csfmt;
    size_t i, l;
    int out_channels = 1;
    
    colorspace_format = CS_FMT(MONO_NATURAL);
    if (self->channels == 4) {
        out_channels = 2;
    }
    
    csfmt = (REAL *)&COLORSPACE_FORMAT_MINMAX[self->colorspace_format];

    if (self->scale > 0.0) {
        scales[0] = self->scale / (REAL)1.0;
        scales[1] = self->scale / (REAL)1.0;
        scales[2] = self->scale / (REAL)1.0;
        scales[3] = self->scale / (REAL)1.0;
    } else {
        scales[0] = (REAL)1.0 / (REAL)csfmt[4];
        scales[1] = (REAL)1.0 / (REAL)csfmt[5];
        scales[2] = (REAL)1.0 / (REAL)csfmt[6];
        scales[3] = (REAL)1.0 / (REAL)csfmt[7];
    }
    
    l = self->width * self->height;
    ptr_in = &(self->data[0]);
    ptr_out = &(self->data[0]);

    if (out_channels == 2) {
        for (i = 0; i < l; i++) {
            value =  ((*ptr_in) * scales[0]) * (REAL)0.299; ptr_in++;
            value += ((*ptr_in) * scales[1]) * (REAL)0.587; ptr_in++;
            value += ((*ptr_in) * scales[2]) * (REAL)0.114; ptr_in++;
            
            *ptr_out++ = value;
            *ptr_out++ = (*ptr_in) * scales[3];
            ptr_in++;
        }
    } else {
        for (i = 0; i < l; i++) {
            value =  ((*ptr_in) * scales[0]) * (REAL)0.299; ptr_in++;
            value += ((*ptr_in) * scales[1]) * (REAL)0.587; ptr_in++;
            value += ((*ptr_in) * scales[2]) * (REAL)0.114; ptr_in++;
            
            *ptr_out++ = value;
        }
    }
    
    self->colorspace = CS(MONO);
    self->colorspace_format = colorspace_format;
    self->channels = out_channels;
    self->scale = (REAL)1.0;
    self->pitch = self->width * self->channels;
    self->data_items = self->width * self->height * self->channels;
    
    self->channel_scales[0] = (REAL)1.0;
    self->channel_scales[1] = (REAL)1.0;
    self->channel_scales[2] = (REAL)1.0;
    self->channel_scales[3] = (REAL)1.0;

    return 0;
}

/* HSV */
PRIVATE
int
hsv_to_rgb(ImageKit_Image *self, int colorspace_format, REAL scale)
{
    REAL hsv[4];
    REAL rgb[4];
    REAL scale_r, scale_g, scale_b, scale_a;
    REAL *ptr;
    REAL *csfmt;
    
    REAL h60, h60f, f, p, q, t;
    uint32_t hi;
    
    size_t i, l;
    
    if (colorspace_format < 0) {
        colorspace_format = CS_FMT(RGB24);
    }
    
    csfmt = (REAL *)&COLORSPACE_FORMAT_MINMAX[colorspace_format];
    
    if (scale <= 0.0) {
        scale_r = (REAL)csfmt[4];
        scale_g = (REAL)csfmt[5];
        scale_b = (REAL)csfmt[6];
        scale_a = (REAL)csfmt[7];
    } else {
        scale_r = (REAL)scale;
        scale_g = (REAL)scale;
        scale_b = (REAL)scale;
        scale_a = (REAL)scale;
    }
    
    l = self->width * self->height;
    ptr = (REAL *)&(self->data[0]);

    for (i = 0; i < l; i++) {
        hsv[0] = *(ptr  );
        hsv[1] = *(ptr+1);
        hsv[2] = *(ptr+2);
        
        h60 = hsv[0] / (REAL)60.0;
        h60f = (REAL)floor(h60);
        hi = ((uint32_t)h60f) % 6;
        
        f = h60 - h60f;
        p = hsv[2] * (1 - hsv[1]);
        q = hsv[2] * (1 - f * hsv[1]);
        t = hsv[2] * (1 - (1 - f) * hsv[1]);
        
        if (hi == 0) {
            rgb[0] = hsv[2];
            rgb[1] = t;
            rgb[2] = p;
        } else if (hi == 1) {
            rgb[0] = q;
            rgb[1] = hsv[2];
            rgb[2] = p;
        } else if (hi == 2) {
            rgb[0] = p;
            rgb[1] = hsv[2];
            rgb[2] = t;
        } else if (hi == 3) {
            rgb[0] = p;
            rgb[1] = q;
            rgb[2] = hsv[2];
        } else if (hi == 4) {
            rgb[0] = t;
            rgb[1] = p;
            rgb[2] = hsv[2];
        } else if (hi == 5) {
            rgb[0] = hsv[2];
            rgb[1] = p;
            rgb[2] = q;
        }
        
        *(ptr  ) = rgb[0] * scale_r;
        *(ptr+1) = rgb[1] * scale_g;
        *(ptr+2) = rgb[2] * scale_b;
        
        if (self->channels > 3) {
            *(ptr+3) *= scale_a;
        }
        
        ptr += self->channels;
    }
    
    self->colorspace = CS(RGB);
    self->colorspace_format = colorspace_format;
    self->scale = scale;
    
    if (scale <= 0.0) {
        self->channel_scales[0] = (REAL)1.0;
        self->channel_scales[1] = (REAL)1.0;
        self->channel_scales[2] = (REAL)1.0;
        self->channel_scales[3] = (REAL)1.0;
    } else {
        self->channel_scales[0] = (REAL)scale / (REAL)csfmt[4];
        self->channel_scales[1] = (REAL)scale / (REAL)csfmt[5];
        self->channel_scales[2] = (REAL)scale / (REAL)csfmt[6];
        self->channel_scales[3] = (REAL)scale / (REAL)csfmt[7];
    }

    return 0;
}

PRIVATE
int
hsv_to_mono(ImageKit_Image *self, int colorspace_format, REAL scale)
{
    /* Convert to RGB, then convert RGB to Mono (more accurate than desaturating) */
    if (ImageKit_Image_toRGB(self, -1, -1) < 0) {
        return -1;
    }
    
    if (ImageKit_Image_toMono(self) < 0) {
        return -1;
    }
    
    return 0;
}

/* Mono */
PRIVATE
int
mono_to_rgb(ImageKit_Image *self, int colorspace_format, REAL scale)
{
    REAL *ptr_in, *ptr_out;
    REAL *buffer;
    REAL scale_r, scale_g, scale_b, scale_a;
    REAL *csfmt;
    
    int channels_out = 3;
    size_t i, l;
    size_t bitems, bsize;
    
    if (colorspace_format < 0) {
        colorspace_format = CS_FMT(RGB24);
    }
    
    csfmt = (REAL *)&COLORSPACE_FORMAT_MINMAX[colorspace_format];
    
    if (self->channels == 2) {
        self->channels = 4;
    }
    
    if (scale <= 0.0) {
        scale_r = (REAL)csfmt[4];
        scale_g = (REAL)csfmt[5];
        scale_b = (REAL)csfmt[6];
        scale_a = (REAL)csfmt[7];
    } else {
        scale_r = (REAL)scale;
        scale_g = (REAL)scale;
        scale_b = (REAL)scale;
        scale_a = (REAL)scale;
    }
    
    bitems = self->width * self->height * channels_out;
    bsize = bitems * sizeof(REAL);
    
    buffer = malloc(bsize);
    if (!buffer) {
        ImageKit_SetError(ImageKit_MemoryError, "Unable to allocate memory");
        return -1;
    }
    
    l = self->width * self->height;
    ptr_in = (REAL *)&(self->data[0]);
    ptr_out = (REAL *)buffer;
    
    if (channels_out == 3) {
        for (i = 0; i < l; i++) {
            *ptr_out++ = (*ptr_in) * scale_r;
            *ptr_out++ = (*ptr_in) * scale_g;
            *ptr_out++ = (*ptr_in) * scale_b;
            ptr_in++;
        }
    } else {
        for (i = 0; i < l; i++) {
            *ptr_out++ = (*ptr_in) * scale_r;
            *ptr_out++ = (*ptr_in) * scale_g;
            *ptr_out++ = (*ptr_in) * scale_b;
            ptr_in++;
            *ptr_out++ = (*ptr_in) * scale_a;
            ptr_in++;
        }
    }

    free(self->data);
    self->data = buffer;
    self->colorspace = CS(RGB);
    self->colorspace_format = colorspace_format;
    self->scale = scale;
    self->channels = channels_out;
    self->data_items = bitems;
    self->data_size = bsize;
    self->pitch = self->width * self->channels;
    
    if (scale <= 0.0) {
        self->channel_scales[0] = (REAL)1.0;
        self->channel_scales[1] = (REAL)1.0;
        self->channel_scales[2] = (REAL)1.0;
        self->channel_scales[3] = (REAL)1.0;
    } else {
        self->channel_scales[0] = (REAL)scale / (REAL)csfmt[4];
        self->channel_scales[1] = (REAL)scale / (REAL)csfmt[5];
        self->channel_scales[2] = (REAL)scale / (REAL)csfmt[6];
        self->channel_scales[3] = (REAL)scale / (REAL)csfmt[7];
    }

    return 0;
}

PRIVATE
int
mono_to_hsv(ImageKit_Image *self, int colorspace_format, REAL scale)
{
    REAL *ptr_in, *ptr_out;
    REAL *buffer;
    int channels_out = 3;
    size_t i, l;
    size_t bitems, bsize;
    
    bitems = self->width * self->height * channels_out;
    bsize = bitems * sizeof(REAL);
    
    if (self->channels == 2) {
        self->channels = 4;
    }
    
    buffer = malloc(bsize);
    if (!buffer) {
        ImageKit_SetError(ImageKit_MemoryError, "Unable to allocate memory");
        return -1;
    }

    l = self->width * self->height;
    ptr_in = (REAL *)&(self->data[0]);
    ptr_out = (REAL *)buffer;

    if (channels_out == 3) {
        for (i = 0; i < l; i++) {
            *ptr_out++ = (REAL)0.0;
            *ptr_out++ = (REAL)0.0;
            *ptr_out++ = *ptr_in++;
        }
    } else {
        for (i = 0; i < l; i++) {
            *ptr_out++ = (REAL)0.0;
            *ptr_out++ = (REAL)0.0;
            *ptr_out++ = *ptr_in++;
            *ptr_out++ = *ptr_in++;
        }
    }
    
    free(self->data);
    self->data = buffer;
    self->colorspace = CS(HSV);
    self->colorspace_format = CS_FMT(HSV_NATURAL);
    self->channels = channels_out;
    self->data_items = bitems;
    self->data_size = bsize;
    self->pitch = self->width * self->channels;
    self->scale = -1;
    
    self->channel_scales[0] = (REAL)1.0;
    self->channel_scales[1] = (REAL)1.0;
    self->channel_scales[2] = (REAL)1.0;
    self->channel_scales[3] = (REAL)1.0;

    return 0;

}

PRIVATE
int
dummy_to_dummy(ImageKit_Image *self, int colorspace_format, REAL scale)
{
    return 0;
}

PRIVATE int (*fn_table [CS(SIZE)] [CS(SIZE)]) (ImageKit_Image *, int, REAL) = {
    /* Mono */
    {
        &dummy_to_dummy,
        &mono_to_rgb,
        &mono_to_hsv,
        &dummy_to_dummy
    },
    /* RGB */
    {
        &rgb_to_mono,
        &dummy_to_dummy,
        &rgb_to_hsv,
        &dummy_to_dummy
    },
    /* HSV */
    {
        &hsv_to_mono,
        &hsv_to_rgb,
        &dummy_to_dummy,
        &dummy_to_dummy
    },
    /* YIQ */
    {
        &dummy_to_dummy,
        &dummy_to_dummy,
        &dummy_to_dummy,
        &dummy_to_dummy
    },
};

API
int
ImageKit_Image_toHSV(ImageKit_Image *self)
{
    if (self->colorspace < CS(SIZE)) {
        return fn_table[self->colorspace][CS(HSV)](
            self,
            -1,
            -1
        );
    }
    
    return -1;
}

API
int
ImageKit_Image_toRGB(ImageKit_Image *self, int colorspace_format, REAL scale)
{
    if (self->colorspace < CS(SIZE)) {
        return fn_table[self->colorspace][CS(RGB)](
            self,
            colorspace_format,
            scale
        );
    }

    return -1;
}

API
int
ImageKit_Image_toMono(ImageKit_Image *self)
{
    if (self->colorspace < CS(SIZE)) {
        return fn_table[self->colorspace][CS(MONO)](
            self,
            -1,
            -1
        );
    }

    return -1;
}
