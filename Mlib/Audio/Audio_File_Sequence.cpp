#include "Audio_File_Sequence.hpp"
#include <Mlib/Json.hpp>
#include <Mlib/Os/Os.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;
using namespace Mlib;

namespace Mlib {

void from_json(const json& j, AudioFileSequenceItem& item) {
    j.at("filename").get_to(item.filename);
    j.at("frequency").get_to(item.frequency);
    j.at("key").get_to(item.key);
}

}

std::vector<AudioFileSequenceItem> Mlib::load_audio_file_sequence(
    const std::string& filename)
{
    auto ifs = create_ifstream(filename);
    if (ifs->fail()) {
        THROW_OR_ABORT("Could not open file \"" + filename + '"');
    }
    json j;
    *ifs >> j;
    if (!ifs->eof() && ifs->fail()) {
        THROW_OR_ABORT("Error reading from file: \"" + filename + '"');
    }
    auto res = j.get<std::vector<AudioFileSequenceItem>>();
    for (auto& i : res) {
        i.filename = (fs::path{filename}.parent_path() / i.filename).string();
    }
    return res;
}
