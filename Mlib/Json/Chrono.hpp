#pragma once
#include <chrono>
#include <nlohmann/json.hpp>

namespace nlohmann
{
template <typename Rep, typename Period>
struct adl_serializer<std::chrono::duration<Rep, Period>>
{
static void from_json(const nlohmann::json& j, std::chrono::duration<Rep, Period>& duration)
{
    duration = std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(
        std::chrono::duration<double>{j.get<double>()});
}
};
}
