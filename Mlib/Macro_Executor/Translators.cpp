#include "Translators.hpp"
#include <Mlib/Macro_Executor/Asset_Group_And_Id.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Variable_And_Hash.hpp>

using namespace Mlib;

Translators::Translators(
    const AssetReferences& asset_references,
    NotifyingJsonMacroArguments& globals)
    : asset_references_{ asset_references }
{
    globals.add_observer([this, &globals](){
        if (language_variable_->empty()) {
            return;
        }
        auto j = globals.json_macro_arguments();
        auto& j1 = (const JsonMacroArguments&)j;
        auto l = j1.try_at<std::string>(*language_variable_);
        if (!l.has_value()) {
            return;
        }
        auto lang = VariableAndHash{ *l };
        if (language_ != lang) {
            std::scoped_lock lock{ mutex_ };
            language_ = lang;
            cached_dictionary_.clear();
        }
    });
}

void Translators::set_language_variable(VariableAndHash<std::string> var) {
    std::scoped_lock lock{ mutex_ };
    if (!language_variable_->empty()) {
        THROW_OR_ABORT("Language variable already set");
    }
    language_variable_ = std::move(var);
}

std::string Translators::translate(
    const VariableAndHash<AssetGroupAndId>& gid,
    const VariableAndHash<std::string>& word) const
{
    std::scoped_lock lock{ mutex_ };
    if (language_->empty()) {
        return "??" + *word + "??";
    }
    auto& dict = cached_dictionary_[gid];
    auto it = dict.find(word);
    if (it == dict.end()) {
        const auto& j = asset_references_[*gid->group].at(*gid->id).rp.database;
        return JsonView{JsonView{j}.at(*word)}.at<std::string>(*language_);
    }
    return it->second;
}
