#pragma once
#include <vector>
#include <memory>
#include "vector.h"

namespace mark {
	namespace terrain {
		class base;
	}
	class world;
	using map_t = std::vector<std::vector<std::shared_ptr<mark::terrain::base>>>;
	std::vector<mark::vector<int>> find_path(const mark::map_t& map, mark::vector<int> start, mark::vector<int> target);
}