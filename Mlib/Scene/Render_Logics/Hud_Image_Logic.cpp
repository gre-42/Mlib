#include "Hud_Image_Logic.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Collision_Query.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <sstream>

using namespace Mlib;

HudImageLogic::HudImageLogic(
    RenderLogic* scene_logic,
    CollisionQuery* collision_query,
    SceneNode* gun_node,
    SceneNode& node_to_hide,
    YawPitchLookAtNodes* ypln,
    AdvanceTimes& advance_times,
    const std::string& image_resource_name,
    ResourceUpdateCycle update_cycle,
    const FixedArray<float, 2>& center,
    const FixedArray<float, 2>& size)
: FillWithTextureLogic{ image_resource_name, update_cycle, ColorMode::RGBA },
  scene_logic_{ scene_logic },
  collision_query_{ collision_query },
  gun_node_{ gun_node },
  node_to_hide_{ node_to_hide },
  ypln_{ ypln },
  advance_times_{ advance_times },
  center_{ center },
  size_{ size },
  is_visible_{ false },
  offset_(0.f),
  smooth_offset_{0.2f},
  vp_(NAN),
  near_plane_{NAN},
  far_plane_{NAN}
{
    if (!gun_node != !scene_logic) {
        THROW_OR_ABORT("Inconsistent nullness for gun node and scene logic");
    }
    if (!gun_node != !collision_query) {
        THROW_OR_ABORT("Inconsistent nullness for gun node and collision query");
    }
}

HudImageLogic::~HudImageLogic() = default;

void HudImageLogic::notify_destroyed(const Object& destroyed_object) {
    advance_times_.delete_advance_time(*this);
}

void HudImageLogic::advance_time(float dt) {
    if (gun_node_ == nullptr) {
        return;
    }
    if (ypln_ != nullptr) {
        float dpitch_head = ypln_->pitch_look_at_node().get_dpitch_head();
        if (!std::isnan(dpitch_head) && (dpitch_head != 0.f)) {
            std::scoped_lock lock{offset_mutex_};
            offset_ = 0.f;
            return;
        }
    }
    FixedArray<double, 4, 4> vp;
    float near_plane;
    float far_plane;
    {
        std::scoped_lock lock{render_mutex_};
        if (!is_visible_) {
            return;
        }
        vp = vp_;
        near_plane = near_plane_;
        far_plane = far_plane_;
    }
    assert_true(!std::isnan(near_plane));
    auto gun_pose = gun_node_->absolute_model_matrix();
    FixedArray<double, 3> intersection_point;
    if (collision_query_->can_see(
        gun_pose.t(),
        gun_pose.t() - 1000.0 * gun_pose.R().column(2).casted<double>(),
        nullptr, // excluded0,
        nullptr, // excluded1
        false, // only_terrain
        PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK,
        &intersection_point))
    {
        std::scoped_lock lock{offset_mutex_};
        offset_ = 0.f;
        return;
    }
    auto position4 = dot1d(vp, homogenized_4(intersection_point));
    {
        // From: https://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer
        float z_n = (float)(position4(2) / position4(3));
        float z_e = 2.f * near_plane * far_plane / (far_plane + near_plane - z_n * (far_plane - near_plane));
        if (z_e < near_plane) {
            std::scoped_lock lock{offset_mutex_};
            offset_ = 0.f;
            return;
        }
    }
    {
        std::scoped_lock lock{offset_mutex_};
        offset_ = {
            float(position4(0) / position4(3)),
            float(position4(1) / position4(3))};
    }
}

void HudImageLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("HudImageLogic::render");
    if (!is_visible_) {
        return;
    }
    float aspect_ratio = lx.flength() / ly.flength();

    FixedArray<float, 2> offset;
    {
        std::scoped_lock lock{offset_mutex_};
        offset = smooth_offset_(offset_);
    }
    float vertices[] = {
        // positions                                                                         // texCoords
        offset(0) + center_(0) - size_(0) / aspect_ratio, offset(1) + center_(1) + size_(1), 0.0f, 1.0f,
        offset(0) + center_(0) - size_(0) / aspect_ratio, offset(1) + center_(1) - size_(1), 0.0f, 0.0f,
        offset(0) + center_(0) + size_(0) / aspect_ratio, offset(1) + center_(1) - size_(1), 1.0f, 0.0f,

        offset(0) + center_(0) - size_(0) / aspect_ratio, offset(1) + center_(1) + size_(1), 0.0f, 1.0f,
        offset(0) + center_(0) + size_(0) / aspect_ratio, offset(1) + center_(1) - size_(1), 1.0f, 0.0f,
        offset(0) + center_(0) + size_(0) / aspect_ratio, offset(1) + center_(1) + size_(1), 1.0f, 1.0f
    };

    CHK(glBindBuffer(GL_ARRAY_BUFFER, va().vertex_buffer));
    CHK(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
    CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    FillWithTextureLogic::render();
}

bool HudImageLogic::node_shall_be_hidden(
    const SceneNode& camera_node,
    const ExternalRenderPass& external_render_pass) const
{
    std::scoped_lock lock{render_mutex_};
    is_visible_ = (&node_to_hide_ == &camera_node);
    vp_ = scene_logic_->vp();
    near_plane_ = scene_logic_->near_plane();
    far_plane_ = scene_logic_->far_plane();
    return false;
}

void HudImageLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "HudImageLogic\n";
}
