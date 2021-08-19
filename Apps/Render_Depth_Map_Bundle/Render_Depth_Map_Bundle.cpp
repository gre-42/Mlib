#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Cv/Depth_Difference.hpp>
#include <Mlib/Cv/Depth_Map_Package.hpp>
#include <Mlib/Cv/Render_Data.hpp>
#include <Mlib/Images/Filters/Median_Filter.hpp>
#include <Mlib/Images/StbImage.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Sfm/Components/Depth_Map_Bundle.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <vector>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

int main(int argc, char** argv) {

    const ArgParser parser(
        "Usage: render_depth_map_bundle"
        " --minus_threshold <threshold>"
        " [--median_filter_radius <r>]"
        " [--rotate]"
        " [--wire_frame]"
        " [--reference_time <milliseconds>]"
        " [--packages <file1> <file2...>]",
        {"--rotate", "--wire_frame"},
        {"--median_filter_radius",
        "--near_plane",
        "--far_plane",
        "--minus_threshold",
        "--reference_time"},
        {"--packages"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unamed(0);

        DepthMapBundle cleaned_bundle;
        {
            DepthMapBundle bundle;
            size_t r = safe_stoz(args.named_value("--median_filter_radius", "0"));
            for (const std::string& filename : args.named_list("--packages")) {
                std::cerr << "Loading \"" << filename << '"' << std::endl;
                DepthMapPackage package = load_depth_map_package(filename);
                if (r != 0) {
                    package.depth.move() = median_filter_2d(package.depth, r);
                }
                bundle.insert(package);
            }
            cleaned_bundle = bundle.delete_pixels_blocking_the_view(safe_stof(args.named_value("--minus_threshold")));
        }
        std::vector<DepthMapPackage> packages;
        packages.reserve(cleaned_bundle.packages().size());
        for (const auto& package : cleaned_bundle.packages()) {
            packages.push_back(package.second);
        }
        const auto& ref = cleaned_bundle.packages().find(std::chrono::milliseconds(safe_stou64(args.named_value("--reference_time"))));
        if (ref == cleaned_bundle.packages().end()) {
            throw std::runtime_error("Could not find package with reference time");
        }
        size_t num_renderings = SIZE_MAX;
        RenderConfig render_config{
            .wire_frame = args.has_named("--wire_frame"),
            .screen_width = (int)ref->second.depth.shape(1),
            .screen_height = (int)ref->second.depth.shape(0)};
        SceneNodeResources scene_node_resources;
        RenderingContextGuard rrg{scene_node_resources, "primary_rendering_resources", render_config.anisotropic_filtering_level, 0};
        Render2 render{ render_config, num_renderings };
        render_depth_maps(
            render,
            packages,
            ref->second.ki,
            ref->second.ke,
            (float)ref->second.depth.shape(1),
            (float)ref->second.depth.shape(0),
            safe_stof(args.named_value("--near_plane", "0.1")),
            safe_stof(args.named_value("--far_plane", "100")),
            safe_stof(args.named_value("--z_offset", "1")),
            args.has_named("--rotate"));
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
