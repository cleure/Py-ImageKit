
/*

TODO: Implement set checking for x/y coords (no duplicates)
TODO: Split __init__() into static method from_list()?

*/

API int Coordinates_init(Coordinates *self, PyObject *args, PyObject *kwargs)
{
    static char *kwargs_names[] = {
                    "max_width",
                    "max_height",
                    "from_list",
                    "size",
                    NULL
    };
    
    struct ListTypeMethods *lm;
    PyObject *a, *b;
    PyObject *list = NULL;
    size_t size = 0;
    size_t i, idx;
    uint32_t max_width, max_height;
    long x, y;
    
    /* Parse arguments */
    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "II|O|n",
            kwargs_names,
            &max_width,
            &max_height,
            &list,
            &size)) {
        return -1;
    }
    
    if (size == 0 && list == NULL) {
        PyErr_SetString(PyExc_ValueError, "One of parameters from_list, size required");
        return -1;
    }
    
    self->max_width = max_width;
    self->max_height = max_height;
    
    if (list != NULL) {
        Py_INCREF(list);
        
        if (PyList_Check(list)) {
            lm = &LIST_METHODS;
        } else if (PyTuple_Check(list)) {
            lm = &TUPLE_METHODS;
        } else {
            Py_DECREF(list);
            PyErr_SetString(PyExc_ValueError, "from_list must be either list or tuple");
            return -1;
        }
        
        size = lm->Size(list);
        if (size % 2) {
            Py_DECREF(list);
            PyErr_SetString(PyExc_ValueError, "from_list must be flat list of x, y pairs");
            return -1;
        }
        
        self->coords = malloc(sizeof(*(self->coords)) * size);
        self->data_size = size;
        self->data_items = size;
        
        size /= 2;
        for (i = 0; i < size; i++) {
            idx = i * 2;
            
            a = lm->GetItem(list, idx);
            b = lm->GetItem(list, idx+1);
            
            Py_INCREF(a);
            Py_INCREF(b);
            
            if (!PyInt_Check(a) || !PyInt_Check(b)) {
                Py_DECREF(a);
                Py_DECREF(b);
                Py_DECREF(list);
                free(self->coords);
                self->coords = NULL;
                PyErr_SetString(PyExc_ValueError, "Elements must be of type int");
                return -1;
            }
            
            x = PyInt_AsLong(a);
            y = PyInt_AsLong(b);
            
            Py_DECREF(a);
            Py_DECREF(b);
            
            if (x < 0 || y < 0) {
                Py_DECREF(list);
                free(self->coords);
                self->coords = NULL;
                PyErr_SetString(PyExc_ValueError, "Elements must be greater than or equal to zero");
                return -1;
            } else if (!(x < self->max_width) || !(y < self->max_height)) {
                Py_DECREF(list);
                free(self->coords);
                self->coords = NULL;
                PyErr_SetString(PyExc_ValueError, "Elements cannot exceed max width/height");
                return -1;
            }
            
            self->coords[idx] = (uint32_t)x;
            self->coords[idx+1] = (uint32_t)y;
        }
        
        Py_DECREF(list);
    } else {
        size *= 2;
        if (size == 0) {
            PyErr_SetString(PyExc_ValueError, "size must be greater than zero");
            return -1;
        }
    
        self->coords = malloc(sizeof(*(self->coords)) * size);
        self->data_size = size;
        self->data_items = 0;
        memset(self->coords, 0, sizeof(*(self->coords)) * size);
    }
    
    return 0;
}

API void Coordinates_dealloc(Coordinates *self)
{
    free(self->coords);
    self->ob_type->tp_free((PyObject *)self);
}

API PyObject *Coordinates_to_list(Coordinates *self, PyObject *args)
{
    PyObject *outer;
    size_t i, length;
    uint32_t *ptr;
    
    if (!PyArg_ParseTuple(args, "")) {
        return NULL;
    }
    
    length = self->data_items;
    ptr = self->coords;
    
    outer = PyList_New(length);
    if (!outer) {
        return NULL;
    }
    
    for (i = 0; i < length; i++) {
        if (PyList_SetItem(outer, i, PyInt_FromLong(ptr[i])) < 0) {
            Py_DECREF(outer);
            return NULL;
        }
    }
    
    return outer;
}

API PyObject *Coordinates_append(Coordinates *self, PyObject *args)
{
    struct ListTypeMethods *lm;
    PyObject *tuple;
    PyObject *a, *b;
    size_t tuple_size;
    long x, y;
    
    if (self->data_items >= self->data_size) {
        PyErr_SetString(PyExc_StandardError, "Capacity exceeded");
        return NULL;
    }
    
    if (!PyArg_ParseTuple(args, "O", &tuple)) {
        return NULL;
    }
    
    Py_INCREF(tuple);
    
    if (PyList_Check(tuple)) {
        lm = &LIST_METHODS;
    } else if (PyTuple_Check(tuple)) {
        lm = &TUPLE_METHODS;
    } else {
        Py_DECREF(tuple);
        PyErr_SetString(PyExc_ValueError, "Argument must be either list or tuple of size 2");
        return NULL;
    }
    
    tuple_size = lm->Size(tuple);
    if (tuple_size != 2) {
        Py_DECREF(tuple);
        PyErr_SetString(PyExc_ValueError, "Argument must be either list or tuple of size 2");
        return NULL;
    }
    
    a = lm->GetItem(tuple, 0);
    b = lm->GetItem(tuple, 1);
    
    Py_INCREF(a);
    Py_INCREF(b);
    
    if (!PyInt_Check(a) || !PyInt_Check(b)) {
        Py_DECREF(a);
        Py_DECREF(b);
        Py_DECREF(tuple);
        PyErr_SetString(PyExc_ValueError, "Elements must be of type int");
        return NULL;
    }
    
    x = PyInt_AsLong(a);
    y = PyInt_AsLong(b);
    
    if (x < 0 || y < 0) {
        Py_DECREF(a);
        Py_DECREF(b);
        Py_DECREF(tuple);
        PyErr_SetString(PyExc_ValueError, "Elements must be greater than or equal to zero");
        return NULL;
    } else if (!(x < self->max_width) || !(y < self->max_height)) {
        Py_DECREF(a);
        Py_DECREF(b);
        Py_DECREF(tuple);
        PyErr_SetString(PyExc_ValueError, "Elements cannot exceed max width/height");
        return NULL;
    }
    
    self->coords[self->data_items++] = (uint32_t)x;
    self->coords[self->data_items++] = (uint32_t)y;
    
    Py_DECREF(a);
    Py_DECREF(b);
    Py_DECREF(tuple);
    
    Py_INCREF(Py_None);
    return Py_None;
}

/*

from imagekit import *
c = Coordinates(32, 32, [1, 2, 3, 4, 5, 6, 7, 8])
c.to_list()

*/
