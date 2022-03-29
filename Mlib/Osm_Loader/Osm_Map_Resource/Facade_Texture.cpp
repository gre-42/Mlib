#include "Facade_Texture.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

FacadeTexture Mlib::parse_facade_texture(const std::string& name) {
    static const DECLARE_REGEX(re, "^([^(]+)(?:\\(min_height:(\\d+)\\))?(?:\\(max_height:(\\d+)\\))?$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(name, match, re)) {
        FacadeTexture result{
            .name = match[1].str(),
            .min_height = match[2].matched ? safe_stof(match[2].str()) : -INFINITY,
            .max_height = match[3].matched ? safe_stof(match[3].str()) : INFINITY};
        return result;
    } else {
        throw std::runtime_error("Could not parse facade texture \"" + name + '"');
    }
}
