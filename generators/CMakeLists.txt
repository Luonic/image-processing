set(HALIDE_DISTRIB_DIR "/home/alex/Code/Halide")
include("${HALIDE_DISTRIB_DIR}/halide_config.cmake")
include("${HALIDE_DISTRIB_DIR}/halide.cmake")

# Adaptive contrast
halide_library(adaptive_contrast SRCS adaptive_contrast.cpp)

# sRGB to Linear RGB
halide_library(srgb_to_linearrgb SRCS sRGB_to_linearRGB.cpp)