#include "Water_Type.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

WaterType Mlib::water_type_from_string(const std::string& wt) {
    if (wt == "undefined") {
        return WaterType::UNDEFINED;
    } else if (wt == "steep_hole") {
        return WaterType::STEEP_HOLE;
    } else if (wt == "shallow_hole") {
        return WaterType::SHALLOW_HOLE;
    } else if (wt == "shallow_lake") {
        return WaterType::SHALLOW_LAKE;
    } else {
        THROW_OR_ABORT("Unknown water type");
    }
}

std::string Mlib::water_type_to_string(WaterType wt) {
    if (wt == WaterType::UNDEFINED) {
        return "undefined";
    } else if (wt == WaterType::STEEP_HOLE) {
        return "steep_hole";
    } else if (wt == WaterType::SHALLOW_HOLE) {
        return "shallow_hole";
    } else if (wt == WaterType::SHALLOW_LAKE) {
        return "shallow_lake";
    } else {
        THROW_OR_ABORT("Unknown water type");
    }
}

std::string Mlib::to_string(WaterType wt) {
    return water_type_to_string(wt);
}
