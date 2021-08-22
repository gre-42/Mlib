#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Cv/Depth_Difference.hpp>
#include <Mlib/Cv/Depth_Map_Package.hpp>
#include <Mlib/Cv/Render/Render_Data.hpp>
#include <Mlib/Images/Filters/Median_Filter.hpp>
#include <Mlib/Images/StbImage.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Render_Results.hpp>
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
        " [--output <filename>]"
        " [--median_filter_radius <r>]"
        " [--register_forward]"
        " [--register_backward]"
        " [--rotate]"
        " [--wire_frame]"
        " [--reference_time <milliseconds>]"
        " [--packages <file1> <file2...>]"
        " [--points <file>]",
        {"--rotate",
        "--wire_frame",
        "--register_forward",
        "--register_backward"},
        {"--median_filter_radius",
        "--near_plane",
        "--far_plane",
        "--minus_threshold",
        "--reference_time",
        "--output",
        "--points"},
        {"--packages"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unamed(0);

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
        bundle = bundle.delete_pixels_blocking_the_view(safe_stof(args.named_value("--minus_threshold")));
        if (args.has_named("--register_forward")) {
            bundle = bundle.reregister(RegistrationDirection::FORWARD);
        }
        if (args.has_named("--register_backward")) {
            bundle = bundle.reregister(RegistrationDirection::BACKWARD);
        }
        Array<FixedArray<float, 3>> points;
        if (args.has_named_value("--points")) {
            Array<float> pts = Array<float>::load_txt_2d(args.named_value("--points"), ArrayShape{ 0, 3 });
            if (pts.shape(1) != 3) {
                throw std::runtime_error("Points dimension not 3");
            }
            points = Array<float>::from_dynamic<3>(pts);
        }

        std::vector<DepthMapPackage> packages;
        packages.reserve(bundle.packages().size());
        for (const auto& package : bundle.packages()) {
            packages.push_back(package.second);
        }
        const auto& ref = bundle.packages().find(std::chrono::milliseconds(safe_stou64(args.named_value("--reference_time"))));
        if (ref == bundle.packages().end()) {
            throw std::runtime_error("Could not find package with reference time");
        }
        size_t num_renderings = SIZE_MAX;
        RenderConfig render_config{
            .wire_frame = args.has_named("--wire_frame"),
            .screen_width = (int)ref->second.depth.shape(1),
            .screen_height = (int)ref->second.depth.shape(0),
            .double_buffer = true};
        SceneNodeResources scene_node_resources;
        RenderingContextGuard rrg{scene_node_resources, "primary_rendering_resources", render_config.anisotropic_filtering_level, 0};
        RenderResults render_results;
        RenderedSceneDescriptor rsd;
        if (args.has_named_value("--output")) {
            render_results.outputs[rsd] = {};
        }
        Render2 render{ render_config, num_renderings, &render_results };
        render_depth_maps(
            render,
            packages,
            points,
            ref->second.ki,
            ref->second.ke,
            (float)ref->second.depth.shape(1),
            (float)ref->second.depth.shape(0),
            safe_stof(args.named_value("--near_plane", "0.1")),
            safe_stof(args.named_value("--far_plane", "100")),
            args.has_named("--rotate"));
        if (args.has_named_value("--output")) {
            const Array<float>& array = render_results.outputs.at(rsd).rgb;
            if (!array.initialized()) {
                throw std::runtime_error("Rendered scene descriptor not initialized");
            }
            StbImage::from_float_rgb(array).save_to_file(args.named_value("--output"));
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
