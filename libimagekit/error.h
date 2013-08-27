#pragma once

/* Maximum Error Size */
#define IMAGEKIT_ERROR_MAX 1024

/* Possible Errors */
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
    ImageKit_ArgumentError,
    IMAGEKIT_NUM_ERRORS
} IMAGEKIT_ERROR;
*/

/*

Uninstall:

py27-wxpython30


*/

#define ImageKit_NotImplementedError    0
#define ImageKit_StandardError          1
#define ImageKit_ValueError             2
#define ImageKit_TypeError              3
#define ImageKit_OSError                4
#define ImageKit_IOError                5
#define ImageKit_MemoryError            6
#define ImageKit_IndexError             7
#define ImageKit_ArgumentError          8
#define IMAGEKIT_NUM_ERRORS             9

/* Error Struct */
typedef struct ImageKit_Error {
    int code;
    char msg[IMAGEKIT_ERROR_MAX + 1];
} ImageKit_Error;

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
* Get Last Error for Thread ID
*
* @param    int *code
* @param    char **msg
* @param    uintptr_t thread_id
* @return   void
**/
API
void
ImageKit_LastErrorForThread(int *code, char **msg, uintptr_t thread_id);

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

/**
* Get Error String, for specified code
*
* @param    int code
* @param    char **msg
* @return   void
**/
API
void
ImageKit_GetErrorString(int code, char **msg);

/**
* Cleanup function, frees and heap allocated data
*
* @return   void
**/
API
void
ImageKit_CleanupError();
