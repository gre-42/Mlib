#pragma once
#include <Mlib/Remote/Config_Server/IHttp_Response_Generator.hpp>

namespace Mlib {

class StaticHttpResponseGenerator: public IHttpResponseGenerator {
public:
    StaticHttpResponseGenerator();
    virtual ~StaticHttpResponseGenerator() override;
    virtual ResponseVariant reply(RequestOverrides& request) override;
};

}
