#include <Mlib/Misc/Floating_Point_Exceptions.hpp>
#include <Mlib/Os/Io/Serialize/Serialize.hpp>

using namespace Mlib;

struct S {
    template <class Archive>
    void serialize(Archive& archive) {
        archive(x);
        archive(y);
    }
    int x;
    int y;
};

struct S1 {
    S1(int x, int y): x{x} , y{y} {}
    template <class Archive>
    void serialize(Archive& archive) {
        archive(x);
        archive(y);
    }
    template<typename Archive, typename Construct>
    static void load_and_construct(
        Archive& archive,
        Construct& construct)
    {
        int x;
        int y;
        archive(x);
        archive(y);
        construct(x, y);
    }
    int x;
    int y;
};

struct S2: public virtual Object {
    S2(int x, int y): x{x} , y{y} {}
    template <class Archive>
    void serialize(Archive& archive) {
        archive(x);
        archive(y);
    }
    template<typename Archive, typename Construct>
    static void load_and_construct(
        Archive& archive,
        Construct& construct)
    {
        int x;
        int y;
        archive(x);
        archive(y);
        construct(x, y);
    }
    int x;
    int y;
};

struct S3 {
    template <class Archive>
    void serialize(Archive& archive) {
        archive(s2);
    }
    std::shared_ptr<S2> s2;
};

void test_serialize_number() {
    std::stringstream sstr;
    {
        uint32_t a = 42;
        BinaryBitwiseWordsWriter writer{sstr};
        SerializationContextWrite ctx;
        save(writer, ctx, a, "a");
    }
    {
        BinaryBitwiseWordsReader reader{sstr, IoVerbosity::SILENT};
        SerializationContextRead ctx;
        auto b = load<uint32_t>(reader, ctx, "a");
        linfo() << "b: " << b;
    }
}

void test_serialize_struct() {
    std::stringstream sstr;
    {
        S a{42, 43};
        BinaryBitwiseWordsWriter writer{sstr};
        SerializationContextWrite ctx;
        save(writer, ctx, a, "a");
    }
    {
        BinaryBitwiseWordsReader reader{sstr, IoVerbosity::SILENT};
        SerializationContextRead ctx;
        auto b = load<S>(reader, ctx, "a");
        linfo() << "b: " << b.x << " " << b.y;
    }
}

void test_serialize_struct1() {
    std::stringstream sstr;
    {
        S1 a{42, 43};
        BinaryBitwiseWordsWriter writer{sstr};
        SerializationContextWrite ctx;
        save(writer, ctx, a, "a");
    }
    {
        BinaryBitwiseWordsReader reader{sstr, IoVerbosity::SILENT};
        SerializationContextRead ctx;
        auto b = load<S1>(reader, ctx, "a");
        linfo() << "b: " << b.x << " " << b.y;
    }
}

void test_serialize_struct2() {
    std::stringstream sstr;
    {
        auto a = std::make_shared<S2>(42, 43);
        BinaryBitwiseWordsWriter writer{sstr};
        SerializationContextWrite ctx;
        save(writer, ctx, a, "a");
    }
    {
        BinaryBitwiseWordsReader reader{sstr, IoVerbosity::SILENT};
        SerializationContextRead ctx;
        auto b = load<std::shared_ptr<S2>>(reader, ctx, "a");
        linfo() << "b: " << b->x << " " << b->y;
    }
}

void test_serialize_struct3() {
    std::stringstream sstr;
    {
        S3 a{std::make_shared<S2>(42, 43)};
        BinaryBitwiseWordsWriter writer{sstr};
        SerializationContextWrite ctx;
        save(writer, ctx, a, "a");
    }
    {
        BinaryBitwiseWordsReader reader{sstr, IoVerbosity::SILENT};
        SerializationContextRead ctx;
        auto b = load<S3>(reader, ctx, "a");
        linfo() << "b: " << b.s2->x << " " << b.s2->y;
    }
}

int main(int argc, const char** argv) {
    enable_floating_point_exceptions();

    try {
        test_serialize_number();
        test_serialize_struct();
        test_serialize_struct1();
        test_serialize_struct2();
        test_serialize_struct3();
    } catch (const std::exception& e) {
        lerr() << "Test failed: " << e.what();
        return 1;
    }
    return 0;
}
