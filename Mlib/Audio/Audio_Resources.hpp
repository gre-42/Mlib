#pragma once
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
};

struct AudioFileSequenceInformation {
    std::string filename;
    float gain;
    float hysteresis_step;
};

class AudioResources {
    AudioResources(const AudioResources&) = delete;
    AudioResources &operator=(const AudioResources&) = delete;

public:
    AudioResources();
    ~AudioResources();
    void add_buffer(const std::string& name, const std::string &filename, float gain);
    float get_buffer_gain(const std::string& name) const;
    std::shared_ptr<AudioBuffer> get_buffer(const std::string& name) const;
    void preload_buffer(const std::string& name) const;

    void add_buffer_sequence(const std::string& name,
                             const std::string &filename,
                             float gain,
                             float hysteresis_step);
    float get_buffer_sequence_gain(const std::string& name) const;
    std::shared_ptr<AudioBufferSequenceWithHysteresis>
    get_buffer_sequence(const std::string& name) const;

private:
    mutable std::map<std::string, AudioFileInformation> buffer_filenames_;
    mutable std::map<std::string, std::shared_ptr<AudioBuffer>> buffers_;
    mutable std::map<std::string, AudioFileSequenceInformation> buffer_sequence_filenames_;
    mutable std::map<std::string, std::shared_ptr<AudioBufferSequenceWithHysteresis>>
        buffer_sequences_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
};

}
