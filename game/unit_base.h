#pragma once
#include <vector>
#include <memory>
#include "iserializable.h"
#include "vector.h"

namespace mark {

	class sprite;
	class world;

	namespace unit {
		class base : public iserializable {
		public:
			base(mark::world& world, mark::vector<double> pos)
				:m_pos(pos), m_world(world) { }
			virtual ~base() = default;
			virtual auto render() const -> std::vector<mark::sprite> = 0;
			virtual void tick(double dt) = 0;
			auto inline pos() const { return m_pos; }
		protected:
			mark::vector<double> m_pos;
			mark::world& m_world;
		};
	}
}