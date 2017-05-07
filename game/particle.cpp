#include <assert.h>
#include "resource_image.h"
#include "particle.h"
#include "sprite.h"

namespace {
	static auto validate(mark::particle::info& attr) {
		assert(attr.image.get());
		assert(attr.pos.x != NAN);
		assert(attr.pos.y != NAN);
		assert(attr.velocity != NAN);
		assert(attr.direction != NAN);
		assert(attr.lifespan != NAN);
		assert(attr.size != NAN);
		return attr;
	}
}

mark::particle::particle(mark::particle::info& info):
	mark::particle::particle(::validate(info), true) { }

mark::particle::particle(mark::particle::info& attr, bool) :
	m_image(std::move(attr.image)),
	m_pos(attr.pos),
	m_lifespan(attr.lifespan),
	m_cur_lifespan(attr.lifespan),
	m_velocity(attr.velocity),
	m_direction(attr.direction),
	m_size(attr.size),
	m_color(attr.color) { }

void mark::particle::tick(double dt, std::map<int, std::vector<mark::sprite>>& sprites) {
	const auto direction = mark::rotate(mark::vector<float>(1.f, 0.f), m_direction);
	m_pos += mark::vector<double>(direction) * dt * static_cast<double>(m_velocity);
	const auto image_size = m_image->getSize();
	const auto frame_count = image_size.x / image_size.y;
	const auto frame = static_cast<unsigned>((1.f - m_cur_lifespan / m_lifespan) * static_cast<float>(frame_count));
	sf::Color color;
	if (frame_count == 1) {
		color = sf::Color(m_color.r, m_color.g, m_color.b, static_cast<uint8_t>(255.f * (m_cur_lifespan / m_lifespan)));
	} else {
		color = sf::Color::White;
	}
	mark::sprite::info info;
	info.image = m_image;
	info.pos = m_pos;
	info.size = m_size;
	info.rotation = m_direction;
	info.frame = frame;
	info.color = color;
	sprites[0].emplace_back(info);
	m_cur_lifespan -= static_cast<float>(dt);
}