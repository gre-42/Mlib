#include "Static_Http_Response_Generator.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Remote/Config_Server/Request_Overrides.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/message_generator.hpp>
#include <map>

using namespace Mlib;

namespace beast = boost::beast;
namespace http = beast::http;

StaticHttpResponseGenerator::StaticHttpResponseGenerator() = default;

StaticHttpResponseGenerator::~StaticHttpResponseGenerator() = default;

IHttpResponseGenerator::ResponseVariant StaticHttpResponseGenerator::reply(
    RequestOverrides& request)
{
    auto extension = request.path_target.extension();
    static const std::map<std::string, std::string> mime_types = {
        {".html", "text/html"},
        {".js", "text/javascript"},
        {".data", "application/octet-stream"},
        {".wasm", "application/wasm"},
    };
    auto mime_type = mime_types.find(extension);
    if (mime_type == mime_types.end()) {
        return http::status::not_implemented;
    }
    http::file_body::value_type body;
    beast::error_code ec;
    body.open(request.path_target.c_str(), beast::file_mode::scan, ec);
    if (ec.failed()) {
        lerr() << "Could not open file \"" << request.path_target.string() << '"';
        return http::status::not_found;
    }
    if (body.size() == 0) {
        lerr() << "File is empty: \"" << request.path_target.string() << '"';
        return http::status::internal_server_error;
    }
    http::response<http::file_body> res{
        request.status.value_or(http::status::ok), 
        request.request.version()
    };
    res.set(http::field::content_type, mime_type->second);
    res.keep_alive(request.request.keep_alive()); 
    request.add_headers(res);
    res.body() = std::move(body);
    res.content_length(res.body().size());
    res.prepare_payload();
    return boost::beast::http::message_generator(std::move(res));
}
