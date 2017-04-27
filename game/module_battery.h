#pragma once
#include "module_base.h"
#include "lfo.h"

namespace sf {
	class Texture;
}

namespace mark {
	namespace resource {
		class manager;
		using image = sf::Texture;
	}
	namespace module {
		class battery : public mark::module::base{
		public:
			battery(mark::resource::manager& manager);
			virtual void tick(mark::tick_context& context) override;
			auto dead() const -> bool override;
			void shoot(mark::vector<double> pos) override;
			auto describe() const -> std::string override;
			auto harvest_energy() -> float override;
			auto energy_ratio() const -> float override;
		private:
			std::shared_ptr<const mark::resource::image> m_image_base;
			std::shared_ptr<const mark::resource::image> m_image_bar;
			float m_cur_energy = 0.f;
			float m_max_energy = 1000.f;
		};
	}
}