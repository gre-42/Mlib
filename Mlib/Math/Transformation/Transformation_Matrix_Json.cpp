#include "Transformation_Matrix_Json.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>

using namespace Mlib;

namespace KnownArgs {

BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(scale);

}

template <class TDir, class TPos, size_t n>
TransformationMatrix<TDir, TPos, n> Mlib::transformation_matrix_from_json(const nlohmann::json& j) {
    JsonView jv{ j };
    jv.validate(KnownArgs::options);
    if constexpr (n == 3) {
        auto R = tait_bryan_angles_2_matrix(jv.at<EFixedArray<TDir, 3>>(KnownArgs::rotation) * (TDir)degrees);
        auto scale = jv.at<TDir>(KnownArgs::scale);
        using I = funpack_t<TPos>;
        auto position = (jv.at<EFixedArray<I, 3>>(KnownArgs::position) * (I)meters).template casted<TPos>();
        return { R * scale, position };
    } else {
        static_assert(n == 3, "Unsupported matrix dimension");
    }
}

namespace Mlib {

template TransformationMatrix<float, double, 3> Mlib::transformation_matrix_from_json(const nlohmann::json& j);
template TransformationMatrix<float, CompressedScenePos, 3> Mlib::transformation_matrix_from_json(const nlohmann::json& j);

}
