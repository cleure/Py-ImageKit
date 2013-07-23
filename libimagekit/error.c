#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "imagekit.h"
#include "hashtable.h"

PRIVATE int  INITIALIZED = 0;
//PRIVATE int  LAST_ERROR_CODE = 0;
//PRIVATE char LAST_ERROR[IMAGEKIT_ERROR_MAX + 1];

/* These will be used to make errors per-thread */
struct error_entry {
    int code;
    char msg[IMAGEKIT_ERROR_MAX + 1];
};

PRIVATE struct htable *thread_table = NULL;
PRIVATE pthread_mutex_t thread_table_mutex = PTHREAD_MUTEX_INITIALIZER;

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

#define BEGIN_SYNCHRONIZED pthread_mutex_lock(&thread_table_mutex);
#define END_SYNCHRONIZED pthread_mutex_unlock(&thread_table_mutex)
#define THREAD_ID() pthread_self()

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
    ent->data = malloc(sizeof(struct error_entry));
    
    memcpy(ent->key, key, sizeof(uintptr_t));
    memcpy(ent->data, value, sizeof(struct error_entry));
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
    struct error_entry *error_ent;
    uintptr_t addr;
    
    BEGIN_SYNCHRONIZED {
    
        *msg = NULL;
        *code = -1;
        
        if (INITIALIZED) {
            addr = (uintptr_t)THREAD_ID();
        
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
    } END_SYNCHRONIZED;
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
    struct error_entry ent;
    uintptr_t addr;

    BEGIN_SYNCHRONIZED {
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
            addr = (uintptr_t)THREAD_ID();
            
            result = htable_add_loop(
                thread_table,
                sizeof(void *),
                &addr,
                &ent,
                8
            );
        }
    } END_SYNCHRONIZED;
    
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
* @param    int code
* @param    char **msg
* @return   void
**/
API
void
ImageKit_CleanupError(int code, char **msg)
{
    BEGIN_SYNCHRONIZED {
        if (INITIALIZED) {
            htable_delete(thread_table);
            //pthread_mutex_destroy(&thread_table_mutex);
        }
        
        INITIALIZED = 0;
    } END_SYNCHRONIZED;
}
