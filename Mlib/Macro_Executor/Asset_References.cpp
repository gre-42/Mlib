#include "Asset_References.hpp"
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Macro_Manifest.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

AssetReferences::AssetReferences() = default;

AssetReferences::~AssetReferences() = default;

void AssetReferences::add_macro_manifest_group(const std::string& group) {
    std::scoped_lock lock{mutex_};
    if (!macro_manifests_.insert({group, {}}).second) {
        THROW_OR_ABORT("Macro manifest group \"" + group + "\" already exists");
    }
}

void AssetReferences::add_replacement_parameter_group(const std::string& group) {
    std::scoped_lock lock{mutex_};
    if (!replacement_parameters_.try_emplace(group).second) {
        THROW_OR_ABORT("Replacement parameter group \"" + group + "\" already exists");
    }
}

void AssetReferences::add_macro_manifest(
    const std::string& group,
    const std::string& filename)
{
    std::scoped_lock lock{mutex_};
    auto it = macro_manifests_.find(group);
    if (it == macro_manifests_.end()) {
        THROW_OR_ABORT("Could not find macro manifest group \"" + group + '"');
    }
    it->second.push_back(MacroManifestAndFilename{
        .filename = filename,
        .manifest = MacroManifest::load_from_json(filename)});
}

void AssetReferences::sort_macro_manifests(const std::string& group) {
    auto& lst = const_cast<std::list<MacroManifestAndFilename>&>(get_macro_manifests(group));
    lst.sort();
}

const std::list<MacroManifestAndFilename>& AssetReferences::get_macro_manifests(
    const std::string& group) const
{
    std::shared_lock lock{mutex_};
    auto it = macro_manifests_.find(group);
    if (it == macro_manifests_.end()) {
        THROW_OR_ABORT("Could not find macro manifest group \"" + group + '"');
    }
    return it->second;
}

const AssetGroupReplacementParameters& AssetReferences::get_replacement_parameters(
    const std::string& group) const
{
    std::shared_lock lock{mutex_};
    auto it = replacement_parameters_.find(group);
    if (it == replacement_parameters_.end()) {
        THROW_OR_ABORT("Could not find replacement parameter group \"" + group + '"');
    }
    return it->second;
}

AssetGroupReplacementParameters& AssetReferences::get_replacement_parameters(
    const std::string& group)
{
    const AssetReferences& a = *this;
    return const_cast<AssetGroupReplacementParameters&>(a.get_replacement_parameters(group));
}
