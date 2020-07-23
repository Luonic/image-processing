#include <stdint.h>
#include "stdlib.h"
#include "image_ops.hpp"
#include <Halide.h>
#include "image_load_save.hpp"
#include "mask_producer.hpp"

class ImageProcessing {
public:
    ImageProcessing() {
        ops = std::vector<Operation*>();
        mask_producer = MaskProducer()
        mask_producer.initialize_segmentation_model();
    };

    Operation* add_op(unsigned int op_id)
    {
        Operation* op_ptr = nullptr;

        switch (op_id)
        {
        case AdaptiveContrastOp::OP_ID:
            {
                op_ptr = new AdaptiveContrastOp();
            }
            break;
        
        default:
            break;
        }
        ops.push_back(op_ptr);
        return op_ptr;
    }

    void delete_op(Operation* op_ptr)
    {
        ops.erase(std::remove(ops.begin(), ops.end(), op_ptr), ops.end()); 
        // op_ptr->~Operation();
    }

    Halide::Buffer<float> realize()
    {
        auto input = source_image;
        for (unsigned long i = 0; i < ops.size(); i++)
        {
            // auto output = ops[i]->realize(input);
            // input = output;
        };
        return input;
    };

private:
    Halide::Runtime::Buffer<float> source_image;
    std::vector<Operation*> ops;
    MaskProducer mask_producer;
};

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void* create_instance()
{
    return new ImageProcessing();
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void* add_op(void* img_proc_ptr, unsigned int op_id)
{
    return static_cast<ImageProcessing*>(img_proc_ptr)->add_op(op_id);
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void set_op_params(void* op_ptr, void* param_struct_ptr)
{

}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void delete_op(void* img_proc_ptr, void* op_ptr)
{
    static_cast<ImageProcessing*>(img_proc_ptr)->delete_op(static_cast<Operation*>(op_ptr));
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void realize(void* img_proc_ptr, void* result_buf_ptr, float max_dim_size, int width, int height)
{
    static_cast<ImageProcessing*>(img_proc_ptr)->realize();
}