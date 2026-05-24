#pragma once
#include <Mlib/Strings/Utf8_Path.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/message_generator_fwd.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/url/parse.hpp>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Mlib {

class RequestOverrides {
    RequestOverrides(const RequestOverrides&) = delete;
    RequestOverrides& operator = (const RequestOverrides&) = delete;
public:
    RequestOverrides(
        #ifdef __EMSCRIPTEN__
        std::shared_ptr<std::vector<std::pair<std::string, std::string>>> headers,
        #else
        std::shared_ptr<std::vector<std::pair<boost::beast::http::field, std::string>>> headers,
        #endif
        boost::beast::http::request<boost::beast::http::string_body> request,
        boost::urls::url_view url_target,
        Utf8Path public_path,
        std::optional<boost::beast::http::status> status = boost::beast::http::status::ok);
    ~RequestOverrides();
    #ifdef __EMSCRIPTEN__
    std::shared_ptr<std::vector<std::pair<std::string, std::string>>> headers;
    #else
    std::shared_ptr<std::vector<std::pair<boost::beast::http::field, std::string>>> headers;
    #endif
    boost::beast::http::request<boost::beast::http::string_body> request;
    boost::urls::url_view url_target;
    Utf8Path public_path;
    std::optional<boost::beast::http::status> status;
    template <class TResponse>
    void add_headers(TResponse& response) const {
        if (headers != nullptr) {
            for (const auto& [field, value] : *headers) {
                response.set(field, value);
            }
        }
    }
    boost::beast::http::message_generator respond_with_string(
        std::string body,
        boost::beast::http::status status,
        boost::beast::string_view content_type) const;
    boost::beast::http::message_generator respond_with_file(
        boost::beast::http::file_body::value_type body,
        boost::beast::http::status status,
        boost::beast::string_view content_type) const;
};

}
