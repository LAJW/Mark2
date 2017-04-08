#pragma once
#include <memory>
#include "terrain_base.h"
#include "resource_image.h"

namespace mark {
	class world;
	namespace resource {
		class manager;
	}

	namespace terrain {
		class floor : public mark::terrain::base {
		public:
			floor(mark::resource::manager& resource_manager);
			auto traversable() const -> bool override { return true; };
			auto render(mark::vector<int> map_pos) const->std::vector<mark::sprite> override;
		private:
			std::shared_ptr<const mark::resource::image> m_image;
		};
	}
}