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
			void tick(mark::tick_context& context) override;
			auto describe() const -> std::string override;
			auto harvest_energy() -> float override;
			auto energy_ratio() const -> float override;
			void serialize(YAML::Emitter&) const override;
		private:
			std::shared_ptr<const mark::resource::image> m_image_base;
			std::shared_ptr<const mark::resource::image> m_image_bar;
			float m_cur_energy = 0.f;
			float m_max_energy = 1000.f;
		};
	}
}