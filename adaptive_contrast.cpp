#include "Halide.h"
#include <stdio.h>

using namespace Halide;

class AdaptiveContrast : public Halide::Generator<AdaptiveContrast> {
public:
    Input<Buffer<float>> input{"input", 3};
    Input<float> turnpoint{"turnpoint"};
    Input<float> strength{"strength"};
    Input<float> protect_whites{"protect_whites"};
    Input<float> protect_blacks{"protect_blacks"};

    Output<Buffer<float>> adaptive_contrast{"adaptive_contrast", 3};

    Var x, y, c;

    void generate() {
        Halide::Expr image = input(x, y, c);
        Expr centered_image = (image - turnpoint);
        Expr image_contrasted = (centered_image * strength + turnpoint);
        Expr whites_mask =  centered_image / (1 - turnpoint);
        Expr blacks_mask = centered_image / -turnpoint;
        Expr opacity = select(image > turnpoint, whites_mask * protect_whites, blacks_mask * protect_blacks);
        image_contrasted = image_contrasted * (1 - opacity) + image * opacity;
        adaptive_contrast(x, y, c) = image_contrasted;
        schedule();
    }

    void schedule() {
        if (get_target().has_feature(Target::OpenGL)) {
            input.dim(2).set_bounds(0, 3);   // specify color range for input
            adaptive_contrast.bound(c, 0, 3);
            adaptive_contrast.glsl(x, y, c);
        } else {
            // adaptive_contrast.vectorize(x, natural_vector_size<float>()).parallel(y);
            adaptive_contrast.parallel(y);
        }
    }
};

HALIDE_REGISTER_GENERATOR(AdaptiveContrast, adaptive_contrast)
