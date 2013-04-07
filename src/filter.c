#ifndef IK_FILTER_DOT_C
#ifdef IK_INTERNAL

/*

Filter Methods:
    apply_matrix()
    apply_cvkernel()
    apply_median()

*/

static PyObject *ImageBuffer_apply_matrix(ImageBuffer *self, PyObject *args)
{
    PyObject *tuple;
    PyObject *tmp;
    struct ListTypeMethods *lm;
    REAL_TYPE matrix[16];
    REAL_TYPE *ptr;
    REAL_TYPE value[4];
    size_t wh, a, b;
    size_t len, i;

    if (!PyArg_ParseTuple(args, "O", &tuple)) {
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
    
    len = lm->Size(tuple);
    if (len/self->channels != self->channels) {
        PyErr_SetString(PyExc_StandardError, "Matrix has to be (channels)x(channels)");
        Py_DECREF(tuple);
        return NULL;
    }
    
    // Load value into matrix
    for (i = 0; i < len; i++) {
        tmp = lm->GetItem(tuple, i);
        Py_INCREF(tmp);
        
        matrix[i] = (REAL_TYPE)PyFloat_AsDouble(tmp);
        Py_DECREF(tmp);
    }
    
    // Apply matrix to image
    ptr = (REAL_TYPE *)&(self->data[0]);
    wh = self->width * self->height;
    for (i = 0; i < wh; i++) {
        for (a = 0; a < self->channels; a++) {
            value[a] = 0;
            
            for (b = 0; b < self->channels; b++) {
                value[a] += *(ptr+b) * matrix[(a*self->channels)+b];
            }
        }
        
        for (a = 0; a < self->channels; a++) {
            *(ptr+a) = value[a];
        }
            
        ptr += self->channels;
    }
    
    Py_DECREF(tuple);
    Py_INCREF(Py_None);
    return Py_None;
}

#endif /* IK_INTERNAL */
#endif /* IK_FILTER_DOT_C */
