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
			void tick(mark::tick_context& context, mark::vector<int> map_pos) const;
			auto collide(mark::vector<double> world_pos, mark::segment_t segment) const ->
				mark::vector<double> override;
		private:
			std::shared_ptr<const mark::resource::image> m_image;
		};
	}
}