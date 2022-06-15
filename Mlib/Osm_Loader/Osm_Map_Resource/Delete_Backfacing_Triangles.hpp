#pragma once
#include <list>
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
class TriangleList;

void delete_backfacing_triangles(
    const std::list<std::shared_ptr<TriangleList<double>>>& lists,
    const std::string& debug_filename);

}
