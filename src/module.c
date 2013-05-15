#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <memory.h>
#include <math.h>

#include "imagekit.h"
#include "imagekit_functions.h"

/*

TODO:
        - Multi-threaded neighbor functions? Would increase performance of rank filters, and convolution kernels. Fairly easy to implement for a small, fixed size of threads.
        - Look into OpenCL.
        - Split project into library and Python Interface?
        - Coordinate system? It would be cool if you could generate shapes as coordinates, and pass them as parameters for the filter functions to use.
        - scale_cubic(), implementing B-Spline, Mitchell and Catmull-Rom.
        - scale_lanczos3()?
        - save() / load(), wrapping around save*() / load*().
        - Mode filter?
        - De-blur?
        - Cleanup build system.
        - in filter methods, ability to take x, y, width, height so boxes can be filtered.
        - Point Functions
            - brightness / contrast
            - gamma
            - polynomial functions
        - fill()
        - blit()
        - crop()
        - rotate()
        - Fix loading grayscale images in ImageBuffer_from_png()
        - Proper exception hierarchy
        - Cleanup error messages
        - saveJPG().
        - Test with RGB30 and RGB48.
        - Python 3.x support.
        - Documentation.

*/

static PyObject *MODULE = NULL;

/* Constants */
static const int HAVE_PNG           = HAVE_LIBPNG;
static const int HAVE_JPEG          = HAVE_LIBJPEG;
static const int HAVE_GIF           = HAVE_LIBGIF;

static const double COLORSPACE_FORMAT_MINMAX[COLORSPACE_FORMAT_SIZE][8] = {
    {0.0, 0.0, 0.0, 0.0,    31.0,      31.0,       31.0,     0.0},
    {0.0, 0.0, 0.0, 0.0,    31.0,      63.0,       31.0,     0.0},
    {0.0, 0.0, 0.0, 0.0,   255.0,     255.0,      255.0,   255.0},
    {0.0, 0.0, 0.0, 0.0,  1023.0,    1023.0,     1023.0,     0.0},
    {0.0, 0.0, 0.0, 0.0, 65535.0,   65535.0,    65535.0, 65535.0},
    {0.0, 0.0, 0.0, 0.0,   360.0,       1.0,        1.0,     1.0},
    {0.0, 0.0, 0.0, 0.0,     1.0,       0.0,        0.0,     1.0},
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

/* Coordinates Type */
static PyTypeObject Coordinates_Type = {
    PyObject_HEAD_INIT(NULL)
};

static PyMemberDef Coordinates_members[] = {
    {NULL}
};

static PyMethodDef Coordinates_methods[] = {
    {
        "to_list",
         (void *)Coordinates_to_list,
         METH_VARARGS,
        ""
    },
    {
        "append",
         (void *)Coordinates_append,
         METH_VARARGS,
        ""
    },
    {NULL, NULL, 0, NULL}
};

/* ImageBuffer Type */
static PyTypeObject ImageBuffer_Type = {
    PyObject_HEAD_INIT(NULL)
};

static PyMemberDef ImageBuffer_members[] = {
    {"width", T_INT, offsetof(ImageBuffer, width), 0, "width",},
    {"height", T_INT, offsetof(ImageBuffer, height), 0, "height",},
    {"channels", T_INT, offsetof(ImageBuffer, channels), 0, "channels",},
    {"colorspace", T_INT, offsetof(ImageBuffer, colorspace), 0, "colorspace",},
    {"colorspace_format", T_INT, offsetof(ImageBuffer, colorspace_format), 0, "colorspace_format",},
    {"scale", T_FLOAT, offsetof(ImageBuffer, scale), 0, "scale",},
    {NULL}
};

static PyMethodDef ImageBuffer_methods[] = {
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
        "fromPNG",
         (void *)ImageBuffer_from_png,
         METH_STATIC | METH_KEYWORDS,
        "Creates an ImageBuffer instance from a PNG file"
    },
    {
        "savePNG",
         (void *)ImageBuffer_save_png,
         METH_VARARGS | METH_KEYWORDS,
        "Saves data contained in instance to PNG file"
    },
    {
        "fromJPEG",
         (void *)ImageBuffer_from_jpeg,
         METH_STATIC | METH_KEYWORDS,
        "Creates an ImageBuffer instance from a JPEG file"
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
         "Takes: int samples, int channel"
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
        "DUMMY"
    },
    {
        "vtline_out",
         (void *)ImageBuffer_vtline_out,
         METH_VARARGS,
        "DUMMY"
    },
    {
        "get_box",
         (void *)ImageBuffer_get_box,
         METH_VARARGS,
        "DUMMY"
    },
    {
        "fill_image",
        (void *)ImageBuffer_fill_image,
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
        "apply_cvkernel",
         (void *)ImageBuffer_apply_cvkernel,
         METH_VARARGS,
        "DUMMY"
    },
    {
        "apply_rankfilter",
         (void *)ImageBuffer_apply_rankfilter,
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
    {
        "toMono",
         (void *)ImageBuffer_to_mono,
         METH_VARARGS | METH_KEYWORDS,
        "DUMMY"
    },
    {
        "scale_nearest",
         (void *)ImageBuffer_scale_nearest,
         METH_VARARGS,
        "DUMMY"
    },
    {
        "scale_bilinear",
         (void *)ImageBuffer_scale_bilinear,
         METH_VARARGS,
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
        
        /* Init Coordinates Type */
        Coordinates_Type.tp_new = PyType_GenericNew;
        Coordinates_Type.tp_name = "imagekit.Coordinates";
        Coordinates_Type.tp_basicsize = sizeof(Coordinates);
        Coordinates_Type.tp_getattro = PyObject_GenericGetAttr;
        Coordinates_Type.tp_setattro = PyObject_GenericSetAttr;
        Coordinates_Type.tp_flags = Py_TPFLAGS_DEFAULT;
        Coordinates_Type.tp_init = (initproc)Coordinates_init;
        Coordinates_Type.tp_dealloc = (destructor)Coordinates_dealloc;
        Coordinates_Type.tp_methods = Coordinates_methods;
        Coordinates_Type.tp_members = Coordinates_members;
        Coordinates_Type.tp_doc = "Doc string for class Bar in module Foo.";
        
        if (PyType_Ready(&Coordinates_Type) < 0) {
            return;
        }

        Py_INCREF(&ImageBuffer_Type);
        PyModule_AddObject(MODULE, "ImageBuffer", (PyObject*)&ImageBuffer_Type);
        PyModule_AddObject(MODULE, "Image", (PyObject*)&ImageBuffer_Type);
        PyModule_AddObject(MODULE, "Coordinates", (PyObject*)&Coordinates_Type);
        
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
