#pragma once
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
			auto render() const->std::vector<mark::sprite> override;
			auto dead() const -> bool override;
		private:
			std::shared_ptr<const mark::resource::image> m_im_base;
			std::shared_ptr<const mark::resource::image> m_im_cannon;
		};
	}
}