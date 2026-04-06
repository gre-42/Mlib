#pragma once
#include <proctree/proctree.hpp>
#include <string>

namespace Proctree
{
    struct PropertiesAndSeed {
        std::string trunk_diffuse;
        std::string twig_diffuse;
        Properties properties;
        unsigned int seed;
    };
}
