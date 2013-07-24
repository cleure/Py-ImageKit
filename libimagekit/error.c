#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "imagekit.h"
#include "threading.h"
#include "hashtable.h"

PRIVATE int  INITIALIZED = 0;
PRIVATE struct htable *thread_table = NULL;
PRIVATE MUTEX_T thread_table_mutex = MUTEX_T_INITIALIZER;

/* Error Strings */
const char *ImageKit_ErrorStrings[IMAGEKIT_NUM_ERRORS] = {
    "Not Implemented",
    "Standard Error",
    "Value Error",
    "Type Error",
    "OS Error",
    "IO Error",
    "Memory Error",
    "Index Error",
    "Argument Error"
};

PRIVATE int thread_table_cmpfn(void *A, void *B)
{
    if (*(void **)A == *(void **)B) {
        return 0;
    }
    
    return -1;
}

static void thread_table_copyfn(struct htable_entry *ent, void *key, void *value)
{
    ent->key = malloc(sizeof(uintptr_t));
    ent->data = malloc(sizeof(ImageKit_Error));
    
    memcpy(ent->key, key, sizeof(uintptr_t));
    memcpy(ent->data, value, sizeof(ImageKit_Error));
}

static void thread_table_freefn(struct htable_entry *ent)
{
    free(ent->key);
    free(ent->data);
}

PRIVATE void init()
{
    srand(time(NULL));

    thread_table = htable_new(
        1024,
        rand(),
        &thread_table_cmpfn,
        &thread_table_copyfn,
        &thread_table_freefn
    );
    
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
    struct htable_entry *table_ent;
    ImageKit_Error *error_ent;
    uintptr_t addr;
    
    BEGIN_SYNCHRONIZED(thread_table_mutex) {
    
        *msg = NULL;
        *code = -1;
        
        if (INITIALIZED) {
            addr = THREAD_ID();
        
            table_ent = htable_get(
                thread_table,
                sizeof(void *),
                &addr
            );
            
            if (table_ent != NULL) {
                error_ent = table_ent->data;
                
                *code = error_ent->code;
                *msg = (char *)&error_ent->msg;
            }
        }
        
    } END_SYNCHRONIZED(thread_table_mutex);
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
    int result;
    ImageKit_Error ent;
    uintptr_t addr;

    BEGIN_SYNCHRONIZED(thread_table_mutex) {
    
        if (!INITIALIZED) {
            init();
        }
    
        if (code < 0 || !(code < IMAGEKIT_NUM_ERRORS)) {
            result = 0;
        } else if (!(strlen(msg) <= IMAGEKIT_ERROR_MAX)) {
            result = 0;
        } else {
            ent.code = code;
            strcpy((char *)&ent.msg, msg);
            addr = THREAD_ID();
            
            result = htable_add_loop(
                thread_table,
                sizeof(void *),
                &addr,
                &ent,
                8
            );
        }
        
    } END_SYNCHRONIZED(thread_table_mutex);
    
    return result;
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

/**
* Cleanup function, frees and heap allocated data
*
* @return   void
**/
API
void
ImageKit_CleanupError()
{
    BEGIN_SYNCHRONIZED(thread_table_mutex) {
        if (INITIALIZED) {
            htable_delete(thread_table);
            //pthread_mutex_destroy(&thread_table_mutex);
        }
        
        INITIALIZED = 0;
    } END_SYNCHRONIZED(thread_table_mutex);
}
