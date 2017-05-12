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
		class engine : public mark::module::base {
		public:
			engine(mark::resource::manager& manager);
			virtual void tick(mark::tick_context& context) override;
			auto describe() const->std::string override;
			auto global_modifiers() const->mark::module::modifiers override;
			void shoot(mark::vector<double> pos, bool release) override;
		private:
			std::shared_ptr<const mark::resource::image> m_image_base;
			bool m_active = false;
		};
	}
}