#include "stdlib.h"
#include "image_ops.hpp"
#include <Halide.h>
#include "image_load_save.hpp"
#include "mask_producer.hpp"

class ImageProcessing {
public:
    ImageProcessing();
    Halide::Runtime::Buffer<float> realize()
    {
        auto input = source_image;
        for (unsigned long i = 0; i < ops.size(); i++)
        {
            auto output = ops[i].realize(input);
            input = output;
        };
        return input;
    };

private:
    Halide::Runtime::Buffer<float> source_image;
    std::vector<Operation> ops;
};

