#pragma once
#include <Mlib/Misc/Pragma_Clang.hpp>
#include <Mlib/Remote/Config_Server/IHttp_Response_Generator.hpp>
#include <Mlib/Strings/Utf8_Path.hpp>
#include <map>
PRAGMA_CLANG_DIAGNOSTIC_PUSH
PRAGMA_CLANG_DIAGNOSTIC_IGNORED(-Wsign-conversion)
PRAGMA_CLANG_DIAGNOSTIC_IGNORED(-Wimplicit-int-float-conversion)
#include <inja/inja.hpp>
PRAGMA_CLANG_DIAGNOSTIC_POP

namespace Mlib {

struct ReplacementParameterAndFilename;
class MacroLineExecutor;

class GameHttpResponseGenerator: public IHttpResponseGenerator {
public:
    explicit GameHttpResponseGenerator(Utf8Path private_dir);
    virtual ~GameHttpResponseGenerator() override;

    // IHttpResponseGenerator
    virtual ResponseVariant reply(RequestOverrides& request) override;
private:
    ResponseVariant reply_with_index(RequestOverrides& request);
    std::map<std::string, std::function<ResponseVariant(RequestOverrides& request)>> responders_;
    Utf8Path private_dir_;
    std::string remote_secret_;
    inja::Environment env_;
};

}
