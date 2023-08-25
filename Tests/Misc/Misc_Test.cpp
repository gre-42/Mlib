#include <Mlib/Assert.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Resource_Ptr.hpp>
#include <Mlib/Regex/Misc.hpp>
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

void test_dangling_unique() {
    std::list<DanglingPtr<int>> lst;
    {
        auto a = make_dunique<int>(5);
        std::cerr << a.ref() << std::endl;
        auto b = a.get();
        std::cerr << b.ref() << std::endl;
        lst.push_back(a.get());
        lst.clear();
    }
}

int main(int argc, const char** argv) {
    enable_floating_point_exceptions();

    test_resource_ptr();
    test_substitute();
    test_dangling_unique();
    return 0;
}
