#pragma once
#include <string>

namespace Mlib::Sfm {

enum class Regularization {
    DTAM,
    DENSE_GEOMETRY,
    FILTERING
};

Regularization regularization_from_string(const std::string& s);

}
