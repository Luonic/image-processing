// g++ lesson_01*.cpp -g -I ../include -L ../bin -lHalide -lpthread -ldl -o lesson_01 -std=c++11
// LD_LIBRARY_PATH=../bin ./lesson_01

#include "Halide.h"
using namespace Halide;

// Support code for loading pngs.
#include "halide_image_io.h"
using namespace Halide::Tools;

#include <stdio.h>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

#include <scale_to_float.h>
#include <scale_from_float.h>
#include <clip_float.h>
#include <adaptive_contrast.h>
#include <srgb_to_linearrgb.h>

int main(int argc, char **argv) {

    // std::cout << fs::current_path();

    // Halide::Buffer<uint8_t> input = load_image("images/rgb.png");
    // Halide::Func brighter;
    // Halide::Var x, y, c;
    // Halide::Expr value = input(x, y, c);
    // value = Halide::cast<float>(value);
    // value = value * 2.0f;
    // value = Halide::min(value, 255.0f);
    // value = Halide::cast<uint8_t>(value);
    // brighter(x, y, c) = value;


    Halide::Runtime::Buffer<float> input = load_and_convert_image("/home/alex/Code/image-processing/images/image.jpg");
    

    // Halide::Func adaptive_contrast("adaptive_contrast");
    // Halide::Var x, y, c;
    // Halide::Var turnpoint, strength, protect_whites_amount, protect_shadows_amount;
    float turnpoint = 0.25f;
    float strength = 3.0f;

    float protect_whites = 1.0;
    float protect_blacks = 1.0;
    
    // For each pixel of the input image.
    // Halide::Expr image = input(x, y, c);
    // Cast it to a floating point value.
    // image = Halide::cast<float>(image);
    // image = image / 255.0f;
    Halide::Runtime::Buffer<float> output = Halide::Runtime::Buffer<float>::make_with_shape_of(input);
    adaptive_contrast(input, turnpoint, strength, protect_whites, protect_blacks, output);
    clip_float(output, output);
    convert_and_save_image(output, "/home/alex/Code/image-processing/images/image_processed.jpg");
    printf("Success!\n");

    return 0;
}
