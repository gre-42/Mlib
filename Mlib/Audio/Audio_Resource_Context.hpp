#pragma once
#include <Mlib/Resource_Context.hpp>
#include <memory>

namespace Mlib {

class AudioResources;

class AudioResourceContext {
public:
    AudioResourceContext();
    ~AudioResourceContext();
    std::shared_ptr<AudioResources> audio_resources;
};

using AudioResourceContextGuard = ResourceContextGuard<AudioResourceContext>;

class AudioResourceContextStack: public ResourceContextStack<AudioResourceContext> {
public:
    static std::shared_ptr<AudioResources> primary_audio_resources();
    static std::shared_ptr<AudioResources> audio_resources();
};

}
