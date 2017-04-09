#pragma once
#include "module_base.h"
#include "resource_image.h"

namespace mark {
	namespace resource {
		class manager;
	}
	namespace module {
		class cargo : public mark::module::base {
		public:
			cargo(mark::resource::manager& manager);
			virtual void tick(mark::tick_context& context) override;
			inline auto dead() const -> bool override { return false; }
		private:
			std::shared_ptr<const mark::resource::image> m_image;
		};
	}
}