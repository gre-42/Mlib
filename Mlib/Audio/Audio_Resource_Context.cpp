#include "Audio_Resource_Context.hpp"
#include <Mlib/Resource_Context.impl.hpp>
#include <Mlib/Audio/Audio_Resources.hpp>

using namespace Mlib;

// template<>
// thread_local std::list<AudioResourceContext> ResourceContextStack<AudioResourceContext>::resource_context_stack_;

AudioResourceContext::AudioResourceContext()
: audio_resources{ std::make_shared<AudioResources>() }
{}

AudioResourceContext::~AudioResourceContext()
{}

std::shared_ptr<AudioResources> AudioResourceContextStack::primary_audio_resources() {
    return primary_resource_context().audio_resources;
}

std::shared_ptr<AudioResources> AudioResourceContextStack::audio_resources() {
    return resource_context().audio_resources;
}

template ResourceContextGuard<AudioResourceContext>::ResourceContextGuard(const AudioResourceContext& resource_context);
template ResourceContextGuard<AudioResourceContext>::~ResourceContextGuard();

template AudioResourceContext ResourceContextStack<AudioResourceContext>::primary_resource_context();
template AudioResourceContext ResourceContextStack<AudioResourceContext>::resource_context();
template std::function<std::function<void()>(std::function<void()>)>
    ResourceContextStack<AudioResourceContext>::generate_thread_runner(
        const AudioResourceContext& primary_resource_context,
        const AudioResourceContext& secondary_resource_context);
