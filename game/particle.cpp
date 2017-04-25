#include "resource_image.h"
#include "particle.h"
#include "sprite.h"

mark::particle::particle(
	std::shared_ptr<const mark::resource::image>& image,
	mark::vector<double> pos,
	float velocity,
	float direction,
	float lifespan,
	const sf::Color& color,
	float size):
	m_image(image),
	m_pos(pos),
	m_velocity(velocity),
	m_direction(direction),
	m_lifespan(lifespan),
	m_cur_lifespan(lifespan),
	m_color(color),
	m_size(size) {

}

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
	sprites[0].push_back(mark::sprite(m_image, m_pos, m_size, m_direction, frame, color));
	m_cur_lifespan -= static_cast<float>(dt);
}
