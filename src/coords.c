#pragma once

static const char *Coords_DOCUMENTATION = "TODO";

/* Coords */
static PyTypeObject Coords_Type = {PyVarObject_HEAD_INIT(NULL, 0)};
static PyMemberDef Coords_members[] = {{NULL}};

API int Coords_init(Coords *self, PyObject *args, PyObject *kwargs)
{
    static char *kw_names[] = {
        "items",
        NULL
    };
    
    uint32_t items;
    
    if (!PyArg_ParseTupleAndKeywords(   args,
                                        kwargs,
                                        "I",
                                        kw_names,
                                        &items)) {
        return -1;
    }
    
    self->coords = ImageKit_Coords_New(items);
    if (!self->coords) {
        PyErr_SetString(PyExc_Exception, "Failed to create object");
        return -1;
    }
    
    return 0;
}

API void Coords_dealloc(Coords *self)
{
    if (self->coords != NULL) {
        ImageKit_Coords_Delete(self->coords);
    }
    
    Py_TYPE(self)->tp_free((PyObject *)self);
}

API PyObject *Coords_from_rect(Coords *self, PyObject *args, PyObject *kwargs)
{
    static char *kwargs_names[] = {"rect", NULL};
    ImageKit_Coords *coords;
    Rect *rect;
    
    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "O",
            kwargs_names,
            &rect)) {
        return NULL;
    }
    
    Py_XINCREF(rect);
    
    if (!PyObject_IsInstance((PyObject *)rect, (PyObject *)&Rect_Type)) {
        Py_XDECREF(rect);
        PyErr_SetString(PyExc_Exception, "Argument must be of type Rect");
        return NULL;
    }
    
    if (!(self = (Coords *)_PyObject_New(&Coords_Type))) {
        Py_XDECREF(rect);
        return NULL;
    }
    
    coords = ImageKit_Coords_FromRect(&(rect->rect));
    if (!coords) {
        PyObject_Del(self);
        Py_XDECREF(rect);
        PyErr_SetString(PyExc_Exception, "Failed to create object");
        return NULL;
    }
    
    self->coords = coords;
    Py_XDECREF(rect);
    return (PyObject *)self;
}

API PyObject *Coords_append(Coords *self, PyObject *args)
{
    DIMENSION x, y;
    size_t new_size;
    
    if (!PyArg_ParseTuple(args, "II", &x, &y)) {
        return NULL;
    }
    
    if (!(self->coords->data_index < self->coords->data_items)) {
        if (self->coords->data_items < 20) {
            new_size = self->coords->data_items * 10;
        } else {
            new_size = self->coords->data_items * 1.5;
        }
        
        if (ImageKit_Coords_Resize(self->coords, new_size) < 1) {
            PyErr_SetString(PyExc_Exception, "Failed to append");
            return NULL;
        }
    }
    
    if (ImageKit_Coords_Append(self->coords, x, y) < 1) {
        PyErr_SetString(PyExc_Exception, "Failed to append");
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *Coords_iter(PyObject *_self)
{
    Coords *self = (Coords *)_self;
    
    self->iter_idx = 0;
    Py_INCREF(self);
    return _self;
}

PyObject *Coords_iternext(PyObject *_self)
{
    Coords *self = (Coords *)_self;
    size_t i;
    PyObject *tuple = NULL,
             *x     = NULL,
             *y     = NULL;
    
    if (!(self->iter_idx < self->coords->data_index)) {
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }
    
    i = self->iter_idx;
    x = PyInt_FromLong(self->coords->coords[i*2]);
    y = PyInt_FromLong(self->coords->coords[i*2+1]);
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

static PyMethodDef Coords_methods[] = {
    {
        "from_rect",
         (void *)Coords_from_rect,
         METH_STATIC | METH_KEYWORDS,
        "Creates a Coords object from a Rect object."
    },
    {
        "append",
         (void *)Coords_append,
         METH_VARARGS,
        "Append x, y pair."
    },
    {NULL, NULL, 0, NULL}
};

static int Coords_InitBindings()
{
    Coords_Type.tp_new = PyType_GenericNew;
    Coords_Type.tp_name = "imagekit.Coords";
    Coords_Type.tp_basicsize = sizeof(Coords);
    Coords_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    Coords_Type.tp_init = (initproc)Coords_init;
    Coords_Type.tp_dealloc = (destructor)Coords_dealloc;
    Coords_Type.tp_methods = Coords_methods;
    Coords_Type.tp_members = Coords_members;
    Coords_Type.tp_getattro = PyObject_GenericGetAttr;
    Coords_Type.tp_setattro = PyObject_GenericSetAttr;
    Coords_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_ITER;
    Coords_Type.tp_iter = Coords_iter;
    Coords_Type.tp_iternext = Coords_iternext;
    Coords_Type.tp_doc = Coords_DOCUMENTATION;
    
    if (PyType_Ready(&Coords_Type) < 0) {
        return -1;
    }

    Py_INCREF(&Coords_Type);
    PyModule_AddObject(MODULE, "Coords", (PyObject*)&Coords_Type);

    return 1;
}
