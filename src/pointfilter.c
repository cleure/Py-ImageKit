#pragma once

static const char *POINTFILTER_DOCUMENTATION = "";

/* Rect */
static PyTypeObject PointFilter_Type = {PyVarObject_HEAD_INIT(NULL, 0)};
static PyMemberDef PointFilter_members[] = {{NULL}};

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
ImageKit_PointFilter_FromCurves(
    ImageKit_Curves *curves_a,
    ImageKit_Curves *curves_b,
    ImageKit_Curves *curves_c,
    ImageKit_Curves *curves_d
);

*/

/*

Methods:
    set_value(channel, index, value, [value_range])
    get_value(channel, index, [value_range])
    set_channel(channel, list, [value_range])
    get_channel(channel, [value_range])
    apply(image)
    
    @static
    fromCurves(curves)

*/

API int PointFilter_init(PointFilter *self, PyObject *args, PyObject *kwargs)
{
    static char *kw_names[] = {"samples", NULL};
    uint32_t samples;

    printf("Init Called\n");

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "I", kw_names, &samples)) {
        return 0;
    }
    
    if (samples == 0) {
        PyErr_SetString(PyExc_ValueError, "Samples must be greater than 0");
        return 0;
    }
    
    self->pointfilter = ImageKit_PointFilter_New(samples);
    if (!self->pointfilter) {
        PyErr_SetString(PyExc_Exception, "Failed to create point filter object");
        return 0;
    }

    return 0;
}

API void PointFilter_dealloc(PointFilter *self)
{
    ImageKit_PointFilter_Delete(self->pointfilter);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

/*

from imagekit import *
b = Image.fromJPEG('/Users/cleure/Downloads/seattle/sxmC7Bz.jpg')
curve = [
    0.0, 0.0,
    1.0, 0.7,
]

curves = Curves.from_bezier(512, curve)
pf = PointFilter.fromCurves(curves, curves, curves, curves)

pf.apply(b)
b.savePNG('output.png')

*/

API PyObject *PointFilter_from_curves(PointFilter *self, PyObject *args, PyObject *kwargs)
{
    static char *kw_names[] = {
        "curves_a",
        "curves_b",
        "curves_c",
        "curves_d",
        NULL
    };
    
    Curves *curves[4] = {NULL, NULL, NULL, NULL};
    ImageKit_Curves *ik_curves[4] = {NULL, NULL, NULL, NULL};
    ImageKit_PointFilter *pf;
    
    int i;
    
    self = NULL;
    if (!PyArg_ParseTupleAndKeywords(   args,
                                        kwargs,
                                        "O|O|O|O",
                                        kw_names,
                                        &curves[0],
                                        &curves[1],
                                        &curves[2],
                                        &curves[3])) {
        return NULL;
    }
    
    for (i = 0; i < 4; i++) {
        Py_XINCREF(curves[i]);
        
        if (curves[i] != NULL && !PyObject_IsInstance(
                (PyObject *)curves[i],
                (PyObject *)&Curves_Type)) {
            
            PyErr_SetString(PyExc_TypeError, "Argument must be of type Curves");
            goto cleanup;
        } else if (curves[i] != NULL) {
            ik_curves[i] = curves[i]->curves;
        }
    }
    
    if (!(self = (PointFilter *)_PyObject_New(&PointFilter_Type))) {
        goto cleanup;
    }
    
    pf = ImageKit_PointFilter_FromCurves(
        ik_curves[0],
        ik_curves[1],
        ik_curves[2],
        ik_curves[3]
    );
    
    if (!pf) {
        Py_XDECREF(self);
        self = NULL;
        PyErr_SetString(PyExc_Exception, "Failed to create Point Filter object");
        goto cleanup;
    }
    
    self->pointfilter = pf;
    
    cleanup:
    for (i = 0; i < 4; i++) {
        Py_XDECREF(curves[i]);
    }
    
    return (PyObject *)self;
}

API PyObject *PointFilter_apply(PointFilter *self, PyObject *args, PyObject *kwargs)
{
    static char *kw_names[] = {
        "image",
        "coords",
        NULL
    };

    int result;
    ImageBuffer *image = NULL;
    Coords *coords = NULL;
    
    if (!PyArg_ParseTupleAndKeywords(   args,
                                        kwargs,
                                        "O|O",
                                        kw_names,
                                        &image,
                                        &coords)) {
        return NULL;
    }
    
    Py_XINCREF(image);
    Py_XINCREF(coords);
    
    if (!PyObject_IsInstance((PyObject *)image, (PyObject *)&ImageBuffer_Type)) {
        Py_XDECREF(image);
        Py_XDECREF(coords);
        PyErr_SetString(PyExc_Exception, "Argument \"image\" must be of type ImageBuffer");
        return NULL;
    }
    
    if (coords != NULL && !PyObject_IsInstance( (PyObject *)coords,
                                                (PyObject *)&Coords_Type)) {
        
        Py_XDECREF(image);
        Py_XDECREF(coords);
        PyErr_SetString(PyExc_Exception, "Argument \"coords\" must be of type Coords");
        return NULL;
    }
    
    if (coords) {
        result = ImageKit_PointFilter_Apply(
            self->pointfilter,
            image->image,
            coords->coords
        );
    } else {
        result = ImageKit_PointFilter_Apply(
            self->pointfilter,
            image->image,
            NULL
        );
    }
    
    Py_XDECREF(image);
    Py_XDECREF(coords);
    if (!result) {
        PyErr_SetString(PyExc_Exception, "Failed to apply Point Filter to Image");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

API PyObject *PointFilter_set_value(PointFilter *self, PyObject *args, PyObject *kwargs)
{
    static char *kw_names[] = {
        "channel",
        "index",
        "value",
        "value_range",
        NULL
    };
    
    uint32_t channel;
    uint32_t index;
    REAL value;
    REAL value_range = 1.0;
    
    REAL *abcd[4] = {
        self->pointfilter->a,
        self->pointfilter->b,
        self->pointfilter->c,
        self->pointfilter->d
    };
    
    if (!PyArg_ParseTupleAndKeywords(   args,
                                        kwargs,
                                        "IIf|f",
                                        kw_names,
                                        &channel,
                                        &index,
                                        &value,
                                        &value_range)) {
        return NULL;
    }
    
    if (index >= self->pointfilter->samples) {
        PyErr_SetString(PyExc_ValueError, "Index must not exceed sample size");
        return NULL;
    }
    
    if (channel > 3) {
        PyErr_SetString(PyExc_ValueError, "Channel must be between 0 and 3");
        return NULL;
    }
    
    abcd[channel][index] = value * (1.0/value_range);
    
    Py_INCREF(Py_None);
    return Py_None;
}

API PyObject *PointFilter_get_value(PointFilter *self, PyObject *args, PyObject *kwargs)
{
    static char *kw_names[] = {
        "channel",
        "index",
        "value_range",
        NULL
    };
    
    uint32_t channel;
    uint32_t index;
    REAL value_range = 1.0;
    REAL value;
    PyObject *result;
    
    REAL *abcd[4] = {
        self->pointfilter->a,
        self->pointfilter->b,
        self->pointfilter->c,
        self->pointfilter->d
    };
    
    if (!PyArg_ParseTupleAndKeywords(   args,
                                        kwargs,
                                        "II|f",
                                        kw_names,
                                        &channel,
                                        &index,
                                        &value_range)) {
        return NULL;
    }
    
    if (index >= self->pointfilter->samples) {
        PyErr_SetString(PyExc_ValueError, "Index must not exceed sample size");
        return NULL;
    }
    
    if (channel > 3) {
        PyErr_SetString(PyExc_ValueError, "Channel must be between 0 and 3");
        return NULL;
    }
    
    value = abcd[channel][index] / (1.0/value_range);
    result = PyFloat_FromDouble(value);
    if (!result) {
        return NULL;
    }
    
    return result;
}

/*
int PointFilter_Print(PyObject *_self, FILE *file, int flags)
{
    PointFilter *self = (PointFilter *)_self;
    fprintf(file, "FIXME: Print");
    
    return 0;
}
*/

/*

from imagekit import *
b = Image.fromJPEG('/Users/cleure/Downloads/seattle/sxmC7Bz.jpg')
pf = PointFilter(512)

for i in range(512):
    pf.set_value(0, i, i, 512)

for c in range(1, 4):
    for i in range(512):
        pf.set_value(c, i, i/2, 512)

pf.apply(b)
b.savePNG('output.png')

*/

static PyMethodDef PointFilter_methods[] = {
    {
        "set_value",
         (void *)PointFilter_set_value,
         METH_VARARGS | METH_KEYWORDS,
        "FIXME: Documentation"
    },
    {
        "get_value",
         (void *)PointFilter_get_value,
         METH_VARARGS | METH_KEYWORDS,
        "FIXME: Documentation"
    },
    {
        "apply",
         (void *)PointFilter_apply,
         METH_VARARGS | METH_KEYWORDS,
        "FIXME: Documentation"
    },
    {
        "fromCurves",
         (void *)PointFilter_from_curves,
         METH_VARARGS | METH_KEYWORDS | METH_STATIC,
        "FIXME: Documentation"
    },
    {NULL, NULL, 0, NULL}
};

static int PointFilter_InitBindings()
{
    PointFilter_Type.tp_new = PyType_GenericNew;
    PointFilter_Type.tp_name = "imagekit.PointFilter";
    PointFilter_Type.tp_basicsize = sizeof(PointFilter);
    PointFilter_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    PointFilter_Type.tp_init = (initproc)PointFilter_init;
    PointFilter_Type.tp_dealloc = (destructor)PointFilter_dealloc;
    PointFilter_Type.tp_methods = PointFilter_methods;
    PointFilter_Type.tp_members = PointFilter_members;
    //PointFilter_Type.tp_print = PointFilter_Print;
    PointFilter_Type.tp_getattro = PyObject_GenericGetAttr;
    PointFilter_Type.tp_setattro = PyObject_GenericSetAttr;
    PointFilter_Type.tp_doc = POINTFILTER_DOCUMENTATION;
    
    if (PyType_Ready(&PointFilter_Type) < 0) {
        return -1;
    }

    Py_INCREF(&PointFilter_Type);
    PyModule_AddObject(MODULE, "PointFilter", (PyObject*)&PointFilter_Type);

    return 1;
}
