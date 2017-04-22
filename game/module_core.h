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
			virtual void tick(mark::tick_context& context) override;
			auto dead() const -> bool override;
			auto detachable() const -> bool { return false; }
		private:
			std::shared_ptr<const mark::resource::image> m_image;
			int m_health = 100;
		};
	}
}
