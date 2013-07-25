cmake_minimum_required (VERSION 2.6)
project (libimagekit)

set (libimagekit_VERSION_MAJOR 0)
set (libimagekit_VERSION_MINOR 1)
set (libimagekit_VERSION_PATCH 0)
set (CMAKE_C_FLAGS "-Wall -std=c99 -g")

set(LINK_LIBRARIES)
set(SOURCE_FILES
    # C-hashtable Sources
    htable/MurmurHash3.c
    htable/hashtable.c
    
    # ImageKit Sources
    coords.c
    error.c
    image.c
    io_jpeg.c
    io_png.c
    colorspace.c
    histogram.c
    fill.c
    blit.c
    curves.c
    filters/pointfilter.c
    filters/matrix.c
)

#add_subdirectory(tests)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(htable)

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

# Threads
find_package (Threads)
LIST(APPEND LINK_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

# Math
LIST(APPEND LINK_LIBRARIES m)

configure_file (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
)

add_library(imagekit ${SOURCE_FILES})

add_executable(tests/build/test-01-png tests/test-01-png.c tests/framework.c)
target_link_libraries(tests/build/test-01-png imagekit ${LINK_LIBRARIES})

add_executable(tests/build/test-02-jpeg tests/test-02-jpeg.c tests/framework.c)
target_link_libraries(tests/build/test-02-jpeg imagekit ${LINK_LIBRARIES})

add_executable(tests/build/test-03-hsv tests/test-03-hsv.c tests/framework.c)
target_link_libraries(tests/build/test-03-hsv imagekit ${LINK_LIBRARIES})

add_executable(tests/build/test-04-mono tests/test-04-mono.c tests/framework.c)
target_link_libraries(tests/build/test-04-mono imagekit ${LINK_LIBRARIES})

add_executable(tests/build/test-05-jpeg tests/test-05-jpeg.c tests/framework.c)
target_link_libraries(tests/build/test-05-jpeg imagekit ${LINK_LIBRARIES})

add_executable(tests/build/test-06-fill tests/test-06-fill.c tests/framework.c)
target_link_libraries(tests/build/test-06-fill imagekit ${LINK_LIBRARIES})

add_executable(tests/build/test-07-histogram tests/test-07-histogram.c tests/framework.c)
target_link_libraries(tests/build/test-07-histogram imagekit ${LINK_LIBRARIES})

add_executable(tests/build/test-08-blit tests/test-08-blit.c tests/framework.c)
target_link_libraries(tests/build/test-08-blit imagekit ${LINK_LIBRARIES})

add_executable(tests/build/test-09-bezier tests/test-09-bezier.c tests/framework.c)
target_link_libraries(tests/build/test-09-bezier imagekit ${LINK_LIBRARIES})

add_executable(tests/build/test-10-pointfilter tests/test-10-pointfilter.c tests/framework.c)
target_link_libraries(tests/build/test-10-pointfilter imagekit ${LINK_LIBRARIES})

add_executable(tests/build/test-11-errors tests/test-11-errors.c tests/framework.c htable/MurmurHash3.c htable/hashtable.c)
target_link_libraries(tests/build/test-11-errors imagekit ${LINK_LIBRARIES})

add_executable(tests/build/test-12-png30 tests/test-12-png30.c tests/framework.c)
target_link_libraries(tests/build/test-12-png30 imagekit ${LINK_LIBRARIES})
