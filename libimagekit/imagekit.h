#pragma once

/* Internal Header */

#include <stdint.h>
#include "types.h"

#define API
#define PRIVATE static

#define CS_FMT(in)  COLORSPACE_FORMAT_##in
#define CS(in)      COLORSPACE_##in

#define HAVE_PNG    1
#define HAVE_JPEG   1
#define HAVE_GIF    0

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

#include "error.h"
#include "imagebuffer.h"
#include "png.h"
#include "coordinates.h"
