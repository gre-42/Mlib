#pragma once
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Misc/Pragma_Clang.hpp>
#include <Mlib/Remote/Config_Server/IHttp_Response_Generator.hpp>
#include <Mlib/Strings/Utf8_Path.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <functional>
#include <list>
#include <memory>
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
    std::string items_path;
    std::string selection_path;
    std::string title;
    std::vector<ReplacementParameterAndFilename> items;
    MacroLineExecutor mle;
};

class IndexHttpResponseGenerator: public IHttpResponseGenerator {
public:
    explicit IndexHttpResponseGenerator(Utf8Path private_dir);
    virtual ~IndexHttpResponseGenerator() override;

    // IHttpResponseGenerator
    virtual ResponseVariant reply(RequestOverrides& request) override;

    // Misc
    void clear_temporaries();
    void add_list(
        std::string items_path,
        std::string selection_path,
        std::string title,
        const std::vector<ReplacementParameterAndFilename>& items,
        const MacroLineExecutor& mle);
    void add_function(
        std::string path,
        std::function<nlohmann::json()> func);
private:
    ResponseVariant reply_with_index(RequestOverrides& request);
    ResponseVariant reply_with_list(NamedList& lst, RequestOverrides& request);
    ResponseVariant reply_with_selection(NamedList& lst, RequestOverrides& request);
    ResponseVariant reply_with_function(const std::function<nlohmann::json()>& func, RequestOverrides& request);
    std::map<std::string, std::function<ResponseVariant(RequestOverrides& request)>> responders_;
    std::list<std::shared_ptr<NamedList>> list_paths_;
    std::list<std::string> function_pathes_;
    Utf8Path private_dir_;
    mutable FastMutex mutex_;
    std::string cert_hash_;
    inja::Environment env_;
};

}
