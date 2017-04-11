#pragma once
#include "adsr.h"
#include "module_base.h"
#include "resource_image.h"

namespace mark {
	namespace resource {
		class manager;
	}
	namespace module {
		class turret : public mark::module::base {
		public:
			turret(mark::resource::manager& manager);
			virtual void tick(mark::tick_context& context) override;
			auto dead() const -> bool override;
		private:
			std::shared_ptr<const mark::resource::image> m_im_base;
			std::shared_ptr<const mark::resource::image> m_im_cannon;
			mark::adsr m_adsr;
			float m_cur_cooldown = 0;
		};
	}
}