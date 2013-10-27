#pragma once

static const char *Curves_DOCUMENTATION = "TODO";

/* Curves */
static PyTypeObject Curves_Type = {PyVarObject_HEAD_INIT(NULL, 0)};
static PyMemberDef Curves_members[] = {{NULL}};

API int Curves_init(Curves *self, PyObject *args, PyObject *kwargs)
{
    self->curves = NULL;
    return 0;
}

API void Curves_dealloc(Curves *self)
{
    if (self->curves != NULL) {
        ImageKit_Curves_Delete(self->curves);
    }
    
    Py_TYPE(self)->tp_free((PyObject *)self);
}

API PyObject *Curves_from_bezier(Curves *self, PyObject *args, PyObject *kwargs)
{
    static char *kwargs_names[] = {"samples", "points", NULL};
    PyObject *points, *tmp;
    struct ListTypeMethods *methods;
    
    ImageKit_Curves *curves;
    REAL *xy;
    REAL value;
    uint32_t samples;
    size_t xy_items, i;
    
    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "IO",
            kwargs_names,
            &samples,
            &points)) {
        return NULL;
    }
    
    Py_XINCREF(points);
    methods = GetListMethods(points);
    if (!methods) {
        Py_XDECREF(points);
        return NULL;
    }
    
    xy_items = methods->Size(points);
    if (xy_items % 2) {
        Py_XDECREF(points);
        PyErr_SetString(PyExc_ValueError, "Input must be flat list/tuple of x/y pairs.");
        return NULL;
    }
    
    xy = malloc(sizeof(*xy) * xy_items);
    if (!xy) {
        Py_XDECREF(points);
        return PyErr_NoMemory();
    }
    
    if (!(self = (Curves *)_PyObject_New(&Curves_Type))) {
        Py_XDECREF(points);
        return NULL;
    }
    
    for (i = 0; i < xy_items; i++) {
        tmp = methods->GetItem(points, i);
        
        Py_INCREF(tmp);
        value = PyFloat_AsDouble(tmp);
        Py_DECREF(tmp);
        
        xy[i] = value;
    }
    
    if (PyErr_Occurred()) {
        free(xy);
        PyObject_Del((PyObject *)self);
        Py_XDECREF(points);
        return NULL;
    }
    
    curves = ImageKit_Curves_FromBezier(samples, xy, xy_items / 2);
    if (!curves) {
        free(xy);
        PyObject_Del((PyObject *)self);
        Py_XDECREF(points);
        
        PyErr_SetString(PyExc_Exception, "Failed to create object");
        return NULL;
    }
 
    self->curves = curves;
    Py_XDECREF(points);
    return (PyObject *)self;
}

PyObject *Curves_iter(PyObject *_self)
{
    Curves *self = (Curves *)_self;
    
    self->iter_idx = 0;
    Py_INCREF(self);
    return _self;
}

PyObject *Curves_iternext(PyObject *_self)
{
    Curves *self = (Curves *)_self;
    size_t i;
    PyObject *tuple = NULL,
             *x     = NULL,
             *y     = NULL;
    
    if (!(self->iter_idx < self->curves->data_items)) {
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }
    
    i = self->iter_idx;
    x = PyFloat_FromDouble(self->curves->coords[i*2]);
    y = PyFloat_FromDouble(self->curves->coords[i*2+1]);
    tuple = PyTuple_New(2);
    
    if (!tuple || !x || !y) {
        Py_XDECREF(x);
        Py_XDECREF(y);
        Py_XDECREF(tuple);
        return NULL;
    }
    
    PyTuple_SetItem(tuple, 0, x);
    PyTuple_SetItem(tuple, 1, y);
    self->iter_idx++;
    
    return tuple;
}

static PyMethodDef Curves_methods[] = {
    {
        "from_bezier",
         (void *)Curves_from_bezier,
         METH_STATIC | METH_KEYWORDS,
        "Creates a Curves object from a Rect object."
    },
    {NULL, NULL, 0, NULL}
};

static int Curves_InitBindings()
{
    Curves_Type.tp_new = PyType_GenericNew;
    Curves_Type.tp_name = "imagekit.Curves";
    Curves_Type.tp_basicsize = sizeof(Curves);
    Curves_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    Curves_Type.tp_init = (initproc)Curves_init;
    Curves_Type.tp_dealloc = (destructor)Curves_dealloc;
    Curves_Type.tp_methods = Curves_methods;
    Curves_Type.tp_members = Curves_members;
    Curves_Type.tp_getattro = PyObject_GenericGetAttr;
    Curves_Type.tp_setattro = PyObject_GenericSetAttr;
    Curves_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_ITER;
    Curves_Type.tp_iter = Curves_iter;
    Curves_Type.tp_iternext = Curves_iternext;
    Curves_Type.tp_doc = Curves_DOCUMENTATION;
    
    if (PyType_Ready(&Curves_Type) < 0) {
        return -1;
    }

    Py_INCREF(&Curves_Type);
    PyModule_AddObject(MODULE, "Curves", (PyObject*)&Curves_Type);

    return 1;
}
