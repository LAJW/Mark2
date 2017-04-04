#pragma once
#include "terrain_base.h"
#include "resource_image.h"

namespace mark {
	class world;
	class sprite;

	namespace terrain {
		class wall : public mark::terrain::base {
		public:
			wall(mark::world& world);
			auto traversable() const -> bool override { return false; };
			auto render(mark::vector<int> map_pos) const->std::vector<mark::sprite> override;
		private:
			std::shared_ptr<const mark::resource::image> m_image;
		};
	}
}