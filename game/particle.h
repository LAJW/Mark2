#pragma once
#include "stdafx.h"

namespace sf {
class Texture;
}

namespace mark {
namespace resource {
class image;
}
struct sprite;
class particle
{
public:
	struct info
	{
		resource::image_ptr image;
		vd pos;
		float velocity = NAN;
		float direction = NAN;
		float lifespan = 0.3f;
		sf::Color color = sf::Color::White;
		float size = 8.f;
		int layer = 0;
	};
	particle(const particle::info& info);
	void update(
		double dt,
		std::map<int, std::vector<std::variant<sprite, path, rectangle>>>&
			sprites);
	inline bool dead() const { return m_cur_lifespan <= 0; }

private:
	particle(const particle::info& info, bool);
	resource::image_ptr m_image;
	vd m_pos;
	float m_lifespan;
	float m_cur_lifespan;
	float m_velocity;
	float m_direction;
	float m_size;
	sf::Color m_color;
	int m_layer;
};
} // namespace mark