
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
#include "module.h"

/*

TODO:
    - Implement other classes:
        - Rect
        - Coordinates
        - Curves
        - PointFilter
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

#include "rect.c"
#include "imagebuffer.c"
#include "coords.c"
#include "curves.c"

/*

from imagekit import *
b = Image.fromPNG('/Users/cleure/Development/Projects/TV4X/input-images/bomberman_1.png')

coords = Coords.from_rect(Rect(16, 16, 96, 96))
b.apply_rankfilter(3, 1.0, coords=coords)
b.savePNG('output.png')

*/

/*

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

*/

static PyObject *MODULE = NULL;
static const char *documentation =
    "TODO: Description";

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
        
        if (    !ImageBuffer_InitBindings() ||
                !Rect_InitBindings() ||
                !Coords_InitBindings() ||
                !Curves_InitBindings()) {
            RETURN_ERROR();
        }
        
        /* Module Constants */
        PyModule_AddStringConstant(MODULE, "__version__", "2.0");
        
        RETURN_SUCCESS();
    }
