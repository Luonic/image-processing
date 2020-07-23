#include "Halide.h"
#include <stdio.h>

using namespace Halide;

Var x, y, c;

Func rgb_to_hsl(Func input)
{
    Func r("r"), g("g"), b("b");
    r(x, y) = input(x, y, 0);
    g(x, y) = input(x, y, 1);
    b(x, y) = input(x, y, 2);

    Func max_val("max_val"), min_val("min_val");
    max_val(x, y) = max(max(r(x, y), g(x, y)), b(x, y));
    min_val(x, y) = min(min(r(x, y), g(x, y)), b(x, y));

    Func lightness("lightness");
    lightness(x, y) = (max_val(x, y) + min_val(x, y)) / 2;

    // MAX - MIN 
    Func l_diff("l_diff");
    l_diff(x, y) = max_val(x, y) - min_val(x, y);

    Func hue("hue");
    hue(x, y) = select(max_val(x, y) == min_val(x, y), 0.0f,
                       max_val(x, y) == r(x, y) & g(x, y) >= b(x, y), (g(x, y) - b(x, y)) / l_diff(x, y) * 60.0f + 0.0f,
                       max_val(x, y) == r(x, y) & g(x, y) < b(x, y), (g(x, y) - b(x, y)) / l_diff(x, y) * 60 + 360,
                       max_val(x, y) == g(x, y), (b(x, y) - r(x, y)) / l_diff(x, y) * 60.0f + 120.0f,
                       max_val(x, y) == b(x, y), (r(x, y) - g(x, y)) / l_diff(x, y) * 60.0f + 240.0f,
                       0.0f);

    Func saturation("saturation");
    saturation(x, y) = select(0 < lightness(x, y) && lightness(x, y) <= 0.5f, l_diff(x, y) / (2 * lightness(x, y)),
                              0.5f < lightness(x, y) && lightness(x, y) < 1.0f, l_diff(x, y) / (2 - 2 * lightness(x, y)),
                              0.0f);

    Func hsl("hsl");
    hsl(x, y, c) = select(c == 0, hue(x, y),
                          c == 1, saturation(x, y),
                          lightness(x, y));
    return hsl;
}

Func hsl_to_rgb(Func input)
{
    Func hue("hue"), saturation("saturation"), lightness("lightness");
    hue(x, y) = input(x, y, 0);
    saturation(x, y) = input(x, y, 1);
    lightness(x, y) = input(x, y, 2);

    Func q("q"), p("p");
    q(x, y) = select(lightness(x, y) < 0.5f, lightness(x, y) * (1.0f + saturation(x, y)),
                     lightness(x, y) + saturation(x, y) - (lightness(x, y) * saturation(x, y)));
    p(x, y) = 2.0f * lightness(x, y) - q(x, y);
    
    Func h_k("h_k"), t_r("t_r"), t_g("t_g"), t_b("t_b"), t_c("t_c");
    h_k(x, y) = hue(x, y) / 360.0f;
    
    t_r(x, y) = h_k(x, y) + 1.0f / 3.0f;
    t_g = h_k;
    t_b(x, y) = h_k(x, y) - 1.0f / 3.0f;

    t_c(x, y, c) = select(c == 0, t_r(x, y),
                          c == 1, t_g(x, y),
                          t_b(x, y));
    t_c(x, y, c) = select(t_c(x, y, c) < 0.0f, t_c(x, y, c) + 1.0f,
                          t_c(x, y, c) > 1.0f, t_c(x, y, c) - 1.0f,
                          t_c(x, y, c));
    
    Func rgb("rgb");
    rgb(x, y, c) = select(t_c(x, y, c) < 1.0f / 6.0f, p(x, y) + ((q(x, y) - p(x, y)) * 6.0f * t_c(x, y, c)),
                          1.0f / 6.0f <= t_c(x, y, c) && t_c(x, y, c) < 1.0f / 2.0f, q(x, y),
                          1.0f / 2.0f <= t_c(x, y, c) && t_c(x, y, c) < 2.0f / 3.0f, p(x, y) + ((q(x, y) - p(x, y)) * (2.0f / 3.0f - t_c(x, y, c)) * 6.0f),
                          p(x, y));
    // rgb(x, y, c) = h_k(x, y);
    return rgb;
}

Func blend_mode_color(Func bottom, Func top, Func opacity_mask)
{
    Func b_hsl("b_hsl"), t_hsl("t_hsl"), blended_hue("blended_hue"), blended("blended"), rgb("rgb");
    b_hsl = rgb_to_hsl(bottom);
    t_hsl = rgb_to_hsl(top);
    blended(x, y, c) = select(c == 0, (b_hsl(x, y, c) + t_hsl(x, y, c) * opacity_mask(x, y)),
                              c == 1, t_hsl(x, y, c) * opacity_mask(x, y) + b_hsl(x, y, c) * (1.0f - opacity_mask(x, y)), 
                              b_hsl(x, y, c));
    rgb = hsl_to_rgb(blended);
    return rgb;
}

Func blend_mode_color_rgb_with_hsl(Func bottom, Func top, Func opacity_mask)
{
    Func b_hsl("b_hsl"), t_hsl("t_hsl"), t_sat("t_sat"), blended_hue("blended_hue"), blended("blended"), rgb("rgb");
    b_hsl = rgb_to_hsl(bottom);
    t_sat(x, y) = top(x, y, 1);
    b_hsl(x, y, 1) = b_hsl(x, y, 1) * (1 - opacity_mask(x, y)) + t_sat(x, y) * opacity_mask(x, y);
    t_hsl(x, y, c) = top(x, y, c);
    t_hsl(x, y, 2) = b_hsl(x, y, 2);
    // blended(x, y, c) = select(c == 0, (b_hsl(x, y, c) + t_hsl(x, y, c) * opacity_mask(x, y)),
    //                           c == 1, t_hsl(x, y, c) * opacity_mask(x, y) + b_hsl(x, y, c) * (1.0f - opacity_mask(x, y)), 
    //                           b_hsl(x, y, c));
    rgb(x, y, c) = bottom(x, y, c) * (1.0f - opacity_mask(x, y)) + hsl_to_rgb(t_hsl)(x, y, c) * opacity_mask(x, y);
    return rgb;
}

class Coloring : public Halide::Generator<Coloring> {
public:
    Input<Buffer<float>> input{"input", 3};
    Input<Buffer<float>> skin_mask{"skin_mask", 2};
    Input<Buffer<float>> background_mask{"background_mask", 2};

    // HSL's H angle of new color
    Input<float> skin_color_angle{"skin_color_angle"};
    Input<float> background_color_angle{"background_color_angle"};

    // HSL's S saturation value for new colors
    Input<float> skin_color_saturation{"skin_color_saturation"};
    Input<float> background_color_saturation{"background_color_saturation"};

    Input<float> skin_output_strength{"skin_output_strength"};
    Input<float> background_output_strength{"background_output_strength"};

    Output<Buffer<float>> output{"output", 3};

    void generate() {
        Func gray("gray");
        gray(x, y) = 0.299f * input(x, y, 0) + 0.587f * input(x, y, 1) + 0.114f * input(x, y, 2);
        
        Func gradient_mask("gradient_mask"), skin_colorization_mask("skin_colorization_mask"), background_colorization_mask("background_colorization_mask");
        gradient_mask(x, y) = 1.0f - abs(gray(x, y) - 0.5f) * 2;
        skin_colorization_mask(x, y) = gradient_mask(x, y) * skin_mask(x, y) * skin_output_strength;
        background_colorization_mask(x, y) = gradient_mask(x, y) * background_mask(x, y) * background_output_strength;

        Func skin_color_3d("skin_color_3d"), background_color_3d("background_color_3d");
        skin_color_3d(x, y, c) = select(c == 0, skin_color_angle,
                                        c == 1, skin_color_saturation,
                                        0.5f);
        background_color_3d(x, y, c) = select(c == 0, background_color_angle,
                                              c == 1, background_color_saturation,
                                              0.5f);

        Func intermediate("intermediate");
        intermediate(x, y, c) = blend_mode_color_rgb_with_hsl(input, skin_color_3d, skin_colorization_mask)(x, y, c);
        output(x, y, c) = blend_mode_color_rgb_with_hsl(intermediate, background_color_3d, background_colorization_mask)(x, y, c);
        


        input.dim(2).set_bounds(0, 3);   // specify color range for input
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
        } else if (get_target().has_feature(Target::OpenGL)) {    
            output.glsl(x, y, c);
        } else if (get_target().has_feature(Target::OpenGLCompute)) {
            // Var xt, yt;
            output
            .reorder_storage(c, x, y)
            .reorder(c, x, y)
            .unroll(c)
            .parallel(y);
        } else {
            // Var x_outer("x_outer"), x_inner("x_inner");
            // // output.vectorize(x, natural_vector_size<float>()).parallel(y);
            // output
            // .reorder_storage(c, x, y)
            // .reorder(c, x, y)
            // .unroll(c)
            // .parallel(y);
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

HALIDE_REGISTER_GENERATOR(Coloring, coloring)
