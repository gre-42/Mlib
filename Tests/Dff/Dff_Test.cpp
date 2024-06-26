#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Dff_Array.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Rectangle_Triangulation_Mode.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Transformation_Mode.hpp>

using namespace Mlib;

static void test_teapot() {
    auto teapot = load_dff(
        "Data/teapot.dff",
        LoadMeshConfig<float>{
            .blend_mode = BlendMode::CONTINUOUS,
            .cull_faces_default = true,
            .cull_faces_alpha = true,
            .occluded_pass = ExternalRenderPassType::NONE,
            .occluder_pass = ExternalRenderPassType::NONE,
            .anisotropic_filtering_level = 16,
            .magnifying_interpolation_mode = InterpolationMode::LINEAR,
            .aggregate_mode = AggregateMode::NONE,
            .transformation_mode = TransformationMode::ALL,
            .period_world = 0.f,
            .apply_static_lighting = false,
            .laplace_ao_strength = 0.f,
            .dynamically_lighted = false,
            .physics_material = PhysicsMaterial::NONE,
            .rectangle_triangulation_mode = RectangleTriangulationMode::DELAUNAY,
            .werror = true});
}

int main(int argc, char** argv) {
    enable_floating_point_exceptions();

    try {
        test_teapot();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
