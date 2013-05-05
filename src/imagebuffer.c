
#include "imagekit.h"
#include "imagekit_functions.h"

API void ImageBuffer_init_defaults(ImageBuffer *self)
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

API int
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
    
    if (channels > 4) {
        PyErr_SetString(PyExc_ValueError, "Channels cannot exceed 4");
        return -1;
    }
    
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

API int ImageBuffer_init(ImageBuffer *self, PyObject *args, PyObject *kwargs)
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

API void ImageBuffer_dealloc(ImageBuffer *self)
{
    free(self->data);
    self->ob_type->tp_free((PyObject *)self);
}

API ImageBuffer *ImageBuffer_copy(ImageBuffer *self, PyObject *args)
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

API PyObject *ImageBuffer_get_pixel(ImageBuffer *self, PyObject *args)
{
    PyObject *tuple;
    PyObject *tmp;
    uint32_t x, y, c;

    if (!PyArg_ParseTuple(args, "II", &x, &y)) {
        return NULL;
    }
    
    x = x % self->width;
    y = y % self->height;

    tuple = PyTuple_New(self->channels);
    if (!tuple) {
        return NULL;
    }
    
    for (c = 0; c < self->channels; c++) {
        tmp = PyFloat_FromDouble(
            self->data[PIXEL_INDEX(self, x, y) + c]
        );
        
        if (!tmp) {
            Py_DECREF(tuple);
            return NULL;
        }
        
        PyTuple_SetItem(tuple, c, tmp);
    }

    return tuple;
}

API PyObject *ImageBuffer_set_pixel(ImageBuffer *self, PyObject *args)
{
    PyObject *tuple;
    PyObject *tmp;
    struct ListTypeMethods *lm;
    uint32_t x, y, i;
    size_t c;

    if (!PyArg_ParseTuple(args, "IIO", &x, &y, &tuple)) {
        return NULL;
    }
    
    Py_INCREF(tuple);
    if (PyTuple_Check(tuple)) {
        lm = &TUPLE_METHODS;
    } else if (PyList_Check(tuple)) {
        lm = &LIST_METHODS;
    } else {
        PyErr_SetString(PyExc_ValueError, "Argument must be list or tuple");
        Py_DECREF(tuple);
        return NULL;
    }
    
    c = lm->Size(tuple);
    x = x % self->width;
    y = y % self->height;
    if (c > self->channels) {
        c = self->channels;
    }
    
    for (i = 0; i < c; i++) {
        tmp = lm->GetItem(tuple, i);
        Py_INCREF(tmp);
        
        self->data[PIXEL_INDEX(self, x, y) + i] = (REAL_TYPE)PyFloat_AsDouble(tmp);
        Py_DECREF(tmp);
    }

    Py_DECREF(tuple);
    Py_INCREF(Py_None);
    return Py_None;
}

API PyObject *ImageBuffer_set1(ImageBuffer *self, PyObject *args)
{
    uint32_t x, y;
    REAL_TYPE v1;

    if (!PyArg_ParseTuple(args, "IIf", &x, &y, &v1)) {
        return NULL;
    }
    
    x = x % self->width;
    y = y % self->height;
    
    self->data[PIXEL_INDEX(self, x, y)] = v1;
    
    Py_INCREF(Py_None);
    return Py_None;
}

API PyObject *ImageBuffer_set3(ImageBuffer *self, PyObject *args)
{
    uint32_t x, y;
    REAL_TYPE v1, v2, v3;

    if (!PyArg_ParseTuple(args, "IIfff", &x, &y, &v1, &v2, &v3)) {
        return NULL;
    }
    
    x = x % self->width;
    y = y % self->height;
    
    self->data[PIXEL_INDEX(self, x, y)    ] = v1;
    self->data[PIXEL_INDEX(self, x, y) + 1] = v2;
    self->data[PIXEL_INDEX(self, x, y) + 2] = v3;
    
    Py_INCREF(Py_None);
    return Py_None;
}

API PyObject *ImageBuffer_set4(ImageBuffer *self, PyObject *args)
{
    uint32_t x, y;
    REAL_TYPE v1, v2, v3, v4;

    if (!PyArg_ParseTuple(args, "IIffff", &x, &y, &v1, &v2, &v3, &v4)) {
        return NULL;
    }
    
    x = x % self->width;
    y = y % self->height;
    
    self->data[PIXEL_INDEX(self, x, y)    ] = v1;
    self->data[PIXEL_INDEX(self, x, y) + 1] = v2;
    self->data[PIXEL_INDEX(self, x, y) + 2] = v3;
    self->data[PIXEL_INDEX(self, x, y) + 3] = v4;
    
    Py_INCREF(Py_None);
    return Py_None;
}

API PyObject *ImageBuffer_hzline_in(ImageBuffer *self, PyObject *args)
{
    PyObject *tuple;
    PyObject *tmp;
    struct ListTypeMethods *lm;
    uint32_t y;
    size_t x, c;
    REAL_TYPE *ptr;

    if (!PyArg_ParseTuple(args, "IO", &y, &tuple)) {
        return NULL;
    }
    
    Py_INCREF(tuple);
    
    if (PyTuple_Check(tuple)) {
        lm = &TUPLE_METHODS;
    } else if (PyList_Check(tuple)) {
        lm = &LIST_METHODS;
    } else {
        PyErr_SetString(PyExc_TypeError, "Object must be list or tuple");
        Py_DECREF(tuple);
        return NULL;
    }
    
    c = lm->Size(tuple);
    if (y >= self->height) {
        PyErr_SetString(PyExc_ValueError, "y exceeds height");
        return NULL;
    }
    
    if (c >= self->pitch) {
        c = self->pitch;
    }
    
    ptr = (REAL_TYPE *)&(self->data[PIXEL_INDEX(self, 0, y)]);
    for (x = 0; x < c; x++) {
        tmp = lm->GetItem(tuple, x);
        
        Py_INCREF(tmp);
        *ptr++ = (REAL_TYPE)PyFloat_AsDouble(tmp);
        Py_DECREF(tmp);
    }
    
    Py_DECREF(tuple);
    Py_INCREF(Py_None);
    return Py_None;
}

API PyObject *ImageBuffer_hzline_out(ImageBuffer *self, PyObject *args)
{
    PyObject *list_out;
    PyObject *tmp;
    size_t x, y, nitems;
    uint32_t y_in;
    REAL_TYPE *ptr_in;
    
    if (!PyArg_ParseTuple(args, "I", &y_in)) {
        return NULL;
    }
    
    y = y_in;
    if (y >= self->height) {
        PyErr_SetString(PyExc_ValueError, "y exceeds height");
        return NULL;
    }
    
    nitems = self->width * self->channels;
    list_out = PyList_New(nitems);
    if (!list_out) {
        return NULL;
    }
    
    ptr_in = (REAL_TYPE *)&(self->data[PIXEL_INDEX(self, 0, y)]);
    for (x = 0; x < nitems; x++) {
        tmp = PyFloat_FromDouble((double)(*ptr_in));
        if (!tmp) {
            Py_DECREF(list_out);
            return NULL;
        }
        
        PyList_SetItem(list_out, x, tmp);
        ptr_in++;
    }
    
    return list_out;
}

API PyObject *ImageBuffer_vtline_in(ImageBuffer *self, PyObject *args)
{
    PyObject *tuple;
    PyObject *tmp;
    struct ListTypeMethods *lm;
    
    uint32_t x_in;
    size_t x, y, c, len;
    REAL_TYPE *ptr_out;
    
    if (!PyArg_ParseTuple(args, "IO", &x_in, &tuple)) {
        return NULL;
    }
    
    Py_INCREF(tuple);
    
    if (PyTuple_Check(tuple)) {
        lm = &TUPLE_METHODS;
    } else if (PyList_Check(tuple)) {
        lm = &LIST_METHODS;
    } else {
        PyErr_SetString(PyExc_TypeError, "Object must be list or tuple");
        Py_DECREF(tuple);
        return NULL;
    }
    
    x = x_in;
    if (x >= self->width) {
        PyErr_SetString(PyExc_ValueError, "x exceeds width");
        Py_DECREF(tuple);
        return NULL;
    }
    
    len = lm->Size(tuple);
    if ((len / self->channels) * self->channels != len) {
        PyErr_SetString(PyExc_ValueError, "Number of items in list must be divisible by channels");
        Py_DECREF(tuple);
        return NULL;
    }
    
    len /= self->channels;
    if (len >= self->height) {
        len = self->height;
    }
    
    ptr_out = (REAL_TYPE *)&(self->data[PIXEL_INDEX(self, x, 0)]);
    for (y = 0; y < len; y++) {
        for (c = 0; c < self->channels; c++) {
            tmp = lm->GetItem(tuple, (y * self->channels) + c);
            
            Py_INCREF(tmp);
            *(ptr_out+c) = (REAL_TYPE)PyFloat_AsDouble(tmp);
            Py_DECREF(tmp);
        }
        
        ptr_out += self->pitch;
    }
    
    Py_DECREF(tuple);
    Py_INCREF(Py_None);
    return Py_None;
}

API PyObject *ImageBuffer_vtline_out(ImageBuffer *self, PyObject *args)
{
    PyObject *tuple_out;
    
    REAL_TYPE *ptr_in;
    size_t x, y, c;
    uint32_t x_in;
    
    if (!PyArg_ParseTuple(args, "I", &x_in)) {
        return NULL;
    }
    
    x = x_in;
    if (x >= self->width) {
        PyErr_SetString(PyExc_ValueError, "x exceeds width");
        return NULL;
    }
    
    tuple_out = PyList_New(self->height * self->channels);
    if (!tuple_out) {
        return NULL;
    }
    
    ptr_in = (REAL_TYPE *)&(self->data[PIXEL_INDEX(self, x, 0)]);
    for (y = 0; y < self->height; y++) {
        for (c = 0; c < self->channels; c++) {
            PyList_SetItem(
                tuple_out,
                (y * self->channels) + c,
                PyFloat_FromDouble(*(ptr_in+c)));
        }
        
        ptr_in += self->pitch;
    }

    return tuple_out;
}

API PyObject *ImageBuffer_to_mono(ImageBuffer *self, PyObject *args)
{
    int result;

    if (self->colorspace == COLORSPACE_RGB) {
        result = ImageBuffer_rgb_to_mono(self);
    } else if (self->colorspace == COLORSPACE_HSV) {
        result = ImageBuffer_hsv_to_mono(self);
    }

    if (!result) {
        PyErr_SetString(PyExc_StandardError, "Conversion failed");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

API PyObject *ImageBuffer_to_hsv(ImageBuffer *self, PyObject *args)
{
    int result;

    if (self->colorspace == COLORSPACE_RGB) {
        result = ImageBuffer_rgb_to_hsv(self);
    } else if (self->colorspace == COLORSPACE_MONO) {
        result = ImageBuffer_mono_to_hsv(self);
    }
    
    if (!result) {
        PyErr_SetString(PyExc_StandardError, "Conversion failed");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

API PyObject *ImageBuffer_to_rgb(ImageBuffer *self, PyObject *args, PyObject *kwargs)
{
    static char *kwargs_names[] = {
        "colorspace_format",
        "scale_max",
        NULL
    };

    int result;
    int colorspace_format = -1;
    REAL_TYPE scale_max = -1;
    
    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "|i|f",
            kwargs_names,
            &colorspace_format,
            &scale_max)) {
        return NULL;
    }
    
    if (colorspace_format < 0) {
        colorspace_format = CS_FMT(RGB24);
    }

    if (colorspace_format < CS_FMT(RGB15) || colorspace_format > CS_FMT(RGB48)) {
        PyErr_SetString(PyExc_ValueError, "colorspace_format must be in RGB color space");
        return NULL;
    }

    if (self->colorspace == COLORSPACE_HSV) {
        result = ImageBuffer_hsv_to_rgb(self, colorspace_format, scale_max);
    } else if (self->colorspace == COLORSPACE_MONO) {
        result = ImageBuffer_mono_to_rgb(self, colorspace_format, scale_max);
    }
    
    if (!result) {
        PyErr_SetString(PyExc_StandardError, "Conversion failed");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

API PyObject *ImageBuffer_get_box(ImageBuffer *self, PyObject *args)
{
    int32_t center_x, center_y, c, sx, sy, ex, ey, mid;
    int32_t size = 3;
    
    uint32_t i;
    PyObject *tuple;
    PyObject *nested;
    
    if (!PyArg_ParseTuple(args, "ii|i", &center_x, &center_y, &size)) {
        return NULL;
    }
    
    if (center_x < 0 || center_x >= self->width) {
        PyErr_SetString(PyExc_ValueError, "x must be within width");
        return NULL;
    }
    
    if (center_y < 0 || center_y >= self->height) {
        PyErr_SetString(PyExc_ValueError, "y must be within height");
        return NULL;
    }
    
    if (size < 3) {
        PyErr_SetString(PyExc_ValueError, "size must be at least 3");
        return NULL;
    }
    
    if (size % 2 == 0) {
        PyErr_SetString(PyExc_ValueError, "size must be an odd number");
        return NULL;
    }
    
    mid = size / 2;
    ex = center_x + mid;
    sy = center_y - mid;
    ey = center_y + mid;
    
    tuple = PyList_New(size * size);
    if (!tuple) {
        return NULL;
    }
    
    i = 0;
    while (sy <= ey) {
        sx = center_x - mid;
        while (sx <= ex) {
            nested = PyList_New(self->channels);
            if (!nested) {
                Py_DECREF(tuple);
                return NULL;
            }
            
            if (sx < 0 || sy < 0 || sx >= self->width || sy >= self->height) {
                for (c = 0; c < self->channels; c++) {
                    /* Out of range (zero) */
                    PyList_SetItem(nested, c, PyFloat_FromDouble(0.0));
                }
            } else {
                for (c = 0; c < self->channels; c++) {
                    /* Get pixels */
                    PyList_SetItem(nested, c, PyFloat_FromDouble(
                        self->data[PIXEL_INDEX(self, sx, sy) + c]));
                }
            }
            
            PyList_SetItem(tuple, i, nested);
            i++;
            sx++;
        }
        
        sy++;
    }
    
    return tuple;
}
