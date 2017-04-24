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
	namespace unit {
		class heat_seeker final : public mark::unit::base {
		public:
			heat_seeker(mark::world& world, mark::vector<double> pos, float rotation);
			void tick(mark::tick_context& context) override;
			auto dead() const -> bool override;
			void damage(unsigned amount, mark::vector<double> pos) override { /* TODO */ }
			auto invincible() const -> bool override;
			auto collides(mark::vector<double> pos, float radius) const -> bool override;
		private:
			std::shared_ptr<const mark::resource::image> m_image;
			std::shared_ptr<const mark::resource::image> m_im_tail;
			float m_rotation = 0;
			bool m_dead = false;
		};
	}
}