
#ifndef IMAGEKIT_DOT_H
#define IMAGEKIT_DOT_H

#include <Python.h>
#include <structmember.h>

#define API static
#define PRIVATE static

#define REAL_TYPE float

#define PIXEL_INDEX(self, x, y)\
    (self->pitch * (y)) + ((x) * self->channels)

#define CS_FMT(in) COLORSPACE_FORMAT_##in

#define SORT_FN(ptr, size, elsize, cmpfn) (qsort((ptr), (size), (elsize), (cmpfn)))

struct ListTypeMethods {
    Py_ssize_t (*Size)(PyObject *);
    PyObject * (*GetItem)(PyObject *, Py_ssize_t);
    int (*SetItem)(PyObject *, Py_ssize_t, PyObject *);
    PyObject * (*GetSlice)(PyObject *, Py_ssize_t, Py_ssize_t);
};

/*

typedef struct _Image {
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
    
    REAL_TYPE *data;
};

typedef struct _Coordinates {
    uint32_t max_width;
    uint32_t max_height;
    size_t data_size;
    size_t data_items;
    uint32_t *coords;
};

*/

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
    
    REAL_TYPE *data;
} ImageBuffer;

typedef struct {
    PyObject_HEAD

    uint32_t max_width;
    uint32_t max_height;
    size_t data_size;
    size_t data_items;
    uint32_t *coords;
} Coordinates;

static PyObject *MODULE;
static const char *documentation;
static PyTypeObject ImageBuffer_Type;
static PyTypeObject Coordinates_Type;

/* Constants */
static const int HAVE_PNG;
static const int HAVE_JPEG;
static const int HAVE_GIF;

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
    CS_FMT(MONO_NATURAL),
    COLORSPACE_FORMAT_SIZE
} _COLORSPACE_FORMAT;

static const double COLORSPACE_FORMAT_MINMAX[COLORSPACE_FORMAT_SIZE][8];
static struct ListTypeMethods LIST_METHODS;
static struct ListTypeMethods TUPLE_METHODS;

#endif
