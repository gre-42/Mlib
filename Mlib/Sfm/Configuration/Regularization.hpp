#pragma once
#include <string>

namespace Mlib::Sfm {

enum class Regularization {
    DTAM,
    DENSE_GEOMETRY,
    DENSE_GEOMETRY_PYRAMID,
    FILTERING
};

Regularization regularization_from_string(const std::string& s);

}
