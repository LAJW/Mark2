#pragma once
#include "adsr.h"
#include "unit_base.h"
#include "resource_image.h"

namespace mark {
	class world;
	namespace unit {
		class minion final : public mark::unit::base {
		public:
			minion(mark::world& world, mark::vector<double> pos);
			void tick(mark::tick_context& context) override;
			auto dead() const -> bool override;
			void damage(unsigned amount, mark::vector<double> pos);
		private:
			std::shared_ptr<const mark::resource::image> m_image;
			std::shared_ptr<const mark::resource::image> m_im_shield;
			mark::vector<float> m_direction = { 1, 0 };
			int m_health = 100;
			mark::adsr m_bareer_reaction;
			float m_bareer_direction = 0;
			size_t m_frame = 0;
			float m_clock = 0.f;
		};
	}
}