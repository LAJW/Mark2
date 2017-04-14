#pragma once
#pragma once
#include "unit_base.h"
#include "resource_image.h"

namespace mark {
	class world;
	namespace unit {
		class heat_seeker final : public mark::unit::base {
		public:
			heat_seeker(mark::world& world, mark::vector<double> pos, float rotation);
			void tick(mark::tick_context& context) override;
			auto dead() const -> bool override;
			void damage(unsigned amount, mark::vector<double> pos) override { /* TODO */ }
			auto invincible() const -> bool override;
		private:
			std::shared_ptr<const mark::resource::image> m_image;
			std::shared_ptr<const mark::resource::image> m_im_tail;
			float m_rotation = 0;
			bool m_dead = false;
		};
	}
}