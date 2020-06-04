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

    read_file_as_uchar_pointer("/home/alex/Code/image-processing/images/image3.jpg", buffer, length);
    

    Halide::Buffer<float> input = load_jpeg_from_bytes(buffer, length);

    auto mp = MaskProducer();
    const unsigned char* model_buf = nullptr;
    long model_lengh;

    read_file_as_uchar_pointer("/home/alex/Code/semi-supervised_semantic_segmentation/runs/34_hrnet-classic-transfuse-w48_BCE0.75-DICE0.25_harder-aug_finetune_dataset-5/model.ts", model_buf, model_lengh);

    auto model_stream = memstream(reinterpret_cast<const char*>(model_buf), model_lengh);
    // char* b = static_cast<char*>(malloc(1));
    // model_stream.read(b, 1);
    // std::cout << *b << std::endl;
    // model_stream.read(b, 1);
    // std::cout << *b << std::endl;
    // model_stream.read(b, 1);
    // std::cout << *b << std::endl;
    // model_stream.read(b, 1);
    // std::cout << *b << std::endl;
    // model_stream.read(b, 1);
    // std::cout << *b << std::endl;
    // model_stream.read(b, 1);
    // std::cout << *b << std::endl;

    // model_stream.seekg(0);
    // model_stream.read(b, 1);
    // std::cout << *b << std::endl;
    // model_stream.seekg(0);

    // uint8_t first_short[2];
    // model_stream.read(reinterpret_cast<char*>(&first_short), 2);
    // std::cout << (int(first_short[0]) == 0x80) << (first_short[1] == 0x02) <<std::endl;

    bool is_initialized = mp.initialize_segmentation_model(model_stream);
    std::cout << is_initialized << std::endl;
    // mp.initialize_segmentation_model("/home/alex/Code/semi-supervised_semantic_segmentation/runs/34_hrnet-classic-transfuse-w48_BCE0.75-DICE0.25_harder-aug_finetune_dataset-5/model.ts");
    mp.predict(input);

    Halide::Buffer<float> skin_mask = mp.get_mask(mp.SKIN_CLASS_ID);

    Halide::Func apply_mask;
    Halide::Var x, y, c;
    apply_mask(x, y, c) = input(x, y, c) * skin_mask(x, y);
    apply_mask.parallel(y);
    
    float turnpoint = 0.5f;
    float strength = 2.0f;

    float protect_whites = 1.0;
    float protect_blacks = 1.0;
    
    Halide::Buffer<float> output = Halide::Buffer<float>::make_with_shape_of(input);
    
    // apply_mask.realize(output);
    auto t1 = std::chrono::high_resolution_clock::now();

    // apply_mask.realize(input);
    input.set_host_dirty();

    float skin_coloring_strength = 0.5f;
    float skin_coloring_angle = 20;
    float skin_coloring_saturation = 0.25f; // 0.3

    float background_coloring_strength = 0.5f;
    float background_coloring_angle = 200;
    float background_coloring_saturation = 0.25f;

    coloring(input.raw_buffer(), skin_mask.raw_buffer(), 
             skin_coloring_angle, background_coloring_angle, 
             skin_coloring_saturation, background_coloring_saturation, 
             skin_coloring_strength, background_coloring_strength, 
             output.raw_buffer());

    // coloring(input.raw_buffer(), skin_mask.raw_buffer(), skin_color.raw_buffer(), background_color.raw_buffer(), skin_coloring_strength, background_coloring_strength, output.raw_buffer());
    // adaptive_contrast(output.raw_buffer(), turnpoint, strength, protect_whites, protect_blacks, output.raw_buffer());
    output.copy_to_host();
    clip_float(output.raw_buffer(), output.raw_buffer());
    
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    std::cout << duration << std::endl;
    convert_and_save_image(output, "/home/alex/Code/image-processing/images/image_processed.jpg");
    printf("Success!\n");

    return 0;
}