#pragma once
#include <Mlib/Macro_Executor/Asset_Group_And_Id.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <string>

namespace Mlib {

class Translators;

class Translator {
public:
    explicit Translator(
        const Translators& translators,
        const VariableAndHash<AssetGroupAndId>& gid);
    std::string translate(const VariableAndHash<std::string>& word) const;
private:
    const Translators& translators_;
    VariableAndHash<AssetGroupAndId> gid_;
};

}
