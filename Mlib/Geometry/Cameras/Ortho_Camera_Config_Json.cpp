#include "Ortho_Camera_Config_Json.hpp"
#include <Mlib/Geometry/Cameras/Ortho_Camera_Config.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Misc/Argument_List.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(near_plane);
DECLARE_ARGUMENT(far_plane);
DECLARE_ARGUMENT(left_plane);
DECLARE_ARGUMENT(right_plane);
DECLARE_ARGUMENT(bottom_plane);
DECLARE_ARGUMENT(top_plane);
}

void Mlib::from_json(const nlohmann::json& j, OrthoCameraConfig& config) {
    JsonView jv{j};
    jv.validate(KnownArgs::options);
    config.near_plane = jv.at<float>(KnownArgs::near_plane);
    config.far_plane = jv.at<float>(KnownArgs::far_plane);
    config.left_plane = jv.at<float>(KnownArgs::left_plane);
    config.right_plane = jv.at<float>(KnownArgs::right_plane);
    config.bottom_plane = jv.at<float>(KnownArgs::bottom_plane);
    config.top_plane = jv.at<float>(KnownArgs::top_plane);
}
