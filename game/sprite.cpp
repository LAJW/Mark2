#include "sprite.h"
#include "resource_image.h"

mark::sprite::sprite(mark::sprite::info & args):
	m_image(std::move(args.image)),
	m_x(args.pos.x),
	m_y(args.pos.y),
	m_size(args.size),
	m_rotation(args.rotation),
	m_frame(args.frame),
	m_color(args.color) { }

mark::sprite::sprite(
	std::shared_ptr<const mark::resource::image> image,
	double x,
	double y,
	float size,
	float rotation,
	size_t frame,
	sf::Color color):
	m_image(std::move(image)),
	m_x(x),
	m_y(y),
	m_size(size),
	m_rotation(rotation),
	m_frame(frame),
	m_color(color) {
}

mark::sprite::sprite(
	std::shared_ptr<const mark::resource::image> image,
	const mark::vector<double>& pos,
	float size,
	float rotation,
	size_t frame,
	sf::Color color):
	m_image(std::move(image)),
	m_x(pos.x),
	m_y(pos.y),
	m_size(size),
	m_rotation(rotation),
	m_frame(frame),
	m_color(color) {
}

auto mark::sprite::image() const -> const mark::resource::image& {
	return *m_image;
}

auto mark::sprite::x() const -> double {
	return m_x;
}

auto mark::sprite::y() const -> double {
	return m_y;
}

auto mark::sprite::rotation() const -> float {
	return m_rotation;
}

auto mark::sprite::size() const -> float {
	return m_size;
}