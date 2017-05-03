#pragma once
#include <vector>
#include <memory>
#include "iserializable.h"
#include "property.h"
#include "vector.h"
#include "idamageable.h"

namespace mark {

	class sprite;
	class world;
	class command;
	struct tick_context;

	namespace unit {
		class base:
			public mark::iserializable,
			public mark::idamageable,
			public std::enable_shared_from_this<mark::unit::base> {
		public:
			base(mark::world& world, mark::vector<double> pos)
				:m_pos(pos), m_world(world) { }
			virtual void tick(mark::tick_context& context) = 0;
			virtual void command(const mark::command&) { };
			auto inline pos() const { return m_pos; }
			virtual auto dead() const -> bool = 0;
			virtual void on_death(mark::tick_context& context) { /* no-op */ };
			virtual bool invincible() const = 0;
			virtual void activate(const std::shared_ptr<mark::unit::base>& by) { /* no-op */ };
			virtual auto collides(mark::vector<double> pos, float radius) const -> bool = 0;
			virtual auto collide(const mark::segment_t&) ->
				std::pair<mark::idamageable*, mark::vector<double>> = 0;
			Property<int> team = 0;
		protected:
			virtual ~base() = default;
			mark::vector<double> m_pos;
			mark::world& m_world;
		};
	}
}