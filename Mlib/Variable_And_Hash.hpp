#pragma once
#include <Mlib/Hash.hpp>
#include <compare>

namespace cereal {

template <class T>
class construct;

}

namespace Mlib {

template <class T>
class VariableAndHash {
public:
    VariableAndHash()
        : variable_()
        , hash_{ hash_combine(variable_) }
    {}
    template <class Arg>
        requires std::is_convertible_v<Arg, const T&>
    VariableAndHash(Arg&& value)
        : variable_{ std::forward<Arg>(value) }
        , hash_{ hash_combine(variable_) }
    {}
    const T& operator * () const {
        return variable_;
    }
    const T* operator -> () const {
        return &variable_;
    }
    operator const T&() const {
        return variable_;
    }
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
    }
    template<typename Archive>
    static void load_and_construct(
        Archive &archive,
        cereal::construct<VariableAndHash> &construct)
    {
        T variable;
        archive(variable);
        construct(variable);
    }
private:
    T variable_;
    size_t hash_;
};

}

namespace std {

template <class Key>
struct hash;

}

template <class T>
struct std::hash<Mlib::VariableAndHash<T>>
{
    std::size_t operator() (const Mlib::VariableAndHash<T>& k) const {
        return k.hash();
    }
};
