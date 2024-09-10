#pragma once
#include <Mlib/Hash.hpp>
#include <Mlib/Std_Hash.hpp>
#include <compare>

namespace Mlib {

template <class T>
class VariableAndHash {
public:
    VariableAndHash& operator = (const T& other) {
        *this = VariableAndHash<T>(other);
        return *this;
    }
    VariableAndHash& operator = (T&& other) {
        *this = VariableAndHash<T>(std::move(other));
        return *this;
    }
    VariableAndHash()
        : variable_()
        , hash_{ hash_combine(variable_) }
    {}
    explicit VariableAndHash(const T& v)
        : variable_{ v }
        , hash_{ hash_combine(variable_) }
    {}
    explicit VariableAndHash(T&& v)
        : variable_{ std::move(v) }
        , hash_{ hash_combine(variable_) }
    {}
    VariableAndHash(const VariableAndHash<const T>& other)
        : variable_{ other.variable_ }
        , hash_{ other.hash_ }
    {}
    VariableAndHash(VariableAndHash<const T>&& other)
        : variable_{ std::move(other.variable_) }
        , hash_{ other.hash_ }
    {}
    template <class Arg>
        requires std::is_convertible_v<Arg, const T&>
    explicit VariableAndHash(Arg&& value)
        : variable_{ std::forward<Arg>(value) }
        , hash_{ hash_combine(variable_) }
    {}
    const T& operator * () const {
        return variable_;
    }
    const T* operator -> () const {
        return &variable_;
    }
    // operator const T&() const {
    //     return variable_;
    // }
    std::strong_ordering operator <=> (const VariableAndHash& other) const {
        return hash_ <=> other.hash_;
    }
    bool operator == (const VariableAndHash& other) const {
        return hash_ == other.hash_;
    }
    size_t hash() const {
        return hash_;
    }
    template <class Archive>
    void serialize(Archive& archive) {
        archive(variable_);
        archive(hash_);
    }
private:
    T variable_;
    size_t hash_;
};

}

template <class T>
struct std::hash<Mlib::VariableAndHash<T>>
{
    std::size_t operator() (const Mlib::VariableAndHash<T>& k) const {
        return k.hash();
    }
};
