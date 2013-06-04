#pragma once

#include <stdint.h>

#define API
#define PRIVATE static

#ifndef REAL
    #define REAL float
#endif

#ifndef DIMENSION
    #define DIMENSION uint32_t
#endif

#define CS_FMT(in) COLORSPACE_FORMAT_##in

/* Constants */
const int HAVE_PNG;
const int HAVE_JPEG;
const int HAVE_GIF;

static const enum {
    COLORSPACE_MONO,
    COLORSPACE_RGB,
    COLORSPACE_HSV,
    COLORSPACE_YIQ,
    COLORSPACE_SIZE
} _COLORSPACE;

static const enum {
    CS_FMT(RGB15),
    CS_FMT(RGB16),
    CS_FMT(RGB24),
    CS_FMT(RGB30),
    CS_FMT(RGB48),
    CS_FMT(HSV_NATURAL),
    CS_FMT(MONO_NATURAL),
    COLORSPACE_FORMAT_SIZE
} _COLORSPACE_FORMAT;

const double COLORSPACE_FORMAT_MINMAX[COLORSPACE_FORMAT_SIZE][8];
