#pragma once
#include <vector>
#include "idamageable.h"
#include "iserializable.h"
#include "unit_modular.h"
#include "vector.h"

namespace sf {
	class Texture;
}

namespace mark {
	namespace resource {
		using image = sf::Texture;
	}
	class sprite;

	namespace module {
		// grid size - width/height of an 1x1 module
		static constexpr float size = 16.f;

		// Maximum module width and height
		const unsigned max_dimension = 4;

		class base:
			public idamageable,
			public iserializable {
			friend mark::unit::modular::socket;
		public:
			base(mark::vector<unsigned> size, const std::shared_ptr<const mark::resource::image>& thumbnail);
			virtual ~base() = default;

			virtual void tick(mark::tick_context& context) = 0;

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

			// Start shooting at
			virtual void shoot(mark::vector<double> pos) { /* no op */ }

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
			auto grid_pos()-> const mark::vector<int>;

			// Default damage handling
			auto damage(const mark::idamageable::attributes& attr) -> bool override;

			// called on module's death
			virtual void on_death(mark::tick_context& context);

		protected:
			auto parent() const -> const mark::unit::modular&;
			auto parent() -> mark::unit::modular&;
			float m_cur_health = 100.f;
			float m_max_health = 100.f;
		private:
			std::shared_ptr<const mark::resource::image> m_thumbnail;
			const mark::vector<unsigned> m_size;
			mark::unit::modular::socket* m_socket = nullptr;
		};
	}
}
