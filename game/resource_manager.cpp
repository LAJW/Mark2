#include "resource_manager.h"
#include "resource_image.h"
#include "algorithm.h"
#include "stdafx.h"

auto mark::resource::manager_impl::image(const std::string& filename)
	-> resource::image_ptr
{
	auto& image_ptr = m_images[filename];
	if (const auto image = image_ptr.lock()) {
		return image;
	}
	let new_image = mark::make_shared<resource::image_impl>(filename);
	image_ptr = new_image.get();
	return new_image.get();
}

auto mark::resource::manager_stub::image(const std::string&)
	-> resource::image_ptr
{
	return std::make_shared<resource::image_stub>();
}
