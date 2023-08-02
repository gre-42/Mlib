#pragma once
#include <list>
#include <map>
#include <memory>
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

    bool contains(const std::string& group) const;
    void add(const std::string& group);

    const AssetGroupReplacementParameters& get(const std::string& group) const;
    AssetGroupReplacementParameters& get(const std::string& group);

private:
    std::map<std::string, AssetGroupReplacementParameters> replacement_parameters_;
    mutable std::shared_mutex mutex_;
};

}
