#pragma once
#include "unit_base.h"

namespace sf {
	class Texture;
}

namespace mark {
	namespace resource {
		using image = sf::Texture;
	}
	class world;
	namespace module {
		class base;
	}
	namespace unit {
		class modular;
		class landing_pad final : public mark::unit::base {
		public:
			landing_pad(mark::world& world, mark::vector<double> pos);
			void tick(mark::tick_context& context) override;
			auto dead() const -> bool override { return false; };
			auto invincible() const -> bool override { return true; };
			void dock(mark::unit::modular* ship);
			void activate(const std::shared_ptr<mark::unit::base>& by) override;
			void command(const mark::command& command) override;
			virtual auto collide(const mark::segment_t&) ->
				std::pair<mark::idamageable*, mark::vector<double>> override;
		private:
			std::shared_ptr<const mark::resource::image> m_image;
			std::weak_ptr<mark::unit::modular> m_ship;
			std::unique_ptr<mark::module::base> m_grabbed;
			mark::vector<double> m_mousepos;
		};
	}
}