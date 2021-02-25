#pragma once
#include <list>
#include <memory>

namespace Mlib {

class TriangleList;

void delete_backfacing_triangles(
    const std::list<std::shared_ptr<TriangleList>>& lists,
    const std::string& debug_filename);

}
