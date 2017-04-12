#pragma once
#include <vector>
#include <memory>
#include "iserializable.h"
#include "property.h"
#include "vector.h"

namespace mark {

	class sprite;
	class world;
	class command;
	struct tick_context;

	namespace unit {
		class base : public iserializable {
		public:
			base(mark::world& world, mark::vector<double> pos)
				:m_pos(pos), m_world(world) { }
			virtual void tick(mark::tick_context& context) = 0;
			virtual void command(const mark::command&) { };
			auto inline pos() const { return m_pos; }
			virtual auto dead() const -> bool = 0;
			virtual void damage(unsigned amount, mark::vector<double> pos) = 0;
			virtual bool invincible() const = 0;
			Property<int> team = 0;
		protected:
			virtual ~base() = default;
			mark::vector<double> m_pos;
			mark::world& m_world;
		};
	}
}