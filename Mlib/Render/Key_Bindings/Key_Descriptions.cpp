#include "Key_Descriptions.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Render/Key_Bindings/Key_Description.hpp>

namespace KeyDescriptionArgs {
    BEGIN_ARGUMENT_LIST;
    DECLARE_ARGUMENT(required);
    DECLARE_ARGUMENT(unique);
    DECLARE_ARGUMENT(id);
    DECLARE_ARGUMENT(section);
    DECLARE_ARGUMENT(title);
}

namespace Mlib {

void from_json(const nlohmann::json& j, KeyDescription& obj)
{
    validate(j, KeyDescriptionArgs::options);
    if (j.contains(KeyDescriptionArgs::required)) {
        j.at(KeyDescriptionArgs::required).get_to(obj.required);
    }
    j.at(KeyDescriptionArgs::unique).get_to(obj.unique);
    j.at(KeyDescriptionArgs::id).get_to(obj.id);
    j.at(KeyDescriptionArgs::section).get_to(obj.section);
    j.at(KeyDescriptionArgs::title).get_to(obj.title);
}

}

using namespace Mlib;

KeyDescriptions::KeyDescriptions() = default;

KeyDescriptions::~KeyDescriptions() = default;

void KeyDescriptions::load(const std::string& filename)
{
    nlohmann::json j;
    {
        auto f = create_ifstream(filename);
        if (f->fail()) {
            THROW_OR_ABORT("Could not open file " + filename);
        }
        *f >> j;
        if (f->fail()) {
            THROW_OR_ABORT("Could not read from file " + filename);
        }
    }
    for (auto&& e : j.get<std::vector<KeyDescription>>()) {
        append(std::move(e));
    }
}

void KeyDescriptions::append(KeyDescription key_description) {
    key_descriptions_.emplace_back(std::move(key_description));
}

const KeyDescription& KeyDescriptions::operator [] (size_t i) const {
    return key_descriptions_.at(i);
}

size_t KeyDescriptions::size() const {
    return key_descriptions_.size();
}

std::vector<KeyDescription>::const_iterator KeyDescriptions::begin() const {
    return key_descriptions_.begin();
}

std::vector<KeyDescription>::const_iterator KeyDescriptions::end() const {
    return key_descriptions_.end();
}
