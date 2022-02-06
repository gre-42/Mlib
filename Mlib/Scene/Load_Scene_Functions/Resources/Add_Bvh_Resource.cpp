#include "Add_Bvh_Resource.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Geometry/Mesh/Load_Bvh.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Resources/Bvh_File_Resource.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

LoadSceneUserFunction AddBvhResource::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*add_bvh_resource"
        "\\s+name=([\\w+-.]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+smooth_radius=([\\w+-.]+)"
        "\\s+smooth_alpha=([\\w+-.]+)"
        "\\s+periodic=(0|1)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        execute(match, args);
        return true;
    } else {
        return false;
    }
};

void AddBvhResource::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    BvhConfig cfg = blender_bvh_config;
    cfg.smooth_radius = safe_stoz(match[3].str());
    cfg.smooth_alpha = safe_stof(match[4].str());
    cfg.periodic = safe_stob(match[5].str());
    args.scene_node_resources.add_resource_loader(
        match[1].str(),
        [filename=args.fpath(match[2].str()).path, cfg](){
            return std::make_shared<BvhFileResource>(
                filename,
                cfg);});
}
