#pragma once
#include <Mlib/Geometry/Intersection/Interval.hpp>
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

class AudioBuffer;
class AudioBufferSequenceWithHysteresis;

struct AudioFileInformation {
    std::string filename;
    float gain;
    std::optional<Interval<float>> distance_clamping;
};

struct AudioFileSequenceInformation {
    std::string filename;
    float gain;
    std::optional<Interval<float>> distance_clamping;
    float hysteresis_step;
};

class AudioResources {
    AudioResources(const AudioResources&) = delete;
    AudioResources &operator=(const AudioResources&) = delete;

public:
    AudioResources();
    ~AudioResources();
    void add_buffer(
        const VariableAndHash<std::string>& name,
        const std::string &filename,
        float gain,
        const std::optional<Interval<float>>& distance_clamping);
    const AudioFileInformation& get_buffer_meta(const VariableAndHash<std::string>& name) const;
    std::shared_ptr<AudioBuffer> get_buffer(const VariableAndHash<std::string>& name) const;
    void preload_buffer(const VariableAndHash<std::string>& name) const;

    void add_buffer_sequence(
        const VariableAndHash<std::string>& name,
        const std::string& filename,
        float gain,
        const std::optional<Interval<float>>& distance_clamping,
        float hysteresis_step);
    float get_buffer_sequence_gain(const VariableAndHash<std::string>& name) const;
    std::shared_ptr<AudioBufferSequenceWithHysteresis>
    get_buffer_sequence(const VariableAndHash<std::string>& name) const;

private:
    mutable StringWithHashUnorderedMap<AudioFileInformation> buffer_filenames_;
    mutable StringWithHashUnorderedMap<std::shared_ptr<AudioBuffer>> buffers_;
    mutable StringWithHashUnorderedMap<AudioFileSequenceInformation> buffer_sequence_filenames_;
    mutable StringWithHashUnorderedMap<std::shared_ptr<AudioBufferSequenceWithHysteresis>>
        buffer_sequences_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
};

}
