#include "resource_manager.h"
#include "resource_image.h"
#include "algorithm.h"
#include "stdafx.h"

mark::resource::manager_impl::manager_impl()
	: m_gen(m_rd())
{}

auto mark::resource::manager_impl::image(const std::string& filename)
	-> resource::image_ptr
{
	auto& image_ptr = m_images[filename];
	auto image = image_ptr.lock();
	if (!image) {
		let new_image = mark::make_shared<resource::image_impl>(filename);
		image_ptr = new_image.get();
		return new_image.get();
	}
	return image;
}

auto mark::resource::manager_impl::random_signed(int64_t min, int64_t max)
	-> int64_t
{
	std::uniform_int_distribution<int64_t> dist(min, max);
	return dist(m_gen);
}

auto mark::resource::manager_impl::random_unsigned(uint64_t min, uint64_t max)
	-> uint64_t
{
	std::uniform_int_distribution<uint64_t> dist(min, max);
	return dist(m_gen);
}

auto mark::resource::manager_impl::random_double(double min, double max)
	-> double
{
	std::uniform_real_distribution<> dist(min, max);
	return dist(m_gen);
}

auto mark::resource::manager_stub::image(const std::string&)
	-> resource::image_ptr
{
	return std::make_shared<resource::image_stub>();
}

auto mark::resource::manager_stub::random_signed(int64_t min, int64_t max)
	-> int64_t
{
	return (max - min) / 2 + min;
}

auto mark::resource::manager_stub::random_unsigned(uint64_t min, uint64_t max)
	-> uint64_t
{
	return (max - min) / 2 + min;
}

auto mark::resource::manager_stub::random_double(double min, double max)
	-> double
{
	return (max - min) / 2 + min;
}
