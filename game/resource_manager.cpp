#include "resource_manager.h"
#include "resource_image.h"

#ifndef UNIT_TEST

mark::resource::manager::manager():
	m_gen(m_rd()) { }

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

auto mark::resource::manager::random_int(int min, int max) -> int {
	std::uniform_int_distribution<> dist(min, max);
	return dist(m_gen);
}

auto mark::resource::manager::random_double(double min, double max) -> double {
	std::uniform_real_distribution<> dist(min, max);
	return dist(m_gen);
}

#else

auto mark::resource::manager::image(const std::string& filename)->std::shared_ptr<const mark::resource::image> {
	return std::make_shared<mark::resource::image>();
};
auto mark::resource::manager::random_int(int min, int max) -> int {
	return (max - min) / 2 + min;
}
auto mark::resource::manager::random_double(double min, double max) -> double {
	return (max - min) / 2 + min;
}

#endif