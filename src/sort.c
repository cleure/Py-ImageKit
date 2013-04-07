#ifndef IK_SORT_DOT_C
#ifdef IK_INTERNAL

/* Routines and structures for sorting image buffers **/

typedef struct {
    uint32_t sort;
    void *ptr;
} SortIndex;

/*

        self.sortby = ( self.rgb[0] * 0.299 +
                        self.rgb[1] * 0.587 +
                        self.rgb[2] * 0.114)

*/

/*

from imagekit import *
b = ImageBuffer.fromPNG('/Users/cleure/avatar-100x100.png')

*/

int __ik_sort_compare_uint32(const void *A, const void *B)
{
    uint32_t a = ((SortIndex *)A)->sort;
    uint32_t b = ((SortIndex *)B)->sort;
    return a - b;
}

static struct SortIndex *
SortIndex_UsingLuma(ImageBuffer *self)
{
    size_t i, count;
    REAL_TYPE luma;
    REAL_TYPE scale;
    REAL_TYPE *ptr;
    SortIndex *index;
    
    /*
    
    FIXME: Requires 3 channels
    FIXME: Support grayscale
    FIXME: Support colorspaces
    
    */
    
    count = self->width * self->height;
    index = malloc(count * sizeof(*index));
    
    if (self->scale <= 0.0) {
        scale = (REAL_TYPE)1.0;
    } else {
        scale = (REAL_TYPE)COLORSPACE_FORMAT_MINMAX[self->colorspace_format][3];
    }
    
    ptr = (REAL_TYPE *)&(self->data[0]);
    if (self->channels < 3) {
        /* 1-2 channels (mono or mono + alpha) */
        for (i = 0; i < count; i++) {
            index[i].sort = (uint32_t)((*(ptr)) * 1000);
            index[i].ptr = ptr;
            ptr += self->channels;
        }
    } else {
        /* 3-4 channels (RGB or RGB + alpha) */
        for (i = 0; i < count; i++) {
            luma =      *(ptr  ) * scale * (REAL_TYPE)0.299 +
                        *(ptr+1) * scale * (REAL_TYPE)0.587 +
                        *(ptr+2) * scale * (REAL_TYPE)0.114;
        
            index[i].sort = (uint32_t)(luma * 1000);
            index[i].ptr = ptr;
        
            ptr += self->channels;
        }
    }
    
    qsort(index, count, sizeof(*index), &__ik_sort_compare_uint32);
    
    free(index);
    //return index;
    return NULL;
}

#endif /* IK_INTERNAL */
#endif /* IK_SORT_DOT_C */
