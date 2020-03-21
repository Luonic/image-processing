#include "Halide.h"
#include <stdio.h>

using namespace Halide;

class SRGBToLinearRGB : public Halide::Generator<SRGBToLinearRGB> {
public:
    Input<Buffer<float>> input{"input", 3};

    Output<Buffer<float>> output{"output", 3};

    Var x, y, c;

    void generate() {
        Halide::Expr image = input(x, y, c);
        image = select(image <= 0.04045f,
                       image / 12.92f,
                       pow((image + 0.055f) / 1.055f, 2.4f));
        output(x, y, c) = image;
    }

    void schedule() {
        output.vectorize(x, natural_vector_size<float>()).parallel(y);
    }
};

HALIDE_REGISTER_GENERATOR(SRGBToLinearRGB, srgb_to_linearrgb)

class LinearRGBToSRGB : public Halide::Generator<LinearRGBToSRGB> {
public:
    Input<Buffer<float>> input{"input", 3};

    Output<Buffer<float>> output{"output", 3};

    Var x, y, c;

    void generate() {
        Halide::Expr image = input(x, y, c);
        image = select(image <= 0.0031308f,
                       image * 12.92f,
                       1.055f * pow(image, 1.0f / 2.4f) - 0.055f);
        output(x, y, c) = image;
    }

    void schedule() {
        output.vectorize(x, natural_vector_size<float>()).parallel(y);
    }
};

HALIDE_REGISTER_GENERATOR(LinearRGBToSRGB, linearrgb_to_srgb)