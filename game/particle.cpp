#include "particle.h"
#include "sprite.h"

mark::particle::particle(
	std::shared_ptr<const mark::resource::image>& image,
	mark::vector<double> pos,
	float velocity,
	float direction,
	float lifespan) :
	m_image(image),
	m_pos(pos),
	m_velocity(velocity),
	m_direction(direction),
	m_lifespan(lifespan),
	m_cur_lifespan(lifespan) {

}

void mark::particle::tick(double dt, std::map<int, std::vector<mark::sprite>>& sprites) {
	const auto direction = mark::rotate(mark::vector<float>(1.f, 0.f), m_direction);
	m_pos += mark::vector<double>(direction) * dt * static_cast<double>(m_velocity);
	const auto color = sf::Color(255, 255, 255, static_cast<uint8_t>(255.f * (m_cur_lifespan / m_lifespan)));
	sprites[0].push_back(mark::sprite(m_image, m_pos, 8.f, 0.f, 0, color));
	m_cur_lifespan -= static_cast<float>(dt);
}
