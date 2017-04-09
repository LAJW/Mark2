#pragma once
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
		private:
			std::shared_ptr<const mark::resource::image> m_image;
			mark::vector<float> m_direction = { 1, 0 };
			int m_health = 100;
		};
	}
}