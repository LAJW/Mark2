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
			floor(mark::resource::manager& resource_manager, int variant = 4);
			auto traversable() const -> bool override { return true; };
			auto render(mark::vector<int> map_pos) const->std::vector<mark::sprite> override;
			void variant(int variant);
		private:
			std::shared_ptr<const mark::resource::image> m_image;
			mark::resource::manager& m_resource_manager;
			int m_variant;
		};
	}
}