#pragma once
#include <Mlib/Array/Non_Copying_Vector.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Type_Traits/Shared_Ptr.hpp>
#include <concepts>
#include <type_traits>

namespace Mlib {

class SerializationContextRead;
class SerializationContextWrite;

class WritingArchive {
public:
    WritingArchive(
        BinaryBitwiseWordsWriter& writer,
        SerializationContextWrite& ctx,
        std::string_view message)
        : writer_{writer}
        , ctx_{ctx}
        , message_{message}
    {}
    struct is_saving {
        static const bool value = true;
    };
    void operator () (const auto& element) {
        writer_.flush_partial(message_);
        save(writer_, ctx_, element, message_);
    }
private:
    BinaryBitwiseWordsWriter& writer_;
    SerializationContextWrite& ctx_;
    std::string_view message_;
};

class ReadingArchive {
public:
    inline ReadingArchive(
        BinaryBitwiseWordsReader& reader,
        SerializationContextRead& ctx,
        std::string_view message)
        : reader_{reader}
        , ctx_{ctx}
        , message_{message}
    {}
    struct is_saving {
        static const bool value = false;
    };
    void operator () (auto& element) {
        reader_.align_to_next_word();
        element = load<std::remove_reference_t<decltype(element)>>(reader_, ctx_, message_);
    }
private:
    BinaryBitwiseWordsReader& reader_;
    SerializationContextRead& ctx_;
    std::string_view message_;
};

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
    SerializationContextWrite& ctx,
    const T& value,
    std::string_view message)
{
    WritingArchive archive{writer, ctx, message};
    const_cast<T&>(value).serialize(archive);
}

template <HasSerializeWithoutLoadAndConstructNoSharedPtr T>
T load(
    BinaryBitwiseWordsReader& reader,
    SerializationContextRead& ctx,
    std::string_view message)
{
    ReadingArchive archive{reader, ctx, message};
    T result;
    result.serialize(archive);
    return result;
}

template <HasLoadAndConstructNoSharedPtr T>
T load(
    BinaryBitwiseWordsReader& reader,
    SerializationContextRead& ctx,
    std::string_view message)
{
    ReadingArchive archive{reader, ctx, message};
    ObjectBlob<T> buffer;
    ConstructInplace<T> construct{buffer};
    T::load_and_construct(archive, construct);
    T result = std::move((T&)buffer);
    ((T&)buffer).~T();
    return result;
}

}
