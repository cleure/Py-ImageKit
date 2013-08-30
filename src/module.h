#pragma once

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
static PyMethodDef moduleMethods[];
static const char *documentation;

/* ImageBuffer */
static int ImageBuffer_InitBindings();

static PyTypeObject ImageBuffer_Type;
static PyMethodDef ImageBuffer_methods[];
static PyMemberDef ImageBuffer_members[];

typedef struct {
    PyObject_HEAD
    ImageKit_Image *image;
} ImageBuffer;

/* ListTypeMethods */
struct ListTypeMethods {
    Py_ssize_t (*Size)(PyObject *);
    PyObject * (*GetItem)(PyObject *, Py_ssize_t);
    int (*SetItem)(PyObject *, Py_ssize_t, PyObject *);
    PyObject * (*GetSlice)(PyObject *, Py_ssize_t, Py_ssize_t);
};

static struct ListTypeMethods LIST_METHODS;
static struct ListTypeMethods TUPLE_METHODS;
static struct ListTypeMethods *GetListMethods(PyObject *object);

/* Rect */
static int Rect_InitBindings();

static PyTypeObject Rect_Type;
static PyMethodDef Rect_methods[];
static PyMemberDef Rect_members[];

typedef struct {
    PyObject_HEAD
    ImageKit_Rect rect;
} Rect;

/*

typedef struct ImageKit_Rect {
    DIMENSION x;
    DIMENSION y;
    DIMENSION w;
    DIMENSION h;
} ImageKit_Rect;

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

*/
