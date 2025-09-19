#include "Hud_Tracker.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>

using namespace Mlib;

HudErrorBehavior Mlib::hud_error_behavior_from_string(const std::string& s) {
    static const std::map<std::string, HudErrorBehavior> m{
        {"hide", HudErrorBehavior::HIDE},
        {"center", HudErrorBehavior::CENTER} };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown HUD error behavior: \"" + s + '"');
    }
    return it->second;
}

HudTrackerTimeAdvancer::HudTrackerTimeAdvancer(HudTracker& tracker)
    : tracker_{ tracker }
    , vp_{ uninitialized }
{
    std::scoped_lock lock{ tracker.render_mutex_ };
    is_visible_ = tracker.is_visible_;
    if (!is_visible_) {
        return;
    } else {
        vp_ = tracker.vp_;
        near_plane_ = tracker.near_plane_;
        far_plane_ = tracker.far_plane_;
    }
    assert_true(!std::isnan(near_plane_));
}

bool HudTrackerTimeAdvancer::is_visible() const {
    return is_visible_;
}

void HudTrackerTimeAdvancer::advance_time(const FixedArray<ScenePos, 3>& point) {
    if (!is_visible_) {
        verbose_abort("HudTrackerTimeAdvancer::advance_time on invisible tracker");
    }
    auto position4 = dot1d(vp_, homogenized_4(point));
    {
        // From: https://stackoverflow.com/questions/6652253/getting-the-true-z-value-from-the-depth-buffer
        float z_n = (float)(position4(2) / position4(3));
        float z_e = 2.f * near_plane_ * far_plane_ / (far_plane_ + near_plane_ - z_n * (far_plane_ - near_plane_));
        if (z_e < near_plane_) {
            tracker_.invalidate();
            return;
        }
    }
    {
        std::scoped_lock lock{ tracker_.offset_mutex_ };
        tracker_.offset_ = {
            float(position4(0) / position4(3)),
            float(position4(1) / position4(3)) };
    }
}

HudTracker::HudTracker(
    const std::optional<std::vector<DanglingBaseClassPtr<const SceneNode>>>& exclusive_nodes,
    HudErrorBehavior hud_error_behavior,
    const FixedArray<float, 2>& center,
    const FixedArray<float, 2>& size,
    const std::shared_ptr<ITextureHandle>& texture)
    : FillWithTextureLogic{
        texture,
        CullFaceMode::CULL,
        ContinuousBlendMode::ALPHA,
        nullptr }
    , center_{ center }
    , size_{ size }
    , hud_error_behavior_{ hud_error_behavior }
    , offset_(NAN)
    , smooth_offset_{ 0.2f }
    , is_visible_{ false }
    , vp_(NAN)
    , near_plane_{ NAN }
    , far_plane_{ NAN }
{
    if (exclusive_nodes.has_value()) {
        exclusive_nodes_.emplace();
        for (const auto& element : *exclusive_nodes) {
            if (!exclusive_nodes_->emplace(element, CURRENT_SOURCE_LOCATION).second) {
                THROW_OR_ABORT("Duplicate exclusive nodes");
            }
        }
    }
}

HudTracker::~HudTracker() = default;

void HudTracker::invalidate() {
    std::scoped_lock lock{ offset_mutex_ };
    offset_ = NAN;
    smooth_offset_.reset();
}

HudTrackerTimeAdvancer HudTracker::time_advancer() {
    return HudTrackerTimeAdvancer(*this);
}

void HudTracker::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup& setup)
{
    {
        std::scoped_lock lock0{ render_mutex_ };
        if (!exclusive_nodes_.has_value()) {
            is_visible_ = true;
        } else if (setup.camera_node == nullptr) {
            // Hide the HUD if the camera node was deleted.
            // If the camera node had been the exclusive node,
            // the HudTracker would no longer exist by now.
            is_visible_ = false;
        } else {
            is_visible_ = exclusive_nodes_->contains(setup.camera_node);
        }
        vp_ = setup.vp;
        near_plane_ = setup.camera->get_near_plane();
        far_plane_ = setup.camera->get_far_plane();
    }
    if (!is_visible_) {
        return;
    }
    float aspect_ratio = lx.flength() / ly.flength();

    FixedArray<float, 2> offset = uninitialized;
    {
        std::scoped_lock lock{ offset_mutex_ };
        if (any(Mlib::isnan(offset_))) {
            if (hud_error_behavior_ == HudErrorBehavior::HIDE) {
                return;
            } else if (hud_error_behavior_ == HudErrorBehavior::CENTER) {
                offset = smooth_offset_(fixed_zeros<float, 2>());
            } else {
                THROW_OR_ABORT("Unknown HUD error behavior");
            }
        } else {
            offset = smooth_offset_(offset_);
        }
    }
    float vertices[] = {
        // positions                                                                         // tex_coords
        offset(0) + center_(0) - size_(0) / aspect_ratio, offset(1) + center_(1) + size_(1), 0.0f, 1.0f,
        offset(0) + center_(0) - size_(0) / aspect_ratio, offset(1) + center_(1) - size_(1), 0.0f, 0.0f,
        offset(0) + center_(0) + size_(0) / aspect_ratio, offset(1) + center_(1) - size_(1), 1.0f, 0.0f,

        offset(0) + center_(0) - size_(0) / aspect_ratio, offset(1) + center_(1) + size_(1), 0.0f, 1.0f,
        offset(0) + center_(0) + size_(0) / aspect_ratio, offset(1) + center_(1) - size_(1), 1.0f, 0.0f,
        offset(0) + center_(0) + size_(0) / aspect_ratio, offset(1) + center_(1) + size_(1), 1.0f, 1.0f
    };

    va();  // Initialize if necessary
    vertices_.bind();
    CHK(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
    CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    FillWithTextureLogic::render(ClearMode::OFF);
}
