#include "Halide.h"
#include <stdio.h>

using namespace Halide;

Var x("x"), y("y"), c("c");
Var x_outer("x_outer"), x_vectors("x_vectors");

Func adaptive_contrast(Func input, Func mask, Expr turnpoint, Expr strength, Expr protect_whites, Expr protect_blacks)
{
    Expr centered_image = (input(x, y, c) - turnpoint);
    Func image_contrasted;
    image_contrasted(x, y, c) = centered_image * strength + turnpoint;
    Expr whites_mask =  centered_image / (1 - turnpoint);
    Expr blacks_mask = centered_image / -turnpoint;
    Expr opacity = select(input(x, y, c) > turnpoint, whites_mask * protect_whites, blacks_mask * protect_blacks);
    image_contrasted(x, y, c) = image_contrasted(x, y, c) * (1 - opacity) + input(x, y, c) * opacity;
    Func output;
    output(x, y, c) = image_contrasted(x, y, c) * mask(x, y) + input(x, y, c) * (1.0f - mask(x, y));
    return output;
}

class AdaptiveContrast : public Halide::Generator<AdaptiveContrast> {
public:
    Input<Buffer<float>> input{"input", 3};

    Input<Buffer<float>> skin_mask{"skin_mask", 2};
    Input<Buffer<float>> background_mask{"back_mask", 2};

    Input<float> skin_turnpoint{"skin_turnpoint"};
    Input<float> background_turnpoint{"background_turnpoint"};

    Input<float> skin_strength{"skin_strength"};
    Input<float> background_strength{"background_strength"};

    Input<float> skin_protect_whites{"skin_protect_whites"};
    Input<float> background_protect_whites{"background_protect_whites"};

    Input<float> skin_protect_blacks{"skin_protect_blacks"};
    Input<float> background_protect_blacks{"background_protect_blacks"};

    Output<Buffer<float>> output{"output", 3};

    

    void generate() {
        Func intermediate("intermediate");
        intermediate(x, y, c) = adaptive_contrast(input, skin_mask, 
            Expr(skin_turnpoint), Expr(skin_strength), Expr(skin_protect_whites), Expr(skin_protect_blacks))(x, y, c);
        output(x, y, c) = adaptive_contrast(intermediate, background_mask,
            Expr(background_turnpoint), Expr(background_strength), Expr(background_protect_whites), Expr(background_protect_blacks))(x, y, c);
        
        input.dim(2).set_bounds(0, 3);
        output.bound(c, 0, 3);

        if (auto_schedule) {
            int max_height = 1536;
            int max_width = 2560;
            input.dim(0).set_estimate(0, max_height);
            input.dim(1).set_estimate(0, max_width);
            skin_mask.dim(0).set_estimate(0, max_height);
            skin_mask.dim(1).set_estimate(0, max_width);
            background_mask.dim(0).set_estimate(0, max_height);
            background_mask.dim(1).set_estimate(0, max_width);
            output.dim(0).set_estimate(0, max_height);
            output.dim(1).set_estimate(0, max_width);
        } else {
            intermediate
            .compute_at(output, x)
            .reorder(c, x, y)
            .unroll(c)
            // .split(x, x_outer, x_vectors, natural_vector_size(output.type()))
            .vectorize(x, natural_vector_size(output.type()))
            .parallel(y);

            output
            .compute_root()
            .reorder(c, x, y)
            .unroll(c)
            .vectorize(x, natural_vector_size(output.type()))
            .parallel(y);  
        } 
    }

    void schedule() {
        
    }
};

HALIDE_REGISTER_GENERATOR(AdaptiveContrast, adaptive_contrast)
