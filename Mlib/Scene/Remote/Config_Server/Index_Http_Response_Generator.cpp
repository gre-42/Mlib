#include "Index_Http_Response_Generator.hpp"
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Json/Json_Object_File.hpp>
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
    Utf8Path private_dir)
    : private_dir_(std::move(private_dir))
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

        // static const std::string loop_template =
        //     "<h2>{{ title }}</h2>\n"
        //     "<ul>\n"
        //     "  {% for item in items %}\n"
        //     "    <li>\n"
        //     "      <a href=\"{{ item.key | url_encode }}={{ item.id | url_encode }}\">{{ item.title | escape_html }}</a>\n"
        //     "    </li>\n"
        //     "  {% endfor %}\n"
        //     "</ul>\n";
        // return env_.render(loop_template, current_node);
        static const std::string loop_template =
            "<div>\n"
            "  <h2>{{ title }}</h2>\n"
            "  <select data-items=\"{{ items_path | escape_html }}\"\n"
            "          data-selected_item=\"{{ selection_path | escape_html }}\" size=\"10\">\n"
            "    <option value=\"\">Loading data...</option>\n"
            "  </select>\n"
            "  <div class=\"status\"></div>\n"
            "</div>\n";

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

    responders_["/"] = [this](auto& request){ return reply_with_index(request); };
}

IndexHttpResponseGenerator::~IndexHttpResponseGenerator() = default;

IHttpResponseGenerator::ResponseVariant IndexHttpResponseGenerator::reply(
    RequestOverrides& request)
{
    std::scoped_lock lock{mutex_};
    auto it = responders_.find(request.url_target.buffer());
    if (it == responders_.end()) {
        return std::monostate{};
    }
    return it->second(request);
}

IHttpResponseGenerator::ResponseVariant IndexHttpResponseGenerator::reply_with_index(
    RequestOverrides& request)
{
    nlohmann::json data;
    if (cert_hash_.empty()) {
        data["cert_hash"] = nullptr;
    } else {
        data["cert_hash"] = boost::urls::encode(cert_hash_, boost::urls::unreserved_chars);
    }
    nlohmann::json lists = std::vector<nlohmann::json>(list_paths_.size());
    for (const auto& [i, path] : enumerate(list_paths_)) {
        lists[i] = nlohmann::json{
            {"title", path->mle.eval<std::string>(path->title)},
            {"items_path", path->items_path},
            {"selection_path", path->selection_path}};
    }
    data["config"] = nlohmann::json{{"lists", std::move(lists)}};
    std::string body = env_.render_file(private_dir_ / "server" / "index.inja.html", data);
    return request.respond_with_string(
        std::move(body),
        request.status.value_or(http::status::ok),
        "text/html");
}

IHttpResponseGenerator::ResponseVariant IndexHttpResponseGenerator::reply_with_list(
    NamedList& lst, RequestOverrides& request)
{
    auto items = nlohmann::json::array();
    for (const auto& item : lst.items) {
        if (lst.mle.eval_boolean_expression(item.rp.required.dynamic, item.rp.database)) {
            items.push_back(nlohmann::json{
                {"id", item.rp.id},
                {"title", lst.mle.eval(item.rp.title, item.rp.database)}});
        }
    }
    return request.respond_with_string(
        items.dump(2),
        request.status.value_or(http::status::ok),
        "application/json");
}

IHttpResponseGenerator::ResponseVariant IndexHttpResponseGenerator::reply_with_selection(
    NamedList& lst, RequestOverrides& request)
{
    switch (request.request.method()) {
        case http::verb::put: {
            // Parameters: input, parser_callback, allow_exceptions
            auto j = nlohmann::json::parse(request.request.body(), nullptr, false);
            if (j.is_discarded() || !j.is_string()) {
                return boost::beast::http::status::bad_request;
            }
            auto s = j.get<std::string>();
            auto w = lst.mle.writable_json_macro_arguments();
            w->set(lst.selection_path, s);
            w.unlock_and_notify();
        }
        case http::verb::get: {
            nlohmann::json selected_id = lst.mle.at<std::string>(lst.selection_path);
            return request.respond_with_string(
                selected_id.dump(2),
                request.status.value_or(http::status::ok),
                "application/json");
        }
        default:
            return boost::beast::http::status::method_not_allowed;
    }
}

IHttpResponseGenerator::ResponseVariant IndexHttpResponseGenerator::reply_with_function(
    const std::function<nlohmann::json()>& func, RequestOverrides& request)
{
    nlohmann::json response = func();
    return request.respond_with_string(
        response.dump(2),
        request.status.value_or(http::status::ok),
        "application/json");
}

void IndexHttpResponseGenerator::clear_temporaries() {
    std::scoped_lock lock{mutex_};
    while (!list_paths_.empty()) {
        if (!responders_.erase('/' + list_paths_.back()->items_path)) {
            verbose_abort("Could not erase items endpoint");
        }
        if (!responders_.erase('/' + list_paths_.back()->selection_path)) {
            verbose_abort("Could not erase selection endpoint");
        }
        list_paths_.pop_back();
    }
    while (!function_pathes_.empty()) {
        if (!responders_.erase('/' + function_pathes_.back())) {
            verbose_abort("Could not erase function endpoint");
        }
        function_pathes_.pop_back();
    }
}

void IndexHttpResponseGenerator::add_list(
    std::string items_path,
    std::string selection_path,
    std::string title,
    const std::vector<ReplacementParameterAndFilename>& items,
    const MacroLineExecutor& mle)
{
    linfo() << "Path " << items_path << ": Add " << items.size() << " items";
    auto items_path2 = '/' + items_path;
    auto selection_path2 = '/' + selection_path;
    std::scoped_lock lock{mutex_};
    if (responders_.contains(items_path2)) {
        throw std::runtime_error("Endpoint with name \"" + items_path2 + "\" already exists");
    }
    if (responders_.contains(selection_path2)) {
        throw std::runtime_error("Endpoint with name \"" + selection_path2 + "\" already exists");
    }
    if (items_path == selection_path) {
        throw std::runtime_error("Endpoints have the same name: \"" + items_path + '"');
    }
    std::vector<ReplacementParameterAndFilename> filtered_list;
    filtered_list.reserve(items.size());
    for (const auto& item : items) {
        if (mle.eval_boolean_expression(item.rp.required.fixed, item.rp.database)) {
            filtered_list.emplace_back(item);
        }
    }
    filtered_list.shrink_to_fit();
    auto lst = std::make_shared<NamedList>(
        items_path,
        selection_path,
        mle.eval(title),
        std::move(filtered_list),
        std::move(mle));
    if (!responders_.try_emplace(
        items_path2,
        [this, lst](RequestOverrides& request){ return reply_with_list(*lst, request); }).second)
    {
        verbose_abort("add_list data race (0)");
    }
    if (!responders_.try_emplace(
        selection_path2,
        [this, lst](RequestOverrides& request){ return reply_with_selection(*lst, request); }).second)
    {
        verbose_abort("add_list data race (1)");
    }
    list_paths_.push_back(lst);
}

void IndexHttpResponseGenerator::add_function(
    std::string path,
    std::function<nlohmann::json()> func)
{
    linfo() << "Register endpoint " << path;
    std::scoped_lock lock{mutex_};
    if (!responders_.try_emplace(
        '/' + path,
        [this, f=std::move(func)](RequestOverrides& request){ return reply_with_function(f, request); }).second)
    {
        throw std::runtime_error("Endpoint with name \"" + path + "\" already exists");
    }
    function_pathes_.push_back(path);
}
