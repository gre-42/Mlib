#pragma once
#include <Mlib/Map/Map.hpp>
#include <map>
#include <set>
#include <string>

namespace Mlib {

struct Node;
struct Way;
struct Relation;

struct NodesAndWays {
    Map<std::string, Node> nodes;
    Map<std::string, Way> ways;
    Map<std::string, Relation> relations;
};

}
