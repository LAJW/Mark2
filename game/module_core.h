#pragma once
#include <vector>
#include "module_base.h"
#include "resource_image.h"

namespace mark {
	namespace resource {
		class manager;
		class sprite;
	};

	namespace module {
		class core : public base {
		public:
			core(mark::resource::manager& resource_manager);
			void tick(mark::tick_context& context) override;
			auto detachable() const -> bool;
			auto describe() const -> std::string override;
		private:
			std::shared_ptr<const mark::resource::image> m_image;
		};
	}
}
