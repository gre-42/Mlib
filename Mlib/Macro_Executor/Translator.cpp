#include "Translator.hpp"
#include <Mlib/Macro_Executor/Translators.hpp>

using namespace Mlib;

Translator::Translator(
    const Translators& translators,
    const VariableAndHash<AssetGroupAndId>& gid)
    : translators_{ translators }
    , gid_{ gid }
{}

std::string Translator::translate(const VariableAndHash<std::string>& word) const {
    return translators_.translate(gid_, word);
}
