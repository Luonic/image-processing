cmake_minimum_required(VERSION 3.16)
project(image-processing VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

find_library(PTHREAD_LIBRARY pthread)
find_library(JPEG_LIBRARY jpeg ~/Code/libjpeg-turbo/build/)
find_library(PNG_LIBRARY png)
find_library(GLFW_LIBRARY glfw)
find_library(GLU_LIBRARY GLU)
find_library(GL_LIBRARY GL)
find_library(X11_LIBRARY X11)

find_package(Torch REQUIRED PATHS ~/Code/libtorch/share/cmake/Torch/)
find_package(Halide REQUIRED PATHS ~/Code/Halide/build/Halide_install/lib/cmake/Halide/)


# *** GENERATORS ***
# Adaptive contrast generator
add_executable(adaptive_contrast.generator adaptive_contrast.cpp)
target_link_libraries(adaptive_contrast.generator PRIVATE Halide::Generator)
add_halide_library(adaptive_contrast FROM adaptive_contrast.generator)

# Coloring (split toning)
add_executable(coloring.generator coloring.cpp)
target_link_libraries(coloring.generator PRIVATE Halide::Generator)
add_halide_library(coloring FROM coloring.generator)

# Scaling to float from uint8
add_executable(scale_to_float.generator normalization.cpp)
target_link_libraries(scale_to_float.generator PRIVATE Halide::Generator)
add_halide_library(scale_to_float FROM scale_to_float.generator)

# Scaling from float to uint8
add_executable(scale_from_float.generator normalization.cpp)
target_link_libraries(scale_from_float.generator PRIVATE Halide::Generator)
add_halide_library(scale_from_float FROM scale_from_float.generator)

# Clip float to [0.0, 1.0]
add_executable(clip_float.generator normalization.cpp)
target_link_libraries(clip_float.generator PRIVATE Halide::Generator)
add_halide_library(clip_float FROM clip_float.generator)

# Project 2d mask to 3d image
add_executable(show_mask.generator show_mask.cpp)
target_link_libraries(show_mask.generator PRIVATE Halide::Generator)
add_halide_library(show_mask FROM show_mask.generator)

# sRGB to Linear RGB colorspace conversion
add_executable(srgb_to_linearrgb.generator sRGB_to_linearRGB.cpp)
target_link_libraries(srgb_to_linearrgb.generator PRIVATE Halide::Generator)
add_halide_library(srgb_to_linearrgb FROM srgb_to_linearrgb.generator)
# *** 

# Image processing library with C interface
add_library(image-processing SHARED image_processing.cpp)

# Linking torch lib
target_link_libraries(image-processing PUBLIC ${TORCH_LIBRARIES})

# Linking Halide-related things
target_link_libraries(image-processing
                      PUBLIC
                      Halide::Halide 
                      Halide::Tools 
                      Halide::ImageIO
                      adaptive_contrast
                      coloring
                      scale_to_float
                      scale_from_float
                      clip_float
                      show_mask
                      srgb_to_linearrgb)

# Linking other libs
target_link_libraries(image-processing PUBLIC 
                      ${PTHREAD_LIBRARY} 
                      ${JPEG_LIBRARY} 
                      ${PNG_LIBRARY} 
                      ${CMAKE_DL_LIBS})

# Linking GLFW to let offload computation to OpenGL Compute
target_link_libraries(image-processing PUBLIC 
                      ${GLFW_LIBRARY})

# demo library runner
add_executable(runner run.cpp)
target_link_libraries(runner PUBLIC  
                      adaptive_contrast 
                      coloring 
                      show_mask)

add_executable(frontend frontend.cpp)
target_link_libraries(frontend PRIVATE image-processing)
