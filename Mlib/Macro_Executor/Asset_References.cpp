#include "Asset_References.hpp"
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Macro_Manifest.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

AssetReferences::AssetReferences() = default;

AssetReferences::~AssetReferences() = default;

void AssetReferences::add_macro_manifest_group(const std::string& group) {
    std::unique_lock lock{mutex_};
    if (!macro_manifests_.insert({group, {}}).second) {
        THROW_OR_ABORT("Macro manifest group \"" + group + "\" already exists");
    }
}

void AssetReferences::add_replacement_parameter_group(const std::string& group) {
    std::unique_lock lock{mutex_};
    if (!replacement_parameters_.insert({group, {}}).second) {
        THROW_OR_ABORT("Replacement parameter group \"" + group + "\" already exists");
    }
}

void AssetReferences::add_macro_manifest(
    const std::string& group,
    const std::string& filename)
{
    std::unique_lock lock{mutex_};
    auto it = macro_manifests_.find(group);
    if (it == macro_manifests_.end()) {
        THROW_OR_ABORT("Could not find macro manifest group \"" + group + '"');
    }
    it->second.push_back(MacroManifestAndFilename{
        .filename = filename,
        .manifest = MacroManifest::from_json(filename)});
}

void AssetReferences::add_replacement_parameter(
    const std::string& group,
    const std::string& filename,
    const MacroLineExecutor& mle)
{
    std::unique_lock lock{mutex_};
    auto it = replacement_parameters_.find(group);
    if (it == replacement_parameters_.end()) {
        THROW_OR_ABORT("Could not find replacement parameter group \"" + group + '"');
    }
    auto rp = ReplacementParameter::from_json(filename);
    auto mlecd = mle.changed_script_filename(filename);
    for (const auto& l : rp.on_init) {
        mlecd(l, nullptr);
    }
    it->second.push_back(rp);
}

void AssetReferences::sort_macro_manifests(const std::string& group) {
    auto& lst = const_cast<std::list<MacroManifestAndFilename>&>(get_macro_manifests(group));
    lst.sort();
}

void AssetReferences::sort_replacement_parameters(const std::string& group) {
    auto& lst = const_cast<std::list<ReplacementParameter>&>(get_replacement_parameters(group));
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

const std::list<ReplacementParameter>& AssetReferences::get_replacement_parameters(
    const std::string& group) const
{
    std::shared_lock lock{mutex_};
    auto it = replacement_parameters_.find(group);
    if (it == replacement_parameters_.end()) {
        THROW_OR_ABORT("Could not find replacement parameter group \"" + group + '"');
    }
    return it->second;
}
