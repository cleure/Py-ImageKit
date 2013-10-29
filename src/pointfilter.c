#pragma once

static const char *POINTFILTER_DOCUMENTATION = "TODO";

/* Rect */
static PyTypeObject PointFilter_Type = {PyVarObject_HEAD_INIT(NULL, 0)};
static PyMemberDef PointFilter_members[] = {{NULL}};

API int PointFilter_init(PointFilter *self, PyObject *args, PyObject *kwargs)
{
    static char *kw_names[] = {"samples", NULL};
    uint32_t samples;

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

API PyObject *PointFilter_from_curves(PointFilter *self, PyObject *args, PyObject *kwargs)
{
    static char *kw_names[] = {
        "a",
        "b",
        "c",
        "d",
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
        PyObject_Del(self);
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

API PyObject *PointFilter_set_channel(PointFilter *self, PyObject *args, PyObject *kwargs)
{
    static char *kw_names[] = {
        "channel",
        "list",
        "value_range",
        NULL
    };

    size_t i, l;
    PyObject *tuple, *tmp;
    struct ListTypeMethods *methods;
    uint32_t channel;
    REAL value;
    REAL scale;
    REAL value_range = 1.0;
    
    REAL *abcd[4] = {
        self->pointfilter->a,
        self->pointfilter->b,
        self->pointfilter->c,
        self->pointfilter->d
    };
    
    if (!PyArg_ParseTupleAndKeywords(   args,
                                        kwargs,
                                        "IO|f",
                                        kw_names,
                                        &channel,
                                        &tuple,
                                        &value_range)) {
        return NULL;
    }
    
    if (channel > 3) {
        PyErr_SetString(PyExc_ValueError, "Channel must be between 0 and 3");
        return NULL;
    }
    
    Py_XINCREF(tuple);
    if (!(methods = GetListMethods(tuple))) {
        Py_XDECREF(tuple);
        return NULL;
    }
    
    scale = 1.0/value_range;
    l = methods->Size(tuple);
    if (self->pointfilter->samples < l) {
        l = self->pointfilter->samples;
    }
    
    for (i = 0; i < l; i++) {
        tmp = methods->GetItem(tuple, i);
        Py_XINCREF(tmp);
        value = PyFloat_AsDouble(tmp) * scale;
        Py_XDECREF(tmp);
        
        abcd[channel][i] = value;
    }
    
    Py_XDECREF(tuple);
    if (PyErr_Occurred()) {
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

API PyObject *PointFilter_get_channel(PointFilter *self, PyObject *args, PyObject *kwargs)
{
    static char *kw_names[] = {
        "channel",
        "value_range",
        NULL
    };
    
    size_t i, l;
    PyObject *list, *tmp;
    uint32_t channel;
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
                                        "I|f",
                                        kw_names,
                                        &channel,
                                        &value_range)) {
        return NULL;
    }
    
    if (channel > 3) {
        PyErr_SetString(PyExc_ValueError, "Channel must be between 0 and 3");
        return NULL;
    }
    
    l = self->pointfilter->samples;
    list = PyList_New(l);
    if (!list) {
        return NULL;
    }
    
    for (i = 0; i < l; i++) {
        value = abcd[channel][i] * value_range;
        tmp = PyFloat_FromDouble(value);
        if (!tmp) {
            Py_XDECREF(list);
            return NULL;
        }
        
        PyList_SetItem(list, i, tmp);
    }
    
    if (PyErr_Occurred()) {
        Py_XDECREF(list);
        return NULL;
    }

    return list;
}

static PyMethodDef PointFilter_methods[] = {
    {
        "set_value",
         (void *)PointFilter_set_value,
         METH_VARARGS | METH_KEYWORDS,
        "set_value(int channel, int index, float value, [float value_range])"
    },
    {
        "get_value",
         (void *)PointFilter_get_value,
         METH_VARARGS | METH_KEYWORDS,
        "get_value(int channel, int index, [float value_range])"
    },
    {
        "set_channel",
         (void *)PointFilter_set_channel,
         METH_VARARGS | METH_KEYWORDS,
        "set_channel(channel, list, [value_range])"
    },
    {
        "get_channel",
         (void *)PointFilter_get_channel,
         METH_VARARGS | METH_KEYWORDS,
        "get_channel(channel, [value_range])"
    },
    {
        "apply",
         (void *)PointFilter_apply,
         METH_VARARGS | METH_KEYWORDS,
        "apply(ImageBuffer image)"
    },
    {
        "from_curves",
         (void *)PointFilter_from_curves,
         METH_VARARGS | METH_KEYWORDS | METH_STATIC,
        "from_curves(Curves a, [Curves b], [Curves c], [Curves d])"
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
