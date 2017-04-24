#include "model_animated.h"
#include "sprite.h"
#include "resource_manager.h"
#include <SFML/Graphics/Texture.hpp>

mark::model::animated::animated(const std::shared_ptr<const mark::resource::image>& image):
	m_image(image) {

}

void mark::model::animated::tick(double dt) {
	const auto size = m_image->getSize();
	const auto frame_count = size.x / size.y;
	m_state = std::fmod(m_state + static_cast<float>(dt), 1.f);
}

mark::sprite mark::model::animated::render(mark::vector<double> pos, float size, float rotation, const sf::Color & color) {
	const auto image_size = m_image->getSize();
	const auto frame_count = image_size.x / image_size.y;
	const auto frame = static_cast<unsigned>(std::floor(m_state)) * frame_count;
	return mark::sprite(m_image, pos, size, rotation, frame, color);
}
