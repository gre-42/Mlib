#include <stb_image/stb_image.h>
#include <stb_image/stb_image_write.h>
#include <stb_image/stb_mipmaps.h>

void downsample_file(const char* in_filename, const char* out_prefix) {
    int width, height, channels;
    stbi_uc* data = stbi_load(
        in_filename,
        &width,
        &height,
        &channels,
        4);
    if (data == nullptr) {
        throw std::runtime_error("could not load " + std::string(in_filename));
    }
    if (channels != 4) {
        throw std::runtime_error("channels != 4");
    }
    RgbaDownsampler rds{data, width, height};
    for(RgbaImage im = rds.next(); im.data != nullptr; im = rds.next()) {
        if (!stbi_write_png(
            (out_prefix + std::to_string(im.width) + "x" + std::to_string(im.width) + ".png").c_str(),
            im.width,
            im.height,
            channels,
            im.data,
            0))
        {
            throw std::runtime_error("could not write image");
        }
    }
}

int main() {
    downsample_file("image.png", "image-");
    return 0;
}
