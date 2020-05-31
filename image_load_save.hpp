#include "Halide.h"
#include "jpeglib.h"

namespace TypeConversions {
    inline void convert(uint8_t in, float &out) {out = in/255.0f;}
}

Halide::Buffer<float> load_jpeg_from_bytes(const unsigned char *image_buffer, unsigned long image_buffer_size)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    // jpeg_stdio_src(&cinfo, f.f);
    jpeg_mem_src(&cinfo, image_buffer, image_buffer_size);
    // jpeg_read_header(&cinfo, TRUE);
    int rc = jpeg_read_header(&cinfo, TRUE);

	if (rc != 1) {
		exit(1);
	}
    jpeg_start_decompress(&cinfo);

    Halide::Buffer<float> *im;
    int channels = cinfo.output_components;
    im = new Halide::Buffer<float>(cinfo.output_width, cinfo.output_height, 3);
    std::vector<JSAMPLE> row(im->width() * channels);

    for (int y = 0; y < im->height(); y++) {
        JSAMPLE *src = row.data();
        jpeg_read_scanlines(&cinfo, &src, 1);
        if (channels > 1) {
            for (int x = 0; x < im->width(); x++) {
                for (int c = 0; c < channels; c++) {
                    typename Halide::Buffer<float>::ElemType out;
                    TypeConversions::convert(*src++, out);
                    (*im)(x, y, c) = out;
                }
            }
        } else {
            for (int x = 0; x < im->width(); x++) {
                typename Halide::Buffer<float>::ElemType out;
                TypeConversions::convert(*src++, out);
                for (int c = 0; c < channels; c++) {
                    (*im)(x, y, c) = out;
                }
            }
        }
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return *im;
};

// bool save_as_jpeg_to_bytes(Halide::Buffer<float> &im, const std::string &filename) {
//     im.copy_to_host();

//     int channels = 3;
//     if ((im.dimensions() != 3) || (im.channels() != 3)) {
//         return false;
//     }

//     // TODO: Make this an argument?
//     const int quality = 99;

//     struct jpeg_compress_struct cinfo;
//     struct jpeg_error_mgr jerr;

//     Internal::FileOpener f(filename.c_str(), "wb");
//     if (!check(f.f != nullptr,
//                "File %s could not be opened for writing\n", filename.c_str())) {
//         return false;
//     }

//     cinfo.err = jpeg_std_error(&jerr);
//     jpeg_create_compress(&cinfo);
//     jpeg_stdio_dest(&cinfo, f.f);

//     cinfo.image_width = im.width();
//     cinfo.image_height = im.height();
//     cinfo.input_components = channels;
//     if (channels == 3) {
//         cinfo.in_color_space = JCS_RGB;
//     } else { // channels must be 1
//         cinfo.in_color_space = JCS_GRAYSCALE;
//     }

//     jpeg_set_defaults(&cinfo);
//     jpeg_set_quality(&cinfo, quality, TRUE);

//     jpeg_start_compress(&cinfo, TRUE);

//     std::vector<JSAMPLE> row(im.width() * channels);

//     for (int y = 0; y < im.height(); y++) {
//         JSAMPLE *dst = row.data();
//         if (im.dimensions() == 2) {
//             for (int x = 0; x < im.width(); x++) {
//                 Internal::convert(im(x, y), *dst++);
//             }
//         } else {
//             for (int x = 0; x < im.width(); x++) {
//                 for (int c = 0; c < channels; c++) {
//                     Internal::convert(im(x, y, c), *dst++);
//                 }
//             }
//         }
//         JSAMPROW row_ptr = row.data();
//         jpeg_write_scanlines(&cinfo, &row_ptr, 1);
//     }

//     jpeg_finish_compress(&cinfo);
//     jpeg_destroy_compress(&cinfo);

//     return true;
// };
