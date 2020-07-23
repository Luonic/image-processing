#include "Halide.h"
using namespace Halide;

// Support code for loading pngs.
#include "halide_image_io.h"
using namespace Halide::Tools;

#include <chrono>
#include <stdio.h>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

#include <scale_to_float.h>
#include <scale_from_float.h>
#include <clip_float.h>
#include <adaptive_contrast.h>
#include <coloring.h>
#include <show_mask.h>
#include <srgb_to_linearrgb.h>
#include "image_load_save.hpp"
#include "mask_producer.hpp"
#include "charstream.hpp"

void read_file_as_uchar_pointer(const char * filename, const unsigned char*& buffer, long& length) {
    void* typelessBuffer = nullptr;
    FILE* f = fopen(filename, "rb");

    if (f)
    {
        fseek (f, 0, SEEK_END);
        length = ftell(f);
        fseek (f, 0, SEEK_SET);
        typelessBuffer = malloc(length);
        if (typelessBuffer)
        {
            fread(typelessBuffer, 1, length, f);
        }
        fclose (f);
    }

    // if (buffer)
    // {
    // // start to process your data / extract strings here...
    // }

    buffer = reinterpret_cast<const unsigned char*>(typelessBuffer);
}

int main(int argc, char **argv) {

    // Halide::Runtime::Buffer<float> input = load_and_convert_image("/home/alex/Code/image-processing/images/image.jpg");

    const unsigned char* buffer = nullptr;
    long length;

    read_file_as_uchar_pointer("/home/alex/Code/image-processing/images/image4.jpg", buffer, length);
    

    Halide::Buffer<float> input = load_jpeg_from_bytes(buffer, length);

    auto mp = MaskProducer();
    const unsigned char* model_buf = nullptr;
    long model_lengh;

    read_file_as_uchar_pointer("/home/alex/Code/semi-supervised_semantic_segmentation/runs/34_hrnet-classic-transfuse-w48_BCE0.75-DICE0.25_harder-aug_finetune_dataset-5/model.ts", model_buf, model_lengh);

    auto model_stream = memstream(reinterpret_cast<const char*>(model_buf), model_lengh);

    bool is_initialized = mp.initialize_segmentation_model(model_stream);
    std::cout << is_initialized << std::endl;
    mp.predict(input);

    Halide::Buffer<float>& skin_mask = mp.skin_mask;
    Halide::Buffer<float>& background_mask = mp.background_mask;
    
    float skin_turnpoint = 0.2f;
    float background_turnpoint = 0.2f;

    float skin_strength = 1.5f;
    float background_strength = 1.5f;

    float skin_protect_whites = 1.0;
    float background_protect_whites = 1.0;

    float skin_protect_blacks = 1.0;
    float background_protect_blacks = 1.0;
    
    Halide::Buffer<float> output = Halide::Buffer<float>::make_with_shape_of(input);
    
    // apply_mask.realize(output);
    auto t1 = std::chrono::high_resolution_clock::now();

    input.set_host_dirty();

    float skin_coloring_strength = 0.75f;
    float skin_coloring_angle = 20;
    float skin_coloring_saturation = 0.3f; // 0.3

    float background_coloring_strength = 0.5f;
    float background_coloring_angle = 200;
    float background_coloring_saturation = 0.3f;

    coloring(input.raw_buffer(), skin_mask.raw_buffer(), background_mask.raw_buffer(),
             skin_coloring_angle, background_coloring_angle, 
             skin_coloring_saturation, background_coloring_saturation, 
             skin_coloring_strength, background_coloring_strength, 
             output.raw_buffer());

    
    adaptive_contrast(output.raw_buffer(), skin_mask.raw_buffer(), background_mask.raw_buffer(), 
        skin_turnpoint, background_turnpoint,
        skin_strength, background_strength, 
        skin_protect_whites, background_protect_whites, 
        skin_protect_blacks, background_protect_blacks, 
        output.raw_buffer());

    output.copy_to_host();
    clip_float(output.raw_buffer(), output.raw_buffer());
    // show_mask(skin_mask.raw_buffer(), output.raw_buffer());
    
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    std::cout << duration << std::endl;
    convert_and_save_image(output, "/home/alex/Code/image-processing/images/image_processed.jpg");
    printf("Success!\n");

    return 0;
}