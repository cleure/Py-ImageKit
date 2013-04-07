#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <setjmp.h>
#include <memory.h>
#include <math.h>
#include <png.h>
#include <Python.h>
#include <structmember.h>

#define IK_INTERNAL

/*

TODO:
        - Cleanup error messages
        - get_box()
        - Convert between colorspaces.
        - Grayscale support
        - Convert colorspace to native in save*()/load*() methods.
        - Filters
            - apply_cvkernel()
            - apply_median()
        - Scale nearest/linear
        - fromJPG() / saveJPG().
        - Test with RGB30 and RGB48.
        - Python 3.x support.
        - Documentation.

*/

#define REAL_TYPE float

#define PIXEL_INDEX(self, x, y)\
    (self->pitch * (y)) + ((x) * self->channels)

#define CS_FMT(in) COLORSPACE_FORMAT_##in

static PyObject *MODULE = NULL;

/* Constants */
static const int HAVE_PNG           = HAVE_LIBPNG;
static const int HAVE_JPEG          = HAVE_LIBJPEG;
static const int HAVE_GIF           = HAVE_LIBGIF;

static const enum {
    COLORSPACE_MONO,
    COLORSPACE_RGB,
    COLORSPACE_HSV,
    COLORSPACE_YIQ,
    COLORSPACE_SIZE
} _COLORSPACE;

static const enum {
    CS_FMT(RGB15),
    CS_FMT(RGB16),
    CS_FMT(RGB24),
    CS_FMT(RGB30),
    CS_FMT(RGB48),
    CS_FMT(HSV_NATURAL),
    COLORSPACE_FORMAT_SIZE
} _COLORSPACE_FORMAT;

static const double COLORSPACE_FORMAT_MINMAX[COLORSPACE_FORMAT_SIZE][8] = {
    {0.0, 0.0, 0.0, 0.0,    31.0,      31.0,       31.0,     0.0},
    {0.0, 0.0, 0.0, 0.0,    31.0,      63.0,       31.0,     0.0},
    {0.0, 0.0, 0.0, 0.0,   255.0,     255.0,      255.0,   255.0},
    {0.0, 0.0, 0.0, 0.0,  1023.0,    1023.0,     1023.0,     0.0},
    {0.0, 0.0, 0.0, 0.0, 65535.0,   65535.0,    65535.0, 65535.0},
    {0.0, 0.0, 0.0, 0.0,   360.0,       1.0,        1.0,     1.0},
};

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

static const char *documentation =
    "TODO: Description";

typedef struct {
    PyObject_HEAD
    
    REAL_TYPE scale;
    REAL_TYPE channel_scales[4];
    int colorspace;
    int colorspace_format;
    
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    uint32_t pitch;
    size_t data_size;
    size_t data_items;
    
    float *data;
} ImageBuffer;

static PyTypeObject ImageBuffer_Type = {
    PyObject_HEAD_INIT(NULL)
};

//#include "sort.c"
#include "cs-convert.c"

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
    //printf("Object Destroyed\n");
    free(self->data);
    self->ob_type->tp_free((PyObject *)self);
}

static PyObject *ImageBuffer_get_pixel(ImageBuffer *self, PyObject *args)
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

static PyObject *ImageBuffer_set_pixel(ImageBuffer *self, PyObject *args)
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

static PyObject *ImageBuffer_set1(ImageBuffer *self, PyObject *args)
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

static PyObject *ImageBuffer_set3(ImageBuffer *self, PyObject *args)
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

static PyObject *ImageBuffer_set4(ImageBuffer *self, PyObject *args)
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

static PyObject *ImageBuffer_hzline_in(ImageBuffer *self, PyObject *args)
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

/*

from imagekit import *
bi = ImageBuffer.fromPNG('/Users/cleure/Development/Projects/image-processing/py_src/source/source-img2.png')
bo = ImageBuffer(width=bi.height, height=bi.width, channels=bi.channels)

for y in xrange(bi.height):
    li = bi.hzline_out(y)
    bo.vtline_in(bi.height-y-1, li)

bo.savePNG('output.png')

*/

static PyObject *ImageBuffer_hzline_out(ImageBuffer *self, PyObject *args)
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

static PyObject *ImageBuffer_vtline_in(ImageBuffer *self, PyObject *args)
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

static PyObject *ImageBuffer_vtline_out(ImageBuffer *self, PyObject *args)
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

static PyObject *ImageBuffer_to_hsv(ImageBuffer *self, PyObject *args)
{
    if (self->colorspace == COLORSPACE_RGB) {
        ImageBuffer_rgb_to_hsv(self);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *ImageBuffer_to_rgb(ImageBuffer *self, PyObject *args, PyObject *kwargs)
{
    static char *kwargs_names[] = {
        "colorspace_format",
        "scale_max",
        NULL
    };

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
        ImageBuffer_hsv_to_rgb(self, colorspace_format, scale_max);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

/*

from imagekit import *
matrix = [
    0.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 0.0,
]
b = ImageBuffer(256, 256, 3)
for x in range(256):
    for y in range(10):
        b.set_pixel(x, y+32, [255, 255, 255])

b.savePNG('1.png')
b.apply_matrix(matrix)
b.savePNG('2.png')

*/

/* Include source files */
#include "filter.c"
#include "png.c"

static PyMemberDef ImageBuffer_members[] = {
    {
        "width",
        T_INT,
        offsetof(ImageBuffer, width),
        0,
        "width",
    },
    {
        "height",
        T_INT,
        offsetof(ImageBuffer, height),
        0,
        "height",
    },
    {
        "channels",
        T_INT,
        offsetof(ImageBuffer, channels),
        0,
        "channels",
    },
    {
        "colorspace",
        T_INT,
        offsetof(ImageBuffer, colorspace),
        0,
        "colorspace",
    },
    {
        "colorspace_format",
        T_INT,
        offsetof(ImageBuffer, colorspace_format),
        0,
        "colorspace_format",
    },
    {
        "scale",
        T_FLOAT,
        offsetof(ImageBuffer, scale),
        0,
        "scale",
    },
    {NULL}
};

static PyMethodDef ImageBuffer_methods[] = {
    {
        "fromPNG",
         (void *)ImageBuffer_from_png,
         METH_STATIC | METH_KEYWORDS,
        "DUMMY"
    },
    {
        "savePNG",
         (void *)ImageBuffer_save_png,
         METH_VARARGS | METH_KEYWORDS,
        "DUMMY"
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
        "set1",
         (void *)ImageBuffer_set1,
         METH_VARARGS,
         "Set channel 0 at x/y\n"
         "(int x, int y, float v)"
    },
    {
        "set3",
         (void *)ImageBuffer_set3,
         METH_VARARGS,
         "Set channels 0, 1 and 2 at x/y. "
         "(int x, int y, float v1, float v2, float v3)"
    },
    {
        "set4",
         (void *)ImageBuffer_set4,
         METH_VARARGS,
         "Set channels 0, 1, 2, and 3 at x/y. "
         "(int x, int y, float v1, float v2, float v3, float v4)"
    },
    {
        "hzline_in",
         (void *)ImageBuffer_hzline_in,
         METH_VARARGS,
        "DUMMY"
    },
    {
        "hzline_out",
         (void *)ImageBuffer_hzline_out,
         METH_VARARGS,
        "DUMMY"
    },
    {
        "vtline_in",
         (void *)ImageBuffer_vtline_in,
         METH_VARARGS,
        "DUMMY"
    },
    {
        "vtline_out",
         (void *)ImageBuffer_vtline_out,
         METH_VARARGS,
        "DUMMY"
    },
    {
        "apply_matrix",
         (void *)ImageBuffer_apply_matrix,
         METH_VARARGS,
        "DUMMY"
    },
    {
        "toHSV",
         (void *)ImageBuffer_to_hsv,
         METH_VARARGS,
        "DUMMY"
    },
    {
        "toRGB",
         (void *)ImageBuffer_to_rgb,
         METH_VARARGS | METH_KEYWORDS,
        "DUMMY"
    },
    {NULL, NULL, 0, NULL}
};

static PyMethodDef moduleMethods[] = {
    {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION <= 2
    /* Python 1.x/2.x */
    
    #ifdef __cplusplus
        extern "C"
    #endif
    PyMODINIT_FUNC
    initimagekit(void)
    {
        /* Init Module */
        MODULE = Py_InitModule3(    "imagekit",
                                    moduleMethods,
                                    (char *)documentation);
        
        if (!MODULE) {
            return;
        }
        
        /* Init ImageBuffer Type */
        ImageBuffer_Type.tp_new = PyType_GenericNew;
        ImageBuffer_Type.tp_name = "imagekit.ImageBuffer";
        ImageBuffer_Type.tp_basicsize = sizeof(ImageBuffer);
        ImageBuffer_Type.tp_getattro = PyObject_GenericGetAttr;
        ImageBuffer_Type.tp_setattro = PyObject_GenericSetAttr;
        ImageBuffer_Type.tp_flags = Py_TPFLAGS_DEFAULT;
        ImageBuffer_Type.tp_init = (initproc)ImageBuffer_init;
        ImageBuffer_Type.tp_dealloc = (destructor)ImageBuffer_dealloc;
        ImageBuffer_Type.tp_methods = ImageBuffer_methods;
        ImageBuffer_Type.tp_members = ImageBuffer_members;
        ImageBuffer_Type.tp_doc = "Doc string for class Bar in module Foo.";
        
        if (PyType_Ready(&ImageBuffer_Type) < 0) {
            return;
        }

        Py_INCREF(&ImageBuffer_Type);
        PyModule_AddObject(MODULE, "ImageBuffer", (PyObject*)&ImageBuffer_Type);
        PyModule_AddObject(MODULE, "Image", (PyObject*)&ImageBuffer_Type);
        
        /* Module Constants */
        PyModule_AddStringConstant(MODULE, "__version__", "2.0");
        PyModule_AddIntConstant(MODULE, "HAVE_PNG", HAVE_PNG);
        PyModule_AddIntConstant(MODULE, "HAVE_JPEG", HAVE_JPEG);
        PyModule_AddIntConstant(MODULE, "HAVE_GIF", HAVE_GIF);
        
        PyModule_AddIntConstant(MODULE, "COLORSPACE_MONO", COLORSPACE_MONO);
        PyModule_AddIntConstant(MODULE, "COLORSPACE_RGB", COLORSPACE_RGB);
        PyModule_AddIntConstant(MODULE, "COLORSPACE_HSV", COLORSPACE_HSV);
        PyModule_AddIntConstant(MODULE, "COLORSPACE_YIQ", COLORSPACE_YIQ);
        
        PyModule_AddIntConstant(MODULE, "COLORSPACE_FORMAT_RGB15", CS_FMT(RGB15));
        PyModule_AddIntConstant(MODULE, "COLORSPACE_FORMAT_RGB16", CS_FMT(RGB16));
        PyModule_AddIntConstant(MODULE, "COLORSPACE_FORMAT_RGB24", CS_FMT(RGB24));
        PyModule_AddIntConstant(MODULE, "COLORSPACE_FORMAT_RGB30", CS_FMT(RGB30));
        PyModule_AddIntConstant(MODULE, "COLORSPACE_FORMAT_RGB48", CS_FMT(RGB48));
        PyModule_AddIntConstant(MODULE, "COLORSPACE_FORMAT_HSV_NATURAL", CS_FMT(HSV_NATURAL));
    }
#else
    /* Python 3.x */

/*
    static PyModuleDef murmur3_module = {
        PyModuleDef_HEAD_INIT,
        "murmur3",
        documentation,
        -1,
        methods,
        NULL,
        NULL,
        NULL,
        NULL
    };

    #ifdef __cplusplus
        extern "C"
    #endif
    PyMODINIT_FUNC
    PyInit_murmur3(void)
    {
        PyObject *module;
    
        module = PyModule_Create(&murmur3_module);
        if (!module) return module;
        PyModule_AddStringConstant(module, "__version__", MODULE_VERSION);
        
        return module;
    }*/
#endif
