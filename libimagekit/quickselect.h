
/**
* Type Agnostic QuickSelect Library.
*
* QuickSelect is an algorithm for finding the Kth smallest item in an unsorted
* array. It's commonly used for finding min, max, and median. It works by
* partially sorting the array with QuickSort, until Kth is found.
*
* @author       Cameron Eure <cleure@websprockets.com>
* @copyright    ©2012-2013 Cameron Eure
* @license      GPL v3.0
**/

/* Data Type. */
#ifndef QUICK_SELECT_TYPE
    #define QUICK_SELECT_TYPE int
#endif

/* Default linkage is static. */
#ifndef QUICK_SELECT_LINKAGE
    #define QUICK_SELECT_LINKAGE static
#endif

/* Useful for working with multiple data types, or symbol clashes. */
#ifndef QUICK_SELECT_SYMBOL
    #define QUICK_SELECT_SYMBOL(sym) sym
#endif

QUICK_SELECT_LINKAGE
QUICK_SELECT_TYPE
QUICK_SELECT_SYMBOL(quick_select)
(QUICK_SELECT_TYPE *a, size_t low, size_t high, size_t k)
{
    #define swap(pos_a, pos_b)\
        tmp = pos_a;\
        pos_a = pos_b;\
        pos_b = tmp;
    
    /* Sub Array Select Variables */
    size_t pivot_idx;
    size_t left_subarray_size;
    
    /* Loop Variables */
    size_t left;
    size_t partition_idx;
    size_t pivot_idx_loop;
    QUICK_SELECT_TYPE pivot;
    QUICK_SELECT_TYPE tmp;
    
    k++;
    while (1) {
        if (low == high) {
            return a[low];
        }
        
        /* Setup variables for partition */
        left = low;
        partition_idx = low;
        pivot_idx_loop = low + (high - low) / 2;
        pivot = a[pivot_idx_loop];
        
        swap(a[pivot_idx_loop], a[high]);
        
        /* Partition */
        pivot_idx_loop = high;
        while (left < high) {
            if (a[left] < pivot) {
                swap(a[left], a[partition_idx]);
                partition_idx++;
            }
        
            left++;
        }
    
        a[pivot_idx_loop] = a[partition_idx];
        a[partition_idx] = pivot;
        
        /* Setup variable for select */
        pivot_idx = partition_idx;
        left_subarray_size = pivot_idx - low + 1;
        
        /* Select subarray, or exit loop */
        if (left_subarray_size > k) {
            /* Left */
            high = pivot_idx - 1;
        } else if (left_subarray_size < k) {
            /* Right */
            low = pivot_idx + 1;
            k -= left_subarray_size;
        } else {
            /* Done (return) */
            break;
        }
    }
    
    #undef swap
    return a[pivot_idx];
}

#undef QUICK_SELECT_TYPE
#undef QUICK_SELECT_EXPORT
#undef QUICK_SELECT_SYMBOL
