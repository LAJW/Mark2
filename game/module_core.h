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
			auto render() const -> std::vector<mark::sprite> override;
			auto dead() const -> bool override;
		private:
			std::shared_ptr<const mark::resource::image> m_image;
			int m_health = 100;
		};
	}
}
