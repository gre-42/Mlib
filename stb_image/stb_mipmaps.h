#include <memory>

void downsample_rgba_inplace(
    unsigned char* data,
    unsigned char* downsampled_data,
    int width,
    int height);

struct RgbaImage {
    unsigned char* data;
    int width;
    int height;
};

class RgbaDownsampler {
public:
    RgbaDownsampler(unsigned char* data, int width, int height);
    RgbaImage next();
private:
    unsigned char* data_;
    unsigned char* downsampled_data_;
    int width_;
    int height_;
    std::unique_ptr<unsigned char[]> buffer_;
};
