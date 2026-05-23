#include "Index_Http_Response_Generator.hpp"
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Json/Json_Object_File.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Os/Env.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Remote/Config_Server/Request_Overrides.hpp>
#include <Mlib/Strings/Base64.hpp>
#include <Mlib/Strings/Escape_Html.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/message_generator.hpp>
#include <boost/url/encode.hpp>
#include <boost/url/rfc/unreserved_chars.hpp>
#include <mutex>

using namespace Mlib;

namespace beast = boost::beast;
namespace http = beast::http;

IndexHttpResponseGenerator::IndexHttpResponseGenerator(
    Utf8Path static_dir)
    : static_dir_(std::move(static_dir))
{
    cert_hash_ = []() -> std::string {
        auto filename = try_getenv("CERT_HASH_FILENAME");
        if (filename.has_value()) {
            linfo() << "Reading CERT_HASH_FILENAME";
            JsonObjectFile obj;
            obj.load_from_file(*filename);
            auto s = obj.at<std::string>("hash");
            auto v = decode_base64(s);
            if (v.size() != 32) {
                throw std::runtime_error("Cert hash does not have 32 bytes");
            }
            linfo() << "CERT_HASH_FILENAME has been read";
            return s;
        }
        linfo() << "CERT_HASH_FILENAME not set";
        return "";
    }();

    env_.add_callback("escape_html", 1, [](inja::Arguments& args) {
        std::string raw_string = args.at(0)->get<std::string>();
        return escape_html(raw_string);
    });

    env_.add_callback("url_encode", 1, [](inja::Arguments& args) {
        std::string raw_string = args.at(0)->get<std::string>();
        return boost::urls::encode(raw_string, boost::urls::unreserved_chars);
    });

    env_.add_callback("render_list", 1, [this](inja::Arguments& args) {
        // Get the unnamed argument at position 0
        auto current_node = args.at(0)->get<nlohmann::json>();

        static const std::string loop_template =
            "<h2>{{ title }}</h2>\n"
            "<ul>\n"
            "  {% for item in items %}\n"
            "    <li>\n"
            "      <a href=\"{{ item.key | url_encode }}={{ item.id | url_encode }}\">{{ item.title | escape_html }}</a>\n"
            "    </li>\n"
            "  {% endfor %}\n"
            "</ul>\n";

        return env_.render(loop_template, current_node);
    });

    env_.add_callback("render_lists", 1, [this](inja::Arguments& args) {
        // Get the unnamed argument at position 0
        auto current_node = args.at(0)->get<nlohmann::json>();

        static const std::string loop_template =
            "{% for items in lists %}\n"
            "  {{ items | render_list }}\n"
            "{% endfor %}\n";

        return env_.render(loop_template, current_node);
    });
}

IndexHttpResponseGenerator::~IndexHttpResponseGenerator() = default;

IHttpResponseGenerator::ResponseVariant IndexHttpResponseGenerator::reply(
    RequestOverrides& request)
{
    if (request.url_target.buffer() != "/") {
        return std::monostate{};
    }
    nlohmann::json data;
    data["cert_hash"] = boost::urls::encode(cert_hash_, boost::urls::unreserved_chars);
    {
        std::scoped_lock lock{mutex_};
        nlohmann::json lists = std::vector<nlohmann::json>(lists_.size());
        for (const auto& [i, lst] : enumerate(lists_)) {
            lists[i]["title"] = lst.title;
            lists[i]["items"] = lst.items;
        }
        data["config"] = nlohmann::json::object();
        data["config"]["lists"] = lists;
    }
    std::string body = env_.render_file(static_dir_ / "server" / "index.inja", data);
    http::response<http::string_body> res{
        request.status.value_or(http::status::ok), 
        request.request.version()
    };
    res.set(http::field::content_type, "text/html");
    res.keep_alive(request.request.keep_alive()); 
    request.add_headers(res);
    res.body() = std::move(body);
    res.content_length(res.body().size());
    res.prepare_payload();
    return boost::beast::http::message_generator(std::move(res));
}

void IndexHttpResponseGenerator::clear_lists() {
    std::scoped_lock lock{mutex_};
    lists_.clear();
}

void IndexHttpResponseGenerator::add_list(
    std::string title,
    const std::vector<ReplacementParameterAndFilename>& items,
    const MacroLineExecutor& mle)
{
    linfo() << "Add " << items.size() << " levels";
    std::scoped_lock lock{mutex_};
    NamedList nls{
        .title = mle.eval(title)
    };
    nls.items = std::vector<nlohmann::json>(items.size());
    for (size_t i = 0; i < items.size(); ++i) {
        nls.items[i]["title"] = mle.eval(items[i].rp.title, items[i].rp.database);
        nls.items[i]["key"] = nls.key;
        nls.items[i]["id"] = items[i].rp.id;
    }
    lists_.emplace_back(std::move(nls));
}
