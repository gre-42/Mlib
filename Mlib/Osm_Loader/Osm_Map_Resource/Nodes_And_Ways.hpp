#pragma once
#include <map>
#include <set>
#include <string>

namespace Mlib {

struct Node;
struct Way;

struct NodesAndWays {
    std::map<std::string, Node> nodes;
    std::map<std::string, Way> ways;
};

}
