cmake_minimum_required(VERSION 3.13)
project(image-processing VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

include(CTest)
enable_testing()

add_subdirectory(generators)
# include(${CMAKE_CURRENT_SOURCE_DIR}/generators/CMakeLists.txt)

add_executable(image-processing main.cpp)

target_include_directories(image-processing PUBLIC ~/Code/Halide/include ~/Code/Halide/tools)

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
