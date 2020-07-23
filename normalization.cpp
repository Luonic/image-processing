#include "Halide.h"
#include <stdio.h>

using namespace Halide;

class ScaleToFloat : public Halide::Generator<ScaleToFloat> {
public:
    Input<Buffer<float>> input{"input", 3};
    Output<Buffer<float>> output{"output", 3};

    Var x, y, c;

    void generate() {
        Expr image = input(x, y, c);
        Expr image_scaled = image / 255;
        output(x, y, c) = image_scaled;
    }

    void schedule() {
        output.vectorize(x, natural_vector_size<float>()).parallel(y);
    }
};

HALIDE_REGISTER_GENERATOR(ScaleToFloat, scale_to_float)

class ScaleFromFloat : public Halide::Generator<ScaleFromFloat> {
public:
    Input<Buffer<float>> input{"input", 3};
    Output<Buffer<float>> output{"output", 3};

    Var x, y, c;

    void generate() {
        Expr image = input(x, y, c);
        Expr image_scaled = image * 255;
        image_scaled = min(image_scaled, 255);
        output(x, y, c) = image_scaled;
    }

    void schedule() {
        output.vectorize(x, natural_vector_size<float>()).parallel(y);
    }
};

HALIDE_REGISTER_GENERATOR(ScaleFromFloat, scale_from_float)

class ClipFloat : public Halide::Generator<ClipFloat> {
public:
    Input<Buffer<float>> input{"input", 3};
    Output<Buffer<float>> output{"output", 3};

    Var x, y, c;

    void generate() {
        Expr image = input(x, y, c);
        image = min(image, 1.0f);
        image = max(image, 0.0f);
        output(x, y, c) = image;
    }

    void schedule() {
        output.vectorize(x, natural_vector_size(output.type())).parallel(y);
    }
};

HALIDE_REGISTER_GENERATOR(ClipFloat, clip_float)

