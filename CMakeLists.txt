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

# *** GENERATORS ***
# Adaptive contrast
halide_library(adaptive_contrast SRCS adaptive_contrast.cpp)

# sRGB to Linear RGB
halide_library(srgb_to_linearrgb SRCS sRGB_to_linearRGB.cpp)
# *** 

add_executable(image-processing main.cpp)

target_include_directories(image-processing PUBLIC ~/Code/Halide/include ~/Code/Halide/tools)
# target_include_directories(image-processing PUBLIC /home/alex/Code/image-processing/build/genfiles/adaptive_contrast/adaptive_contrast.h)

find_library(HALIDE_LIBRARY Halide ~/Code/Halide/bin)
find_library(PTHREAD_LIBRARY pthread)
find_library(JPEG_LIBRARY jpeg)
find_library(PNG_LIBRARY png)

target_link_libraries(image-processing PUBLIC adaptive_contrast)
target_link_libraries(image-processing PUBLIC srgb_to_linearrgb)
target_link_libraries(image-processing PUBLIC ${HALIDE_LIBRARY} ${PTHREAD_LIBRARY} ${JPEG_LIBRARY} ${PNG_LIBRARY} ${CMAKE_DL_LIBS})


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
