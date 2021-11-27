#pragma once
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace Mlib {

class AudioBuffer;

struct AudioFileInformation {
    std::string filename;
    float gain;
};

class AudioResources {
public:
    void add_buffer(const std::string& name, const std::string& filename, float gain);
    float get_gain(const std::string& name) const;
    std::shared_ptr<AudioBuffer> get_buffer(const std::string& name) const;
private:
    mutable std::map<std::string, AudioFileInformation> buffer_filenames_;
    mutable std::map<std::string, std::shared_ptr<AudioBuffer>> audio_buffers_;
    mutable std::mutex mutex_;
};

}
