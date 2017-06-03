#pragma once
#include <memory>
#include "vector.h"

namespace sf {
	class Color;
}

namespace mark {
	class sprite;
	namespace resource {
		class manager;
		class image;
	}
	namespace model {
		class animated {
		public:
			animated(const std::shared_ptr<const mark::resource::image>& image);
			void tick(double dt);
			mark::sprite render(mark::vector<double> pos, float size, float rotation, const sf::Color& color);
		private:
			std::shared_ptr<const mark::resource::image> m_image;
			float m_state = 0.f;
		};
	}
}