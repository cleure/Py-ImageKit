
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <stddef.h>
#include "imagekit.h"

/*

http://giflib.sourceforge.net/gif_lib.html#idp28811056

*/

#ifdef HAVE_GIF

#include <gif_lib.h>

API
ImageKit_Image *
ImageKit_Image_FromGIF(const char *filepath, REAL scale)
{
    return NULL;
}

API
int
ImageKit_Image_SaveGIF(ImageKit_Image *self, const char *filepath)
{
    return -1;
}

#else

API
ImageKit_Image *
ImageKit_Image_FromGIF(const char *filepath, REAL scale) {
    ImageKit_SetError(ImageKit_StandardError, "Not compiled with GIF support");
    return NULL;
}

API
int
ImageKit_Image_SaveGIF(ImageKit_Image *self, const char *filepath) {
    ImageKit_SetError(ImageKit_StandardError, "Not compiled with PNG support");
    return -1;
}

#endif
