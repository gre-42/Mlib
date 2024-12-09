#include <Mlib/Cv/Render/Render_Data.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Test.hpp>
#include <Mlib/Render/Input_Config.hpp>
#include <Mlib/Render/Render.hpp>
#include <Mlib/Render/Render.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Stats/Fixed_Random_Arrays.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>

using namespace Mlib;
using namespace Mlib::Cv;

void test_scene_node() {
    SceneNode node;
    node.set_position(fixed_random_uniform_array<double, 3>(1), std::chrono::steady_clock::now());
    node.set_rotation(fixed_random_uniform_array<float, 3>(2), std::chrono::steady_clock::now());
    node.set_scale(5);
    assert_allclose(
        node.relative_model_matrix().affine(),
        inv(node.absolute_view_matrix().affine()).value());
}

void test_render() {
    StbImage3 img = StbImage3::load_from_file("Data/Depth/vid001.png");
    Array<float> depth = Array<float>::load_binary("Data/Depth/masked-depth-0-0-388-0-190.array");
    TransformationMatrix<float, float, 2> intrinsic_matrix{ FixedArray<float, 3, 3>{ Array<float>::load_txt_2d("Data/Depth/camera_intrinsic.m") } };
    if (!all(depth.shape() == img.shape())) {
        throw std::runtime_error("Depth and image shape differ");
    }
    {
        RenderConfig render_config;
        InputConfig input_config;
        RenderResults render_results;
        RenderedSceneDescriptor rsd;
        render_results.outputs[rsd] = {};
        std::atomic_size_t num_renderings = SIZE_MAX;
        SetFps set_fps{ nullptr };
        Render render{ render_config, input_config, num_renderings, set_fps, [](){ return std::chrono::steady_clock::now(); }, &render_results };
        render_depth_map(
            render,
            img.to_float_rgb(),
            depth,
            intrinsic_matrix,
            0.1f,   // near_plane
            100.f,  // far_plane
            false,  // rotate
            1,      // scale
            2,      // camera_z
            SceneGraphConfig());
        draw_nan_masked_rgb(render_results.outputs.at(rsd).rgb, 0, 1).save_to_file("TestOut/rendered.png");
    }
    {
        RenderConfig render_config;
        InputConfig input_config;
        RenderResults render_results;
        RenderedSceneDescriptor rsd;
        render_results.outputs[rsd] = {};
        std::atomic_size_t num_renderings = SIZE_MAX;
        SetFps set_fps{ nullptr };
        Render render{ render_config, input_config, num_renderings, set_fps, [](){ return std::chrono::steady_clock::now(); }, &render_results};
        render_depth_map(
            render,
            img.to_float_rgb(),
            depth,
            intrinsic_matrix,
            0.1f,   // near_plane
            100.f,  // far_plane
            false,  // rotate
            1,      // scale
            2,      // camera_z
            SceneGraphConfig());
        draw_nan_masked_rgb(render_results.outputs.at(rsd).rgb, 0, 1).save_to_file("TestOut/rendered2.png");
    }
}


int main(int argc, char **argv) {
    enable_floating_point_exceptions();

    test_scene_node();
    test_render();
    return 0;
}
