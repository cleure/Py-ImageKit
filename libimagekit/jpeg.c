
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <stddef.h>
#include <setjmp.h>
#include "imagekit.h"

#ifdef HAVE_JPEG

#include <jpeglib.h>

API
ImageKit_Image *
ImageKit_Image_FromJPEG(const char *filepath);

API
int
ImageKit_Image_SaveJPEG(ImageKit_Image *self, const char *filepath);

#else

API
ImageKit_Image *
ImageKit_Image_FromJPEG(const char *filepath) {
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
