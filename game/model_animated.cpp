#include "stdafx.h"
#include "model_animated.h"
#include "sprite.h"
#include "resource_manager.h"
#include "resource_image.h"

mark::model::animated::animated(const std::shared_ptr<const resource::image>& image):
	m_image(image) {

}

void mark::model::animated::tick(double dt) {
	const auto size = m_image->size();
	const auto frame_count = size.x / size.y;
	m_state = std::fmod(m_state + static_cast<float>(dt), 1.f);
}

mark::sprite mark::model::animated::render(vector<double> pos, float size, float rotation, const sf::Color & color) {
	const auto image_size = m_image->size();
	const auto frame_count = image_size.x / image_size.y;
	sprite info;
	info.image = m_image;
	info.frame = static_cast<unsigned>(std::floor(m_state)) * frame_count;
	info.pos = pos;
	info.size = size;
	info.rotation = rotation;
	info.color = color;
	return sprite(info);
}
