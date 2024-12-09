#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Cv/Depth_Map_Package.hpp>
#include <Mlib/Cv/Render/Render_Data.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Coordinates/Coordinate_Conversion.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Save_Obj.hpp>
#include <Mlib/Images/Filters/Median_Filter.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Render/Input_Config.hpp>
#include <Mlib/Render/Render.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Particle_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resource_Managers/Trail_Resources.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Sfm/Components/Depth_Map_Bundle.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>
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
        " [--show_beacon]"
        " [--rotate]"
        " [--wire_frame]"
        " [--reference_time <milliseconds>]"
        " [--packages <file1> <file2> ...]"
        " [--filter_references <id1> <id2> ...]"
        " [--points <file>]"
        " [--point_radius <radius>]"
        " [--normal_radius <radius>]"
        " [--normal_k <k>]"
        " [--boundary_radius <radius>"
        " [--z_thickness <thickness>"
        " [--cos_min_angle]"
        " [--largest_cos_in_triangle]"
        " [--cull_faces]",
        {"--rotate",
        "--wire_frame",
        "--register_forward",
        "--register_backward",
        "--convert_to_points",
        "--convert_to_mesh",
        "--show_beacon",
        "--cull_faces"},
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
        "--duplicate_distance",
        "--boundary_radius",
        "--z_thickness",
        "--cos_min_angle",
        "--largest_cos_in_triangle",
        "--beacon_scale",
        "--obj_out"},
        {"--packages", "--filter_references"});
    try {
        const auto args = parser.parsed(argc, argv);

        args.assert_num_unnamed(0);

        DepthMapBundle bundle;
        size_t r = safe_stoz(args.named_value("--median_filter_radius", "0"));
        for (const std::string& filename : args.named_list("--packages")) {
            lerr() << "Loading \"" << filename << '"';
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
        Array<TransformationMatrix<float, float, 3>> point_transformations{ ArrayShape{ 0 } };
        if (args.has_named_value("--points")) {
            Array<float> pts = Array<float>::load_txt_2d(args.named_value("--points"), ArrayShape{ 0, 3 });
            if (pts.shape(1) != 3) {
                throw std::runtime_error("Points dimension not 3");
            }
            for (const FixedArray<float, 3>& p : Array<float>::from_dynamic<3>(pts).flat_iterable()) {
                point_transformations.append(TransformationMatrix<float, float, 3>{fixed_zeros<float, 3, 3>(), p});
            }
        }

        std::vector<DepthMapPackage> packages;
        std::list<std::shared_ptr<ColoredVertexArray<float>>> mesh;
        std::vector<TransformationMatrix<float, float, 3>> beacon_locations;
        if (args.has_named("--convert_to_points") || args.has_named("--convert_to_mesh") || args.has_named("--show_beacon")) {
            Array<TransformationMatrix<float, float, 3>> dense_point_transformations =
                bundle.points_and_normals(
                    safe_stoz(args.named_value("--normal_k", "5")),
                    safe_stof(args.named_value("--normal_radius", "0.2")),
                    safe_stof(args.named_value("--duplicate_distance", "0.1")));
            if (args.has_named("--convert_to_mesh")) {
                mesh = bundle.mesh(
                    dense_point_transformations,
                    safe_stof(args.named_value("--boundary_radius", "0.05")),
                    safe_stof(args.named_value("--z_thickness", "0.02")),
                    safe_stof(args.named_value("--cos_min_angle", "0.1")),
                    safe_stof(args.named_value("--largest_cos_in_triangle", "0.99")));
            } else {
                point_transformations.append(dense_point_transformations);
            }
            if (args.has_named("--show_beacon")) {
                beacon_locations.reserve(dense_point_transformations.length());
                for (size_t i = 0; i < dense_point_transformations.length(); ++i) {
                    auto& bl = beacon_locations.emplace_back(dense_point_transformations(i));
                    bl.R *= safe_stof(args.named_value("--beacon_scale", "0.1"));
                }
            }
        } else {
            packages.reserve(bundle.packages().size());
            for (const auto& package : bundle.packages()) {
                packages.push_back(package.second);
            }
        }
        if (args.has_named_value("--obj_out")) {
            std::list<FixedArray<ColoredVertex<float>, 3>> triangles;
            auto m = cv_to_opengl_matrix();
            for (const auto& lst : mesh) {
                for (const auto& t : lst->triangles) {
                    triangles.push_back(t.applied([&m](const ColoredVertex<float>& t){return t.transformed(m, m.R);}));
                }
            }
            save_obj(
                args.named_value("--obj_out"),
                IndexedFaceSet<float, float, size_t>{ triangles },
                nullptr);  // material
        }
        const auto& ref = bundle.packages().find(std::chrono::milliseconds(safe_stou64(args.named_value("--reference_time"))));
        if (ref == bundle.packages().end()) {
            throw std::runtime_error("Could not find package with reference time");
        }
        std::atomic_size_t num_renderings = SIZE_MAX;
        RenderConfig render_config{
            .cull_faces = args.has_named("--cull_faces")
                ? BoolRenderOption::ON
                : BoolRenderOption::UNCHANGED,
            .wire_frame = args.has_named("--wire_frame")
                ? BoolRenderOption::ON
                : BoolRenderOption::UNCHANGED,
            .windowed_width = (int)ref->second.depth.shape(1),
            .windowed_height = (int)ref->second.depth.shape(0),
            .double_buffer = true};
        InputConfig input_config;
        SceneNodeResources scene_node_resources;
        ParticleResources particle_resources;
        TrailResources trail_resources;
        RenderingResources rendering_resources{
            "primary_rendering_resources",
            16 };
        RenderingContext primary_rendering_context{
            .scene_node_resources = scene_node_resources,
            .particle_resources = particle_resources,
            .trail_resources = trail_resources,
            .rendering_resources = rendering_resources,
            .z_order = 0 };
        RenderingContextGuard rcg{ primary_rendering_context };
        RenderResults render_results;
        RenderedSceneDescriptor rsd;
        if (args.has_named_value("--output")) {
            render_results.outputs[rsd] = {};
        }
        SetFps set_fps{ nullptr };
        Render render{ render_config, input_config, num_renderings, set_fps, []() { return std::chrono::steady_clock::now(); }, &render_results };
        render_depth_maps(
            render,
            packages,
            point_transformations,
            mesh,
            beacon_locations,
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
            safe_stof(args.named_value("--point_radius", "0.01")),
            safe_stof(args.named_value("--cos_threshold", "0")));
        if (args.has_named_value("--output")) {
            const Array<float>& array = render_results.outputs.at(rsd).rgb;
            if (!array.initialized()) {
                throw std::runtime_error("Rendered scene descriptor not initialized");
            }
            StbImage3::from_float_rgb(array).save_to_file(args.named_value("--output"));
        }
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
