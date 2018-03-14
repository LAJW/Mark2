#include "stdafx.h"
#include "resource_manager.h"
#include "resource_image.h"

mark::resource::manager_impl::manager_impl():
	m_gen(m_rd()) { }

auto mark::resource::manager_impl::image(const std::string& filename) -> std::shared_ptr<const resource::image> {
	auto& image_ptr = m_images[filename];
	auto image = image_ptr.lock();
	if (!image) {
		auto new_image = std::make_shared<resource::image_impl>(filename);
		image_ptr = new_image;
		return new_image;
	} else {
		return image;
	}
}

auto mark::resource::manager_impl::random_int(int min, int max) -> int {
	std::uniform_int_distribution<> dist(min, max);
	return dist(m_gen);
}

auto mark::resource::manager_impl::random_double(double min, double max) -> double {
	std::uniform_real_distribution<> dist(min, max);
	return dist(m_gen);
}

auto mark::resource::manager_stub::image(const std::string&)->std::shared_ptr<const resource::image> {
	return std::make_shared<resource::image_stub>();
};
auto mark::resource::manager_stub::random_int(int min, int max) -> int {
	return (max - min) / 2 + min;
}
auto mark::resource::manager_stub::random_double(double min, double max) -> double {
	return (max - min) / 2 + min;
}
template<>
auto mark::resource::manager::random<bool>(bool min, bool max) -> bool {
	return this->random_int(min, max);
}
template<>
auto mark::resource::manager::random<int>(int min, int max) -> int {
	return this->random_int(min, max);
}

template<>
auto mark::resource::manager::random<unsigned>(unsigned min, unsigned max) -> unsigned {
	return static_cast<unsigned>(this->random_int(min, max));
}
template<>
auto mark::resource::manager::random<size_t>(size_t min, size_t max) -> size_t {
	return static_cast<size_t>(this->random_int(min, max));
}
template<>
auto mark::resource::manager::random<double>(double min, double max) -> double {
	return this->random_double(min, max);
}
template<>
auto mark::resource::manager::random<float>(float min, float max) -> float {
	return static_cast<float>(this->random_double(min, max));
}