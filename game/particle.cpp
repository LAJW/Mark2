#include "particle.h"
#include <resource/image.h>
#include "sprite.h"
#include "stdafx.h"

#ifdef _DEBUG
constexpr const bool debug_mode = true;
#else
constexpr const bool debug_mode = false;
#endif

static auto validate(const mark::particle::info& attr)
{
	assert(attr.image.get());
	assert(attr.pos.x != NAN);
	assert(attr.pos.y != NAN);
	assert(attr.velocity != NAN);
	assert(attr.direction != NAN);
	assert(attr.lifespan != NAN);
	assert(attr.size != NAN);
	return attr;
}

mark::particle::particle(const particle::info& info)
	: particle::particle(::validate(info), true)
{}

mark::particle::particle(const particle::info& attr, bool)
	: m_image(move(attr.image))
	, m_pos(attr.pos)
	, m_lifespan(attr.lifespan)
	, m_cur_lifespan(attr.lifespan)
	, m_velocity(attr.velocity)
	, m_direction(attr.direction)
	, m_size(attr.size)
	, m_color(attr.color)
	, m_layer(attr.layer)
{
	if constexpr (debug_mode) {
		m_cur_lifespan = 0;
	}
}

void mark::particle::update(
	double dt,
	std::map<int, std::vector<renderable>>& sprites)
{
	sprites[m_layer].emplace_back([&] {
		let frame_count = m_image->size().x / m_image->size().y;
		let progress = 1.f - m_cur_lifespan / m_lifespan;
		let frame =
			static_cast<unsigned>(progress * static_cast<float>(frame_count));
		let color = frame_count == 1
			? sf::Color(
				  m_color.r,
				  m_color.g,
				  m_color.b,
				  static_cast<uint8_t>(255.f * (m_cur_lifespan / m_lifespan)))
			: sf::Color::White;
		sprite info;
		info.image = m_image;
		info.pos = m_pos;
		info.size = m_size;
		info.rotation = m_direction;
		info.frame = frame;
		info.color = color;
		return info;
	}());
	m_pos += [&] {
		let direction = rotate(vector<float>(1.f, 0.f), m_direction);
		return vd(direction) * dt * static_cast<double>(m_velocity);
	}();
	m_cur_lifespan -= static_cast<float>(dt);
}
