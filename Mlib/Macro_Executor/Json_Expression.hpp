#pragma once
#include <Mlib/Json/Misc.hpp>
#include <string>

namespace Mlib {

class AssetReferences;
class JsonView;

nlohmann::json eval(
    std::string_view expression,
    const JsonView& globals,
    const JsonView& locals,
    const JsonView& block,
    const AssetReferences& asset_references);

nlohmann::json eval(
    std::string_view expression,
    const JsonView& globals,
    const JsonView& locals,
    const AssetReferences& asset_references);

nlohmann::json eval(
    std::string_view expression,
    const JsonView& variables,
    const AssetReferences& asset_references);

nlohmann::json eval(
    std::string_view expression,
    const JsonView& variables);

template <class T>
T eval(
    std::string_view expression,
    const JsonView& globals,
    const JsonView& locals,
    const JsonView& block,
    const AssetReferences& asset_references);

template <class T>
T eval(
    std::string_view expression,
    const JsonView& globals,
    const JsonView& locals,
    const AssetReferences& asset_references);

template <class T>
T eval(
    std::string_view expression,
    const JsonView& variables,
    const AssetReferences& asset_references);

template <class T>
T eval(
    std::string_view expression,
    const JsonView& variables);

}
