#pragma once
#include "terrain_base.h"

namespace sf {
	class Texture;
}

namespace mark {
	namespace resource {
		class manager;
		using image = sf::Texture;
	}
	class sprite;

	namespace terrain {
		class wall : public mark::terrain::base {
		public:
			wall(mark::resource::manager& resource_manager);
			auto traversable() const -> bool override { return false; };
			auto render(mark::vector<int> map_pos) const->std::vector<mark::sprite> override;
		private:
			std::shared_ptr<const mark::resource::image> m_image;
		};
	}
}