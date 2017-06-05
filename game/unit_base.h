#pragma once
#include "stdafx.h"
#include "iserializable.h"
#include "property.h"
#include "idamageable.h"

namespace mark {

	class sprite;
	class world;
	class command;
	struct tick_context;

	namespace unit {
		class base;
		auto deserialize(
			mark::world& world,
			const YAML::Node& node) ->
			std::shared_ptr<mark::unit::base>;

		class base:
			public mark::iserializable,
			public mark::idamageable,
			public std::enable_shared_from_this<mark::unit::base> {
		public:
			base(mark::world& world, mark::vector<double> pos);
			virtual void tick(mark::tick_context& context) = 0;
			virtual void command(const mark::command&) { };
			virtual auto dead() const -> bool = 0;
			virtual void on_death(mark::tick_context& context) { /* no-op */ };
			virtual bool invincible() const = 0;
			virtual void activate(const std::shared_ptr<mark::unit::base>& by) { /* no-op */ };
			virtual auto collide(const mark::segment_t&) ->
				std::pair<mark::idamageable*, mark::vector<double>> = 0;
			virtual auto collide(mark::vector<double> center, float radius) ->
				std::vector<std::reference_wrapper<mark::idamageable>>;
			// Resolve references after deserializing
			virtual void resolve_ref(
				const YAML::Node&,
				const std::unordered_map<uint64_t, std::weak_ptr<mark::unit::base>>& units);

			Property<int> team = 0;
			Property<mark::vector<double>> pos;
		protected:
			base(mark::world& world, const YAML::Node&);
			void serialize_base(YAML::Emitter&) const;
			virtual ~base() = default;

			mark::world& m_world;
		};
	}
}