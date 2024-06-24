#include <Mlib/Assert.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Memory/Resource_Ptr.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Misc.hpp>
#include <Mlib/Regex/Template_Regex.hpp>
#include <Mlib/Threads/Dispatcher.hpp>
#include <Mlib/Try_Find.hpp>
#include <iostream>

using namespace Mlib;

struct A { int a = 3; };

void test_resource_ptr() {
    resource_ptr_target<float> v{new float(5)};
    {
        resource_ptr<float> pv = v.instantiate();
        assert_isequal(*pv, 5.f);
    }
    resource_ptr_target<A> a{new A};
    {
        resource_ptr<A> b = a.instantiate();
        assert_isequal(b->a, 3);
    }
}

void test_substitute() {
    {
        std::string str = "macro_playback create-CAR_NAME SUFFIX:SUFFIX IF_WITH_PHYSICS: IF_RACING:IF_RACING IF_RALLY:IF_RALLY";
        std::string replacements = "CAR_NAME:_XZ SUFFIX:_npc1 IF_WITH_PHYSICS: IF_RACING:# IF_RALLY:";
        assert_true(substitute(str, replacements_to_map(replacements)) == "macro_playback create_XZ SUFFIX:_npc1 IF_WITH_PHYSICS: IF_RACING:# IF_RALLY:");
    }
    {
        std::string str = "IS_SMALL";
        std::string replacements = "IS_SMALL:0";
        assert_true(substitute(str, replacements_to_map(replacements)) == "0");
    }
}

DP_IMPLEMENT(int);

void test_dangling_unique() {
    std::list<DanglingPtr<int>> lst;
    {
        auto a = make_dunique<int>(5);
        auto b = a.get(DP_LOC);
        lst.push_back(a.get(DP_LOC));
        lst.clear();
        assert_true(*b == 5);
    }
}

struct Ads {
    int i;
    Ads* ptr() { return this; }
};

DP_IMPLEMENT(Ads);

struct D {
    D(DanglingRef<Ads> p) : p{ p } {}
    DanglingRef<Ads> p;
};

void test_dangling_unique2() {
    for (size_t i = 0; i < 1000; ++i) {
        auto n = make_dunique<Ads>(5);
        D n0(DanglingRef<Ads>::from_object(*n->ptr(), DP_LOC));
        D n1(DanglingRef<Ads>::from_object(*n->ptr(), DP_LOC));
        n.get(DP_LOC);
    }
}

void test_template_regex() {
    using namespace TemplateRegex;
    auto re_hello = str("hello");
    auto re_world = str("world");
    SMatch match;
    assert_true(regex_match("hello world", match, re_hello));
    assert_true(!regex_match("hello world", match, re_world));
    {
        auto re_group_hello = group(re_hello);
        assert_true(regex_match("hello world", match, re_group_hello));
        assert_true(match[1].matched);
        assert_true(match[1].str() == "hello");
    }
    {
        auto re_group_world = group(re_world);
        assert_true(!regex_match("hello world", match, re_group_world));
        assert_true(!match[1].matched);
        assert_true(match[1].str() == "");
    }
    {
        auto re_group_world = group(star(re_hello));
        assert_true(regex_match("", match, re_group_world));
        assert_true(match[1].matched);
        assert_true(match[1].str() == "");
    }
    {
        auto re_group_world = group(plus(re_hello));
        assert_true(!regex_match("", match, re_group_world));
        assert_true(!match[1].matched);
        assert_true(match[1].str() == "");
    }
    {
        auto re_group_world = group(plus(re_hello));
        assert_true(regex_match("hello", match, re_group_world));
        assert_true(match[1].matched);
        assert_true(match[1].str() == "hello");
    }
    {
        auto re_group_world = seq(group(plus(re_hello)), space);
        assert_true(regex_match("hello world", match, re_group_world));
        assert_true(match[1].matched);
        assert_true(match[1].str() == "hello");
    }
    {
        auto re_group_world = seq(group(plus(re_hello)), no_space);
        assert_true(!regex_match("hello world", match, re_group_world));
        assert_true(match[1].matched);
        assert_true(match[1].str() == "hello");
    }
    // {
    //     auto re_group_world = seq(bdry, str("hello"), bdry);
    //     assert_true(regex_match("hello world", match, re_group_world));
    // }
}

void test_parallel_block() {
    std::atomic_int ctr = 0;
    Dispatcher dispatcher{std::chrono::milliseconds{1000}};
    std::thread t0{[&](){
        for (size_t i = 0; i < 10; ++i) {
            dispatcher.produce();
            ++ctr;
        }
    }};
    std::thread t1{[&](){
        for (size_t i = 0; i < 5; ++i) {
            dispatcher.wait_for_data();
            dispatcher.consume();
            ++ctr;
        }
    }};
    t0.join();
    t1.join();
    assert_true(ctr == 15);
}

void test_destruction_functions() {
    DestructionFunctions df;
    DestructionFunctionsRemovalTokens rt{ df, CURRENT_SOURCE_LOCATION };
    rt.add([]() { linfo() << "f"; }, CURRENT_SOURCE_LOCATION);
    df.clear();
}

struct MyClass: DanglingBaseClass {
    int a = 5;
};

struct MyDerived : public MyClass {};

struct MyContainer {
    DanglingBaseClassRef<Object> o;
};

void test_dangling_base_class() {
    MyDerived a;
    DanglingBaseClassRef<MyDerived> b{ a, CURRENT_SOURCE_LOCATION };
    // a.ref<Object>(CURRENT_SOURCE_LOCATION);
    // new MyContainer{ a.ref<Object>(CURRENT_SOURCE_LOCATION) };
    auto xx = b.ptr();
    linfo() << xx->a;
    DanglingBaseClassRef<MyClass> V{ b };
    linfo() << V->a;
}

void test_object_pool_std() {
    struct A: Object {
        int i = 5;
    };
    ObjectPool p{ InObjectPoolDestructor::CLEAR };
    auto& a = p.create<A>(CURRENT_SOURCE_LOCATION);
    linfo() << a.i;
}

void test_object_pool_unique() {
    struct A: Object {
        int i = 5;
    };
    ObjectPool p{ InObjectPoolDestructor::CLEAR };
    auto& a = p.add(std::make_unique<A>(), CURRENT_SOURCE_LOCATION);
    linfo() << a.i;
}

void test_try_find() {
    std::map<int, std::string> m;
    if (try_find(m, 42) != nullptr) {
        throw std::runtime_error("Expected nullptr");
    }
}

void test_log() {
    linfo() << "a\nbc\nd";
}

int main(int argc, const char** argv) {
    enable_floating_point_exceptions();

    test_resource_ptr();
    test_substitute();
    test_dangling_unique();
    test_template_regex();
    test_parallel_block();
    test_destruction_functions();
    test_dangling_base_class();
    test_object_pool_std();
    test_object_pool_unique();
    test_dangling_unique2();
    test_try_find();
    test_log();
    return 0;
}
