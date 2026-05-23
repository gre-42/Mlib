#include "Request_Overrides.hpp"
#include <Mlib/Strings/Utf8_Path.hpp>

using namespace Mlib;

RequestOverrides::RequestOverrides(
    #ifdef __EMSCRIPTEN__
    std::shared_ptr<std::vector<std::pair<std::string, std::string>>> headers,
    #else
    std::shared_ptr<std::vector<std::pair<boost::beast::http::field, std::string>>> headers,
    #endif
    boost::beast::http::request<boost::beast::http::string_body> request,
    boost::urls::url_view url_target,
    Utf8Path path_target,
    std::optional<boost::beast::http::status> status)
    : headers{std::move(headers)}
    , request{std::move(request)}
    , url_target{url_target}
    , path_target(std::move(path_target))
    , status{status}
{}

RequestOverrides::~RequestOverrides() = default;
