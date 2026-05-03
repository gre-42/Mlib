
#include "Audio_Scene.hpp"
#include <Mlib/Audio/Audio_Distance_Model.hpp>
#include <Mlib/Audio/Audio_Entity_State.hpp>
#include <Mlib/Audio/Audio_Listener.hpp>
#include <Mlib/Audio/Audio_Source.hpp>
#include <Mlib/Audio/CHK.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <mutex>
#include <string>
#ifdef __EMSCRIPTEN__
#include <Mlib/AGameHelper/Emscripten/AAnimation_Frame_Worker.hpp>
#endif

using namespace Mlib;

FastMutex AudioScene::mutex_;
float AudioScene::default_alpha_ = 1.f;
VerboseUnorderedMap<AudioSource*, AudioSourceNode> AudioScene::source_nodes_{
    "Audio source",
    [](const AudioSource* s) { return (std::stringstream() << s).str(); }
};
DanglingBaseClassPtr<SceneNode> AudioScene::listener_node_ = nullptr;
std::optional<DestructionFunctionsRemovalTokens> AudioScene::on_destroy_ = std::nullopt;

void AudioScene::set_default_alpha(float alpha) {
    std::scoped_lock lock{ mutex_ };
    default_alpha_ = alpha;
}

void AudioScene::add_source(AudioSource& source, float alpha) {
    std::scoped_lock lock{ mutex_ };
    source_nodes_.add(&source, AudioSourceNode{
        .relative_position{std::isnan(alpha) ? default_alpha_ : alpha, uninitialized},
        .relative_velocity{std::isnan(alpha) ? default_alpha_ : alpha, uninitialized}
        });
}

void AudioScene::remove_source(AudioSource& source) {
    std::scoped_lock lock{ mutex_ };
    source_nodes_.remove(&source);
}

void AudioScene::set_listener(
    const DanglingBaseClassRef<SceneNode>& node,
    std::chrono::steady_clock::time_point time,
    std::chrono::steady_clock::duration velocity_dt)
{
    std::scoped_lock lock{ mutex_ };
    if (node.ptr() != listener_node_) {
        for (auto& [_, node] : source_nodes_) {
            node.relative_position.reset();
            node.relative_velocity.reset();
        }
        listener_node_ = node.ptr();
        on_destroy_.emplace(node->on_destroy.deflt, CURRENT_SOURCE_LOCATION);
        on_destroy_->add([](){
            listener_node_ = nullptr;
            }, CURRENT_SOURCE_LOCATION);
    }
    AudioListener::set_transformation(AudioListenerState{
        .pose = node->absolute_model_matrix(time),
        .velocity = node->velocity(time, velocity_dt)
        });
}

void AudioScene::set_source_transformation(
    AudioSource& source,
    const AudioSourceState<ScenePos>& state)
{
    auto relpos = AudioListener::get_relative_position(state);
    if (!relpos.has_value()) {
        return;
    }
    std::scoped_lock lock{ mutex_ };
    auto& node = source_nodes_.get(&source);
    const auto& smooth_position = node.relative_position(relpos->position);
    const auto& smooth_velocity = node.relative_velocity(relpos->velocity);
    source.set_position(AudioSourceState<float>{
        .position = smooth_position,
        .velocity = smooth_velocity
    });
}

void AudioScene::set_distance_model(AudioDistanceModel model) {
    switch (model) {
    case AudioDistanceModel::INVERSE_DISTANCE_CLAMPED:
        AL_CHK(alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED));
        return;
    case AudioDistanceModel::LINEAR_DISTANCE_CLAMPED:
        AL_CHK(alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED));
        return;
    }
    throw std::runtime_error("Unknown audio distance model: " + std::to_string((int)model));
}

void AudioScene::print(std::ostream& ostr) {
    std::scoped_lock lock{ mutex_ };
    for (const auto& [source, node] : source_nodes_) {
        ostr << source << " pos: ";
        {
            const auto& rpos = node.relative_position.xhat();
            if (rpos.has_value()) {
                ostr << *rpos;
            } else {
                ostr << "?";
            }
        }
        ostr << " | vel: ";
        {
            const auto& rvel = node.relative_velocity.xhat();
            if (rvel.has_value()) {
                ostr << *rvel;
            } else {
                ostr << "?";
            }
        }
        ostr << '\n';
    }
}

void AudioScene::flush_sources() {
#ifdef __EMSCRIPTEN__
    std::scoped_lock lock{ mutex_ };
    execute_in_main_thread([](){
        for (auto& [s, _] : source_nodes_) {
            if (s->position_requirement_ == PositionRequirement::WAITING_FOR_POSITION) {
                continue;
            }
            AL_CHK(alSourcefv(s->source_, AL_POSITION, (s->position_.position / meters).flat_begin()));
            AL_CHK(alSourcefv(s->source_, AL_VELOCITY, (s->position_.velocity / (meters / seconds)).flat_begin()));
            
            AL_CHK(alSourcef(s->source_, AL_PITCH, s->pitch_));
            AL_CHK(alSourcei(s->source_, AL_LOOPING, s->loop_ ? AL_TRUE : AL_FALSE));
            AL_CHK(alSourcef(s->source_, AL_REFERENCE_DISTANCE, s->distance_clamping_.min));
            AL_CHK(alSourcef(s->source_, AL_MAX_DISTANCE, s->distance_clamping_.max));

            alGetSourcei(s->source_, AL_SOURCE_STATE, &s->last_source_state_);

            if (s->pending_command_.has_value()) {
                switch (*s->pending_command_) {
                    case AL_PLAYING:
                        AL_CHK(alSourcePlay(s->source_));
                        break;
                    case AL_STOPPED:
                        AL_CHK(alSourceStop(s->source_));
                        break;
                    case AL_PAUSED:
                        AL_CHK(alSourcePause(s->source_));
                        break;
                    default:
                        throw std::runtime_error("Unknown AL source state: " + std::to_string(*s->pending_command_));
                }
                s->pending_command_.reset();
            }
            AL_CHK(alSourcef(s->source_, AL_GAIN, s->gain_));
        }
    });
#endif
}
