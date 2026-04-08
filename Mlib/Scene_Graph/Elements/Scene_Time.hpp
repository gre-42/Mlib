#pragma once
#include <chrono>
#include <variant>

namespace Mlib {

enum class SceneTimeType {
    NONE = 0,
    INITIAL_NO_TIME = 1 << 0,
    INITIAL_WITH_TIME = 1 << 1,
    SUCCESSOR = 1 << 2,
    STANDARD = 1 << 3,

    INITIAL = INITIAL_NO_TIME | INITIAL_WITH_TIME,
    WITH_TIME = INITIAL_WITH_TIME | STANDARD
};

inline bool any(SceneTimeType type) {
    return type != SceneTimeType::NONE;
}

inline SceneTimeType operator & (SceneTimeType a, SceneTimeType b) {
    return (SceneTimeType)((int)a & (int)b);
}

class SceneTime {
public:
    static inline SceneTime initial() {
        return {SceneTimeType::INITIAL_NO_TIME, std::chrono::steady_clock::time_point()};
    }
    static inline SceneTime initial(std::chrono::steady_clock::time_point time) {
        return {SceneTimeType::INITIAL_WITH_TIME, time};
    }
    static inline SceneTime successor() {
        return {SceneTimeType::SUCCESSOR, std::chrono::steady_clock::time_point()};
    }
    static inline SceneTime standard(std::chrono::steady_clock::time_point time) {
        return {SceneTimeType::STANDARD, time};
    }
    inline SceneTimeType type() const {
        return type_;
    }
    inline std::chrono::steady_clock::time_point time() const {
        return time_;
    }
private:
    inline SceneTime(SceneTimeType type, std::chrono::steady_clock::time_point time)
        : type_{type}, time_{time}
    {}
    SceneTimeType type_;
    std::chrono::steady_clock::time_point time_;
};

}