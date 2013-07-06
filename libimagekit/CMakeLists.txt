cmake_minimum_required (VERSION 2.6)
project (libimagekit)

set (libimagekit_VERSION_MAJOR 0)
set (libimagekit_VERSION_MINOR 1)
set (libimagekit_VERSION_PATCH 0)
set (CMAKE_C_FLAGS "-Wall -std=c99 -g")

# JPEG
find_package (JPEG)
if (JPEG_FOUND)
    include_directories(${JPEG_INCLUDE_DIRS})
    add_definitions(-DHAVE_JPEG)
endif (JPEG_FOUND)

# PNG
find_package (PNG)
if (PNG_FOUND)
    include_directories(${PNG_INCLUDE_DIRS})
    add_definitions(-DHAVE_PNG)
endif (PNG_FOUND)

configure_file (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
)

include_directories("${PROJECT_BINARY_DIR}")

set(SOURCE_FILES
    coords.c
    error.c
    image.c
    jpeg.c
    png.c
)

add_library(libimagekit ${SOURCE_FILES})

add_executable(tests/build/test-01-png tests/test-01-png.c tests/framework.c)
target_link_libraries(tests/build/test-01-png libimagekit ${JPEG_LIBRARIES} ${PNG_LIBRARIES})

add_executable(tests/build/test-02-jpeg tests/test-02-jpeg.c tests/framework.c)
target_link_libraries(tests/build/test-02-jpeg libimagekit ${JPEG_LIBRARIES} ${PNG_LIBRARIES})
