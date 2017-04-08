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
			auto render() const->std::vector<mark::sprite> override;
		private:
			std::shared_ptr<const mark::resource::image> m_image;
		};
	}
}
