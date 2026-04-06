#pragma once
#include <list>
#include <map>
#include <memory>
#include <unordered_set>

namespace Mlib {

class IGpuVertexData;
struct VertexDataAndSortedInstances;
struct SortableDeferredVertexData;
struct ExtendableVertexArrayInstances;
struct ExtendableSortableVertexArrayInstances;

using VertexDatasAndSortedInstances = std::list<VertexDataAndSortedInstances>;
using VertexDatasAndInstances = std::map<std::shared_ptr<IGpuVertexData>, ExtendableVertexArrayInstances>;
using SortableDeferredVertexDatasAndSortableInstances = std::map<SortableDeferredVertexData, ExtendableSortableVertexArrayInstances>;

}
