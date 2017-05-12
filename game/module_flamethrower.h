#pragma once
#include "module_base.h"

namespace sf {
	class Texture;
}

namespace mark {
	namespace resource {
		class manager;
		using image = sf::Texture;
	}
	namespace module {
		class flamethrower : public mark::module::base{
		public:
			flamethrower(mark::resource::manager& manager);
			void tick(mark::tick_context& context) override;
			void shoot(mark::vector<double> pos, bool release) override;
			auto describe() const -> std::string override;
			auto harvest_energy() -> float override;
			auto energy_ratio() const -> float override;
		private:
			std::shared_ptr<const mark::resource::image> m_image_base;
			bool m_shoot = false;
		};
	}
}
