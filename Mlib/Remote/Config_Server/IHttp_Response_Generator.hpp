#pragma once
#include <boost/beast/http/message_fwd.hpp>
#include <boost/beast/http/message_generator_fwd.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body_fwd.hpp>
#include <optional>
#include <variant>

namespace Mlib {

class RequestOverrides;

class IHttpResponseGenerator {
public:
    using ResponseVariant = std::variant<std::monostate, boost::beast::http::message_generator, boost::beast::http::status>;
    virtual ~IHttpResponseGenerator() = default;
    virtual ResponseVariant reply(RequestOverrides& request) = 0;
};

}
