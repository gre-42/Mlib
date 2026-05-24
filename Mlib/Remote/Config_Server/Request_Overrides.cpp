#include "Request_Overrides.hpp"
#include <Mlib/Strings/Utf8_Path.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/message_generator.hpp>
#include <boost/beast/http/string_body.hpp>

using namespace Mlib;

RequestOverrides::RequestOverrides(
    #ifdef __EMSCRIPTEN__
    std::shared_ptr<std::vector<std::pair<std::string, std::string>>> headers,
    #else
    std::shared_ptr<std::vector<std::pair<boost::beast::http::field, std::string>>> headers,
    #endif
    boost::beast::http::request<boost::beast::http::string_body> request,
    boost::urls::url_view url_target,
    Utf8Path public_path,
    std::optional<boost::beast::http::status> status)
    : headers{std::move(headers)}
    , request{std::move(request)}
    , url_target{url_target}
    , public_path(std::move(public_path))
    , status{status}
{}

RequestOverrides::~RequestOverrides() = default;

boost::beast::http::message_generator RequestOverrides::respond_with_string(
    std::string body,
    boost::beast::http::status status,
    boost::beast::string_view content_type) const // Geändert zu boost::beast::string_view
{
    boost::beast::http::response<boost::beast::http::string_body> res{
        status, 
        this->request.version()
    };
    res.set(boost::beast::http::field::content_type, content_type);
    res.keep_alive(this->request.keep_alive()); 
    this->add_headers(res);
    res.body() = std::move(body);
    res.content_length(res.body().size());
    res.prepare_payload();
    return boost::beast::http::message_generator(std::move(res));
}

boost::beast::http::message_generator RequestOverrides::respond_with_file(
        boost::beast::http::file_body::value_type body,
        boost::beast::http::status status,
        boost::beast::string_view content_type) const // std::string verhindert Boost-Typkonflikte
{
    boost::beast::http::response<boost::beast::http::file_body> res{
        status, 
        this->request.version()
    };
    res.set(boost::beast::http::field::content_type, content_type);
    res.keep_alive(this->request.keep_alive()); 
    this->add_headers(res);
    res.body() = std::move(body);
    res.content_length(res.body().size());
    res.prepare_payload();
    
    // Das Zurückgeben als message_generator sorgt dafür, dass Beast die Datei 
    // im asynchronen Schreib-Loop der HttpSession stückweise (non-blocking) versendet!
    return boost::beast::http::message_generator(std::move(res));
}
