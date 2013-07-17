
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
    uint32_t width, height, channels;
    //uint32_t depth;
    
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
    //depth = 8;
    channels = cinfo.output_components;
    
    /* Create image instance */
    self = ImageKit_Image_New(
                    width,
                    height,
                    channels,
                    scale,
                    colorspace,
                    colorspace_format
    );
    
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
ImageKit_Image_SaveJPEG(ImageKit_Image *self, const char *filepath, int quality)
{
    ImageKit_Image *copy = NULL;
    
    /* JPEG variables */
    struct _jpeg_error_handler error_handler;
    struct jpeg_compress_struct cinfo;
    JSAMPROW row_pointer[1];
    DIMENSION bpp = 3;
    int colorspace = JCS_RGB;
    
    REAL *format;
    REAL scales[4];
    REAL *ptr_in;
    int32_t value;
    uint8_t *pixel_row, *ptr_out;
    size_t x, y, c;
    FILE *fp;
    
    fp = fopen(filepath, "wb");
    if (!fp) {
        ImageKit_SetError(ImageKit_IOError, "Unable to open file");
        return -1;
    }
    
    pixel_row = malloc(sizeof(*pixel_row) * self->width * bpp);
    if (!pixel_row) {
        ImageKit_SetError(ImageKit_MemoryError, "Failed to allocate memory");
        return -1;
    }
    
    /* Convert to RGB, if not already */
    if (self->colorspace != CS(RGB) || self->colorspace_format != CS_FMT(RGB24)) {
        copy = ImageKit_Image_Clone(self);
        ImageKit_Image_toRGB(copy, CS_FMT(RGB24), -1);
        self = copy;
    }
    
    format = (REAL *)&COLORSPACE_FORMAT_MINMAX[self->colorspace_format];
    
    /* Get output scales */
    scales[0] = (REAL)1.0 / self->channel_scales[0];
    scales[1] = (REAL)1.0 / self->channel_scales[1];
    scales[2] = (REAL)1.0 / self->channel_scales[2];
    scales[3] = (REAL)1.0 / self->channel_scales[3];
    
    /* Setup JPEG Errors */
    cinfo.err = jpeg_std_error(&error_handler.err);
    error_handler.err.error_exit = &_jpeg_error_exit;
    error_handler.err.emit_message = &_jpeg_emit_message;
    error_handler.err.output_message = &_jpeg_output_message;
    
    /* Exception style error handling */
    if (setjmp(error_handler.jmpbuf)) {
        jpeg_destroy_compress(&cinfo);
        fclose(fp);
        ImageKit_Image_Delete(copy);
        
        /* TODO: Should be JPEG_ERROR or something */
        ImageKit_SetError(ImageKit_StandardError, (char *)(&error_handler.error_msg));
        return -1;
    }
    
    //cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fp);
    
    // Params
    cinfo.image_width = self->width;
    cinfo.image_height = self->height;
    cinfo.input_components = bpp;
    cinfo.in_color_space = colorspace;
    
    jpeg_set_defaults(&cinfo);
    cinfo.num_components = bpp;
    
    cinfo.dct_method = JDCT_FLOAT;
    jpeg_set_quality(&cinfo, quality, TRUE);
    
    jpeg_start_compress(&cinfo, TRUE);
    
    ptr_in = (REAL *)(&self->data[0]);
    for (y = 0; y < self->height; y++) {
        ptr_out = pixel_row;
        
        for (x = 0; x < self->width; x++) {
            for (c = 0; c < bpp; c++) {
                value = (int32_t)((*ptr_in++) * scales[c]);
                
                if (value < format[c]) {
                    value = (int32_t)format[c];
                } else if (value > format[c+4]) {
                    value = (int32_t)format[c+4];
                }
                
                *ptr_out++ = (uint8_t)value;
            }
            
            // Discard extra channels (eg: transparency)
            for (; c < self->channels; c++) {
                ptr_in++;
            }
        }
        
        row_pointer[0] = &pixel_row[0];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(fp);
    free(pixel_row);
    ImageKit_Image_Delete(copy);
    
    return 0;
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
