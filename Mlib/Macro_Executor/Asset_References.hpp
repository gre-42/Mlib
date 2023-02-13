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
class RegexSubstitutionCache;

class AssetReferences {
public:
    AssetReferences();
    ~AssetReferences();

    void add_macro_manifest_group(const std::string& group);
    void add_replacement_parameter_group(const std::string& group);

    void add_macro_manifest(
        const std::string& group,
        const std::string& filename);
    void add_replacement_parameter(
        const std::string& group,
        const std::string& filename,
        const MacroLineExecutor& mle,
        const RegexSubstitutionCache& rsc);
    
    void sort_macro_manifests(const std::string& group);
    void sort_replacement_parameters(const std::string& group);

    const std::list<MacroManifestAndFilename>& get_macro_manifests(
        const std::string& group) const;
    const std::list<ReplacementParameter>& get_replacement_parameters(
        const std::string& group) const;

private:
    std::map<std::string, std::list<MacroManifestAndFilename>> macro_manifests_;
    std::map<std::string, std::list<ReplacementParameter>> replacement_parameters_;
    mutable std::shared_mutex mutex_;
};

}
