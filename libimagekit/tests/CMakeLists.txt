
include_directories(..)

add_executable(build/test-01-png test-01-png.c framework.c)
target_link_libraries(build/test-01-png libimagekit ${LINK_LIBRARIES})

add_executable(build/test-02-jpeg test-02-jpeg.c framework.c)
target_link_libraries(build/test-02-jpeg libimagekit ${LINK_LIBRARIES})

add_executable(build/test-03-hsv test-03-hsv.c framework.c)
target_link_libraries(build/test-03-hsv libimagekit ${LINK_LIBRARIES})

add_executable(build/test-04-mono test-04-mono.c framework.c)
target_link_libraries(build/test-04-mono libimagekit ${LINK_LIBRARIES})

add_executable(build/test-05-jpeg test-05-jpeg.c framework.c)
target_link_libraries(build/test-05-jpeg libimagekit ${LINK_LIBRARIES})

add_executable(build/test-06-fill test-06-fill.c framework.c)
target_link_libraries(build/test-06-fill libimagekit ${LINK_LIBRARIES})

add_executable(build/test-07-histogram test-07-histogram.c framework.c)
target_link_libraries(build/test-07-histogram libimagekit ${LINK_LIBRARIES})

add_executable(build/test-08-blit test-08-blit.c framework.c)
target_link_libraries(build/test-08-blit libimagekit ${LINK_LIBRARIES})

add_executable(build/test-09-bezier test-09-bezier.c framework.c)
target_link_libraries(build/test-09-bezier libimagekit ${LINK_LIBRARIES})

