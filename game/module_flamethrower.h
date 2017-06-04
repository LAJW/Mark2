#pragma once
#include "stdafx.h"
#include "module_base.h"

namespace mark {
	namespace resource {
		class manager;
		class image;
	}
	namespace module {
		class flamethrower : public mark::module::base{
		public:
			static constexpr const char* type_name = "module_flamethrower";

			flamethrower(mark::resource::manager&, const YAML::Node&);
			flamethrower(mark::resource::manager& manager);
			void tick(mark::tick_context& context) override;
			void shoot(mark::vector<double> pos, bool release) override;
			auto describe() const -> std::string override;
			auto harvest_energy() -> float override;
			auto energy_ratio() const -> float override;
			void serialize(YAML::Emitter&) const override;
		private:
			std::shared_ptr<const mark::resource::image> m_image_base;
			bool m_shoot = false;
		};
	}
}
