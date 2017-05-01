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
			// collide a ray with the terrain
			// pos - position of the segment in the world
			// line - line to be collided with
			// returns point of collision or [ NAN, NAN ] if there was no collision
			virtual auto collide(mark::vector<double> world_pos, mark::segment_t line) const -> mark::vector<double> = 0;
		};
	}
}