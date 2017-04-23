#pragma once
#pragma once
#include "unit_base.h"
#include "resource_image.h"

namespace mark {
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
			void damage(unsigned amount, mark::vector<double> pos) override { }
			auto invincible() const -> bool override { return true; };
			void dock(mark::unit::modular* ship);
			void activate(const std::shared_ptr<mark::unit::base>& by) override;
			void command(const mark::command& command) override;
			auto collides(mark::vector<double> pos, float radius) const -> bool override;
		private:
			std::shared_ptr<const mark::resource::image> m_image;
			std::weak_ptr<mark::unit::modular> m_ship;
			std::unique_ptr<mark::module::base> m_grabbed;
			mark::vector<double> m_mousepos;
		};
	}
}