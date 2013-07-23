#pragma once

#include <stdint.h>
#include <limits.h>

#define MAJOR_VERSION 0
#define MINOR_VERSION 1
#define PATCH_VERSION 0

#ifdef __HT_INTERNAL
  #define HT_EXTERN
#else
  #define HT_EXTERN extern
#endif

#ifndef HT_EXPORT
    #define HT_EXPORT(SYM) SYM
#endif

#define HT_ARGS(SYM) SYM

struct HT_EXPORT(htable_entry);
struct HT_EXPORT(htable);

/* htable_copyfn type definition */
typedef
void (* HT_EXPORT(htable_copyfn))
HT_ARGS((
    struct HT_EXPORT(htable_entry) *dst,
    void *key,
    void *data
));

/* htable_freefn type definition */
typedef
void (* HT_EXPORT(htable_freefn))
HT_ARGS((
    struct HT_EXPORT(htable_entry) *ptr
));

/* htable_cmpfn type definition. Returns 0 if A == B. */
typedef
int (* HT_EXPORT(htable_cmpfn))
HT_ARGS((
    void *A,
    void *B
));

/* Hash Table Entry */
struct HT_EXPORT(htable_entry) {
    uint32_t key_size;
    void *key;
    void *data;
    uint32_t entry;
    uint32_t hash;
};

/* Hash Table */
struct HT_EXPORT(htable) {
    struct HT_EXPORT(htable_entry) *table;
    struct HT_EXPORT(htable_entry) **entries;
    uint32_t size;
    uint32_t used;
    uint32_t seed;
    
    HT_EXPORT(htable_copyfn) copyfn;
    HT_EXPORT(htable_freefn) freefn;
    HT_EXPORT(htable_cmpfn) cmpfn;
};

/* A collection of hash table entries */
struct HT_EXPORT(htable_collection) {
    uint32_t size;
    uint32_t used;
    struct HT_EXPORT(htable_entry) **list;
};

/************************************************************************
* Built-in comparison functions. See: htable_cmpfn typedef
************************************************************************/

#if __WORDSIZE == 64

/**
* Built int comparison function for 64-bit integers.
*
* @param    void *A
* @param    void *B
* @return   int, zero if equal
**/
HT_EXTERN int
HT_EXPORT(htable_int64_cmpfn)
HT_ARGS((
    void *A,
    void *B
));

#endif

/**
* Built int comparison function for 32-bit integers.
*
* @param    void *A
* @param    void *B
* @return   int, zero if equal
**/
HT_EXTERN int
HT_EXPORT(htable_int32_cmpfn)
HT_ARGS((
    void *A,
    void *B
));

/**
* Built int comparison function for 16-bit integers.
*
* @param    void *A
* @param    void *B
* @return   int, zero if equal
**/
HT_EXTERN int
HT_EXPORT(htable_int16_cmpfn)
HT_ARGS((
    void *A,
    void *B
));

/**
* Built int comparison function for 8-bit integers.
*
* @param    void *A
* @param    void *B
* @return   int, zero if equal
**/
HT_EXTERN int
HT_EXPORT(htable_int8_cmpfn)
HT_ARGS((
    void *A,
    void *B
));

/**
* Built int comparison function for C Strings.
*
* @param    void *A
* @param    void *B
* @return   int, zero if equal
**/
HT_EXTERN int
HT_EXPORT(htable_cstring_cmpfn)
HT_ARGS((
    void *A,
    void *B
));

/************************************************************************
* Creation and maintenance functions
************************************************************************/

/**
* htable_new()
*
* Create a new hash table
*
* @param    uint32_t size
* @param    uint32_t seed
* @param    htable_cmpfn cmpfn
*               - See function typedef prototypes for more information
* @param    htable_copyfn copyfn
*               - See function typedef prototypes for more information
* @param    htable_freefn freefn
*               - See function typedef prototypes for more information
* @return   struct htable *
*               NULL on error
**/
HT_EXTERN struct HT_EXPORT(htable) *
HT_EXPORT(htable_new)
HT_ARGS((
    uint32_t size,
    uint32_t random_seed,
    HT_EXPORT(htable_cmpfn) cmpfn,
    HT_EXPORT(htable_copyfn) copyfn,
    HT_EXPORT(htable_freefn) freefn
));

/**
* htable_clone()
*
* Clone hash table, returning new object. If table->copyfn is not NULL,
* it will be used to copy data into the new hash table.
*
* @param    struct htable *src
* @return   struct htable *
*               NULL on error
**/
HT_EXTERN struct HT_EXPORT(htable) *
HT_EXPORT(htable_clone)
HT_ARGS((
    struct HT_EXPORT(htable) *src
));

/**
* htable_delete()
*
* Delete hash table created by htable_new(). If table->freefn is not NULL,
* it will be called for each element, so allocated memory can be freed.
*
* @param    struct htable *table
* @return   void
**/
HT_EXTERN void
HT_EXPORT(htable_delete)
HT_ARGS((
    struct HT_EXPORT(htable) *table
));

/**
* htable_resize()
*
* Resize hash table, normally for growing, but can also be used to
* shrink the size of the table.
*
* @param    struct htable *table
* @param    uint8_t load_thresh
*               Number between 0 and 100. If load factor is below it,
*               then resize won't trigger. Use 0 to disable.
*
* @param    uint32_t new_size
* @return   0 on error, 1 on success
**/
HT_EXTERN int
HT_EXPORT(htable_resize)
HT_ARGS((
    struct HT_EXPORT(htable) *table,
    uint8_t load_thresh,
    uint32_t new_size
));

/**
* Create new htable_collection object.
*
* @param    uint32_t size
* @return   NULL on error
**/
HT_EXTERN
struct HT_EXPORT(htable_collection) *
HT_EXPORT(htable_collection_new)
HT_ARGS((
    uint32_t size
));

/**
* Delete htable_collection object created by htable_collection_new
*
* @param    struct htable_collection *
* @return   void
**/
HT_EXTERN void
HT_EXPORT(htable_collection_delete)
HT_ARGS((
    struct HT_EXPORT(htable_collection) *collection
));

/**
* Resize htable_collection object.
*
* @param    struct htable_collection *
* @param    uint32_t size
* @return   NULL on error
**/
int
HT_EXPORT(htable_collection_resize)
HT_ARGS((
    struct HT_EXPORT(htable_collection) *collection,
    uint32_t size
));

/************************************************************************
* Manipulation functions
************************************************************************/

/**
* htable_add()
*
* Add item to hash table. 
*
* @param    struct htable *table
* @param    uint32_t key_size
*               - sizeof(key) for ints
*               - strlen(key) for strings
* @param    void *key
* @param    void *data
*
* @return   0 on error, 1 on success
**/
HT_EXTERN int
HT_EXPORT(htable_add)
HT_ARGS((
    struct HT_EXPORT(htable) *table,
    uint32_t key_size,
    void *key,
    void *data
));

/**
* htable_add_loop()
*
* Unlink htable_add(), which will fail on its first try, htable_add_loop()
* will try a maximum of "max_loops" times to add an item before giving up.
* It uses a quadratic resizing algorithm, so its best to keep max_loops
* relatively small.
*
* @param    struct htable *table
* @param    uint32_t key_size
*               - sizeof(key) for ints
*               - strlen(key) for strings
* @param    void *key
* @param    void *data
* @param    int max_loops
*
* @return   0 on failure, otherwise the number of loops it took to
*           successfully add item.
**/
HT_EXTERN int
HT_EXPORT(htable_add_loop)
HT_ARGS((
    struct HT_EXPORT(htable) *table,
    uint32_t key_size,
    void *key,
    void *data,
    int max_loops
));

/**
* htable_remove()
*
* Remove item from hash table.
*
*
* @param    struct htable *table
* @param    uint32_t key_size
*               - sizeof(key) for ints
*               - strlen(key) for strings
* @param    void *key
*
* @return   0 on error, 1 on success
**/
HT_EXTERN int
HT_EXPORT(htable_remove)
HT_ARGS((
    struct HT_EXPORT(htable) *table,
    uint32_t key_size,
    void *key
));

/**
* htable_get()
*
* Get entry from hash table.
*
* @param    struct htable *table
* @param    uint32_t key_size
*               - sizeof(key) for ints
*               - strlen(key) for strings
* @param    void *key
*
* @return   NULL on error, pointer on success
**/
HT_EXTERN struct HT_EXPORT(htable_entry) *
HT_EXPORT(htable_get)
HT_ARGS((
    struct HT_EXPORT(htable) *table,
    uint32_t key_size,
    void *key
));

/************************************************************************
* Utility functions
************************************************************************/

/**
* htable_intersect()
*
* Get intersection of two hash tables, by key. Entries in the list
* are pointers to elements in b, thus free()'ing b and then trying
* to access elements in the list will likely cause a segfault. The
* returned list can be freed via the htable_collection_delete()
* function.
*
* Usage:
*
* uint32_t i;
* struct htable_collection *list = htable_difference(a, b);
* 
* for (i = 0; i < list->used; i++) {
*     printf("%s\n", (char *)(list->list[i]->key));
* }
*
* htable_collection_delete(list);
*
* @param    struct htable *a
* @param    struct htable *b
* @return   struct htable_collection *
*               NULL on error
**/
HT_EXTERN struct HT_EXPORT(htable_collection) *
HT_EXPORT(htable_intersect)
HT_ARGS((
    struct HT_EXPORT(htable) *a,
    struct HT_EXPORT(htable) *b
));

/**
* htable_difference()
*
* Get difference of two hash tables, by key. Entries in the list
* are pointers to elements in b, thus free()'ing b and then trying
* to access elements in the list will likely cause a segfault. The
* returned list can be freed via the htable_collection_delete()
* function.
*
* Usage:
*
* uint32_t i;
* struct htable_collection *list = htable_difference(a, b);
* 
* for (i = 0; i < list->used; i++) {
*     printf("%s\n", (char *)(list->list[i]->key));
* }
*
* htable_collection_delete(list);
*
* @param    struct htable *a
* @param    struct htable *b
* @return   struct htable_collection *
*               NULL on error
**/
HT_EXTERN struct HT_EXPORT(htable_collection) *
HT_EXPORT(htable_difference)
HT_ARGS((
    struct HT_EXPORT(htable) *a,
    struct HT_EXPORT(htable) *b
));

#ifndef __HT_INTERNAL
  #undef HT_EXTERN
  #undef HT_ARGS
  #undef HT_EXPORT
#endif
