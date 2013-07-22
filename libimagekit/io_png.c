
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include "imagekit.h"

#ifdef HAVE_PNG

#include <png.h>

API
ImageKit_Image *
ImageKit_Image_FromPNG(const char *filepath, REAL scale)
{
    FILE *fp;
    ImageKit_Image *self;
    
    REAL *ptr_out;
    int colorspace = CS(RGB);
    int colorspace_format = CS_FMT(RGB24);
    
    size_t x, y, c;
    DIMENSION width, height, channels, depth;
    
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep *row_pointers;
    png_uint_16p *row_pointers16;
    
    fp = fopen(filepath, "rb");
    if (!fp) {
        ImageKit_SetError(ImageKit_IOError, "Unable to open file");
        return NULL;
    }
    
    // Initialize
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        
        ImageKit_SetError(ImageKit_StandardError, "libpng error");
        return NULL;
    }
    
    // Error handling
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        if (fp) {
            fclose(fp);
        }
        
        ImageKit_SetError(ImageKit_StandardError, "libpng error");
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
    
    self = ImageKit_Image_New(
        width,
        height,
        channels,
        scale,
        colorspace,
        colorspace_format
    );
    
    if (!self) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return NULL;
    }
    
    // Setup buffers
    row_pointers = png_get_rows(png_ptr, info_ptr);
    
    // Get data pointer
    ptr_out = (REAL *)&(self->data[0]);
    
    if (depth == 16) {
        // 16 Bits Per Channel
        row_pointers16 = (png_uint_16p *)row_pointers;
        
        // Copy data, perform scaling, etc
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                for (c = 0; c < channels; c++) {
                    *ptr_out++ =    (REAL)(self->channel_scales[c]) *
                                    (REAL)row_pointers16[y][x * channels + c];
                }
            }
        }
    } else {
        // Copy data, perform scaling, etc
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                for (c = 0; c < channels; c++) {
                    *ptr_out++ =    (REAL)(self->channel_scales[c]) *
                                    (REAL)row_pointers[y][x * channels + c];
                }
            }
        }
    }
 
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return self;
}

API
int
ImageKit_Image_SavePNG(ImageKit_Image *self, const char *filepath)
{
    ImageKit_Image *copy = NULL;
    FILE *fp;
    
    REAL *ptr_in;
    REAL scales[4];
    size_t x, y, c;
    uint32_t depth;
    REAL *format;
    
    // libpng stuff
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_byte **row_pointers = NULL;
    png_uint_16 **row_pointers16 = NULL;
    int png_colortype = PNG_COLOR_TYPE_RGB;
    
    int colorspace_format = -1;
    int32_t value;
    
    if (self->colorspace == CS(HSV)) {
        copy = ImageKit_Image_Clone(self);
        if (!copy) {
            return -1;
        }
        
        if (!ImageKit_Image_toRGB(copy, CS_FMT(RGB24), -1)) {
            return -1;
        }
        
        self = copy;
    } else if (self->colorspace == CS(MONO)) {
        copy = ImageKit_Image_Clone(self);
        if (!copy) {
            return -1;
        }
        
        if (!ImageKit_Image_toRGB(copy, CS_FMT(RGB24), -1)) {
            return -1;
        }
        
        self = copy;
    } else if (     self->colorspace_format == CS_FMT(RGB15) ||
                    self->colorspace_format == CS_FMT(RGB16)) {
        copy = ImageKit_Image_Clone(self);
        if (!copy) {
            return -1;
        }
        
        if (!ImageKit_Image_toRGB(copy, CS_FMT(RGB24), -1)) {
            return -1;
        }
        
        self = copy;
    }
    
    /* Get output scales */
    scales[0] = (REAL)1.0 / self->channel_scales[0];
    scales[1] = (REAL)1.0 / self->channel_scales[1];
    scales[2] = (REAL)1.0 / self->channel_scales[2];
    scales[3] = (REAL)1.0 / self->channel_scales[3];
    
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
            colorspace_format = CS_FMT(RGB24);
            depth = 8;
            break;
    }
    
    format = (REAL *)&COLORSPACE_FORMAT_MINMAX[colorspace_format];

    fp = fopen(filepath, "wb");
    if (!fp) {
        ImageKit_Image_Delete(copy);
        ImageKit_SetError(ImageKit_IOError, "Unable to open file");
        return -1;
    }
    
    // Initialize
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        ImageKit_Image_Delete(copy);
        fclose(fp);
        ImageKit_SetError(ImageKit_StandardError, "libpng error");
        return -1;
    }
    
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        ImageKit_Image_Delete(copy);
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        
        ImageKit_SetError(ImageKit_StandardError, "libpng error");
        return -1;
    }

    // Error Handling
    if (setjmp(png_jmpbuf(png_ptr))) {
        ImageKit_Image_Delete(copy);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        
        ImageKit_SetError(ImageKit_StandardError, "libpng error");
        return -1;
    }

    // Add alpha channel?
    if (self->channels > 3) {
        png_colortype = PNG_COLOR_TYPE_RGBA;
    }
    
    png_set_IHDR(   png_ptr,
                    info_ptr,
                    self->width,
                    self->height,
                    depth,
                    png_colortype,
                    PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT,
                    PNG_FILTER_TYPE_DEFAULT);
    
    ptr_in = (REAL *)(&(self->data[0]));
    
    if (depth == 16) {
        // 16 Bits Per Channel
        row_pointers16 = png_malloc(png_ptr, sizeof(png_uint_16 *) * self->height);
        
        // Scale, clamp, copy
        for (y = 0; y < self->height; y++) {
            row_pointers16[y] = png_malloc(png_ptr, sizeof(png_uint_16) * self->width * self->channels);
            for (x = 0; x < self->width; x++) {
                for (c = 0; c < self->channels; c++) {
                    value = (int32_t)((*ptr_in++) * scales[c]);
                
                    if (value < format[c]) {
                        value = (int)(format[c]);
                    } else if (value > format[4+c]) {
                        value = (int)(format[4+c]);
                    }
                
                    row_pointers16[y][x * self->channels + c] = value;
                }
            }
        }
        
        // Write file
        png_init_io(png_ptr, fp);
        png_set_rows(png_ptr, info_ptr, (png_bytepp)row_pointers16);
        png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
        
        for (y = 0; y < self->height; y++) {
            png_free(png_ptr, row_pointers16[y]);
        }
    
        png_free(png_ptr, row_pointers16);
    } else {
        // 8 Bits Per Channels
        row_pointers = png_malloc(png_ptr, sizeof(png_byte *) * self->height);
    
        // Scale, clamp, copy
        for (y = 0; y < self->height; y++) {
            row_pointers[y] = png_malloc(png_ptr, sizeof(png_byte) * self->width * self->channels);
            for (x = 0; x < self->width; x++) {
                for (c = 0; c < self->channels; c++) {
                    value = (int32_t)((*ptr_in++) * scales[c]);
                
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
    }
    
    png_destroy_write_struct(&png_ptr, &info_ptr);
    ImageKit_Image_Delete(copy);
    fclose(fp);

    return 1;
}

#else

API
ImageKit_Image *
ImageKit_Image_FromPNG(const char *filepath, REAL scale) {
    ImageKit_SetError(ImageKit_StandardError, "Not compiled with PNG support");
    return NULL;
}

API
int
ImageKit_Image_SavePNG(ImageKit_Image *self, const char *filepath) {
    ImageKit_SetError(ImageKit_StandardError, "Not compiled with PNG support");
    return -1;
}

#endif
