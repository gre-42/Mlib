#include "Draw_Distance_Db.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Template_Regex.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Strings/String_View_To_Number.hpp>

using namespace Mlib;
using namespace Mlib::TemplateRegex;

DrawDistanceDb::DrawDistanceDb() = default;

DrawDistanceDb::~DrawDistanceDb() = default;

void DrawDistanceDb::add_ide(const std::string& filename) {
	auto f = create_ifstream(filename);
	if (f->fail()) {
		THROW_OR_ABORT("Could not open \"" + filename + "\" for reading");
	}
	std::string line;
	std::string section;
	while (std::getline(*f, line)) {
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
		auto set_distances = [&](const std::string& resource_name, float dist){
			if (resource_name.starts_with("LOD_")) {
				center_distances_.try_emplace(resource_name + ".dff", dist, INFINITY);
			} else {
				center_distances_.try_emplace(resource_name + ".dff", -INFINITY, dist);
			}
			};
		static const auto c = str(", ");
		static const auto n = group(plus(CharPredicate{ [](char c) {return c != ','; } }));
		if (section == "objs") {
			// From: https://gtamods.com/wiki/OBJS
			// Type 1
			static const auto reg1 = seq(n, c, n, c, n, c, n, c, n, c, n, eof);
			static const auto reg2 = seq(n, c, n, c, n, c, n, c, n, c, n, c, n, eof);
			static const auto reg3 = seq(n, c, n, c, n, c, n, c, n, c, n, c, n, c, n, eof);

			SMatch match;
			if (regex_match(line, match, reg1)) {
				set_distances(std::string{ match[2].str() }, safe_stof(match[5].str()));
			} else if (regex_match(line, match, reg2)) {
				set_distances(std::string{ match[2].str() }, safe_stof(match[5].str()));
			} else if (regex_match(line, match, reg3)) {
				set_distances(std::string{ match[2].str() }, safe_stof(match[5].str()));
			} else {
				THROW_OR_ABORT("Could not parse line \"" + line + '"');
			}
		} else if (section == "tobj") {
			// From: https://gtamods.com/wiki/OBJS
			// Type 1
			static const auto reg1 = seq(n, c, n, c, n, c, n, c, n, c, n, c, n, c, n, eof);

			SMatch match;
			if (regex_match(line, match, reg1)) {
				set_distances(std::string{ match[2].str() }, safe_stof(match[5].str()));
			} else {
				THROW_OR_ABORT("Could not parse line \"" + line + '"');
			}
		}
	}
	if (f->fail() && !f->eof()) {
		THROW_OR_ABORT("Could not read line of file \"" + filename + '"');
	}
}

FixedArray<float, 2> DrawDistanceDb::get_center_distances(
	const std::string& resource_name,
	float radius) const
{
	return maximum(center_distances_.get(resource_name) + radius, 0.f);
}
