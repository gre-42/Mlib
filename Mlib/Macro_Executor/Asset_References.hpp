#pragma once
#include <list>
#include <map>
#include <shared_mutex>
#include <string>

namespace Mlib {

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

    const AssetGroupReplacementParameters& get_replacement_parameters(
        const std::string& group) const;
    AssetGroupReplacementParameters& get_replacement_parameters(
        const std::string& group);

private:
    std::map<std::string, AssetGroupReplacementParameters> replacement_parameters_;
    mutable std::shared_mutex mutex_;
};

}
