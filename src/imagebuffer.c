#ifndef IK_IMAGEBUFFER_DOT_C
#ifdef IK_INTERNAL

static void ImageBuffer_init_defaults(ImageBuffer *self)
{
    if (self) {
        self->scale = -1.0f;
        self->colorspace = COLORSPACE_RGB;
        self->colorspace_format = COLORSPACE_FORMAT_RGB24;
        self->width = 0;
        self->height = 0;
        self->channels = 0;
        self->pitch = 0;
        self->data_size = 0;
        self->data_items = 0;
    }
}

static int
ImageBuffer_init_real(  ImageBuffer *self,
                        uint32_t width,
                        uint32_t height,
                        uint32_t channels,
                        REAL_TYPE scale,
                        int colorspace,
                        int colorspace_format)
{

    size_t bsize;
    size_t bitems;
    
    bitems = width * height * channels;
    bsize = sizeof(REAL_TYPE) * bitems;
    
    self->data = malloc(bsize);
    if (!self->data) {
        PyErr_NoMemory();
        return -1;
    }
    
    self->data_size = bsize;
    self->data_items = bitems;
    memset(self->data, 0, bsize);
    
    self->scale = scale;
    
    // Check colorspace format is valid
    if (colorspace == COLORSPACE_RGB) {
        switch (colorspace_format) {
            case CS_FMT(RGB15):
            case CS_FMT(RGB16):
            case CS_FMT(RGB24):
            case CS_FMT(RGB30):
            case CS_FMT(RGB48):
                break;
            default:
                PyErr_SetString(PyExc_ValueError, "RGB Colorspace must use RGB format");
                return -1;
                break;
        }
    } else if (colorspace == COLORSPACE_HSV && colorspace_format != CS_FMT(HSV_NATURAL)) {
        PyErr_SetString(PyExc_ValueError, "HSV Colorspace must use HSV format");
        return -1;
    }
    
    if (colorspace > 0 && colorspace < COLORSPACE_SIZE) {
        self->colorspace = colorspace;
    }
    
    if (colorspace_format > 0 && colorspace < COLORSPACE_FORMAT_SIZE) {
        self->colorspace_format = colorspace_format;
    }
    
    /* Get channel scales */
    if (self->scale <= 0.0) {
        self->channel_scales[0] = (REAL_TYPE)1.0;
        self->channel_scales[1] = (REAL_TYPE)1.0;
        self->channel_scales[2] = (REAL_TYPE)1.0;
        self->channel_scales[3] = (REAL_TYPE)1.0;
    } else {
        self->channel_scales[0] =
            self->scale / (REAL_TYPE)COLORSPACE_FORMAT_MINMAX[self->colorspace_format][4];
        self->channel_scales[1] =
            self->scale / (REAL_TYPE)COLORSPACE_FORMAT_MINMAX[self->colorspace_format][5];
        self->channel_scales[2] =
            self->scale / (REAL_TYPE)COLORSPACE_FORMAT_MINMAX[self->colorspace_format][6];
        self->channel_scales[3] =
            self->scale / (REAL_TYPE)COLORSPACE_FORMAT_MINMAX[self->colorspace_format][7];
    }
    
    self->width = width;
    self->height = height;
    self->channels = channels;
    self->pitch = width * channels;

    return 0;
}

static int ImageBuffer_init(ImageBuffer *self, PyObject *args, PyObject *kwargs)
{
    static char *kwargs_names[] = {
                    "width",
                    "height",
                    "channels",
                    "scale_max",
                    "colorspace",
                    "colorspace_format",
                    NULL
    };
    
    REAL_TYPE scale = -1;
    int colorspace = -1;
    int colorspace_format = -1;
    
    uint32_t width;
    uint32_t height;
    uint32_t channels;

    /* Defaults */
    ImageBuffer_init_defaults(self);

    /* Parse arguments */
    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "III|fii",
            kwargs_names,
            &width,
            &height,
            &channels,
            &scale,
            &colorspace,
            &colorspace_format)) {
        return -1;
    }
    
    /* Call real init */
    return ImageBuffer_init_real(
                self,
                width,
                height,
                channels,
                scale,
                colorspace,
                colorspace_format
    );
}

static void ImageBuffer_dealloc(ImageBuffer *self)
{
    free(self->data);
    self->ob_type->tp_free((PyObject *)self);
}

static ImageBuffer *ImageBuffer_copy(ImageBuffer *self, PyObject *args)
{
    ImageBuffer *new_self;
    
    new_self = (ImageBuffer *)PyObject_CallMethod(MODULE, "ImageBuffer", "IIIfii",
                    self->width,
                    self->height,
                    self->channels,
                    self->scale,
                    self->colorspace,
                    self->colorspace_format
    );
    
    if (!new_self) {
        return NULL;
    }
    
    memcpy(new_self->data, self->data, self->data_size);
    return new_self;
}

#endif /* IK_INTERNAL */
#endif /* IK_IMAGEBUFFER_DOT_C */
