#ifndef IK_CS_CONVERT_DOT_C
#ifdef IK_INTERNAL

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
    
    /*
    
    FIXME: Set these somewhere else (in common code)
    
    */
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
    
    /*
    
    FIXME: Set these somewhere else (in common code)
    
    */
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

#endif /* IK_INTERNAL */
#endif /* IK_CS_CONVERT_DOT_C */
