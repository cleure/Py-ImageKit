#ifndef IK_SORT_DOT_C
#ifdef IK_INTERNAL

/* Routines and structures for sorting image buffers **/

/*

        self.sortby = ( self.rgb[0] * 0.299 +
                        self.rgb[1] * 0.587 +
                        self.rgb[2] * 0.114)

*/

/*

from imagekit import *
b = ImageBuffer.fromPNG('/Users/cleure/avatar-100x100.png')

*/

#define SORT_ELEMENT_SIZE(self) (sizeof(REAL_TYPE) * (self)->channels)

/*

static int compare_rgb_luma(const void *A, const void *B)
{
    #define RESULT(in) (in[0] * 0.299 + in[1] * 0.587 + in[2] * 0.114)

    float *a = (float *)A;
    float *b = (float *)B;
    
    if (RESULT(a) > RESULT(b)) {
        return 1;
    }
    
    return -1;
    
    #undef RESULT
}

SORT_FN(self->data,
        self->width * self->height * self->channels,
        SORT_ELEMENT_SIZE(self),
        &compare_rgb_luma);

*/

#endif /* IK_INTERNAL */
#endif /* IK_SORT_DOT_C */
