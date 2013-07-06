
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>
#include "imagekit.h"

#ifdef HAVE_JPEG

#include <jpeglib.h>

struct _jpeg_error_handler {
    struct jpeg_error_mgr err;
    char error_msg[1024];
    jmp_buf jmpbuf;
};

PRIVATE void _jpeg_error_exit(j_common_ptr ptr)
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

PRIVATE void _jpeg_emit_message(j_common_ptr ptr, int level) {return;}
PRIVATE void _jpeg_output_message(j_common_ptr ptr) {return;}

API
ImageKit_Image *
ImageKit_Image_FromJPEG(const char *filepath, REAL scale)
{
    ImageKit_Image *self;
    FILE *fp;
    
    /* libjpeg variables */
    struct _jpeg_error_handler error_handler;
    struct jpeg_decompress_struct cinfo;
    int pitch, res;
    JSAMPARRAY buffer;
    
    REAL *ptr_out;
    int colorspace = CS(RGB);
    int colorspace_format = CS_FMT(RGB24);
    //REAL *format;

    size_t x;
    uint32_t width, height, channels, depth;
    
    fp = fopen(filepath, "rb");
    if (!fp) {
        ImageKit_SetError(ImageKit_IOError, "Unable to open file");
        return NULL;
    }
    
    /* Setup JPEG Errors */
    cinfo.err = jpeg_std_error(&error_handler.err);
    error_handler.err.error_exit = &_jpeg_error_exit;
    error_handler.err.emit_message = &_jpeg_emit_message;
    error_handler.err.output_message = &_jpeg_output_message;

    /* Exception style error handling */
    if (setjmp(error_handler.jmpbuf)) {
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        
        /* TODO: Should be JPEG_ERROR or something */
        ImageKit_SetError(ImageKit_StandardError, (char *)(&error_handler.error_msg));
        return NULL;
    }
    
    /* JPEG Stuff */
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp);
    jpeg_read_header(&cinfo, 1);
    
    /* Set decompress options */
    cinfo.dct_method = JDCT_FLOAT;
    cinfo.dither_mode = JDITHER_NONE;
    
    jpeg_start_decompress(&cinfo);
    
    width = cinfo.output_width;
    height = cinfo.output_height;
    depth = 8;
    channels = cinfo.output_components;
    
    /* Create image instance */
    self = ImageKit_Image_New(
                    cinfo.image_width,
                    cinfo.image_height,
                    cinfo.output_components,
                    scale,
                    colorspace,
                    colorspace_format
    );
    
    /*
enum {
    ImageKit_NotImplementedError,
    ImageKit_StandardError,
    ImageKit_ValueError,
    ImageKit_TypeError,
    ImageKit_OSError,
    ImageKit_IOError,
    ImageKit_MemoryError,
    ImageKit_IndexError,
    IMAGEKIT_NUM_ERRORS
} IMAGEKIT_ERROR;
    */
    
    if (!self) {
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        return NULL;
    }

    pitch = cinfo.output_width * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, pitch, 1);

    ptr_out = (REAL *)&(self->data[0]);
    while (cinfo.output_scanline < cinfo.output_height) {
        res = jpeg_read_scanlines(&cinfo, buffer, 1);
        if (res < 1) {
            break;
        }
        
        for (x = 0; x < self->pitch; x++) {
            ptr_out[x] =    (REAL)(self->channel_scales[(x % self->channels)]) *
                            (REAL)(buffer[0][x]);
        }
        
        ptr_out += self->pitch;
    }
    
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);

    return self;
}

API
int
ImageKit_Image_SaveJPEG(ImageKit_Image *self, const char *filepath)
{
    // FIXME
    return -1;
}

#else

API
ImageKit_Image *
ImageKit_Image_FromJPEG(const char *filepath, REAL scale) {
    ImageKit_SetError(ImageKit_StandardError, "Not compiled with JPEG support");
    return NULL;
}

API
int
ImageKit_Image_SaveJPEG(ImageKit_Image *self, const char *filepath) {
    ImageKit_SetError(ImageKit_StandardError, "Not compiled with JPEG support");
    return -1;
}

#endif
