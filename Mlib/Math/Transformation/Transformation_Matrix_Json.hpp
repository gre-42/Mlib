#pragma once
#include <nlohmann/json_fwd.hpp>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

template <class TDir, class TPos, size_t n>
TransformationMatrix<TDir, TPos, n> transformation_matrix_from_json(const nlohmann::json& j);

}
