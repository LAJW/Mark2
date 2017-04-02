#include "resource_manager.h"
#include "resource_image.h"

auto mark::resource::manager::image(const std::string& filename) -> std::shared_ptr<const mark::resource::image> {
	auto& image_ptr = m_images[filename];
	auto image = image_ptr.lock();
	if (!image) {
		auto new_image = std::make_shared<mark::resource::image>();
		new_image->loadFromFile("resource/texture/" + filename);
		image_ptr = new_image;
		return new_image;
	} else {
		return image;
	}
}