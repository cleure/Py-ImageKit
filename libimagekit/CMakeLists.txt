cmake_minimum_required (VERSION 2.6)
project (libimagekit)

set (libimagekit_VERSION_MAJOR 0)
set (libimagekit_VERSION_MINOR 1)
set (libimagekit_VERSION_PATCH 0)
set (CMAKE_C_FLAGS "-Wall -std=c99 -g")

set(LINK_LIBRARIES)
set(SOURCE_FILES
    coords.c
    error.c
    image.c
    jpeg.c
    png.c
    colorspace.c
    histogram.c
    fill.c
    blit.c
    curves.c
)

add_subdirectory(tests)

# JPEG
find_package (JPEG)
if (JPEG_FOUND)
    include_directories(${JPEG_INCLUDE_DIRS})
    LIST(APPEND LINK_LIBRARIES ${JPEG_LIBRARIES})
    add_definitions(-DHAVE_JPEG)
endif (JPEG_FOUND)

# PNG
find_package (PNG)
if (PNG_FOUND)
    include_directories(${PNG_INCLUDE_DIRS})
    LIST(APPEND LINK_LIBRARIES ${PNG_LIBRARIES})
    add_definitions(-DHAVE_PNG)
endif (PNG_FOUND)

# Math
LIST(APPEND LINK_LIBRARIES m)

configure_file (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
)

add_library(libimagekit ${SOURCE_FILES})

