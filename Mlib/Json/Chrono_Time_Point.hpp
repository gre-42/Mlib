#pragma once
#include <chrono>
#include <nlohmann/json.hpp>

// std::chrono::steady_clock::time_point
namespace nlohmann
{
template <>
struct adl_serializer<std::chrono::steady_clock::time_point>
{
    static void from_json(const nlohmann::json& j, std::chrono::steady_clock::time_point& time_point)
    {
        time_point = std::chrono::steady_clock::time_point{std::chrono::steady_clock::duration{j.get<std::chrono::steady_clock::rep>()}};
    }
    static void to_json(json& j, const std::chrono::steady_clock::time_point& tp)
    {
        j = tp.time_since_epoch().count();
    }
};
}
