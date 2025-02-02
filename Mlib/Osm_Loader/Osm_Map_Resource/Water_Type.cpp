#include "Water_Type.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

WaterType Mlib::water_type_from_string(const std::string& wt) {
    if (wt == "undefined") {
        return WaterType::UNDEFINED;
    } else if (wt == "hole") {
        return WaterType::HOLE;
    } else {
        THROW_OR_ABORT("Unknown water type");
    }
}

std::string Mlib::water_type_to_string(WaterType wt) {
    if (wt == WaterType::UNDEFINED) {
        return "undefined";
    } else if (wt == WaterType::HOLE) {
        return "hole";
    } else {
        THROW_OR_ABORT("Unknown water type");
    }
}

std::string Mlib::to_string(WaterType wt) {
    return water_type_to_string(wt);
}
