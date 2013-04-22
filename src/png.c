#ifndef IK_PNG_DOT_C
#ifdef IK_INTERNAL
#ifdef HAVE_LIBPNG

#include <png.h>

static PyObject *ImageBuffer_from_png(ImageBuffer *self, PyObject *args, PyObject *kwargs)
{

    /*
    
    FIXME: Perform colorspace conversion
    TODO: Test with 30/48 bit RGB (probably broken)
    
    */

    static char *kwargs_names[] = {
                    "path",
                    "scale_max",
                    //"colorspace",
                    //"colorspace_format",
                    NULL
    };

    FILE *fp;
    const char *filepath;
    
    REAL_TYPE *ptr_out;
    REAL_TYPE scale = -1;
    int colorspace = COLORSPACE_RGB;
    int colorspace_format = CS_FMT(RGB24);
    double *format;

    size_t x, y, c;
    uint32_t width, height, channels, depth;
    
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep *row_pointers;
    
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
        PyErr_SetString(PyExc_IOError, "Unable to open file");
        return NULL;
    }
    
    // Initialize
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        
        PyErr_SetString(PyExc_StandardError, "PNG Error");
        return NULL;
    }
    
    // Error handling
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        if (fp) {
            fclose(fp);
        }
        
        PyErr_SetString(PyExc_StandardError, "PNG error");
        return NULL;
    }
    
    // Init PNG stuff
    png_init_io(png_ptr, fp);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY | PNG_TRANSFORM_EXPAND, NULL);
    fclose(fp);
    fp = NULL;
    
    // Get width / height, channels
    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    channels = png_get_channels(png_ptr, info_ptr);
    depth = png_get_bit_depth(png_ptr, info_ptr);
 
    // Handle input format
    switch (depth) {
        case 16:
            colorspace_format = CS_FMT(RGB48);
            break;
        case 10:
            colorspace_format = CS_FMT(RGB30);
            break;
        case 8:
        default:
            colorspace_format = CS_FMT(RGB24);
            break;
    }
    
    format = (double *)&COLORSPACE_FORMAT_MINMAX[colorspace_format];
    
    // Init self
    self = (ImageBuffer *)PyObject_CallMethod(MODULE, "ImageBuffer", "IIIfii",
                    width, height, channels, scale, colorspace, colorspace_format);
    
    if (!self) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return NULL;
    }
    
    // Setup buffers
    row_pointers = png_get_rows(png_ptr, info_ptr);
    
    // Get data pointer
    ptr_out = (REAL_TYPE *)&(self->data[0]);
        
    // Copy data, perform scaling, etc
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            for (c = 0; c < channels; c++) {
                *ptr_out++ =    (REAL_TYPE)(self->channel_scales[c]) *
                                (REAL_TYPE)row_pointers[y][x * channels + c];
            }
        }
    }
 
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return (PyObject *)self;
}

static PyObject *ImageBuffer_save_png(ImageBuffer *self, PyObject *args, PyObject *kwargs)
{
    static char *kwargs_names[] = {
                    "path",
                    "colorspace_format",
                    NULL
    };
    
    /*
    
    FIXME: Convert from self->colorspace to RGB
    TODO: Test with 30/48 bit RGB (probably broken)
    TODO: Allow user to change PNG parameters (alpha, interlacing, color format, etc)
    
    */
    
    FILE *fp;
    char *filepath;
    
    REAL_TYPE *ptr_in;
    REAL_TYPE scales[4];
    size_t x, y, c;
    uint32_t depth;
    double *format;
    
    // libpng stuff
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_byte **row_pointers = NULL;
    int png_colortype = PNG_COLOR_TYPE_RGB;
    
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
    
    _colorspace_format = colorspace_format;
    
    /* Convert HSV to RGB */
    if (self->colorspace == COLORSPACE_HSV) {
        if (_colorspace_format < 0 || _colorspace_format > CS_FMT(RGB48)) {
            _colorspace_format = CS_FMT(RGB24);
        }
        
        ImageBuffer_hsv_to_rgb(self, _colorspace_format, -1);
    }
    
    if (colorspace_format < 0) {
        colorspace_format = self->colorspace_format;
    }
    
    switch (colorspace_format) {
        case CS_FMT(RGB48):
            depth = 16;
            break;
        case CS_FMT(RGB30):
            depth = 10;
            break;
        case CS_FMT(RGB24):
        default:
            depth = 8;
            break;
    }
    
    format = (double *)&COLORSPACE_FORMAT_MINMAX[colorspace_format];
    
    /* Get output scales */
    scales[0] = (REAL_TYPE)1.0 / self->channel_scales[0];
    scales[1] = (REAL_TYPE)1.0 / self->channel_scales[1];
    scales[2] = (REAL_TYPE)1.0 / self->channel_scales[2];
    scales[3] = (REAL_TYPE)1.0 / self->channel_scales[3];

    fp = fopen(filepath, "wb");
    if (!fp) {
        PyErr_SetString(PyExc_IOError, "Unable to open file");
        return NULL;
    }
    
    // Initialize
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        fclose(fp);

        PyErr_SetString(PyExc_StandardError, "PNG Error");
        return NULL;
    }
    
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        
        PyErr_SetString(PyExc_StandardError, "PNG Error");
        return NULL;
    }

    // Error Handling
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        
        PyErr_SetString(PyExc_StandardError, "PNG error");
        return NULL;
    }
    
    /* Add alpha channel? */
    if (self->channels > 3) {
        png_colortype = PNG_COLOR_TYPE_RGBA;
    }
    
    png_set_IHDR(   png_ptr,
                    info_ptr,
                    self->width,
                    self->height,
                    8,
                    png_colortype,
                    PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT,
                    PNG_FILTER_TYPE_DEFAULT);

    // Init in/out buffers
    row_pointers = png_malloc(png_ptr, sizeof(png_byte *) * self->height);
    ptr_in = (REAL_TYPE *)(&(self->data[0]));
    
    // Scale, clamp, copy
    for (y = 0; y < self->height; y++) {
        row_pointers[y] = png_malloc(png_ptr, sizeof(png_byte) * self->width * self->channels);
        for (x = 0; x < self->width; x++) {
            for (c = 0; c < self->channels; c++) {
                value = (png_byte)((*ptr_in++) * scales[c]);
                
                if (value < format[c]) {
                    value = (int)(format[c]);
                } else if (value > format[4+c]) {
                    value = (int)(format[4+c]);
                }
                
                row_pointers[y][x * self->channels + c] = value;
            }
        }
    }

    // Write file
    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    
    for (y = 0; y < self->height; y++) {
        png_free(png_ptr, row_pointers[y]);
    }
    
    png_free(png_ptr, row_pointers);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    Py_INCREF(Py_None);
    return Py_None;
}

#else

static PyObject *ImageBuffer_from_png(ImageBuffer *self, PyObject *args, PyObject *kwargs)
{
    PyErr_SetString(PyExc_StandardError, "Library was not compiled with PNG support");
    return NULL;
}

static PyObject *ImageBuffer_save_png(ImageBuffer *self, PyObject *args, PyObject *kwargs)
{
    PyErr_SetString(PyExc_StandardError, "Library was not compiled with PNG support");
    return NULL;
}

#endif /* HAVE_LIBPNG */
#endif /* IK_INTERNAL */
#endif /* IK_PNG_DOT_C */
