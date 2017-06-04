#include "stdafx.h"
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