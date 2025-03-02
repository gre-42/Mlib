#pragma once
#include <Mlib/Macro_Executor/Asset_Group_And_Id.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <string>
#include <unordered_map>

namespace Mlib {

class AssetReferences;
struct AssetGroupAndId;
class NotifyingJsonMacroArguments;

class Translators {
public:
    explicit Translators(
        const AssetReferences& asset_references,
        NotifyingJsonMacroArguments& globals);
    void set_language_variable(VariableAndHash<std::string> var);
    std::string translate(
        const VariableAndHash<AssetGroupAndId>& gid,
        const VariableAndHash<std::string>& word) const;
private:
    mutable std::unordered_map<
        VariableAndHash<AssetGroupAndId>,
        std::unordered_map<VariableAndHash<std::string>, std::string>> cached_dictionary_;
    mutable FastMutex mutex_;
    VariableAndHash<std::string> language_;
    VariableAndHash<std::string> language_variable_;
    const AssetReferences& asset_references_;
};

}
