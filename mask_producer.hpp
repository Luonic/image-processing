#include <torch/script.h>

#include <iostream>
#include <memory>

#include "Halide.h"

class MaskProducer
{
public:
    int BACKGROUND_CLASS_ID = 0;
    int SKIN_CLASS_ID = 1;

    bool initialize_segmentation_model(std::istream& in)
    {
        try {
            // Deserialize the ScriptModule from a file using torch::jit::load().
            seg_module = torch::jit::load(in);
            return true;
        }
        catch (const c10::Error& e) {
            return false;
        }
    };

    bool initialize_segmentation_model(const std::string& filename)
    {
        try {
            // Deserialize the ScriptModule from a file using torch::jit::load().
            seg_module = torch::jit::load(filename);
            return true;
        }
        catch (const c10::Error& e) {
            return false;
        }
    };

    void predict(Halide::Buffer<float> &image)
    {
        // Create a vector of inputs.
        std::vector<torch::jit::IValue> inputs;
        at::Tensor image_tensor = torch::from_blob(
            image.data(), {image.channels(), image.height(), image.width()}, torch::kFloat).clone();
        image_tensor = image_tensor.permute({0, 2, 1});
        inputs.push_back(image_tensor);

        // Execute the model and turn its output into a tensor.
        torch::NoGradGuard no_grad;
        torch::jit::IValue output = seg_module.forward(inputs);
        auto output_tuple = output.toTuple();
        auto binary_mask = output_tuple->elements()[0].toTensor();
        auto prob_mask = output_tuple->elements()[1].toTensor();
        masks = prob_mask.permute({0, 2, 1});
    };

    Halide::Buffer<float> get_mask(int class_id)
    {
        at::Tensor maskSlice = masks.slice(0, class_id, class_id + 1, 1).contiguous();
        auto w = static_cast<int32_t>(maskSlice.size(2));
        auto h = static_cast<int32_t>(maskSlice.size(1));
        auto c = static_cast<int32_t>(maskSlice.size(0));
        halide_dimension_t bufShape[] = {
            {0, w, 1},
            {0, h, w},
            /*{0, c, w * h}*/
        };
        auto halideMask = Halide::Buffer<float>(
            static_cast<float*>(maskSlice.data_ptr()), 
            /* num of dimensions */2, 
            bufShape);
        return halideMask;
    };

private:
    torch::jit::script::Module seg_module;
    at::Tensor masks;
};