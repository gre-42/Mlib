#pragma once
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Os/Io/Serialize/Serialization_Context_Read.hpp>
#include <Mlib/Os/Io/Serialize/Serialization_Context_Write.hpp>
#include <Mlib/Type_Traits/Shared_Ptr.hpp>
#include <memory>

namespace Mlib {

template <typename T>
class ConstructShared {
public:
    explicit ConstructShared(std::shared_ptr<T>& v): v_{v} {}
    void operator () (auto&&... args) {
        v_ = std::make_shared<T>(std::forward<decltype(args)>(args)...);
    }
private:
    std::shared_ptr<T>& v_;
};

template <SharedPtr TSharedPtr>
void save(
    BinaryBitwiseWordsWriter& writer,
    SerializationContextWrite& ctx,
    const TSharedPtr& value,
    std::string_view message)
{
    auto [id, added] = ctx.add_or_get(value);
    writer.write_binary(id, message);
    if (added) {
        save(writer, ctx, *value, message);
    }
}

template <SharedPtr TSharedPtr>
TSharedPtr load(
    BinaryBitwiseWordsReader& reader,
    SerializationContextRead& ctx,
    std::string_view message)
{
    using T = TSharedPtr::element_type;
    auto id = reader.read_binary<uint32_t>("shared ptr ID");
    auto res = ctx.try_get<T>(id);
    if (res.has_value()) {
        return *res;
    }
    ReadingArchive archive{reader, ctx, message};
    std::shared_ptr<T> result;
    ConstructShared<T> construct{result};
    T::load_and_construct(archive, construct);
    ctx.add(result);
    return result;
}

}
