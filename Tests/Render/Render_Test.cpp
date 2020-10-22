#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/Render.hpp>
#include <Mlib/Render/Render2.hpp>
#include <Mlib/Render/Render_Results.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Stats/Fixed_Random_Arrays.hpp>
#include <fenv.h>

using namespace Mlib;

void test_scene_node() {
    SceneNode node;
    node.set_position(fixed_random_uniform_array<float, 3>(1));
    node.set_rotation(fixed_random_uniform_array<float, 3>(2));
    node.set_scale(5);
    assert_allclose(
        node.relative_model_matrix().to_array(),
        inv(node.absolute_view_matrix().to_array()));
}

void test_render() {
    PpmImage img = PpmImage::load_from_file("Data/Depth/vid001.ppm");
    Array<float> depth = Array<float>::load_binary("Data/Depth/masked-depth-0-0-388-0-190.array");
    Array<float> intrinsic_matrix = Array<float>::load_txt_2d("Data/Depth/camera_intrinsic.m");
    if (!all(intrinsic_matrix.shape() == ArrayShape{3, 3})) {
        throw std::runtime_error("Intrinsic matrix has incorrect shape");
    }
    if (!all(depth.shape() == img.shape())) {
        throw std::runtime_error("Depth and image shape differ");
    }
    {
        Array<float> output;
        ::Mlib::render_depth_map(
            img.to_float_rgb(),
            depth,
            FixedArray<float, 3, 3>{intrinsic_matrix},
            1,      // z_offset
            false,  // rotate
            &output);
        draw_nan_masked_rgb(output, 0, 1).save_to_file("TestOut/rendered.ppm");
    }
    {
        Array<float> output;
        RenderResults render_results{output: &output};
        size_t num_renderings = SIZE_MAX;
        Render2{num_renderings, &render_results, RenderConfig{}}.render_depth_map(
            img.to_float_rgb(),
            depth,
            FixedArray<float, 3, 3>{intrinsic_matrix},
            false,  // rotate
            1,      // scale
            SceneGraphConfig{},
            CameraConfig{});
        draw_nan_masked_rgb(output, 0, 1).save_to_file("TestOut/rendered2.ppm");
    }
}

int main(int argc, char **argv) {
    #ifndef __MINGW32__
    feenableexcept(FE_INVALID);
    #endif

    test_scene_node();
    test_render();
    return 0;
}
