cmake_minimum_required(VERSION 3.13)
project(image-processing VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

include(CTest)
enable_testing()

set(HALIDE_DISTRIB_DIR "/home/alex/Code/Halide")
include("${HALIDE_DISTRIB_DIR}/halide_config.cmake")
include("${HALIDE_DISTRIB_DIR}/halide.cmake")

set(Torch_DIR ~/Code/libtorch/share/cmake/Torch/)
set(ARCH x86)
set(BITS 64)

# *** GENERATORS ***
# Adaptive contrast
halide_library(adaptive_contrast 
               SRCS adaptive_contrast.cpp 
               HALIDE_TARGET host)

# Teal-Orange split image coloring
halide_library(coloring 
               SRCS coloring.cpp 
               HALIDE_TARGET host)

# sRGB to Linear RGB
halide_library(srgb_to_linearrgb 
               SRCS sRGB_to_linearRGB.cpp
               HALIDE_TARGET host)

# Image range scaling
halide_library(scale_to_float SRCS normalization.cpp)
halide_library(scale_from_float SRCS normalization.cpp)
halide_library(clip_float SRCS normalization.cpp)
# *** 


# image processing library
add_library(image-processing SHARED image_processing.cpp)

target_include_directories(image-processing PUBLIC ~/Code/Halide/include ~/Code/Halide/tools)

find_library(HALIDE_LIBRARY Halide ~/Code/Halide/bin)
find_library(PTHREAD_LIBRARY pthread)
find_library(JPEG_LIBRARY jpeg ~/Code/libjpeg-turbo/build/)
find_library(PNG_LIBRARY png)
find_library(GLFW_LIBRARY glfw)
find_library(GLU_LIBRARY GLU)
find_library(GL_LIBRARY GL)
find_library(X11_LIBRARY X11)

find_package(Torch REQUIRED)

target_link_libraries(image-processing PUBLIC adaptive_contrast)
target_link_libraries(image-processing PUBLIC coloring)
target_link_libraries(image-processing PUBLIC srgb_to_linearrgb)
target_link_libraries(image-processing PUBLIC scale_to_float)
target_link_libraries(image-processing PUBLIC scale_from_float)
target_link_libraries(image-processing PUBLIC clip_float)
target_link_libraries(image-processing PUBLIC ${HALIDE_LIBRARY} ${PTHREAD_LIBRARY} ${JPEG_LIBRARY} ${PNG_LIBRARY} ${CMAKE_DL_LIBS})
target_link_libraries(image-processing PUBLIC ${TORCH_LIBRARIES})
target_link_libraries(image-processing PUBLIC ${GLFW_LIBRARY})
target_link_libraries(image-processing PUBLIC ${X11_LIBRARY})
target_link_libraries(image-processing PUBLIC ${GLU_LIBRARY})
target_link_libraries(image-processing PUBLIC ${GL_LIBRARY})


# demo library runner
add_executable(runner run.cpp)
target_link_libraries(runner PUBLIC image-processing adaptive_contrast coloring ${HALIDE_LIBRARY})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
