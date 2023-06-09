#pragma once
#include <list>
#include <map>
#include <shared_mutex>
#include <string>

namespace Mlib {

struct MacroManifest;
struct MacroManifestAndFilename;
struct ReplacementParameter;
class MacroLineExecutor;
class AssetGroupReplacementParameters;

class AssetReferences {
public:
    AssetReferences();
    ~AssetReferences();

    void add_macro_manifest_group(const std::string& group);
    void add_replacement_parameter_group(const std::string& group);

    void add_macro_manifest(
        const std::string& group,
        const std::string& filename);

    void sort_macro_manifests(const std::string& group);

    const std::list<MacroManifestAndFilename>& get_macro_manifests(
        const std::string& group) const;
    const AssetGroupReplacementParameters& get_replacement_parameters(
        const std::string& group) const;
    AssetGroupReplacementParameters& get_replacement_parameters(
        const std::string& group);

private:
    std::map<std::string, std::list<MacroManifestAndFilename>> macro_manifests_;
    std::map<std::string, AssetGroupReplacementParameters> replacement_parameters_;
    mutable std::shared_mutex mutex_;
};

}
