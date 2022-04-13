#include "Facade_Texture.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(SELECTOR);
DECLARE_OPTION(FACADE);
DECLARE_OPTION(MIN_HEIGHT);
DECLARE_OPTION(MAX_HEIGHT);
DECLARE_OPTION(FACADE_EDGE_SIZE_X);
DECLARE_OPTION(FACADE_EDGE_SIZE_Y);
DECLARE_OPTION(FACADE_INNER_SIZE_X);
DECLARE_OPTION(FACADE_INNER_SIZE_Y);
DECLARE_OPTION(INTERIOR_SIZE_X);
DECLARE_OPTION(INTERIOR_SIZE_Y);
DECLARE_OPTION(INTERIOR_SIZE_Z);
DECLARE_OPTION(LEFT);
DECLARE_OPTION(RIGHT);
DECLARE_OPTION(FLOOR);
DECLARE_OPTION(CEILING);
DECLARE_OPTION(BACK);

FacadeTexture Mlib::parse_facade_texture(const std::string& text) {
    static const DECLARE_REGEX(re,
        "^"
        "(?:\\s*selector:(\\S+))?"
        "\\s*facade:(\\S+)"
        "(?:\\s*min_height:(\\S+))?"
        "(?:\\s*max_height:(\\S+))?"
        "(?:\\s*facade_edge_size:(\\S+) (\\S+)"
        "\\s*facade_inner_size:(\\S+) (\\S+)"
        "\\s*interior_size:(\\S+) (\\S+) (\\S+)"
        "\\s*left:(\\S+)"
        "\\s*right:(\\S+)"
        "\\s*floor:(\\S+)"
        "\\s*ceiling:(\\S+)"
        "\\s*back:(\\S+))?"
        "\\s*$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(text, match, re)) {
        return FacadeTexture{
            .selector = match[SELECTOR].str(),
            .min_height = match[MIN_HEIGHT].matched ? safe_stof(match[MIN_HEIGHT].str()) : -INFINITY,
            .max_height = match[MAX_HEIGHT].matched ? safe_stof(match[MAX_HEIGHT].str()) : INFINITY,
            .descriptor = FacadeTextureDescriptor{
                .name = match[FACADE].str(),
                .interior_textures = InteriorTextures{
                    .facade_edge_size = {
                        match[FACADE_EDGE_SIZE_X].matched ? safe_stof(match[FACADE_EDGE_SIZE_X].str()) : 0.f,
                        match[FACADE_EDGE_SIZE_Y].matched ? safe_stof(match[FACADE_EDGE_SIZE_Y].str()) : 0.f },
                    .facade_inner_size = {
                        match[FACADE_INNER_SIZE_X].matched ? safe_stof(match[FACADE_INNER_SIZE_X].str()) : 0.f,
                        match[FACADE_INNER_SIZE_Y].matched ? safe_stof(match[FACADE_INNER_SIZE_Y].str()) : 0.f},
                    .interior_size = {
                        match[INTERIOR_SIZE_X].matched ? safe_stof(match[INTERIOR_SIZE_X].str()) : 0.f,
                        match[INTERIOR_SIZE_Y].matched ? safe_stof(match[INTERIOR_SIZE_Y].str()) : 0.f,
                        match[INTERIOR_SIZE_Z].matched ? safe_stof(match[INTERIOR_SIZE_Z].str()) : 0.f },
                    .left = match[LEFT].str(),
                    .right = match[RIGHT].str(),
                    .floor = match[FLOOR].str(),
                    .ceiling = match[CEILING].str(),
                    .back = match[BACK].str(),
                }}};
    } else {
        throw std::runtime_error("Could not parse facade textures \"" + text + '"');
    }
}
