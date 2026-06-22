#pragma once
#include <Mlib/Array/Non_Copying_Vector.hpp>
#include <Mlib/Type_Traits/Shared_Ptr.hpp>
#include <concepts>
#include <type_traits>

namespace Mlib {

class SerializationContextRead;
class SerializationContextWrite;
class BinaryBitwiseWordsReader;
class BinaryBitwiseWordsWriter;

template <typename T>
class ConstructInplace {
public:
    explicit ConstructInplace(ObjectBlob<T>& v): v_{v} {}
    void operator () (auto&&... args) {
        new(v_.data) T(std::forward<decltype(args)>(args)...);
    }
private:
    ObjectBlob<T>& v_;
};

struct DefaultArchive {};
struct DefaultConstruct {};

template <typename T, typename Archive = DefaultArchive>
concept HasSerialize = requires(T& obj, Archive& archive) {
    obj.serialize(archive);
};

template <typename T, typename Archive = DefaultArchive, typename Construct = DefaultConstruct>
concept HasLoadAndConstruct = requires(
    T& obj,
    Archive& archive,
    Construct& construct
) {
    obj.load_and_construct(archive, construct);
};

template <typename T>
concept HasSerializeWithoutLoadAndConstruct = HasSerialize<T> && !HasLoadAndConstruct<T>;

template <typename T>
concept HasLoadAndConstructNoSharedPtr = HasLoadAndConstruct<T> && !SharedPtr<T>;

template <typename T>
concept HasSerializeNoSharedPtr = HasSerialize<T> && !SharedPtr<T>;

template <typename T>
concept HasSerializeWithoutLoadAndConstructNoSharedPtr = HasSerializeWithoutLoadAndConstruct<T> && !SharedPtr<T>;

template <HasSerializeNoSharedPtr T>
void save(
    BinaryBitwiseWordsWriter& writer,
    const T& value,
    std::string_view message);

template <HasSerializeWithoutLoadAndConstructNoSharedPtr T>
void load(
    BinaryBitwiseWordsReader& reader,
    T& result,
    std::string_view message);

template <HasLoadAndConstructNoSharedPtr T>
T load(
    BinaryBitwiseWordsReader& reader,
    std::string_view message);

class WritingArchive {
public:
    WritingArchive(
        BinaryBitwiseWordsWriter& writer,
        std::string_view message)
        : writer_{writer}
        , message_{message}
    {}
    struct is_saving {
        static const bool value = true;
    };
    void operator () (const auto& element) {
        save(writer_, element, message_);
    }
private:
    BinaryBitwiseWordsWriter& writer_;
    std::string_view message_;
};

class ReadingArchive {
public:
    inline ReadingArchive(
        BinaryBitwiseWordsReader& reader,
        std::string_view message)
        : reader_{reader}
        , message_{message}
    {}
    struct is_saving {
        static const bool value = false;
    };
    void operator () (auto& element) {
        load(reader_, element, message_);
    }
private:
    BinaryBitwiseWordsReader& reader_;
    std::string_view message_;
};

}
