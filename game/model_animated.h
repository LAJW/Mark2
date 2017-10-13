#pragma once
#include "stdafx.h"

namespace mark {
	struct sprite;
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