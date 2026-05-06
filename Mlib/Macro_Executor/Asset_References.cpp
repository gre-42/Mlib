#include "Asset_References.hpp"
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <mutex>
#include <stdexcept>

using namespace Mlib;

AssetReferences::AssetReferences() = default;

AssetReferences::~AssetReferences() = default;

bool AssetReferences::contains(const std::string& group) const {
    std::shared_lock lock{mutex_};
    return replacement_parameters_.contains(group);
}

void AssetReferences::add(const std::string& group) {
    std::scoped_lock lock{mutex_};
    if (!replacement_parameters_.try_emplace(group).second) {
        throw std::runtime_error("Replacement parameter group \"" + group + "\" already exists");
    }
}

const AssetGroupReplacementParameters& AssetReferences::operator [] (const std::string& group) const
{
    std::shared_lock lock{mutex_};
    auto it = replacement_parameters_.find(group);
    if (it == replacement_parameters_.end()) {
        throw std::runtime_error("Could not find replacement parameter group \"" + group + '"');
    }
    return it->second;
}

AssetGroupReplacementParameters& AssetReferences::operator [] (const std::string& group)
{
    const AssetReferences& a = *this;
    return const_cast<AssetGroupReplacementParameters&>(a[group]);
}
