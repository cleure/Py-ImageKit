#pragma once

static const char *RECT_DOCUMENTATION = "r = Rect(x, y, w, h)";

/* Rect */
static PyTypeObject Rect_Type = {PyVarObject_HEAD_INIT(NULL, 0)};

static PyMemberDef Rect_members[] = {
    {"x", T_UINT, (offsetof(Rect, rect) + offsetof(ImageKit_Rect, x)), 0, "x"},
    {"y", T_UINT, (offsetof(Rect, rect) + offsetof(ImageKit_Rect, y)), 0, "y"},
    {"w", T_UINT, (offsetof(Rect, rect) + offsetof(ImageKit_Rect, w)), 0, "w"},
    {"h", T_UINT, (offsetof(Rect, rect) + offsetof(ImageKit_Rect, h)), 0, "h"},
    {NULL}
};

API int Rect_init(Rect *self, PyObject *args, PyObject *kwargs)
{
    static char *kw_names[] = {
        "x",
        "y",
        "w",
        "h",
        NULL
    };
    
    long x, y, w, h;
    
    if (!PyArg_ParseTupleAndKeywords(   args,
                                        kwargs,
                                        "llll",
                                        kw_names,
                                        &x,
                                        &y,
                                        &w,
                                        &h)) {
        return -1;
    }
    
    if (x < 0 || y < 0 || w < 0 || h < 0) {
        PyErr_SetString(PyExc_Exception, "Values must be greater than -1");
        return -1;
    }
    
    if (x > UINT32_MAX || y > UINT32_MAX || w > UINT32_MAX || h > UINT32_MAX) {
        PyErr_SetString(PyExc_Exception, "Values must not exceed UINT32_MAX");
        return -1;
    }
    
    self->rect.x = (uint32_t)x;
    self->rect.y = (uint32_t)y;
    self->rect.w = (uint32_t)w;
    self->rect.h = (uint32_t)h;
    
    return 0;
}

API void Rect_dealloc(Rect *self)
{
    Py_TYPE(self)->tp_free((PyObject *)self);
}

int Rect_Print(PyObject *_self, FILE *file, int flags)
{
    Rect *self = (Rect *)_self;
    
    fprintf(    file,
                "x=%u, y=%u, w=%u, h=%u",
                self->rect.x,
                self->rect.y,
                self->rect.w,
                self->rect.h);
    
    return 0;
}

static PyMethodDef Rect_methods[] = {
    {NULL, NULL, 0, NULL}
};

static int Rect_InitBindings()
{
    Rect_Type.tp_new = PyType_GenericNew;
    Rect_Type.tp_name = "imagekit.Rect";
    Rect_Type.tp_basicsize = sizeof(Rect);
    Rect_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    Rect_Type.tp_init = (initproc)Rect_init;
    Rect_Type.tp_dealloc = (destructor)Rect_dealloc;
    Rect_Type.tp_methods = Rect_methods;
    Rect_Type.tp_members = Rect_members;
    Rect_Type.tp_print = Rect_Print;
    Rect_Type.tp_getattro = PyObject_GenericGetAttr;
    Rect_Type.tp_setattro = PyObject_GenericSetAttr;
    Rect_Type.tp_doc = RECT_DOCUMENTATION;
    
    if (PyType_Ready(&Rect_Type) < 0) {
        return -1;
    }

    Py_INCREF(&Rect_Type);
    PyModule_AddObject(MODULE, "Rect", (PyObject*)&Rect_Type);

    return 1;
}
