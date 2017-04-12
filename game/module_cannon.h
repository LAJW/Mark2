#pragma once
#include "module_base.h"
#include "resource_image.h"
#include "cooldown.h"

namespace mark {
	namespace resource {
		class manager;
	}
	namespace module {
		class cannon : public mark::module::base{
		public:
			cannon(mark::resource::manager& manager);
			virtual void tick(mark::tick_context& context) override;
			inline auto dead() const -> bool override { return false; }
		private:
			std::shared_ptr<const mark::resource::image> m_im_body;
			std::shared_ptr<const mark::resource::image> m_im_ray;
			mark::cooldown m_frame_cooldown;
			unsigned m_frame = 0;
		};
	}
}