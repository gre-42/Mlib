#include "Regularization.hpp"
#include <stdexcept>

using namespace Mlib::Sfm;

Regularization Mlib::Sfm::regularization_from_string(const std::string& s) {
    if (s == "dtam") {
        return Regularization::DTAM;
    } else if (s == "dense_geometry") {
        return Regularization::DENSE_GEOMETRY;
    } else if (s == "dense_geometry_pyramid") {
        return Regularization::DENSE_GEOMETRY_PYRAMID;
    } else if (s == "filter") {
        return Regularization::FILTERING;
    } else {
        throw std::runtime_error("Unknown regularization mode: \"" + s + '"');
    }
}
