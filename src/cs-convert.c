#ifndef IK_CS_CONVERT_DOT_C
#ifdef IK_INTERNAL

/*

TODO: These functions should error out, if an error occurs.

*/

/* Converts Mono ImageBuffer object to RGB */
static void ImageBuffer_mono_to_rgb(ImageBuffer *self, int colorspace_format, float scale_max)
{
    REAL_TYPE *ptr_in, *ptr_out;
    REAL_TYPE *buffer;
    REAL_TYPE scale_r, scale_g, scale_b, scale_a;
    int channels_out = 3;
    double *csfmt;
    size_t i, l;
    size_t bitems, bsize;
    
    if (colorspace_format < 0) {
        colorspace_format = CS_FMT(RGB24);
    }
    
    csfmt = (double *)&COLORSPACE_FORMAT_MINMAX[colorspace_format];
    
    if (self->channels == 2) {
        self->channels = 4;
    }
    
    if (scale_max <= 0.0) {
        scale_r = (REAL_TYPE)csfmt[4];
        scale_g = (REAL_TYPE)csfmt[5];
        scale_b = (REAL_TYPE)csfmt[6];
        scale_a = (REAL_TYPE)csfmt[7];
    } else {
        scale_r = (REAL_TYPE)scale_max;
        scale_g = (REAL_TYPE)scale_max;
        scale_b = (REAL_TYPE)scale_max;
        scale_a = (REAL_TYPE)scale_max;
    }
    
    bitems = self->width * self->height * channels_out;
    bsize = bitems * sizeof(REAL_TYPE);
    
    buffer = malloc(bsize);
    if (!buffer) {
        /* FIXME: Error out */
        return;
    }
    
    l = self->width * self->height;
    ptr_in = (REAL_TYPE *)&(self->data[0]);
    ptr_out = (REAL_TYPE *)buffer;
    
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
    self->colorspace = COLORSPACE_RGB;
    self->colorspace_format = colorspace_format;
    self->scale = scale_max;
    self->channels = channels_out;
    self->data_items = bitems;
    self->data_size = bsize;
    self->pitch = self->width * self->channels;
    
    if (scale_max <= 0.0) {
        self->channel_scales[0] = (REAL_TYPE)1.0;
        self->channel_scales[1] = (REAL_TYPE)1.0;
        self->channel_scales[2] = (REAL_TYPE)1.0;
        self->channel_scales[3] = (REAL_TYPE)1.0;
    } else {
        self->channel_scales[0] = (REAL_TYPE)scale_max / (REAL_TYPE)csfmt[4];
        self->channel_scales[1] = (REAL_TYPE)scale_max / (REAL_TYPE)csfmt[5];
        self->channel_scales[2] = (REAL_TYPE)scale_max / (REAL_TYPE)csfmt[6];
        self->channel_scales[3] = (REAL_TYPE)scale_max / (REAL_TYPE)csfmt[7];
    }
}

/* Converts Mono ImageBuffer object to HSV */
static void ImageBuffer_mono_to_hsv(ImageBuffer *self)
{
    REAL_TYPE *ptr_in, *ptr_out;
    REAL_TYPE *buffer;
    int channels_out = 3;
    size_t i, l;
    size_t bitems, bsize;
    
    bitems = self->width * self->height * channels_out;
    bsize = bitems * sizeof(REAL_TYPE);
    
    if (self->channels == 2) {
        self->channels = 4;
    }
    
    buffer = malloc(bsize);
    if (!buffer) {
        /* FIXME: Error out */
        return;
    }
    
    l = self->width * self->height;
    ptr_in = (REAL_TYPE *)&(self->data[0]);
    ptr_out = (REAL_TYPE *)buffer;

    if (channels_out == 3) {
        for (i = 0; i < l; i++) {
            *ptr_out++ = (REAL_TYPE)0.0;
            *ptr_out++ = (REAL_TYPE)0.0;
            *ptr_out++ = *ptr_in++;
        }
    } else {
        for (i = 0; i < l; i++) {
            *ptr_out++ = (REAL_TYPE)0.0;
            *ptr_out++ = (REAL_TYPE)0.0;
            *ptr_out++ = *ptr_in++;
            *ptr_out++ = *ptr_in++;
        }
    }

    free(self->data);
    self->data = buffer;
    self->colorspace = COLORSPACE_HSV;
    self->colorspace_format = CS_FMT(HSV_NATURAL);
    self->channels = channels_out;
    self->data_items = bitems;
    self->data_size = bsize;
    self->pitch = self->width * self->channels;
    self->scale = -1;
    
    self->channel_scales[0] = (REAL_TYPE)1.0;
    self->channel_scales[1] = (REAL_TYPE)1.0;
    self->channel_scales[2] = (REAL_TYPE)1.0;
    self->channel_scales[3] = (REAL_TYPE)1.0;
}

/* Converts HSV ImageBuffer object to RGB */
static void ImageBuffer_hsv_to_rgb(ImageBuffer *self, int colorspace_format, float scale_max)
{
    REAL_TYPE hsv[4];
    REAL_TYPE rgb[4];
    REAL_TYPE scale_r, scale_g, scale_b, scale_a;
    REAL_TYPE *ptr;
    
    REAL_TYPE h60, h60f, f, p, q, t;
    uint32_t hi;
    
    size_t i, l;
    double *csfmt;
    
    if (colorspace_format < 0) {
        colorspace_format = CS_FMT(RGB24);
    }
    
    csfmt = (double *)&COLORSPACE_FORMAT_MINMAX[colorspace_format];
    
    if (scale_max <= 0.0) {
        scale_r = (REAL_TYPE)csfmt[4];
        scale_g = (REAL_TYPE)csfmt[5];
        scale_b = (REAL_TYPE)csfmt[6];
        scale_a = (REAL_TYPE)csfmt[7];
    } else {
        scale_r = (REAL_TYPE)scale_max;
        scale_g = (REAL_TYPE)scale_max;
        scale_b = (REAL_TYPE)scale_max;
        scale_a = (REAL_TYPE)scale_max;
    }
    
    l = self->width * self->height;
    ptr = (REAL_TYPE *)&(self->data[0]);
    
    for (i = 0; i < l; i++) {
        hsv[0] = *(ptr  );
        hsv[1] = *(ptr+1);
        hsv[2] = *(ptr+2);
        
        h60 = hsv[0] / (REAL_TYPE)60.0;
        h60f = (REAL_TYPE)floor(h60);
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
    
    self->colorspace = COLORSPACE_RGB;
    self->colorspace_format = colorspace_format;
    self->scale = scale_max;
    
    if (scale_max <= 0.0) {
        self->channel_scales[0] = (REAL_TYPE)1.0;
        self->channel_scales[1] = (REAL_TYPE)1.0;
        self->channel_scales[2] = (REAL_TYPE)1.0;
        self->channel_scales[3] = (REAL_TYPE)1.0;
    } else {
        self->channel_scales[0] = (REAL_TYPE)scale_max / (REAL_TYPE)csfmt[4];
        self->channel_scales[1] = (REAL_TYPE)scale_max / (REAL_TYPE)csfmt[5];
        self->channel_scales[2] = (REAL_TYPE)scale_max / (REAL_TYPE)csfmt[6];
        self->channel_scales[3] = (REAL_TYPE)scale_max / (REAL_TYPE)csfmt[7];
    }
}

/* Converts RGB ImageBuffer object to HSV */
static void ImageBuffer_rgb_to_hsv(ImageBuffer *self)
{
    REAL_TYPE scale_r, scale_g, scale_b, scale_a;
    REAL_TYPE rgb[4];
    REAL_TYPE hue, saturation;
    REAL_TYPE mn, mx, df;
    REAL_TYPE *ptr;
    size_t i, l;
    double *csfmt;
    
    csfmt = (double *)&COLORSPACE_FORMAT_MINMAX[self->colorspace_format];
    
    /* Get Scales */
    if (self->scale <= 0.0) {
        scale_r = (REAL_TYPE)1.0 / (REAL_TYPE)csfmt[4];
        scale_g = (REAL_TYPE)1.0 / (REAL_TYPE)csfmt[5];
        scale_b = (REAL_TYPE)1.0 / (REAL_TYPE)csfmt[6];
        scale_a = (REAL_TYPE)1.0 / (REAL_TYPE)csfmt[7];
    } else {
        scale_r = (REAL_TYPE)1.0 / self->scale;
        scale_g = (REAL_TYPE)1.0 / self->scale;
        scale_b = (REAL_TYPE)1.0 / self->scale;
        scale_a = (REAL_TYPE)1.0 / self->scale;
    }
    
    l = self->width * self->height;
    ptr = (REAL_TYPE *)&(self->data[0]);

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
        if (mx == mn)           hue =  (REAL_TYPE)0.0;
        else if (mx == rgb[0])  hue = ((REAL_TYPE)60.0 * ((rgb[1] - rgb[2]) / df) + 360);
        else if (mx == rgb[1])  hue = ((REAL_TYPE)60.0 * ((rgb[2] - rgb[0]) / df) + 120);
        else if (mx == rgb[2])  hue = ((REAL_TYPE)60.0 * ((rgb[0] - rgb[1]) / df) + 240);
        
        /* Clamp */
        if (hue >= 360) hue -= 360;
        
        /* Saturation */
        if (mx == 0) {
            saturation = (REAL_TYPE)0.0;
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
    
    self->colorspace = COLORSPACE_HSV;
    self->colorspace_format = CS_FMT(HSV_NATURAL);
    self->scale = -1;
    
    /* HSV is never scaled */
    self->channel_scales[0] = (REAL_TYPE)1.0;
    self->channel_scales[1] = (REAL_TYPE)1.0;
    self->channel_scales[2] = (REAL_TYPE)1.0;
    self->channel_scales[3] = (REAL_TYPE)1.0;

    return;
}

/* Converts RGB ImageBuffer object to Mono */
static void ImageBuffer_rgb_to_mono(ImageBuffer *self)
{
    REAL_TYPE *ptr_in, *ptr_out;
    REAL_TYPE value;
    REAL_TYPE scales[4];
    double *csfmt;
    size_t i, l;
    int out_channels = 1;
    int colorspace_format = CS_FMT(MONO_NATURAL);
    
    if (self->channels == 4) {
        out_channels = 2;
    }
    
    csfmt = (double *)&COLORSPACE_FORMAT_MINMAX[self->colorspace_format];
    
    if (self->scale > 0.0) {
        scales[0] = self->scale / (REAL_TYPE)1.0;
        scales[1] = self->scale / (REAL_TYPE)1.0;
        scales[2] = self->scale / (REAL_TYPE)1.0;
        scales[3] = self->scale / (REAL_TYPE)1.0;
    } else {
        scales[0] = (REAL_TYPE)1.0 / (REAL_TYPE)csfmt[4];
        scales[1] = (REAL_TYPE)1.0 / (REAL_TYPE)csfmt[5];
        scales[2] = (REAL_TYPE)1.0 / (REAL_TYPE)csfmt[6];
        scales[3] = (REAL_TYPE)1.0 / (REAL_TYPE)csfmt[7];
    }
    
    l = self->width * self->height;
    ptr_in = &(self->data[0]);
    ptr_out = &(self->data[0]);
    
    if (out_channels == 2) {
        for (i = 0; i < l; i++) {
            value =  ((*ptr_in) * scales[0]) * (REAL_TYPE)0.299; ptr_in++;
            value += ((*ptr_in) * scales[1]) * (REAL_TYPE)0.587; ptr_in++;
            value += ((*ptr_in) * scales[2]) * (REAL_TYPE)0.114; ptr_in++;
            
            *ptr_out++ = value;
            *ptr_out++ = (*ptr_in) * scales[3];
            ptr_in++;
        }
    } else {
        for (i = 0; i < l; i++) {
            value =  ((*ptr_in) * scales[0]) * (REAL_TYPE)0.299; ptr_in++;
            value += ((*ptr_in) * scales[1]) * (REAL_TYPE)0.587; ptr_in++;
            value += ((*ptr_in) * scales[2]) * (REAL_TYPE)0.114; ptr_in++;
            
            *ptr_out++ = value;
        }
    }
    
    self->colorspace = COLORSPACE_MONO;
    self->colorspace_format = colorspace_format;
    self->channels = out_channels;
    self->scale = (REAL_TYPE)1.0;
    self->pitch = self->width * self->channels;
    self->data_items = self->width * self->height * self->channels;
    
    self->channel_scales[0] = (REAL_TYPE)1.0;
    self->channel_scales[1] = (REAL_TYPE)1.0;
    self->channel_scales[2] = (REAL_TYPE)1.0;
    self->channel_scales[3] = (REAL_TYPE)1.0;
}

/* Converts HSV ImageBuffer object to Mono */
static void ImageBuffer_hsv_to_mono(ImageBuffer *self)
{
    /* Convert to RGB, then convert RGB to Mono (more accurate than desaturating) */
    ImageBuffer_hsv_to_rgb(self, CS_FMT(RGB24), (REAL_TYPE)-1.0);
    ImageBuffer_rgb_to_mono(self);
    return;
}

#endif /* IK_INTERNAL */
#endif /* IK_CS_CONVERT_DOT_C */
