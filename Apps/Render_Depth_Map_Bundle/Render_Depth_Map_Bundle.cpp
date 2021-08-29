#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Cv/Depth_Map_Package.hpp>
#include <Mlib/Cv/Render/Render_Data.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
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
    enable_floating_point_exceptions();

    const ArgParser parser(
        "Usage: render_depth_map_bundle"
        " [--filter_threshold <threshold>]"
        " [--minus_threshold <threshold>]"
        " [--cos_threshold <threshold>]"
        " [--output <filename>]"
        " [--median_filter_radius <radius>]"
        " [--register_forward]"
        " [--register_backward]"
        " [--convert_to_points]"
        " [--convert_to_mesh]"
        " [--rotate]"
        " [--wire_frame]"
        " [--reference_time <milliseconds>]"
        " [--packages <file1> <file2...>]"
        " [--points <file>]"
        " [--point_radius <radius>]"
        " [--normal_radius <radius>]"
        " [--normal_k <k>]"
        " [--boundary_radius <radius>"
        " [--z_thickness <thickness>"
        " [--cos_min_angle]"
        " [--largest_cos_in_triangle]",
        {"--rotate",
        "--wire_frame",
        "--register_forward",
        "--register_backward",
        "--convert_to_points",
        "--convert_to_mesh"},
        {"--median_filter_radius",
        "--near_plane",
        "--far_plane",
        "--filter_threshold",
        "--minus_threshold",
        "--cos_threshold",
        "--reference_time",
        "--output",
        "--points",
        "--point_radius",
        "--normal_k",
        "--normal_radius",
        "--boundary_radius",
        "--z_thickness",
        "--cos_min_angle",
        "--largest_cos_in_triangle"},
        {"--packages", "--filter_references"});
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
        if (args.has_named_value("--filter_threshold")) {
            bool has_filter_references = args.has_named_list("--filter_references");
            std::set<std::chrono::milliseconds> filter_references;
            if (has_filter_references) {
                for (const std::string& s : args.named_list("--filter_references")) {
                    filter_references.insert(std::chrono::milliseconds{ safe_stou64(s) });
                }
            }
            bundle = bundle.filtered(
                safe_stof(args.named_value("--filter_threshold")),
                has_filter_references ? &filter_references : nullptr);
        }
        if (args.has_named_value("--minus_threshold")) {
            bundle = bundle.delete_pixels_blocking_the_view(safe_stof(args.named_value("--minus_threshold")));
        }
        if (args.has_named("--register_forward")) {
            bundle = bundle.reregistered(RegistrationDirection::FORWARD);
        }
        if (args.has_named("--register_backward")) {
            bundle = bundle.reregistered(RegistrationDirection::BACKWARD);
        }
        Array<TransformationMatrix<float, 3>> point_transformations{ ArrayShape{ 0 } };
        if (args.has_named_value("--points")) {
            Array<float> pts = Array<float>::load_txt_2d(args.named_value("--points"), ArrayShape{ 0, 3 });
            if (pts.shape(1) != 3) {
                throw std::runtime_error("Points dimension not 3");
            }
            for (const FixedArray<float, 3>& p : Array<float>::from_dynamic<3>(pts).flat_iterable()) {
                point_transformations.append(TransformationMatrix<float, 3>{fixed_zeros<float, 3, 3>(), p});
            }
        }

        std::vector<DepthMapPackage> packages;
        std::list<std::shared_ptr<ColoredVertexArray>> mesh;
        if (args.has_named("--convert_to_points") || args.has_named("--convert_to_mesh")) {
            Array<TransformationMatrix<float, 3>> dense_point_transformations =
                bundle.points_and_normals(
                    safe_stoz(args.named_value("--normal_k", "5")),
                    safe_stof(args.named_value("--normal_radius", "0.1")));
            if (args.has_named("--convert_to_mesh")) {
                mesh = bundle.mesh(
                    dense_point_transformations,
                    safe_stof(args.named_value("--boundary_radius")),
                    safe_stof(args.named_value("--z_thickness")),
                    safe_stof(args.named_value("--cos_min_angle", "0.1")),
                    safe_stof(args.named_value("--largest_cos_in_triangle", "0.9")));
            } else {
                point_transformations.append(dense_point_transformations);
            }
        } else {
            packages.reserve(bundle.packages().size());
            for (const auto& package : bundle.packages()) {
                packages.push_back(package.second);
            }
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
            point_transformations,
            mesh,
            ref->second.ki,
            ref->second.ke,
            (float)ref->second.depth.shape(1),
            (float)ref->second.depth.shape(0),
            safe_stof(args.named_value("--near_plane", "0.1")),
            safe_stof(args.named_value("--far_plane", "100")),
            args.has_named("--rotate"),
            1.f,  // scale
            0.f,  // camera_z
            SceneGraphConfig(),
            safe_stof(args.named_value("--point_radius", "0.1")),
            safe_stof(args.named_value("--cos_threshold", "0")));
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
