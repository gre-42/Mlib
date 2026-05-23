#pragma once
#include <Mlib/Misc/Pragma_Clang.hpp>
#include <Mlib/Remote/Config_Server/IHttp_Response_Generator.hpp>
#include <Mlib/Strings/Utf8_Path.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <list>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>
PRAGMA_CLANG_DIAGNOSTIC_PUSH
PRAGMA_CLANG_DIAGNOSTIC_IGNORED(-Wsign-conversion)
PRAGMA_CLANG_DIAGNOSTIC_IGNORED(-Wimplicit-int-float-conversion)
#include <inja/inja.hpp>
PRAGMA_CLANG_DIAGNOSTIC_POP

namespace Mlib {

struct ReplacementParameterAndFilename;
class MacroLineExecutor;

struct NamedList {
    std::string key;
    std::string title;
    nlohmann::json items;
};

class IndexHttpResponseGenerator: public IHttpResponseGenerator {
public:
    explicit IndexHttpResponseGenerator(Utf8Path static_dir);
    virtual ~IndexHttpResponseGenerator() override;

    // IHttpResponseGenerator
    virtual ResponseVariant reply(RequestOverrides& request) override;

    // Misc
    void clear_lists();
    void add_list(
        std::string title,
        const std::vector<ReplacementParameterAndFilename>& items,
        const MacroLineExecutor& mle);
private:
    Utf8Path static_dir_;
    mutable FastMutex mutex_;
    std::list<NamedList> lists_;
    std::string cert_hash_;
    inja::Environment env_;
};

}
