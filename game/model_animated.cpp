#include "model_animated.h"
#include "resource_image.h"
#include "resource_manager.h"
#include "sprite.h"
#include "stdafx.h"

mark::model::animated::animated(
	const std::shared_ptr<const resource::image>& image)
	: m_image(image)
{}

void mark::model::animated::tick(double dt)
{
	let size = m_image->size();
	let frame_count = size.x / size.y;
	m_state = std::fmod(m_state + static_cast<float>(dt), 1.f);
}

auto mark::model::animated::render(
	vector<double> pos,
	float size,
	float rotation,
	const sf::Color& color) const -> mark::sprite
{
	return sprite([&] {
		sprite _;
		_.image = m_image;
		let image_size = m_image->size();
		let frame_count = image_size.x / image_size.y;
		_.frame = static_cast<unsigned>(
			std::floor(m_state * static_cast<float>(frame_count)));
		_.pos = pos;
		_.size = size;
		_.rotation = rotation;
		_.color = color;
		return _;
	}());
}
