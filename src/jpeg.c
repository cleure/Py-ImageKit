#ifndef IK_JPEG_DOT_C
#ifdef IK_INTERNAL
#ifdef HAVE_LIBJPEG

#include <jpeglib.h>
#include <stddef.h>
#include <setjmp.h>

struct _jpeg_error_handler {
    struct jpeg_error_mgr err;
    char error_msg[1024];
    jmp_buf jmpbuf;
};

static void _jpeg_error_exit(j_common_ptr ptr)
{
    /* Get error pointer */
    struct _jpeg_error_handler *err = (struct _jpeg_error_handler *)(ptr->err);

    /* Set error message */
    memset(&(err->error_msg), 0, 1024);
    err->err.format_message(ptr, (char *)&(err->error_msg));

    /* Long jump, so error can be handled */
    longjmp(err->jmpbuf, 1);
    return;
}

static void _jpeg_emit_message(j_common_ptr ptr, int level) {return;}
static void _jpeg_output_message(j_common_ptr ptr) {return;}

static ImageBuffer *ImageBuffer_from_jpeg(ImageBuffer *self, PyObject *args, PyObject *kwargs)
{

    static char *kwargs_names[] = {
                    "path",
                    "scale_max",
                    //"colorspace",
                    //"colorspace_format",
                    NULL
    };

    FILE *fp;
    const char *filepath;
    
    /* libjpeg variables */
    struct _jpeg_error_handler error_handler;
    struct jpeg_decompress_struct cinfo;
    int pitch, res;
    JSAMPARRAY buffer;
    
    REAL_TYPE *ptr_out;
    REAL_TYPE scale = -1;
    int colorspace = COLORSPACE_RGB;
    int colorspace_format = CS_FMT(RGB24);
    double *format;

    size_t x;
    uint32_t width, height, channels, depth;
    
    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "s|f|ii",
            kwargs_names,
            &filepath,
            &scale,
            &colorspace,
            &colorspace_format)) {
        return NULL;
    }

    fp = fopen(filepath, "rb");
    if (!fp) {
        PyErr_SetString(PyExc_OSError, "Unable to open file");
        return NULL;
    }

    // Setup JPEG Errors
    cinfo.err = jpeg_std_error(&error_handler.err);
    error_handler.err.error_exit = &_jpeg_error_exit;
    error_handler.err.emit_message = &_jpeg_emit_message;
    error_handler.err.output_message = &_jpeg_output_message;
    
    // Exception style error handling
    if (setjmp(error_handler.jmpbuf)) {
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        
        /* FIXME: Should be JPEG_ERROR or something */
        PyErr_SetString(PyExc_StandardError, (char *)(&error_handler.error_msg));
        return NULL;
    }
    
    // JPEG Stuff
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp);
    jpeg_read_header(&cinfo, 1);
    
    // Set decompress options
    cinfo.dct_method = JDCT_FLOAT;
    cinfo.dither_mode = JDITHER_NONE;
    
    jpeg_start_decompress(&cinfo);
    
    width = cinfo.output_width;
    height = cinfo.output_height;
    depth = 8;
    channels = cinfo.output_components;
    
    // Init self
    self = (ImageBuffer *)PyObject_CallMethod(MODULE, "ImageBuffer", "IIIfii",
                    cinfo.image_width,
                    cinfo.image_height,
                    cinfo.output_components,
                    scale,
                    colorspace,
                    colorspace_format);
    
    if (!self) {
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        return NULL;
    }
    
    pitch = cinfo.output_width * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, pitch, 1);
    
    ptr_out = &(self->data[0]);
    while (cinfo.output_scanline < cinfo.output_height) {
        res = jpeg_read_scanlines(&cinfo, buffer, 1);
        if (res < 1) {
            break;
        }
        
        for (x = 0; x < self->pitch; x++) {
            ptr_out[x] =    (REAL_TYPE)(self->channel_scales[(x % self->channels)]) *
                            (REAL_TYPE)(buffer[0][x]);
        }
        
        ptr_out += self->pitch;
    }
    
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);

    return self;
}

static PyObject *ImageBuffer_save_jpeg(ImageBuffer *self, PyObject *args, PyObject *kwargs)
{
    static char *kwargs_names[] = {
                    "path",
                    "colorspace_format",
                    NULL
    };
        
    FILE *fp;
    char *filepath;
    
    REAL_TYPE *ptr_in;
    REAL_TYPE scales[4];
    size_t x, y, c;
    uint32_t depth;
    double *format;
    
    int colorspace_format = -1;
    int _colorspace_format;
    int value;

    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "s|i",
            kwargs_names,
            &filepath,
            &colorspace_format)) {
        return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

#else

static PyObject *ImageBuffer_from_jpeg(ImageBuffer *self, PyObject *args, PyObject *kwargs)
{
    PyErr_SetString(PyExc_StandardError, "Library was not compiled with JPEG support");
    return NULL;
}

static PyObject *ImageBuffer_save_jpeg(ImageBuffer *self, PyObject *args, PyObject *kwargs)
{
    PyErr_SetString(PyExc_StandardError, "Library was not compiled with JPEG support");
    return NULL;
}

#endif /* HAVE_LIBJPEG */
#endif /* IK_INTERNAL */
#endif /* IK_JPEG_DOT_C */
