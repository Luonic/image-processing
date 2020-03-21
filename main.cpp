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


    Buffer<uint8_t> input = load_image("/home/alex/Code/image-processing/images/image.jpg");

    Halide::Func adaptive_contrast("adaptive_contrast");
    Halide::Var x, y, c;
    // Halide::Var turnpoint, strength, protect_whites_amount, protect_shadows_amount;
    float turnpoint = 0.25f;
    float strength = 2.0f;

    float protect_whites = 1.0;
    float protect_blacks = 1.0;
    
    // For each pixel of the input image.
    Halide::Expr image = input(x, y, c);
    // Cast it to a floating point value.
    image = Halide::cast<float>(image);
    image = image / 255.0f;
    Expr centered_image = (image - turnpoint);
    Expr image_contrasted = (centered_image * strength + turnpoint);
    Expr whites_mask =  centered_image / (1 - turnpoint);
    Expr blacks_mask = centered_image / -turnpoint;
    Expr opacity = select(image > turnpoint, whites_mask * protect_whites, blacks_mask * protect_blacks);
    image_contrasted = image_contrasted * (1 - opacity) + image * opacity;
    image_contrasted = Halide::max(Halide::min(image_contrasted * 255, 255.0f), 0.0f);
    image_contrasted = Halide::cast<uint8_t>(image_contrasted);
    adaptive_contrast(x, y, c) = image_contrasted;
    Halide::Buffer<uint8_t> output = adaptive_contrast.realize(input.width(), input.height(), input.channels());
    save_image(output, "/home/alex/Code/image-processing/images/image_processed.jpg");

    printf("Success!\n");

    return 0;
}
