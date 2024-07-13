#pragma once
#include <Mlib/Map/Map.hpp>
#include <cstddef>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

class DrawDistanceDb {
public:
	DrawDistanceDb();
	~DrawDistanceDb();
	void add_ide(const std::string& filename);
	const FixedArray<float, 2>& get_center_distances(
		const std::string& resource_name) const;
private:
	Map<std::string, FixedArray<float, 2>> center_distances_;
};

}