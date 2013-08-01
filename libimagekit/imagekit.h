#pragma once

/* Internal Header */

#include <stdint.h>
#include "types.h"

#if defined(__GNUC__)
    #define INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define INLINE __forceinline
#else
    #define INLINE
#endif

#define API
#define PRIVATE static

#define CS_FMT(in)  COLORSPACE_FORMAT_##in
#define CS(in)      COLORSPACE_##in

#define PIXEL_INDEX(self, x, y)\
    (self->pitch * (y)) + ((x) * self->channels)

static const enum {
    CS(MONO),
    CS(RGB),
    CS(HSV),
    CS(YIQ),
    
    CS(SIZE) // Size
} _COLORSPACE;

static const enum {
    CS_FMT(RGB15),
    CS_FMT(RGB16),
    CS_FMT(RGB24),
    CS_FMT(RGB30),
    CS_FMT(RGB48),
    CS_FMT(HSV_NATURAL),
    CS_FMT(MONO_NATURAL),
    
    CS_FMT(SIZE) // Size
} _COLORSPACE_FORMAT;

extern const REAL COLORSPACE_FORMAT_MINMAX[COLORSPACE_FORMAT_SIZE][8];

API
void
ImageKit_Cleanup();

#include "error.h"
#include "image.h"
#include "io_png.h"
#include "io_jpeg.h"
#include "coords.h"
#include "rect.h"
#include "colorspace.h"
#include "histogram.h"
#include "fill.h"
#include "blit.h"
#include "curves.h"
#include "filters/pointfilter.h"
#include "filters/matrix.h"
#include "filters/convolution.h"
#include "filters/rankfilter.h"
