#ifndef IK_FILTER_DOT_C
#ifdef IK_INTERNAL

/*

TODO:
    crop()
    apply_median()

*/

/**
* Apply Convolution Kernel.
*
* Convolution filters can be used to perform basic filtering, such as moiton blur,
* edge detection, and sharpen, and emboss.
*
* See this link for more information on CV Kernels:
* http://lodev.org/cgtutor/filtering.html
**/
static PyObject *ImageBuffer_apply_cvkernel(ImageBuffer *self, PyObject *args)
{
    PyObject *tuple;
    PyObject *tmp;
    struct ListTypeMethods *lm;
    
    double *csfmt;
    REAL_TYPE result[4];
    REAL_TYPE min[4];
    REAL_TYPE max[4];
    
    REAL_TYPE *output = NULL;
    REAL_TYPE *ptr_out;
    REAL_TYPE *matrix = NULL;
    
    int32_t matrix_size;
    float matrix_size_f;
    float factor = 1.0f;
    float bias = 0.0f;
    size_t tuple_size;
    
    int32_t x, y;
    int32_t c, sx, sy, ex, ey, mid, i;

    if (!PyArg_ParseTuple(args, "O|ff", &tuple, &factor, &bias)) {
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
    
    tuple_size = lm->Size(tuple);
    
    /* Round to first 2 decimals */
    matrix_size_f = floorf(100.0f * sqrtf((float)tuple_size) + 0.5f) / 100.0f;
    matrix_size = (int)matrix_size_f;
    
    /* Check errors */
    if (matrix_size < 3) {
        PyErr_SetString(PyExc_ValueError,
            "Minumum size for matrix is 3x3");
        return NULL;
    }
    
    if ((float)matrix_size != matrix_size_f) {
        PyErr_SetString(PyExc_ValueError,
            "Matrix width and height must be the same (ex: 3x3, 5x5)");
        return NULL;
    }
    
    if (!(matrix_size % 2)) {
        PyErr_SetString(PyExc_ValueError,
            "Matrix width and height must have an odd number of elements (ex: 3x3, 5x5)");
        return NULL;
    }
    
    matrix = malloc(sizeof(*matrix) * tuple_size);
    if (!matrix) {
        Py_DECREF(tuple);
        return PyErr_NoMemory();
    }
    
    /* Read data from Python Tuple to C Array */
    for (i = 0; i < tuple_size; i++) {
        tmp = lm->GetItem(tuple, i);
        Py_INCREF(tmp);
        
        if (PyFloat_Check(tmp)) {
            matrix[i] = (REAL_TYPE)PyFloat_AsDouble(tmp);
        } else if (PyInt_Check(tmp)) {
            matrix[i] = (REAL_TYPE)PyInt_AsLong(tmp);
        } else if (PyLong_Check(tmp)) {
            matrix[i] = (REAL_TYPE)PyLong_AsDouble(tmp);
        } else {
            free(matrix);
            Py_DECREF(tmp);
            Py_DECREF(tuple);
            
            PyErr_SetString(PyExc_ValueError, "Elements must be of type int or float");
            return NULL;
        }
        
        Py_DECREF(tmp);
    }
    
    csfmt = (double *)&COLORSPACE_FORMAT_MINMAX[self->colorspace_format];
    
    /* Get min/max */
    if (self->scale <= 0.0) {
        for (c = 0; c < self->channels; c++) {
            min[c] = (REAL_TYPE)csfmt[c];
            max[c] = (REAL_TYPE)csfmt[c+4];
        }
    } else {
        for (c = 0; c < self->channels; c++) {
            min[c] = (REAL_TYPE)0.0;
            max[c] = (REAL_TYPE)self->scale;
        }
    }
    
    output = malloc(sizeof(*output) * self->width * self->height * self->channels);
    if (!output) {
        free(matrix);
        free(output);
        Py_DECREF(tuple);
        return PyErr_NoMemory();
    }
    
    ptr_out = output;
    mid = matrix_size / 2;
    
    for (y = 0; y < self->height; y++) {
        for (x = 0; x < self->width; x++) {
            
            /* Reset */
            i = 0;
            ex = x + mid;
            ey = y + mid;
            sy = y - mid;
            
            result[0] = 0;
            result[1] = 0;
            result[2] = 0;
            result[3] = 0;
            
            /* Get box */
            while (sy <= ey) {
                sx = x - mid;
                
                while (sx <= ex) {
                    /*
                    for (c = 0; c < self->channels; c++) {
                        result[c] += self->data[PIXEL_INDEX(self, (sx % self->width), (sy % self->height)) + c] * matrix[i];
                    }*/
                    
                    
                    if (sx < 0 || sy < 0 || sx >= self->width || sy >= self->height) {
                        for (c = 0; c < self->channels; c++) {
                            result[c] += self->data[PIXEL_INDEX(self, x, y) + c] * matrix[i];
                        }
                    } else {
                        for (c = 0; c < self->channels; c++) {
                            result[c] += self->data[PIXEL_INDEX(self, sx, sy) + c] * matrix[i];
                        }
                    }
                    
                    i++;
                    sx++;
                }
        
                sy++;
            }
            
            /* Output */
            for (c = 0; c < self->channels; c++) {
                result[c] = factor * result[c] + bias;
                
                /* Clamp */
                if (result[c] > max[c]) {
                    result[c] = max[c];
                } else if (result[c] < min[c]) {
                    result[c] = min[c];
                }
                
                *ptr_out++ = result[c];
            }
        }
    }
    
    /* Free old buffer, and link up new one */
    free(self->data);
    self->data = output;
    
    free(matrix);
    
    Py_DECREF(tuple);
    Py_INCREF(Py_None);
    
    return Py_None;
}

/* Apply matrix to image channels */
static PyObject *ImageBuffer_apply_matrix(ImageBuffer *self, PyObject *args)
{
    /*
    
    FIXME: Clamp
    
    */

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
        
        if (PyFloat_Check(tmp)) {
            matrix[i] = (REAL_TYPE)PyFloat_AsDouble(tmp);
        } else if (PyInt_Check(tmp)) {
            matrix[i] = (REAL_TYPE)PyInt_AsLong(tmp);
        } else if (PyLong_Check(tmp)) {
            matrix[i] = (REAL_TYPE)PyLong_AsDouble(tmp);
        } else {
            Py_DECREF(tmp);
            Py_DECREF(tuple);
            
            PyErr_SetString(PyExc_ValueError, "Elements must be of type int or float");
            return NULL;
        }
        
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
