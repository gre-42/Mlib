#pragma once
#include <chrono>
#include <nlohmann/json.hpp>

// duration[s]
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
    static void to_json(json& j, const std::chrono::duration<Rep, Period>& duration)
    {
        j = std::chrono::duration<double>{duration}.count();
    }
};
}
