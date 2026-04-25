#pragma once
#include <filesystem>
#include <proctree/proctree.hpp>

namespace Proctree
{
    struct PropertiesAndSeed {
        Mlib::u8string trunk_diffuse;
        Mlib::u8string twig_diffuse;
        Properties properties;
        unsigned int seed;
    };
}
