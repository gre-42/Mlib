#include "Invert.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Bvh.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation/Tait_Bryan_Angles.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(rotation);
}

void Mlib::invert(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    invert(
        args.arguments.at<EFixedArray<ScenePos, 3>>(KnownArgs::position, fixed_zeros<ScenePos, 3>()),
        args.arguments.at<EFixedArray<SceneDir, 3>>(KnownArgs::rotation, fixed_zeros<float, 3>()) * degrees);
}

void Mlib::invert(
    const FixedArray<ScenePos, 3>& position,
    const FixedArray<SceneDir, 3>& rotation)
{
    auto trafo = OffsetAndTaitBryanAngles<SceneDir, ScenePos, 3>{
        rotation, position};
    const auto& cfg = blender_bvh_config;
    auto m = trafo.to_matrix(cfg.rotation_order).affine();
    auto n = cfg.parameter_transformation.casted<ScenePos>();
    auto ttrafo = dot2d(n, dot2d(m, n.T()));
    auto imat = TransformationMatrix<SceneDir, ScenePos, 3>{ttrafo}.inverted();
    linfo() << "Position: " << imat.t;
    linfo() << "Rotation: " << matrix_2_tait_bryan_angles(imat.R) / degrees;
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "invert",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                invert(args);
            });
    }
} obj;

}
