#pragma once
#include "unit_base.h"
#include "resource_image.h"

namespace mark {
	class world;
	namespace unit {
		class projectile final : public mark::unit::base {
		public:
			projectile(mark::world& world, mark::vector<double> pos, float rotation);
			auto render() const->std::vector<mark::sprite>;
			void tick(mark::tick_context& context) override;
			auto dead() const -> bool override;
		private:
			std::shared_ptr<const mark::resource::image> m_image;
			float m_rotation = 0;
			bool m_dead = false;
		};
	}
}