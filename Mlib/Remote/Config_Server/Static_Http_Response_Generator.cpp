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
    auto extension = request.public_path.extension();
    
    static const std::map<std::string, std::string> mime_types = {
        {".data", "application/octet-stream"},
        {".js", "text/javascript"},
        {".html", "text/html"},
        {".png", "image/png"},
        {".wasm", "application/wasm"},
    };
    
    auto mime_type = mime_types.find(extension.string());
    if (mime_type == mime_types.end()) {
        return http::status::not_implemented;
    }
    
    beast::http::file_body::value_type body;
    beast::error_code ec;
    
    body.open(request.public_path.string().c_str(), beast::file_mode::scan, ec);
    if (ec.failed()) {
        lerr() << "Could not open file \"" << request.public_path.string() << '"';
        return http::status::not_found;
    }
    
    if (body.size() == 0) {
        lerr() << "File is empty: \"" << request.public_path.string() << '"';
        return http::status::internal_server_error;
    }
    
    return request.respond_with_file(
        std::move(body),
        request.status.value_or(boost::beast::http::status::ok),
        mime_type->second);
}

