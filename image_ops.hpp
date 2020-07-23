#include <iostream>

#include "Halide.h"
#include <scale_to_float.h>
#include <scale_from_float.h>
#include <clip_float.h>
#include <adaptive_contrast.h>
#include <srgb_to_linearrgb.h>

using namespace Halide;

class Operation {
    public:
        static unsigned int const OP_ID = 0;
        Operation() {};
        // virtual void set_params();
        virtual Runtime::Buffer<float> realize(Runtime::Buffer<float> input);
        virtual ~Operation();
    
    private:
        std::string name;
};

class AdaptiveContrastOp : public Operation
{
    public:
        static unsigned int const OP_ID = 1;
        AdaptiveContrastOp() {};
        void set_params(float turnpoint, float strength, float protect_whites, float protect_blacks)
        {
            _turnpoint = turnpoint;
            _strength = strength;
            _protect_whites = protect_whites;
            _protect_blacks = protect_blacks;
        };

        float get_turnpoint() { return _turnpoint; }
        float get_strength() { return _strength; }
        float get_protect_whites() { return _protect_whites; }
        float get_protect_blacks() { return _protect_blacks; }

        Runtime::Buffer<float> realize(Runtime::Buffer<float> input)
        {
            Halide::Runtime::Buffer<float> output = Halide::Runtime::Buffer<float>::make_with_shape_of(input);
            // adaptive_contrast(input, _turnpoint, _strength, _protect_whites, _protect_blacks, output);
            adaptive_contrast(output.raw_buffer(), skin_mask.raw_buffer(), background_mask.raw_buffer(), 
                              skin_turnpoint, background_turnpoint,
                              skin_strength, background_strength, 
                              skin_protect_whites, background_protect_whites, 
                              skin_protect_blacks, background_protect_blacks, 
                              output.raw_buffer());
            return output;
        };


    private:
        std::string name = "AdaptiveContrast";
        float _turnpoint = 0.5f;
        float _strength = 1.0f;
        float _protect_whites = 0.5f;
        float _protect_blacks = 0.5f;
};