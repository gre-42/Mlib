#pragma once
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>

namespace Mlib {

class AudioBuffer;
class AudioBufferSequence;

struct AudioFileInformation {
    std::string filename;
    float gain;
};

struct AudioFileSequenceInformation {
    std::string filename;
    float gain;
};

class AudioResources {
public:
    void add_buffer(const std::string& name, const std::string& filename, float gain);
    float get_buffer_gain(const std::string& name) const;
    std::shared_ptr<AudioBuffer> get_buffer(const std::string& name) const;

    void add_buffer_sequence(const std::string& name, const std::string& filename, float gain);
    float get_buffer_sequence_gain(const std::string& name) const;
    std::shared_ptr<AudioBufferSequence> get_buffer_sequence(const std::string& name) const;
private:
    mutable std::map<std::string, AudioFileInformation> buffer_filenames_;
    mutable std::map<std::string, std::shared_ptr<AudioBuffer>> buffers_;
    mutable std::map<std::string, AudioFileSequenceInformation> buffer_sequence_filenames_;
    mutable std::map<std::string, std::shared_ptr<AudioBufferSequence>> buffer_sequences_;
    mutable std::shared_mutex mutex_;
};

}
