#pragma once
#include <map>
#include <memory>
#include <vector>
#include <SFML/Graphics/Color.hpp>
#include "vector.h"

namespace sf {
	class Texture;
}

namespace mark {
	namespace resource {
		using image = sf::Texture;
	}
	class sprite;
	class particle {
	public:
		struct attributes {
			std::shared_ptr<const mark::resource::image> image;
			mark::vector<double> pos;
			float velocity = NAN;
			float direction = NAN;
			float lifespan = 0.3f;
			sf::Color color = sf::Color::White;
			float size = 8.f;
		};
		particle(mark::particle::attributes& attributes);
		void tick(double dt, std::map<int, std::vector<mark::sprite>>& m_pos);
		inline bool dead() const { return m_cur_lifespan <= 0; }
	private:
		particle(mark::particle::attributes& attributes, bool);
		std::shared_ptr<const mark::resource::image> m_image;
		mark::vector<double> m_pos;
		float m_lifespan;
		float m_cur_lifespan;
		float m_velocity;
		float m_direction;
		float m_size;
		sf::Color m_color;
	};
}