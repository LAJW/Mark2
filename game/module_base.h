#pragma once
#include "stdafx.h"
#include "idamageable.h"
#include "iserializable.h"
#include "lfo.h"
#include "unit_modular.h"

namespace sf {
	class Color;
}

namespace mark {
	namespace resource {
		class image;
		class manager;
	}
	class sprite;

	namespace module {
		// grid size - width/height of an 1x1 module
		static constexpr float size = 16.f;

		// Maximum module width and height
		const unsigned max_dimension = 4;

		// stats altering behavior of parent/adjacent modules
		struct modifiers {
			float velocity = 0.f;
		};

		class base;

		auto deserialize(mark::resource::manager&, const YAML::Node&) ->
			std::unique_ptr<mark::module::base>;

		class base:
			public idamageable,
			public iserializable {
		public:
			static constexpr auto max_heat = 100.f;
			friend mark::unit::modular;
			base(mark::resource::manager&, const YAML::Node&);
			base(mark::vector<unsigned> size, const std::shared_ptr<const mark::resource::image>& thumbnail);
			virtual ~base() = default;

			virtual void tick(mark::tick_context& context);

			// Module's position in the world
			auto pos() const -> mark::vector<double>;

			// Module's image in the inventory
			auto thumbnail() const -> std::shared_ptr<const mark::resource::image>;

			// Size in grid units
			auto size() const -> mark::vector<unsigned>;

			// Describes whether the module is dead or not
			virtual auto dead() const -> bool;

			// Can module be detached from the vessel
			virtual auto detachable() const -> bool { return true; }

			// Target/look at specific position in the world
			virtual void target(mark::vector<double> pos) { /* no op */ }

			// Select target to shoot at
			virtual void shoot(mark::vector<double> pos, bool release) { /* no op */ }

			// Add target to shooting queue
			virtual void queue(mark::vector<double> pos, bool release) { /* no op */ }

			// Find collision point, return pointer to damaged module
			virtual auto collide(const mark::segment_t&) ->
				std::pair<mark::idamageable*, mark::vector<double>>;

			// UI text describing module's properties
			virtual auto describe() const -> std::string = 0;

			// Obtain energy from the module
			virtual auto harvest_energy() -> float { return 0.f; }

			// Current / Maximum energy stored in the module
			virtual auto energy_ratio() const -> float { return 0.f; }

			// Neighbour modules
			auto neighbours()->std::vector<std::reference_wrapper<mark::module::base>>;

			// Position on the grid
			auto grid_pos() const noexcept -> mark::vector<int>;

			// Default damage handling
			auto damage(const mark::idamageable::info& attr) -> bool override;

			// called on module's death
			virtual void on_death(mark::tick_context& context);

			// get modifiers for parent modular ship
			virtual auto global_modifiers() const -> mark::module::modifiers;

		protected:
			auto parent() const -> const mark::unit::modular&;
			auto parent() -> mark::unit::modular&;
			auto heat_color() const -> sf::Color;
			// serialize module::base properties, call only from module serializers
			void serialize_base(YAML::Emitter&) const;

			float m_cur_health = 100.f;
			float m_max_health = 100.f;
			float m_stunned = 0.f;
			float m_cur_heat = 0.f;
		private:
			std::shared_ptr<const mark::resource::image> m_thumbnail;
			const mark::vector<unsigned> m_size;
			mark::unit::modular* m_parent = nullptr;
			mark::vector<int8_t> m_grid_pos;
			float m_stun_lfo;
		};
	}
}
