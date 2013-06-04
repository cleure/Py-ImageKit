#pragma once

#include "imagekit.h"

/* Maximum Error Size */
#define IMAGEKIT_ERROR_MAX 4096

/* Possible Errors */
enum {
    ImageKit_NotImplementedError,
    ImageKit_StandardError,
    ImageKit_ValueError,
    ImageKit_TypeError,
    ImageKit_OSError,
    ImageKit_IOError,
    ImageKit_MemoryError,
    IMAGEKIT_NUM_ERRORS
} IMAGEKIT_ERROR;

/* Error Strings */
extern const char *ImageKit_ErrorStrings[IMAGEKIT_NUM_ERRORS];

/**
* Get Last Error
*
* @param    int *code
* @param    char **msg
* @return   void
**/
API
void
ImageKit_LastError(int *code, char **msg);

/**
* Set Error
*
* @param    int code
* @param    char *msg
* @return   int, 1 on success
**/
API
int
ImageKit_SetError(int code, const char *msg);
