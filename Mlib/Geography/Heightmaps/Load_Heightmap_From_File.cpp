#include "Load_Heightmap_From_File.hpp"
#include <Mlib/Geography/Heightmaps/Cities_Skylines.hpp>
#include <Mlib/Geography/Heightmaps/Terrarium.hpp>
#include <Mlib/Images/Pgm_Image.hpp>
#include <stb_cpp/stb_array.hpp>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

template <class TData>
Array<TData> Mlib::load_heightmap_from_file(const std::string& filename) {
    if (filename.ends_with(".pgm")) {
        return cities_skylines_to_meters<TData>(PgmImage::load_from_file(filename));
    }
    auto imX = stb_load(filename, FlipMode::NONE);
    if (auto* im8 = std::get_if<StbInfo<uint8_t>>(&imX); im8 != nullptr) {
        auto ar = stb_image_2_array(*im8);
        if (ar.shape(0) == 3) {
            return terrarium_to_meters<TData>(ar);
        } else if (ar.shape(0) == 1) {
            return ar[0].template casted<TData>() / TData(255);
        } else {
            THROW_OR_ABORT("Height map is no 16-bit image and does not have 1 or 3 channels");
        }
    }
    if (auto* im16 = std::get_if<StbInfo<uint16_t>>(&imX); im16 != nullptr) {
        auto ar = stb_image_2_array(*im16);
        if (ar.shape(0) == 1) {
            return cities_skylines_to_meters<TData>(ar);
        } else {
            THROW_OR_ABORT("Height map is a 16-bit image and does not have exactly 1 channel");
        }
    }
    THROW_OR_ABORT("Image does not have 8 or 16 bit");
}

template Array<float> Mlib::load_heightmap_from_file<float>(const std::string&);
template Array<double> Mlib::load_heightmap_from_file<double>(const std::string&);
