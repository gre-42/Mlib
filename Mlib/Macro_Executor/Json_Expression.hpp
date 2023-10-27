#pragma once
#include <Mlib/Json/Misc.hpp>
#include <string>

namespace Mlib {

class AssetReferences;
class JsonView;

nlohmann::json eval(
    const std::string& expression,
    const JsonView& globals,
    const JsonView& locals,
    const AssetReferences& asset_references);

nlohmann::json eval(
    const std::string& expression,
    const JsonView& variables);

template <class T>
T eval(
    const std::string& expression,
    const JsonView& globals,
    const JsonView& locals,
    const AssetReferences& asset_references);

template <class T>
T eval(
    const std::string& expression,
    const JsonView& variables,
    const AssetReferences& asset_references);

}
