#pragma once
#include "module_base.h"
#include "resource_image.h"

namespace mark {

	namespace resource {
		class manager;
		class sprite;
	};

	namespace module {
		class shield_generator : public base {
		public:
			shield_generator(mark::resource::manager& resource_manager);
			virtual void tick(mark::tick_context& context) override;
			inline auto dead() const -> bool override { return false; }
		private:
			std::shared_ptr<const mark::resource::image> m_im_generator;
			std::shared_ptr<const mark::resource::image> m_im_shield;
		};
	}
}
