#include "Draw_Distance_Db.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Template_Regex.hpp>
#include <Mlib/Strings/RGetline.hpp>
#include <Mlib/Strings/String_View_To_Number.hpp>
#include <algorithm>
#include <set>

using namespace Mlib;
using namespace Mlib::TemplateRegex;

DrawDistanceDb::DrawDistanceDb() = default;

DrawDistanceDb::~DrawDistanceDb() = default;

void DrawDistanceDb::add_ide(const std::string& filename) {
    auto f = create_ifstream(filename, std::ios::binary);
    if (f->fail()) {
        THROW_OR_ABORT("Could not open \"" + filename + "\" for reading");
    }
    std::string line;
    std::string section;
    while (rgetline(*f, line)) {
        if (f->fail()) {
            THROW_OR_ABORT("Could not read line from file \"" + filename + '"');
        }
        if (line.starts_with('#')) {
            continue;
        }
        if (section.empty()) {
            section = line;
            continue;
        }
        if (line == "end") {
            if (section.empty()) {
                THROW_OR_ABORT("Nested \"end\" statements in file \"" + filename + '"');
            }
            section.clear();
            continue;
        }
        auto set_distances = [&](
            const std::string& resource_name,
            const std::string& texture_dictionary,
            float dist[],
            size_t n,
            IdeFlags flags)
        {
            auto texture_dictionary_lower = texture_dictionary;
            std::transform(texture_dictionary_lower.begin(), texture_dictionary_lower.end(), texture_dictionary_lower.begin(), ::tolower);
            std::set<float> dist_set(dist, dist + n);
            auto dist_range = [&dist_set, dist](size_t i){
                auto it = dist_set.find(dist[i]);
                if (it == dist_set.end()) {
                    verbose_abort("Internal distance DB error");
                }
                if (it == dist_set.begin()) {
                    return AddableStepDistances{ -INFINITY, dist[i] };
                } else {
                    return AddableStepDistances{ *(--it), dist[i] };
                }
            };
            ide_items_.try_emplace(
                resource_name,
                texture_dictionary_lower,
                dist_range(0),
                flags);
            for (size_t i = 0; i < n; ++i) {
                ide_items_.try_emplace(
                    resource_name + "_l" + std::to_string(i),
                    texture_dictionary_lower,
                    dist_range(i),
                    flags);
                ide_items_.try_emplace(
                    resource_name + "_L" + std::to_string(i),
                    texture_dictionary_lower,
                    dist_range(i),
                    flags);
            }
            };
        static const auto c = str(", ");
        static const auto n = group(plus(CharPredicate{ [](char c) {return c != ','; } }));
        if (section == "objs") {
            // From: https://gtamods.com/wiki/OBJS
            // Type 1
            static const auto reg6 = seq(n, c, n, c, n, c, n, c, n, c, n, eof);
            static const auto reg7 = seq(n, c, n, c, n, c, n, c, n, c, n, c, n, eof);
            static const auto reg8 = seq(n, c, n, c, n, c, n, c, n, c, n, c, n, c, n, eof);

            if (SMatch<7> match6; regex_match(line, match6, reg6)) {
                if (match6[4].str() != "1") {
                    THROW_OR_ABORT("Unexpected LOD count in line \"" + line + '"');
                }
                float distances[] = { safe_stof(match6[5].str()) };
                auto flags = (IdeFlags)safe_stoi(match6[6].str());
                set_distances(std::string{ match6[2].str() }, std::string{ match6[3].str() }, distances, 1, flags);
            } else if (SMatch<8> match7; regex_match(line, match7, reg7)) {
                if (match7[4].str() != "2") {
                    THROW_OR_ABORT("Unexpected LOD count in line \"" + line + '"');
                }
                float distances[] = { safe_stof(match7[5].str()), safe_stof(match7[6].str()) };
                auto flags = (IdeFlags)safe_stoi(match7[7].str());
                set_distances(std::string{ match7[2].str() }, std::string{ match7[3].str() }, distances, 2, flags);
            } else if (SMatch<9> match8; regex_match(line, match8, reg8)) {
                if (match8[4].str() != "3") {
                    THROW_OR_ABORT("Unexpected LOD count in line \"" + line + '"');
                }
                float distances[] = { safe_stof(match8[5].str()), safe_stof(match8[6].str()), safe_stof(match8[7].str()) };
                auto flags = (IdeFlags)safe_stoi(match8[8].str());
                set_distances(std::string{ match8[2].str() }, std::string{ match8[3].str() }, distances, 3, flags);
            } else {
                THROW_OR_ABORT("Could not parse line \"" + line + '"');
            }
        } else if (section == "tobj") {
            // From: https://gtamods.com/wiki/OBJS
            // Type 1
            static const auto reg8 = seq(n, c, n, c, n, c, n, c, n, c, n, c, n, c, n, eof);

            if (SMatch<9> match8; regex_match(line, match8, reg8)) {
                if (match8[4].str() != "1") {
                    THROW_OR_ABORT("Unexpected LOD count in line \"" + line + '"');
                }
                float distances[] = { safe_stof(match8[5].str()) };
                auto flags = (IdeFlags)safe_stoi(match8[6].str());
                set_distances(std::string{ match8[2].str() }, std::string{ match8[3].str() }, distances, 1, flags);
            } else {
                THROW_OR_ABORT("Could not parse line \"" + line + '"');
            }
        }
    }
    if (f->fail() && !f->eof()) {
        THROW_OR_ABORT("Could not read line of file \"" + filename + '"');
    }
}

const IdeItem& DrawDistanceDb::get_item(const std::string& resource_name) const
{
    return ide_items_.get(resource_name);
}

SquaredStepDistances IdeItem::center_distances2(float radius) const
{
    return (raw_center_distances + radius).squared();
}
