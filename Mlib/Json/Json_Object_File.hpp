#pragma once
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Strings/Utf8_Path.hpp>
#include <Mlib/Threads/Lockable_Object.hpp>
#include <nlohmann/json.hpp>

namespace Mlib {

class JsonObjectFile: public JsonView {
public:
    JsonObjectFile();
    ~JsonObjectFile();
    void load_from_file(const Utf8Path& filename);
private:
    nlohmann::json j_;
};

using LockableJsonObjectFile = LockableObject<JsonObjectFile>;

}
