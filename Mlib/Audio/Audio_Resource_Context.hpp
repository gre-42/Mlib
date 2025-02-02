#pragma once
#ifndef WITHOUT_ALUT

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

using AudioResourceContextGuard = ResourceContextGuard<const AudioResourceContext>;

class AudioResourceContextStack: public ResourceContextStack<const AudioResourceContext> {
    AudioResourceContextStack() = delete;

public:
    static std::shared_ptr<AudioResources> primary_audio_resources();
    static std::shared_ptr<AudioResources> audio_resources();
};

}

#endif
