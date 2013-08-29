
#ifdef __cplusplus
    extern "C"
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <memory.h>
#include <math.h>
#include <Python.h>
#include <structmember.h>

#include "imagekit.h"

/*

TODO:
    - Better error wrapping
    - ImageBuffer
        - ImageBuffer members (width, height, channels, etc)
        - Blit
        - FillRect
    - Point Filter wrapper
    - Coordinates class wrapper
    - Curves class wrapper
    - Anti-alias option for drawing methods (line, circle, etc)

*/

#if PY_MAJOR_VERSION >= 3
    #define IS_PY3K
    #define PyInt_FromLong PyLong_FromLong
#endif

#ifndef Py_TYPE
    #define Py_TYPE(ob) (((PyObject*)(ob))->ob_type)
#endif

#ifdef API
    #undef API
#endif

#define API static

static PyObject *MODULE;
static const char *documentation;
static PyTypeObject ImageBuffer_Type;

struct ListTypeMethods {
    Py_ssize_t (*Size)(PyObject *);
    PyObject * (*GetItem)(PyObject *, Py_ssize_t);
    int (*SetItem)(PyObject *, Py_ssize_t, PyObject *);
    PyObject * (*GetSlice)(PyObject *, Py_ssize_t, Py_ssize_t);
};

static struct ListTypeMethods LIST_METHODS = {
    &PyList_Size,
    &PyList_GetItem,
    &PyList_SetItem,
    &PyList_GetSlice
};

static struct ListTypeMethods TUPLE_METHODS = {
    &PyTuple_Size,
    &PyTuple_GetItem,
    &PyTuple_SetItem,
    &PyTuple_GetSlice
};

static struct ListTypeMethods *GetListMethods(PyObject *object)
{
    if (PyTuple_Check(object)) {
        return &TUPLE_METHODS;
    } else if (PyList_Check(object)) {
        return &LIST_METHODS;
    }
    
    PyErr_SetString(PyExc_ValueError, "Object must be of type list or tuple");
    return NULL;
}

typedef struct {
    PyObject_HEAD
    ImageKit_Image *image;
} ImageBuffer;

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
    
    ImageKit_Image *image = NULL;
    
    REAL scale = -1;
    int colorspace = -1;
    int colorspace_format = -1;
    
    uint32_t width;
    uint32_t height;
    uint32_t channels;

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
    
    image = ImageKit_Image_New(
        width,
        height,
        channels,
        scale,
        colorspace,
        colorspace_format
    );
    
    if (!image) {
        PyErr_SetString(PyExc_Exception, "Failed to create image object");
        return -1;
    }
    
    self->image = image;
    return 0;
}

API void ImageBuffer_dealloc(ImageBuffer *self)
{
    ImageKit_Image_Delete(self->image);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

API PyObject *ImageBuffer_from_png(ImageBuffer *self, PyObject *args, PyObject *kwargs)
{
    static char *kwargs_names[] = {"path", "scale", NULL};
    char *path;
    REAL scale = -1;
    ImageKit_Image *image;
    
    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "s|f",
            kwargs_names,
            &path,
            &scale)) {
        return NULL;
    }
    
    if (!(self = (ImageBuffer *)_PyObject_New(&ImageBuffer_Type))) {
        return NULL;
    }
    
    image = ImageKit_Image_FromPNG(path, scale);
    if (!image) {
        Py_DECREF(self);
        PyErr_SetString(PyExc_Exception, "Failed to create image object");
        return NULL;
    }
    
    self->image = image;
    return (PyObject *)self;
}

PyObject *ImageBuffer_save_png(ImageBuffer *self, PyObject *args)
{
    int e_code;
    char *e_msg;
    char *path = NULL;
    if (!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }
    
    if (ImageKit_Image_SavePNG(self->image, path) < 1) {
        ImageKit_LastError(&e_code, &e_msg);
        PyErr_SetString(PyExc_Exception, e_msg);
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

API PyObject *ImageBuffer_from_jpeg(ImageBuffer *self, PyObject *args, PyObject *kwargs)
{
    static char *kwargs_names[] = {"path", "scale", NULL};
    char *path;
    REAL scale = -1;
    ImageKit_Image *image;
    
    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "s|f",
            kwargs_names,
            &path,
            &scale)) {
        return NULL;
    }
    
    if (!(self = (ImageBuffer *)_PyObject_New(&ImageBuffer_Type))) {
        return NULL;
    }
    
    image = ImageKit_Image_FromJPEG(path, scale);
    if (!image) {
        Py_DECREF(self);
        PyErr_SetString(PyExc_Exception, "Failed to create image object");
        return NULL;
    }
    
    self->image = image;
    return (PyObject *)self;
}

PyObject *ImageBuffer_save_jpeg(ImageBuffer *self, PyObject *args)
{
    int e_code;
    int quality = 85;
    char *e_msg;
    char *path = NULL;
    
    if (!PyArg_ParseTuple(args, "s|i", &path, &quality)) {
        return NULL;
    }
    
    if (quality < 0 || quality > 100) {
        PyErr_SetString(PyExc_ValueError, "Quality must be between 0 and 100");
        return NULL;
    }
    
    if (ImageKit_Image_SaveJPEG(self->image, path, quality) < 1) {
        ImageKit_LastError(&e_code, &e_msg);
        PyErr_SetString(PyExc_Exception, e_msg);
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_channel_ranges(ImageBuffer *self, PyObject *args)
{
    int i;
    volatile int error = 0;
    REAL min[4] = {0, 0, 0, 0};
    REAL max[4] = {0, 0, 0, 0};
    
    PyObject *rmin = NULL,
             *rmax = NULL,
             *outer = NULL;
    
    if (!(outer = PyTuple_New(2))) goto error_out;
    if (!( rmin = PyTuple_New(4))) goto error_out;
    if (!( rmax = PyTuple_New(4))) goto error_out;
    
    ImageKit_Image_ChannelRanges(self->image, (REAL *)&min, (REAL *)&max);
    for (i = 0; i < 4; i++) {
        PyTuple_SetItem(rmin, i, PyFloat_FromDouble(min[i]));
        PyTuple_SetItem(rmax, i, PyFloat_FromDouble(max[i]));
    }
    
    PyTuple_SetItem(outer, 0, rmin);
    PyTuple_SetItem(outer, 1, rmax);
    
    if (error) {
        error_out:
            Py_XDECREF(outer);
            Py_XDECREF(rmin);
            Py_XDECREF(rmax);
        return NULL;
    }
    
    return outer;
}

PyObject *ImageBuffer_get_histogram(ImageBuffer *self, PyObject *args)
{
    volatile int error = 0;
    
    size_t i, l;
    int32_t channels;
    int32_t samples = 255;
    ImageKit_Histogram *hist;
    PyObject *result[4] = {NULL, NULL, NULL, NULL};
    PyObject *outer = NULL;
    PyObject *tmp;
    
    if (!PyArg_ParseTuple(args, "i", &samples)) {
        return NULL;
    }
    
    if (samples < 1 || samples > 0xffff) {
        PyErr_SetString(PyExc_ValueError, "samples must be between 1 and 65535");
        return NULL;
    }
    
    channels = self->image->channels;
    hist = ImageKit_Histogram_FromImage(self->image, (uint16_t)samples);
    if (!hist) {
        PyErr_SetString(PyExc_Exception, "Failed to create histogram object");
        return NULL;
    }
    
    if (!(outer = PyTuple_New(channels))) goto error_out;
    
    for (i = 0; i < channels; i++) {
        if (!(result[i] = PyTuple_New(samples))) goto error_out;
    }
    
    l = samples * channels;
    for (i = 0; i < l; i++) {
        if (!(tmp = PyInt_FromLong(hist->a[i]))) goto error_out;
        PyTuple_SetItem(result[i / samples], i % samples, tmp);
    }
    
    for (i = 0; i < channels; i++) {
        PyTuple_SetItem(outer, i, result[i]);
    }
    
    ImageKit_Histogram_Delete(hist);
    
    if (error) {
        error_out:
            ImageKit_Histogram_Delete(hist);
            Py_XDECREF(result[0]);
            Py_XDECREF(result[1]);
            Py_XDECREF(result[2]);
            Py_XDECREF(result[3]);
            Py_XDECREF(outer);
        return NULL;
    }
    
    return outer;
}

PyObject *ImageBuffer_get_pixel(ImageBuffer *self, PyObject *args)
{
    REAL *ptr;
    DIMENSION x, y, c;
    PyObject *result;
    
    if (!PyArg_ParseTuple(args, "ii", &x, &y)) {
        return NULL;
    }
    
    if (!(result = PyTuple_New(self->image->channels))) {
        return NULL;
    }
    
    x = x % self->image->width;
    y = y % self->image->height;
    ptr = &(self->image->data[PIXEL_INDEX(self->image, x, y)]);
    
    for (c = 0; c < self->image->channels; c++) {
        PyTuple_SetItem(result, c, PyFloat_FromDouble(*ptr++));
    }
    
    return result;
}

PyObject *ImageBuffer_set_pixel(ImageBuffer *self, PyObject *args)
{
    REAL *ptr;
    REAL value;
    DIMENSION x, y, c;
    PyObject *pixel, *tmp;
    struct ListTypeMethods *pixel_methods;
    
    if (!PyArg_ParseTuple(args, "iiO", &x, &y, &pixel)) {
        return NULL;
    }
    
    if (!(pixel_methods = GetListMethods(pixel))) {
        return NULL;
    }
    
    if (pixel_methods->Size(pixel) < self->image->channels) {
        PyErr_SetString(PyExc_ValueError, "list/tuple argument does not have enough elements");
        return NULL;
    }
    
    x = x % self->image->width;
    y = y % self->image->height;
    ptr = &(self->image->data[PIXEL_INDEX(self->image, x, y)]);
    
    for (c = 0; c < self->image->channels; c++) {
        tmp = pixel_methods->GetItem(pixel, c);
        Py_INCREF(tmp);
        value = (REAL)PyFloat_AsDouble(tmp);
        Py_DECREF(tmp);
        
        *ptr++ = value;
    }
    
    if (PyErr_Occurred()) {
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_hzline_in(ImageBuffer *self, PyObject *args)
{
    size_t i, l;
    DIMENSION y;
    REAL *ptr;
    REAL value;
    PyObject *line, *tmp;
    struct ListTypeMethods *line_methods;
    
    if (!PyArg_ParseTuple(args, "IO", &y, &line)) {
        return NULL;
    }
    
    if (!(line_methods = GetListMethods(line))) {
        return NULL;
    }
    
    y = y % self->image->height;
    l = line_methods->Size(line);
    if (l > (self->image->width * self->image->channels)) {
        l = self->image->width * self->image->channels;
    }
    
    ptr = &(self->image->data[PIXEL_INDEX(self->image, 0, y)]);
    
    for (i = 0; i < l; i++) {
        tmp = line_methods->GetItem(line, i);
        Py_INCREF(tmp);
        value = (REAL)PyFloat_AsDouble(tmp);
        Py_DECREF(tmp);
        
        *ptr++ = value;
    }
    
    if (PyErr_Occurred()) {
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_hzline_out(ImageBuffer *self, PyObject *args)
{
    volatile int error = 0;
    
    size_t i, l;
    DIMENSION y;
    REAL *ptr;
    PyObject *tmp = NULL,
             *line_out = NULL;
    
    if (!PyArg_ParseTuple(args, "I", &y)) {
        return NULL;
    }
    
    y = y % self->image->height;
    l = self->image->width * self->image->channels;
    line_out = PyList_New(l);
    if (!line_out) {
        return NULL;
    }
    
    ptr = &(self->image->data[PIXEL_INDEX(self->image, 0, y)]);
    for (i = 0; i < l; i++) {
        tmp = PyFloat_FromDouble(*ptr);
        if (!tmp) {
            goto error_out;
        }
        
        PyList_SetItem(line_out, i, tmp);
        ptr++;
    }
    
    if (error) {
        error_out:
            Py_XDECREF(line_out);
            return NULL;
    }
    
    return line_out;
}

PyObject *ImageBuffer_vtline_in(ImageBuffer *self, PyObject *args)
{
    size_t y, i, l, c, pitch;
    DIMENSION x;
    REAL *ptr;
    REAL value;

    PyObject *line, *tmp;
    struct ListTypeMethods *line_methods;
    
    if (!PyArg_ParseTuple(args, "IO", &x, &line)) {
        return NULL;
    }
    
    if (!(line_methods = GetListMethods(line))) {
        return NULL;
    }
    
    x = x % self->image->width;
    l = line_methods->Size(line) / self->image->channels;
    if (l > self->image->height) {
        l = self->image->height;
    }
    
    ptr = &(self->image->data[PIXEL_INDEX(self->image, x, 0)]);
    pitch = self->image->pitch;
    i = 0;
    
    for (y = 0; y < l; y++) {
        for (c = 0; c < self->image->channels; c++) {
            tmp = line_methods->GetItem(line, i);
            Py_INCREF(tmp);
            value = PyFloat_AsDouble(tmp);
            Py_DECREF(tmp);
            
            *(ptr+c) = value;
            i++;
        }
        
        ptr += pitch;
    }
    
    if (PyErr_Occurred()) {
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_vtline_out(ImageBuffer *self, PyObject *args)
{
    volatile int error = 0;

    size_t y, i, l, c, pitch;
    DIMENSION x;
    REAL *ptr;
    PyObject *tmp = NULL,
             *line_out = NULL;
    
    if (!PyArg_ParseTuple(args, "I", &x)) {
        return NULL;
    }
    
    pitch = self->image->pitch;
    l = self->image->height;
    
    line_out = PyList_New((l * self->image->channels));
    if (!line_out) {
        return NULL;
    }
    
    ptr = &(self->image->data[PIXEL_INDEX(self->image, x, 0)]);
    i = 0;
    
    for (y = 0; y < l; y++) {
        for (c = 0; c < self->image->channels; c++) {
            tmp = PyFloat_FromDouble(*(ptr+c));
            if (!tmp) {
                goto error_out;
            }
            
            PyList_SetItem(line_out, i, tmp);
            i++;
        }
        
        ptr += pitch;
    }
    
    if (error) {
        error_out:
            Py_XDECREF(line_out);
            return NULL;
    }
    
    return line_out;
}

PyObject *ImageBuffer_to_hsv(ImageBuffer *self, PyObject *args)
{
    if (ImageKit_Image_toHSV(self->image) < 1) {
        PyErr_SetString(PyExc_Exception, "Failed to convert image to HSV");
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_to_rgb(ImageBuffer *self, PyObject *args, PyObject *kwargs)
{
    static char *kwargs_names[] = {"colorspace_format", "scale", NULL};
    REAL scale = -1;
    int fmt = -1;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i|f", kwargs_names, &fmt, &scale)) {
        return NULL;
    }
    
    if (ImageKit_Image_toRGB(self->image, fmt, scale) < 1) {
        PyErr_SetString(PyExc_Exception, "Failed to convert image to RGB");
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_to_mono(ImageBuffer *self, PyObject *args)
{
    if (ImageKit_Image_toMono(self->image) < 1) {
        PyErr_SetString(PyExc_Exception, "Failed to convert image to Mono");
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_fill_image(ImageBuffer *self, PyObject *args)
{
    int i, l;
    PyObject *color, *tmp;
    struct ListTypeMethods *color_methods;
    REAL color4f[4];
    
    if (!PyArg_ParseTuple(args, "O", &color)) {
        return NULL;
    }
    
    if (!(color_methods = GetListMethods(color))) {
        return NULL;
    }
    
    l = (int)color_methods->Size(color);
    if (l > self->image->channels) {
        l = self->image->channels;
    }
    
    for (i = 0; i < l; i++) {
        tmp = color_methods->GetItem(color, i);
        Py_INCREF(tmp);
        color4f[i] = PyFloat_AsDouble(tmp);
        Py_DECREF(tmp);
    }
    
    if (PyErr_Occurred()) {
        return NULL;
    }
    
    if (ImageKit_Image_Fill(self->image, (REAL *)&color4f) < 1) {
        PyErr_SetString(PyExc_Exception, "Failed to fill image");
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_fill_image_channel(ImageBuffer *self, PyObject *args)
{
    REAL value;
    uint32_t channel;
    
    if (!PyArg_ParseTuple(args, "fI", &value, &channel)) {
        return NULL;
    }
    
    if (ImageKit_Image_FillChannel(self->image, value, channel) < 1) {
        PyErr_SetString(PyExc_Exception, "Failed to fill channel");
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_remove_alpha(ImageBuffer *self, PyObject *args)
{
    if (ImageKit_Image_RemoveAlpha(self->image) < 1) {
        PyErr_SetString(PyExc_Exception, "Failed to remove alpha");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_apply_matrix(ImageBuffer *self, PyObject *args)
{
    /*
    
    TODO: Support for coordinates parameter
    
    */

    int i, l;
    PyObject *arg_matrix, *tmp;
    struct ListTypeMethods *arg_methods;
    REAL matrix[4];
    
    if (!PyArg_ParseTuple(args, "O", &arg_matrix)) {
        return NULL;
    }
    
    if (!(arg_methods = GetListMethods(arg_matrix))) {
        return NULL;
    }
    
    l = (int)arg_methods->Size(arg_matrix);
    if (l > self->image->channels) {
        l = self->image->channels;
    }
    
    for (i = 0; i < l; i++) {
        tmp = arg_methods->GetItem(arg_matrix, i);
        Py_INCREF(tmp);
        matrix[i] = PyFloat_AsDouble(tmp);
        Py_DECREF(tmp);
    }
    
    if (PyErr_Occurred()) {
        return NULL;
    }
    
    if (ImageKit_Image_ApplyMatrix(self->image, (REAL *)&matrix, NULL) < 1) {
        PyErr_SetString(PyExc_Exception, "Failed to apply matrix");
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_apply_cvkernel(ImageBuffer *self, PyObject *args, PyObject *kwargs)
{
    /*
    
    TODO: Support for coordinates parameter
    
    */
    
    static char *kwarg_names[] = {
        "matrix",
        "factor",
        "bias",
        "preserve_alpha",
        //"coords",
        NULL
    };

    int i, l, result;
    PyObject *arg_matrix, *tmp;
    struct ListTypeMethods *arg_methods;
    REAL *matrix;
    
    double kernel_size_d;
    uint32_t kernel_size;
    
    REAL factor = 1.0;
    REAL bias = 0.0;
    int32_t preserve_alpha = 1;
    
    if (!PyArg_ParseTupleAndKeywords(   args,
                                        kwargs,
                                        "O|f|f|i",
                                        kwarg_names,
                                        &arg_matrix,
                                        &factor,
                                        &bias,
                                        &preserve_alpha)) {
        return NULL;
    }
    
    if (!(arg_methods = GetListMethods(arg_matrix))) {
        return NULL;
    }
    
    l = (int)arg_methods->Size(arg_matrix);
    kernel_size_d = sqrt(l);
    kernel_size = (int32_t)floor(kernel_size_d);
    
    if (!(kernel_size % 2)) {
        PyErr_SetString(PyExc_ValueError, "Kernel must be an odd size (eg: 3x3, 5x5)");
        return NULL;
    }
    
    if ((round(kernel_size_d * 1000) / 1000) != kernel_size_d) {
        PyErr_SetString(PyExc_ValueError, "Kernel must be square (eg: 3x3, 5x5)");
        return NULL;
    }
    
    matrix = malloc(sizeof(*matrix) * l);
    for (i = 0; i < l; i++) {
        tmp = arg_methods->GetItem(arg_matrix, i);
        Py_INCREF(tmp);
        matrix[i] = PyFloat_AsDouble(tmp);
        Py_DECREF(tmp);
    }
    
    if (PyErr_Occurred()) {
        free(matrix);
        return NULL;
    }
    
    result = ImageKit_Image_ApplyCVKernel(
        self->image,
        matrix,
        kernel_size,
        factor,
        bias,
        preserve_alpha,
        NULL
    );
    
    if (result < 1) {
        free(matrix);
        PyErr_SetString(PyExc_Exception, "Failed to apply convolution kernel");
        return NULL;
    }
    
    free(matrix);
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_apply_rankfilter(ImageBuffer *self, PyObject *args, PyObject *kwargs)
{
    /*
    
    TODO: Support for coordinates parameter
    
    */

    static char *kwarg_names[] = {"matrix_size", "rank", NULL};
    int result;
    uint32_t matrix_size = 3;
    REAL rank = 0.5;
    
    if (!PyArg_ParseTupleAndKeywords(   args,
                                        kwargs,
                                        "|I|f",
                                        kwarg_names,
                                        &matrix_size,
                                        &rank)) {
        return NULL;
    }
    
    if (!(matrix_size % 2)) {
        PyErr_SetString(PyExc_ValueError, "matrix_size must be an odd size (eg: 3x3, 5x5)");
        return NULL;
    }
    
    result = ImageKit_Image_ApplyRankFilter(
        self->image,
        matrix_size,
        rank,
        NULL
    );
    
    if (result < 1) {
        PyErr_SetString(PyExc_Exception, "Failed to apply rank filter");
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_scale_nearest(ImageBuffer *self, PyObject *args)
{
    ImageKit_Image *input, *scaled;
    DIMENSION width, height;
    
    if (!PyArg_ParseTuple(args, "II", &width, &height)) {
        return NULL;
    }
    
    input = self->image;
    scaled = ImageKit_Image_ScaleNearest(input, width, height);
    if (!scaled) {
        PyErr_SetString(PyExc_Exception, "Failed to scale image");
        return NULL;
    }
    
    self->image = scaled;
    ImageKit_Image_Delete(input);
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_scale_bilinear(ImageBuffer *self, PyObject *args)
{
    ImageKit_Image *input, *scaled;
    DIMENSION width, height;
    
    if (!PyArg_ParseTuple(args, "II", &width, &height)) {
        return NULL;
    }
    
    input = self->image;
    scaled = ImageKit_Image_ScaleBilinear(input, width, height);
    if (!scaled) {
        PyErr_SetString(PyExc_Exception, "Failed to scale image");
        return NULL;
    }
    
    self->image = scaled;
    ImageKit_Image_Delete(input);
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_draw_bresenham_line(ImageBuffer *self, PyObject *args)
{
    size_t i, l;
    PyObject *arg_color, *tmp;
    struct ListTypeMethods *arg_color_methods;
    DIMENSION x0, y0, x1, y1;
    REAL color[4] = {0, 0, 0, 0};
    
    if (!PyArg_ParseTuple(args, "IIIIO", &x0, &y0, &x1, &y1, &arg_color)) {
        return NULL;
    }
    
    if (!(arg_color_methods = GetListMethods(arg_color))) {
        return NULL;
    }
    
    l = arg_color_methods->Size(arg_color);
    if (l > self->image->channels) {
        l = self->image->channels;
    }
    
    for (i = 0; i < l; i++) {
        tmp = arg_color_methods->GetItem(arg_color, i);
        Py_INCREF(tmp);
        color[i] = PyFloat_AsDouble(tmp);
        Py_DECREF(tmp);
    }
    
    if (PyErr_Occurred()) {
        return NULL;
    }
    
    if (ImageKit_Image_DrawBresenhamLine(self->image, x0, y0, x1, y1, (REAL *)&color) < 1) {
        PyErr_SetString(PyExc_Exception, "Failed to draw line");
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_draw_bresenham_circle(ImageBuffer *self, PyObject *args)
{
    size_t i, l;
    PyObject *arg_color, *tmp;
    struct ListTypeMethods *arg_color_methods;
    DIMENSION mid_x, mid_y, radius;
    REAL color[4] = {0, 0, 0, 0};
    
    if (!PyArg_ParseTuple(args, "IIIO", &mid_x, &mid_y, &radius, &arg_color)) {
        return NULL;
    }
    
    if (!(arg_color_methods = GetListMethods(arg_color))) {
        return NULL;
    }
    
    l = arg_color_methods->Size(arg_color);
    if (l > self->image->channels) {
        l = self->image->channels;
    }
    
    for (i = 0; i < l; i++) {
        tmp = arg_color_methods->GetItem(arg_color, i);
        Py_INCREF(tmp);
        color[i] = PyFloat_AsDouble(tmp);
        Py_DECREF(tmp);
    }
    
    if (PyErr_Occurred()) {
        return NULL;
    }
    
    if (ImageKit_Image_DrawBresenhamCircle(self->image, mid_x, mid_y, radius, (REAL *)&color) < 1) {
        PyErr_SetString(PyExc_Exception, "Failed to draw line");
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *ImageBuffer_draw_bresenham_ellipse(ImageBuffer *self, PyObject *args)
{
    size_t i, l;
    PyObject *arg_color, *tmp;
    struct ListTypeMethods *arg_color_methods;
    DIMENSION x0, y0, x1, y1;
    REAL color[4] = {0, 0, 0, 0};
    
    if (!PyArg_ParseTuple(args, "IIIIO", &x0, &y0, &x1, &y1, &arg_color)) {
        return NULL;
    }
    
    if (!(arg_color_methods = GetListMethods(arg_color))) {
        return NULL;
    }
    
    l = arg_color_methods->Size(arg_color);
    if (l > self->image->channels) {
        l = self->image->channels;
    }
    
    for (i = 0; i < l; i++) {
        tmp = arg_color_methods->GetItem(arg_color, i);
        Py_INCREF(tmp);
        color[i] = PyFloat_AsDouble(tmp);
        Py_DECREF(tmp);
    }
    
    if (PyErr_Occurred()) {
        return NULL;
    }
    
    if (ImageKit_Image_DrawBresenhamEllipse(self->image, x0, y0, x1, y1, (REAL *)&color) < 1) {
        PyErr_SetString(PyExc_Exception, "Failed to draw line");
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

/*

from imagekit import *
b = Image.fromPNG('/Users/cleure/Development/Projects/TV4X/input-images/bomberman_1.png')
b.draw_line(0, 0, 64, 49, (255, 255, 255))
b.draw_ellipse(0, 0, 64, 49, (0, 255, 255))
b.draw_circle(96, 96, 16, (255, 255, 0))
b.savePNG('output.png')

*/

/*

API
int
ImageKit_Image_FillCoords(ImageKit_Image *self, ImageKit_Coords *coords, REAL *color);

typedef struct Coordinates {
    size_t data_size;
    size_t data_items;
    size_t data_index;
    DIMENSION *coords;
} ImageKit_Coords;

API
ImageKit_Coords *
ImageKit_Coords_New(size_t items);

API
ImageKit_Coords *
ImageKit_Coords_FromRect(ImageKit_Rect *rect);

API
void
ImageKit_Coords_Delete(ImageKit_Coords *self);

API
int
ImageKit_Coords_Resize(ImageKit_Coords *self, size_t items);

API
int
ImageKit_Coords_Append(ImageKit_Coords *self, DIMENSION x, DIMENSION y);


API
int
ImageKit_Image_BlitRect(
    ImageKit_Image *dst,
    ImageKit_Rect *dst_rect,
    ImageKit_Image *src,
    ImageKit_Rect *src_rect
);

API
int
ImageKit_Image_BlitCoords(
    ImageKit_Image *dst,
    DIMENSION dst_x,
    DIMENSION dst_y,
    ImageKit_Image *src,
    ImageKit_Coords *src_coords
);


typedef struct ImageKit_PointFilter {
    uint32_t samples;
    
    REAL *a;
    REAL *b;
    REAL *c;
    REAL *d;
} ImageKit_PointFilter;

API
ImageKit_PointFilter *
ImageKit_PointFilter_New(uint32_t samples);

API
ImageKit_PointFilter *
ImageKit_PointFilter_FromCurves(
    ImageKit_Curves *curves_a,
    ImageKit_Curves *curves_b,
    ImageKit_Curves *curves_c,
    ImageKit_Curves *curves_d
);

API
int
ImageKit_PointFilter_Apply(
    ImageKit_PointFilter *self,
    ImageKit_Image *image,
    ImageKit_Coords *coords
);

API
void
ImageKit_PointFilter_Delete(ImageKit_PointFilter *self);


typedef struct ImageKit_Curves {
    DIMENSION data_items;
    REAL *coords;
} ImageKit_Curves;

API
ImageKit_Curves *
ImageKit_Curves_FromBezier(uint32_t samples, REAL *xy, size_t xy_items);

API
void
ImageKit_Curves_Delete(ImageKit_Curves *self);

typedef struct ImageKit_Rect {
    DIMENSION x;
    DIMENSION y;
    DIMENSION w;
    DIMENSION h;
} ImageKit_Rect;

*/

PyObject *ImageBuffer_getattr(PyObject *_self, char *name)
{
    ImageBuffer *self = (ImageBuffer *)_self;
    PyObject *value = NULL;
    
    if (strcmp(name, "width") == 0) {
        value = PyInt_FromLong(self->image->width);
    } else if (strcmp(name, "height") == 0) {
        value = PyInt_FromLong(self->image->height);
    } else if (strcmp(name, "channels") == 0) {
        value = PyInt_FromLong(self->image->channels);
    } else if (strcmp(name, "colorspace") == 0) {
        value = PyInt_FromLong(self->image->colorspace);
    } else if (strcmp(name, "colorspace_format") == 0) {
        value = PyInt_FromLong(self->image->colorspace_format);
    } else if (strcmp(name, "scale") == 0) {
        value = PyFloat_FromDouble(self->image->scale);
    }
    
    if (value != NULL) {
        return value;
    }
    
    PyErr_SetString(PyExc_AttributeError, "No such attribute");
    return NULL;
}

static PyObject *MODULE = NULL;
static const char *documentation =
    "TODO: Description";

/* ImageBuffer Type */
static PyTypeObject ImageBuffer_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
};

static PyMemberDef ImageBuffer_members[] = {{NULL}};
static PyMethodDef ImageBuffer_methods[] = {
    {
        "fromPNG",
         (void *)ImageBuffer_from_png,
         METH_STATIC | METH_KEYWORDS,
        "Creates an ImageBuffer instance from a PNG file"
    },
    {
        "savePNG",
         (void *)ImageBuffer_save_png,
         METH_VARARGS,
        "Saves data contained in instance to PNG file"
    },
    {
        "fromJPEG",
         (void *)ImageBuffer_from_jpeg,
         METH_STATIC | METH_KEYWORDS,
        "Creates an ImageBuffer instance from a JPEG file"
    },
    {
        "saveJPEG",
         (void *)ImageBuffer_save_jpeg,
         METH_VARARGS,
        "Saves data contained in instance to JPEG file"
    },
    {
        "channel_ranges",
         (void *)ImageBuffer_channel_ranges,
         METH_VARARGS,
         "Get channel min/max ranges. "
         "Returns: ((min), (max)). "
         "min and max tuples contain the same number of elements "
         "as self.channels"
    },
    {
        "get_histogram",
         (void *)ImageBuffer_get_histogram,
         METH_VARARGS,
         "Takes: int samples"
         "Returns list of length samples"
    },
    {
        "get_pixel",
         (void *)ImageBuffer_get_pixel,
         METH_VARARGS,
         "Get tuple at x/y (slower). "
         "(int x, int y)"
    },
    {
        "set_pixel",
         (void *)ImageBuffer_set_pixel,
         METH_VARARGS,
         "Set tuple at x/y (slower). "
         "(int x, int y, tuple of float)"
    },
    {
        "hzline_in",
         (void *)ImageBuffer_hzline_in,
         METH_VARARGS,
        "Copy horizontal line of pixels in. List must be in the "
        "following format: [A, B, C, A, B, C] where A, B, and C "
        "are the color channel values."
    },
    {
        "hzline_out",
         (void *)ImageBuffer_hzline_out,
         METH_VARARGS,
        "Copy horizontal line out to list."
    },
    {
        "vtline_in",
         (void *)ImageBuffer_vtline_in,
         METH_VARARGS,
        "Copy vertical line of pixels in. List must be in the "
        "following format: [A, B, C, A, B, C] where A, B, and C "
        "are the color channel values."
    },
    {
        "vtline_out",
         (void *)ImageBuffer_vtline_out,
         METH_VARARGS,
        "Copy vertical line out to list."
    },
    {
        "toHSV",
         (void *)ImageBuffer_to_hsv,
         METH_VARARGS,
        "Convert image to HSV colorspace"
    },
    {
        "toRGB",
         (void *)ImageBuffer_to_rgb,
         METH_VARARGS | METH_KEYWORDS,
        "Convert image to RGB colorspace"
    },
    {
        "toMono",
         (void *)ImageBuffer_to_mono,
         METH_VARARGS | METH_KEYWORDS,
        "Convert image to Mono/Grayscale"
    },
    {
        "fill_image",
        (void *)ImageBuffer_fill_image,
        METH_VARARGS,
        "Fill entire image with color. Argument is list or tuple"
    },
    {
        "fill_channel",
        (void *)ImageBuffer_fill_image_channel,
        METH_VARARGS,
        "Fill entire channel of image with value. Arguments: "
        "(value, channel)"
    },
    {
        "remove_alpha",
        (void *)ImageBuffer_remove_alpha,
        METH_VARARGS,
        "Remove alpha channel from image."
    },
    {
        "apply_matrix",
         (void *)ImageBuffer_apply_matrix,
         METH_VARARGS,
        "Apply multiplication matrix to image. Matrix must have "
        "the same number of elements as there are channels. Ex:\n"
        "\t# Reduce red by 50%\n"
        "\tb.apply_matrix([0.5, 1.0, 1.0])\n"
    },
    {
        "apply_cvkernel",
         (void *)ImageBuffer_apply_cvkernel,
         METH_VARARGS | METH_KEYWORDS,
        "Apply convolution kernel to image. Matrix must be flat, "
        "with an odd number of elements (eg: 3x3, 5x5, 7x7). "
        "Ex:\n"
        "\tblur_kernel = [\n"
        "\t\t0.111, 0.111, 0.111,\n"
        "\t\t0.111, 0.111, 0.111,\n"
        "\t\t0.111, 0.111, 0.111,\n"
        "\t]\n"
        "\tb.apply_cvkernel(blur_kernel)"
    },
    {
        "apply_rankfilter",
         (void *)ImageBuffer_apply_rankfilter,
         METH_VARARGS | METH_KEYWORDS,
        "Apply rank filter to image. Ex:\n"
        "\t# 3x3 Median\n"
        "\tb.apply_rankfilter(3, 0.5)\n"
        "\t# 5x5 Max\n"
        "\tb.apply_rankfilter(5, 1.0)\n"
        "\t# 7x7 Min\n"
        "\tb.apply_rankfilter(7, 0.0)\n"
    },
    {
        "scale_nearest",
         (void *)ImageBuffer_scale_nearest,
         METH_VARARGS,
        "Scale image using nearest neighbor algorithm. "
        "(new_width, new_height)"
    },
    {
        "scale_bilinear",
         (void *)ImageBuffer_scale_bilinear,
         METH_VARARGS,
        "Scale image using bilinear algorithm. "
        "(new_width, new_height)"
    },
    {
        "draw_line",
        (void *)ImageBuffer_draw_bresenham_line,
        METH_VARARGS,
        "Draw line using bresenham algorithm. Args: "
        "(x0, y0, x1, y1, (color))"
    },
    {
        "draw_circle",
        (void *)ImageBuffer_draw_bresenham_circle,
        METH_VARARGS,
        "Draw circle using bresenham algorithm. Args: "
        "(mid_x, mid_y, radius, (color))"
    },
    {
        "draw_ellipse",
        (void *)ImageBuffer_draw_bresenham_ellipse,
        METH_VARARGS,
        "Draw circle using bresenham algorithm. Args: "
        "(x0, y0, x1, y1, (color))"
    },
    /*
    {
        "blit_rect",
         (void *)ImageBuffer_blit_rect,
         METH_VARARGS,
        "Blit rectangle."
    },
    {
        "__copy__",
         (void *)ImageBuffer_copy,
         METH_VARARGS,
        "Returns Clone of ImageBuffer Object"
    },
    {
        "__deepcopy__",
         (void *)ImageBuffer_copy,
         METH_VARARGS,
        "Returns Clone of ImageBuffer Object"
    },
    {
        "get_box",
         (void *)ImageBuffer_get_box,
         METH_VARARGS,
        "DUMMY"
    },*/
    {NULL, NULL, 0, NULL}
};

static PyMethodDef moduleMethods[] = {
    {NULL, NULL, 0, NULL}
};

#ifdef IS_PY3K
    static PyModuleDef ImageKit_Module = {
        PyModuleDef_HEAD_INIT,
        "imagekit",
        NULL,
        -1,
        moduleMethods,
        NULL,
        NULL,
        NULL,
        NULL
    };
#endif

    PyMODINIT_FUNC
    initimagekit(void)
    {
    #ifdef IS_PY3K
    #define RETURN_ERROR() return NULL
    #define RETURN_SUCCESS() return MODULE
    #else
    #define RETURN_ERROR() return
    #define RETURN_SUCCESS() return
    #endif
    
    #ifdef IS_PY3K
        MODULE = PyModule_Create(&ImageKit_Module);
    #else
        /* Init Module */
        MODULE = Py_InitModule3(    "imagekit",
                                    moduleMethods,
                                    (char *)documentation);
    #endif
    
        if (!MODULE) {
            RETURN_ERROR();
        }
        
        /* Init ImageBuffer Type */
        ImageBuffer_Type.tp_new = PyType_GenericNew;
        ImageBuffer_Type.tp_name = "imagekit.ImageBuffer";
        ImageBuffer_Type.tp_basicsize = sizeof(ImageBuffer);
        ImageBuffer_Type.tp_flags = Py_TPFLAGS_DEFAULT;
        ImageBuffer_Type.tp_init = (initproc)ImageBuffer_init;
        ImageBuffer_Type.tp_dealloc = (destructor)ImageBuffer_dealloc;
        ImageBuffer_Type.tp_methods = ImageBuffer_methods;
        ImageBuffer_Type.tp_members = ImageBuffer_members;
        //ImageBuffer_Type.tp_getattr = ImageBuffer_getattr;
        ImageBuffer_Type.tp_getattro = PyObject_GenericGetAttr;
        ImageBuffer_Type.tp_setattro = PyObject_GenericSetAttr;
        ImageBuffer_Type.tp_doc = "Doc string for class Bar in module Foo.";
        
        if (PyType_Ready(&ImageBuffer_Type) < 0) {
            RETURN_ERROR();
        }

        Py_INCREF(&ImageBuffer_Type);
        PyModule_AddObject(MODULE, "ImageBuffer", (PyObject*)&ImageBuffer_Type);
        PyModule_AddObject(MODULE, "Image", (PyObject*)&ImageBuffer_Type);
        
        /* Module Constants */
        PyModule_AddStringConstant(MODULE, "__version__", "2.0");
        
        RETURN_SUCCESS();
    }
