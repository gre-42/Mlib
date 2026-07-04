#include "Game_Http_Response_Generator.hpp"
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Os/Env.hpp>
#include <Mlib/Remote/Config_Server/Request_Overrides.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/message_generator.hpp>
#include <boost/url/encode.hpp>
#include <boost/url/rfc/unreserved_chars.hpp>
#include <mutex>

using namespace Mlib;

namespace beast = boost::beast;
namespace http = beast::http;

GameHttpResponseGenerator::GameHttpResponseGenerator(
    Utf8Path private_dir)
    : private_dir_(std::move(private_dir))
{
    remote_secret_ = getenv_default("REMOTE_SECRET", "");
    responders_["/client/game"] = [this](auto& request){ return reply_with_index(request); };
}

GameHttpResponseGenerator::~GameHttpResponseGenerator() = default;

IHttpResponseGenerator::ResponseVariant GameHttpResponseGenerator::reply(
    RequestOverrides& request)
{
    auto it = responders_.find(request.url_target.buffer());
    if (it == responders_.end()) {
        return std::monostate{};
    }
    return it->second(request);
}

IHttpResponseGenerator::ResponseVariant GameHttpResponseGenerator::reply_with_index(
    RequestOverrides& request)
{
    nlohmann::json data;
    if (remote_secret_.empty()) {
        data["remote_secret"] = nullptr;
    } else {
        data["remote_secret"] = boost::urls::encode(remote_secret_, boost::urls::unreserved_chars);
    }
    std::string body = env_.render_file(private_dir_ / "client" / "game.inja.html", data);
    return request.respond_with_string(
        std::move(body),
        request.status.value_or(http::status::ok),
        "text/html");
}
