#pragma once
#include <filesystem>
#include <proctree/proctree.hpp>

namespace Proctree
{
    struct PropertiesAndSeed {
        std::u8string trunk_diffuse;
        std::u8string twig_diffuse;
        Properties properties;
        unsigned int seed;
    };
}
