#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <limits.h>

#include "config.h"

#define __HT_INTERNAL
#include "hashtable.h"
#include "MurmurHash3.h"

#define HT_STRUCT(in) struct HT_EXPORT(in)

#if __WORDSIZE == 64

/**
* Built int comparison function for 64-bit integers.
*
* @param    void *A
* @param    void *B
* @return   int, zero if equal
**/
int
HT_EXPORT(htable_int64_cmpfn)
HT_ARGS((
    void *A,
    void *B
)) {
    if (*(int64_t *)A == *(int64_t *)B) {
        return 0;
    }
    
    return 1;
}

#endif

/**
* Built int comparison function for 32-bit integers.
*
* @param    void *A
* @param    void *B
* @return   int, zero if equal
**/
int
HT_EXPORT(htable_int32_cmpfn)
HT_ARGS((
    void *A,
    void *B
)) {
    if (*(int32_t *)A == *(int32_t *)B) {
        return 0;
    }
    
    return 1;
}

/**
* Built int comparison function for 16-bit integers.
*
* @param    void *A
* @param    void *B
* @return   int, zero if equal
**/
int
HT_EXPORT(htable_int16_cmpfn)
HT_ARGS((
    void *A,
    void *B
)) {
    if (*(int16_t *)A == *(int16_t *)B) {
        return 0;
    }
    
    return 1;
}

/**
* Built int comparison function for 8-bit integers.
*
* @param    void *A
* @param    void *B
* @return   int, zero if equal
**/
int
HT_EXPORT(htable_int8_cmpfn)
HT_ARGS((
    void *A,
    void *B
)) {
    if (*(int8_t *)A == *(int8_t *)B) {
        return 0;
    }
    
    return 1;
}

/**
* Built int comparison function for C Strings.
*
* @param    void *A
* @param    void *B
* @return   int, zero if equal
**/
int
HT_EXPORT(htable_cstring_cmpfn)
HT_ARGS((
    void *A,
    void *B
)) {
    char *a = (char *)A;
    char *b = (char *)B;
    
    while (1) {
        if (!*a && !*b) {
            break;
        } else if (*a++ != *b++) {
            return 1;
        }
    }
    
    return 0;
}

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
HT_STRUCT(htable) *
HT_EXPORT(htable_new)
HT_ARGS((
    uint32_t size,
    uint32_t random_seed,
    HT_EXPORT(htable_cmpfn) cmpfn,
    HT_EXPORT(htable_copyfn) copyfn,
    HT_EXPORT(htable_freefn) freefn
)) {
    HT_STRUCT(htable) *table;
    
    /* cmpfn is required */
    if (cmpfn == NULL) {
        return NULL;
    }
    
    table = malloc(sizeof(*table));
    if (!table) {
        return NULL;
    }
    
    memset(table, 0, sizeof(*table));
    table->table = malloc(sizeof(*table->table) * size);
    if (!table->table) {
        free(table);
        return NULL;
    }
    
    table->entries = malloc(sizeof(*table->entries) * size);
    if (!table->entries) {
        free(table->table);
        free(table);
        return NULL;
    }
    
    memset(table->table, 0, sizeof(*table->table) * size);
    memset(table->entries, 0, sizeof(*table->entries) * size);
    table->size = size;
    table->used = 0;
    table->seed = random_seed;
    table->copyfn = copyfn;
    table->freefn = freefn;
    table->cmpfn = cmpfn;
    
    return table;
}

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
HT_EXTERN HT_STRUCT(htable) *
HT_EXPORT(htable_clone)
HT_ARGS((
    HT_STRUCT(htable) *src
)) {
    
    uint32_t i, hash;
    
    HT_STRUCT(htable_entry) *table,
                            **entries;
    
    HT_STRUCT(htable) *dst = HT_EXPORT(htable_new)(
                                    src->size,
                                    src->seed,
                                    src->cmpfn,
                                    src->copyfn,
                                    src->freefn);
    
    if (!dst) {
        return NULL;
    }
    
    /* Retain pointers */
    table = dst->table;
    entries = dst->entries;
    
    /* Zero out tables */
    memset(table, 0, sizeof(*table) * src->size);
    memset(entries, 0, sizeof(*entries) * src->size);
    
    /* Copy memory */
    memcpy(dst, src, sizeof(*dst));
    
    /* Link pointers */
    dst->table = table;
    dst->entries = entries;
    
    if (src->copyfn == NULL) {
        for (i = 0; i < src->used; i++) {
            /* TODO: Use memcpy()??? */
        
            hash = src->entries[i]->hash;

            dst->table[hash].key_size = src->entries[i]->key_size;
            dst->table[hash].key = src->entries[i]->key;
            dst->table[hash].data = src->entries[i]->data;
            dst->table[hash].entry = i;
            dst->table[hash].hash = hash;
            
            dst->entries[i] = &dst->table[hash];
        }
    } else {
        for (i = 0; i < src->used; i++) {
            hash = src->entries[i]->hash;
            
            src->copyfn(&dst->table[hash], src->entries[i]->key, src->entries[i]->data);
            dst->entries[i] = &dst->table[hash];
        }
    }
    
    return dst;
}

/**
* htable_delete()
*
* Delete hash table created by htable_new(). If table->freefn is not NULL,
* it will be called for each element, so allocated memory can be freed.
*
* @param    struct htable *table
* @return   void
**/
void
HT_EXPORT(htable_delete)
HT_ARGS((
    HT_STRUCT(htable) *table
)) {
    
    uint32_t i;
    
    if (table->freefn != NULL) {
        for (i = 0; i < table->used; i++) {
            if (table->entries[i] && table->entries[i]->key) {
                /* Call freefn() */
                table->freefn(table->entries[i]);
            }
        }
    }
    
    free(table->table);
    free(table->entries);
    free(table);
}

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
int
HT_EXPORT(htable_resize)
HT_ARGS((
    HT_STRUCT(htable) *table,
    uint8_t load_thresh,
    uint32_t new_size
)) {

    uint32_t i, res;
    float load_calc;
    
    HT_STRUCT(htable) tmp_table;
    HT_STRUCT(htable_entry) *new_table;
    HT_STRUCT(htable_entry) **new_entries;
    
    /* Check load_thresh before proceeding */
    load_calc = 100.0f * ((float)table->used / (float)table->size);
    if (load_thresh && load_calc < load_thresh) {
        return 1;
    }
    
    new_table = malloc(sizeof(*new_table) * new_size);
    if (!new_table) {
        return 0;
    }
    
    new_entries = malloc(sizeof(*new_entries) * new_size);
    if (!new_entries) {
        free(new_table);
        return 0;
    }
    
    /* Zero out */
    memset(&tmp_table, 0, sizeof(tmp_table));
    memset(new_table, 0, sizeof(*new_table) * new_size);
    memset(new_entries, 0, sizeof(*new_entries) * new_size);
    
    /* Set variables on tmp_table */
    tmp_table.table = new_table;
    tmp_table.entries = new_entries;
    tmp_table.size = new_size;
    tmp_table.used = 0;
    tmp_table.seed = table->seed;
    tmp_table.copyfn = NULL;
    tmp_table.freefn = NULL;
    tmp_table.cmpfn = table->cmpfn;
    
    /* Iterate over source */
    for (i = 0; i < table->used; i++) {
        
        /* Add entries to new table array */
        res = HT_EXPORT(htable_add)(
                        &tmp_table,
                        table->entries[i]->key_size,
                        table->entries[i]->key,
                        table->entries[i]->data);
        
        /* Catch error */
        if (!res) {
            free(new_table);
            free(new_entries);
            return 0;
        }
    }
    
    /* Free old memory */
    free(table->table);
    free(table->entries);
    
    /* Link up new data */
    table->table = new_table;
    table->entries = new_entries;
    table->size = new_size;
    table->used = tmp_table.used;
    
    return 1;
}

/**
* Create new htable_collection object.
*
* @param    uint32_t size
* @return   NULL on error
**/
HT_STRUCT(htable_collection) *
HT_EXPORT(htable_collection_new)
HT_ARGS((
    uint32_t size
)) {
    HT_STRUCT(htable_collection) *collection
        = malloc(sizeof(*collection));
    
    if (!collection) {
        return NULL;
    }
    
    memset(collection, 0, sizeof(*collection));
    collection->list = malloc(sizeof(*(collection->list)) * size);
    if (!collection->list) {
        free(collection);
        return NULL;
    }
    
    collection->size = size;
    collection->used = 0;
    
    return collection;
}

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
    HT_STRUCT(htable_collection) *collection,
    uint32_t size
)) {
    HT_STRUCT(htable_entry) **new_list;
    
    if (size < collection->used) {
        /* Elements wont fit */
        return 0;
    }
    
    new_list = malloc(sizeof(*new_list) * (size + 1));
    if (!new_list) {
        return 0;
    }
    
    memcpy(new_list, collection->list, sizeof(*new_list) * (collection->used + 1));
    free(collection->list);
    collection->list = new_list;
    
    return 1;
}

/**
* Delete htable_collection object created by htable_collection_new
*
* @param    struct htable_collection *
* @return   void
**/
void
HT_EXPORT(htable_collection_delete)
HT_ARGS((
    HT_STRUCT(htable_collection) *collection
)) {
    if (!collection) {
        return;
    }
    
    if (collection->list) {
        free(collection->list);
    }
    
    free(collection);
}

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
int
HT_EXPORT(htable_add)
HT_ARGS((
    HT_STRUCT(htable) *table,
    uint32_t key_size,
    void *key,
    void *data
)) {
    
    uint32_t hash,
             step = 0;
    
    /* Get initial hash */
    MurmurHash3_x86_32(key, key_size, table->seed, &hash);
    
    do {
        /* Quadratic Probing Function:
            h = (h + (step * step - step) / 2) % size */
        hash = (hash + (step * step - step) / 2) % table->size;
        
        if (table->table[hash].key == NULL) {
            goto insert;
        } else if (table->cmpfn(key, table->table[hash].key) == 0) {
            /* Replace */
            if (table->freefn != NULL) {
                /* Call freefn() */
                table->freefn(&(table->table[hash]));
            }
            
            goto replace;
        }
        
        step += 1;
    } while (hash);
    
    return 0;
    
        insert:
            table->table[hash].hash = hash;
            table->table[hash].entry = table->used;
            table->entries[table->used] = &table->table[hash];
            table->used++;
            
        replace:
            table->table[hash].key_size = key_size;
            if (table->copyfn) {
                table->copyfn(&(table->table[hash]), key, data);
            } else {
                table->table[hash].key = key;
                table->table[hash].data = data;
            }
    
    return 1;
}

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
int
HT_EXPORT(htable_add_loop)
HT_ARGS((
    HT_STRUCT(htable) *table,
    uint32_t key_size,
    void *key,
    void *data,
    int max_loops
)) {
    int loop;
    int chunk = table->size;
    uint32_t size = table->size;
    
    max_loops++;
    for (loop = 1; loop < max_loops; loop++) {
        if (HT_EXPORT(htable_add)(table, key_size, key, data)) {
            return loop;
        }
        
        size += chunk;
        chunk += (chunk / 2);
        
        if (!HT_EXPORT(htable_resize)(table, 0, size)) {
            return 0;
        }
    }
    
    return 0;
}

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
int
HT_EXPORT(htable_remove)
HT_ARGS((
    HT_STRUCT(htable) *table,
    uint32_t key_size,
    void *key
)) {

    uint32_t hash,
             step = 0;
    
    /* Get initial hash */
    MurmurHash3_x86_32(key, key_size, table->seed, &hash);
    
    do {
        /* Quadratic Probing */
        hash = (hash + (step * step - step) / 2) % table->size;
        
        if (    table->table[hash].key &&
                table->cmpfn(key, table->table[hash].key) == 0) {
            
            if (table->freefn != NULL) {
                /* Call freefn() */
                table->freefn(&table->table[hash]);
            }
            
            if (table->used > 0) {
                /* Swap current entry with last entry, then NULL out last entry
                   to maintain linear array of pointers to elements. */
                table->entries[table->table[hash].entry] = table->entries[table->used-1];
                table->entries[table->table[hash].entry]->entry = table->table[hash].entry;
                table->entries[table->used-1] = NULL;
            } else {
                table->entries[0] = NULL;
            }
            
            /* Zero out entry */
            memset(&table->table[hash], 0, sizeof(*table->table));
            
            /* Decrement used count */
            table->used--;
            return 1;
        }
        
        step += 1;
    } while (hash);
    
    return 0;
}

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
HT_STRUCT(htable_entry) *
HT_EXPORT(htable_get)
HT_ARGS((
    HT_STRUCT(htable) *table,
    uint32_t key_size,
    void *key
)) {
    
    uint32_t hash,
             step = 0;
    
    /* Get initial hash */
    MurmurHash3_x86_32(key, key_size, table->seed, &hash);
    
    do {
        /* Quadratic Probing */
        hash = (hash + (step * step - step) / 2) % table->size;
        
        if (    table->table[hash].key &&
                table->cmpfn(key, table->table[hash].key) == 0) {
            return &(table->table[hash]);
        }
        
        step += 1;
    } while (hash);
    
    return NULL;
}

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
* struct htable_collection *list = htable_intersect(a, b);
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
HT_STRUCT(htable_collection) *
HT_EXPORT(htable_intersect)
HT_ARGS((
    HT_STRUCT(htable) *a,
    HT_STRUCT(htable) *b
)) {

    uint32_t max_size, i;
    
    HT_STRUCT(htable_collection) *collection;
    HT_STRUCT(htable_entry) **list,
                            *tmp;

    if (a->used > b->used) {
        max_size = a->used;
    } else {
        max_size = a->used;
    }
    
    collection = HT_EXPORT(htable_collection_new)(max_size+1);
    if (!collection) {
        return NULL;
    }
    
    list = collection->list;
    for (i = 0; i < a->used; i++) {
        tmp = HT_EXPORT(htable_get)(b, a->entries[i]->key_size, a->entries[i]->key);
        
        if (tmp != NULL) {
            list[0] = tmp;
            list++;
            collection->used++;
        }
    }
    
    list[0] = NULL;
    return collection;
}

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
HT_STRUCT(htable_collection) *
HT_EXPORT(htable_difference)
HT_ARGS((
    HT_STRUCT(htable) *a,
    HT_STRUCT(htable) *b
)) {

    uint32_t max_size, i;
    
    HT_STRUCT(htable_collection) *collection;
    HT_STRUCT(htable_entry) **list, *tmp;
    
    if (a->used > b->used) {
        max_size = a->used;
    } else {
        max_size = b->used;
    }
    
    collection = HT_EXPORT(htable_collection_new)(max_size+1);
    if (!collection) {
        return NULL;
    }
    
    list = collection->list;
    for (i = 0; i < a->used; i++) {
        tmp = HT_EXPORT(htable_get)(b, a->entries[i]->key_size, a->entries[i]->key);
        if (!tmp) {
            list[0] = a->entries[i];
            list++;
        }
    }
    
    list[0] = NULL;
    return collection;
}
