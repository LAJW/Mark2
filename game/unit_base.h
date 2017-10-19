#pragma once
#include "stdafx.h"
#include "interface_serializable.h"
#include "interface_damageable.h"
#include "property.h"

namespace mark {

	struct sprite;
	class world;
	class command;
	struct tick_context;

	namespace unit {
		class base;
		auto deserialize(
			world& world,
			const YAML::Node& node) ->
			std::shared_ptr<unit::base>;

		class base:
			public interface::serializable,
			public std::enable_shared_from_this<unit::base> {
		public:
			virtual void tick(tick_context& context) = 0;
			virtual void command(const command&) { };
			virtual auto dead() const -> bool = 0;
			virtual void on_death(tick_context&) { /* no-op */ };
			virtual void activate(const std::shared_ptr<unit::base>&) { /* no-op */ };
			// Resolve references after deserializing
			virtual void resolve_ref(
				const YAML::Node&,
				const std::unordered_map<uint64_t, std::weak_ptr<unit::base>>& units);

			Property<int> team = 0;
			Property<vector<double>> pos;
		protected:
			base(world& world, vector<double> pos);
			base(world& world, const YAML::Node&);
			void serialize_base(YAML::Emitter&) const;
			virtual ~base() = default;

			world& m_world;
		};
	}
}