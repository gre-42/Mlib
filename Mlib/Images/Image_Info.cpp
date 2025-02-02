#include "Image_Info.hpp"
#include <Mlib/Images/Dds_Info.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

ImageInfo ImageInfo::load(const std::string& filename, const std::vector<std::byte>* data)
{
    ImageInfo result{
        .size = uninitialized
    };
    auto extension = std::filesystem::path{filename}.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
        ::tolower);
    if ((extension == ".jpg") ||
        (extension == ".png") ||
        (extension == ".bmp"))
    {
        int width;
        int height;
        int comp;
        if (data == nullptr) {
            if (stbi_info(filename.c_str(), &width, &height, &comp) == 0) {
                THROW_OR_ABORT("Could not load image info from file \"" + filename + '"');
            }
        } else {
            if (stbi_info_from_memory((uint8_t*)data->data(), integral_cast<int>(data->size()), &width, &height, &comp) == 0) {
                THROW_OR_ABORT("Could not load image info from file \"" + filename + '"');
            }
        }
        result.size = {
            integral_cast<size_t>(width),
            integral_cast<size_t>(height)};
    } else if (extension == ".dds") {
        // std::stringstream sstr;
        // for (uint8_t c : data) {
        //     sstr << c;
        // }
        // nv_dds::CDDSImage image;
        // image.load(sstr);
        // matInfo << ' ' << image.get_width() << 'x' << image.get_height() << " compressed: " << (int)image.is_compressed() << " format: " << image.get_format();
        DdsInfo info;
        if (data == nullptr) {
            info = DdsInfo::load_from_file(filename);
        } else {
            try {
                info = DdsInfo::load_from_buffer(*data);
            } catch (const std::exception& e) {
                throw std::runtime_error("Could not load \"" + filename + "\": " + e.what());
            }
        }
        result.size = {
            integral_cast<size_t>(info.width),
            integral_cast<size_t>(info.height)};
    } else {
        THROW_OR_ABORT("Unknown texture file extension: \"" + filename + '"');
    }
    return result;
}
