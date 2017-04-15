#pragma once
#include <vector>
#include "iserializable.h"
#include "vector.h"

namespace mark {
	class sprite;
	namespace terrain {
		constexpr double grid_size = 32.0;
		class base : public iserializable {
		public:
			virtual ~base() = default;
			virtual bool traversable() const = 0;
			virtual auto render(mark::vector<int> map_pos) const->std::vector<mark::sprite> = 0;
		};
	}
}