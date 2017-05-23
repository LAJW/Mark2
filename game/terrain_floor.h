#pragma once
#include <memory>
#include "terrain_base.h"

namespace sf {
	class Texture;
}

namespace mark {
	class world;
	namespace resource {
		class manager;
		using image = sf::Texture;
	}

	namespace terrain {
		class floor : public mark::terrain::base {
		public:
			floor(mark::resource::manager& resource_manager, const YAML::Node&);
			floor(mark::resource::manager& resource_manager, int variant = 4);
			auto traversable() const -> bool override { return true; };
			void tick(mark::tick_context& context, mark::vector<int> map_pos) const override;
			void variant(int variant);
			auto collide(mark::vector<double> world_pos, mark::segment_t line) const -> mark::vector<double> override;
			void serialize(YAML::Emitter&) const override;
		private:
			std::shared_ptr<const mark::resource::image> m_image;
			std::shared_ptr<const mark::resource::image> m_image_normal;
			int m_variant;
		};
	}
}