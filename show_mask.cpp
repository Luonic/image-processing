#include "Halide.h"
#include <stdio.h>

using namespace Halide;

class ShowMask : public Halide::Generator<ShowMask> {
public:
    Input<Buffer<float>> mask{"mask", 2};
    Output<Buffer<float>> output{"output", 3};

    Var x, y, c;

    void generate() {
        output(x, y, c) = mask(x, y);
    }

    void schedule() {
        output.vectorize(x, natural_vector_size<float>()).parallel(y);
    }
};

HALIDE_REGISTER_GENERATOR(ShowMask, show_mask)