#include "Json_Object_File.hpp"
#include <Mlib/Os/Os.hpp>
#include <stdexcept>

using namespace Mlib;

JsonObjectFile::JsonObjectFile()
    : JsonView{j_, CheckIsObjectBehavior::NO_CHECK}
    , j_{nlohmann::json::object()}
{}

JsonObjectFile::~JsonObjectFile() = default;

void JsonObjectFile::load_from_file(const std::filesystem::path& filename) {
    nlohmann::json j;
    auto f = create_ifstream(filename);
    if (f->fail()) {
        throw std::runtime_error("Could not open for read: \"" + filename.string() + '"');
    }
    *f >> j;
    if (f->fail()) {
        throw std::runtime_error("Could not read: \"" + filename.string() + '"');
    }
    if (j.type() != nlohmann::detail::value_t::object) {
        throw std::runtime_error("File is not a JSON object: \"" + filename.string() + '"');
    }
    j_ = std::move(j);
}
