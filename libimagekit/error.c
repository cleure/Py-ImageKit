#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imagekit.h"

PRIVATE int  INITIALIZED = 0;
PRIVATE int  LAST_ERROR_CODE = 0;
PRIVATE char LAST_ERROR[IMAGEKIT_ERROR_MAX + 1];

/* Error Strings */
const char *ImageKit_ErrorStrings[IMAGEKIT_NUM_ERRORS] = {
    "Not Implemented",
    "Standard Error",
    "Value Error",
    "Type Error",
    "OS Error",
    "IO Error",
    "Memory Error",
    "Index Error"
};

PRIVATE void init()
{
    memset(&LAST_ERROR, 0, sizeof(LAST_ERROR));
    INITIALIZED = 1;
}

/**
* Get Last Error
*
* @param    int *code
* @param    char **msg
* @return   void
**/
API
void
ImageKit_LastError(int *code, char **msg)
{
    if (!INITIALIZED) {
        init();
    }
    
    if (!LAST_ERROR[0]) {
        *msg = NULL;
        *code = -1;
    }
    
    *msg = (char *)&LAST_ERROR;
    *code = LAST_ERROR_CODE;
}

/**
* Set Error
*
* @param    int code
* @param    char *msg
* @return   int, 1 on success
**/
API
int
ImageKit_SetError(int code, const char *msg)
{
    if (!INITIALIZED) {
        init();
    }
    
    if (code < 0 || !(code < IMAGEKIT_NUM_ERRORS)) {
        return 0;
    }
    
    if (!(strlen(msg) <= IMAGEKIT_ERROR_MAX)) {
        return 0;
    }

    LAST_ERROR_CODE = code;
    strcpy((char *)&LAST_ERROR, msg);
    
    return 1;
}

/**
* Get Error String, for specified code
*
* @param    int code
* @param    char **msg
* @return   void
**/
API
void
ImageKit_GetErrorString(int code, char **msg)
{
    if (code < 0 || !(code < IMAGEKIT_NUM_ERRORS)) {
        *msg = NULL;
        return;
    }
    
    *msg = (char *)(ImageKit_ErrorStrings[code]);
}
