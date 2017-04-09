#pragma once
#include <vector>
#include <memory>
#include "iserializable.h"
#include "vector.h"

namespace mark {

	class sprite;
	class world;
	class command;
	class tick_context;

	namespace unit {
		class base : public iserializable {
		public:
			base(mark::world& world, mark::vector<double> pos)
				:m_pos(pos), m_world(world) { }
			virtual auto render() const -> std::vector<mark::sprite> = 0;
			virtual void tick(mark::tick_context& context) = 0;
			virtual void command(const mark::command&) { };
			auto inline pos() const { return m_pos; }
			virtual auto dead() const -> bool = 0;
		protected:
			virtual ~base() = default;
			mark::vector<double> m_pos;
			mark::world& m_world;
		};
	}
}